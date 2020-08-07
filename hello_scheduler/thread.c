#include "thread.h"
#include "cpu.h"
#include "gd32vf103.h"
#include "n200_func.h"
#include "riscv_encoding.h"
#include <string.h>

scheduler_t scheduler_state;


#define thread_msip_set()           *( volatile uint8_t * )( TIMER_CTRL_ADDR + TIMER_MSIP ) = 0x01
#define thread_msip_clear()         *( volatile uint8_t * )( TIMER_CTRL_ADDR + TIMER_MSIP ) = 0x00
#define thread_mtime_clear()        *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME ) = 0
#define thread_state(id)            ((id) >= 0 && (id) < THREAD_MAX) ? (&scheduler_state.threads[(id)]) : NULL
#define systick_service()           ++scheduler_state.systick;      \
                                    thread_mtime_clear();
#define scheduler_service()         register cpu_reg_t context_sp;              \
                                    if ( ( context_sp = next_context() ) != 0 ) \
                                        cpu_wr_sp( context_sp );

static int       thread_new_id( void );
static cpu_reg_t next_context ( void );
static void      thread_exit  ( void );

void thread_init( void )
{
    memset(&scheduler_state,0,sizeof(scheduler_state));
    for(int n=0; n < THREAD_MAX; n++)
        scheduler_state.threads[n].prio = THREAD_PRIO_INACTIVE;

    thread_create( "main", NULL, NULL, 0 );
}

void thread_yield( void )
{
    thread_msip_set();
}

static void thread_exit( void )
{
    // TODO - de-allocate, clean up
    // wait to die
    for(;;);
}

int thread_create( const char* name, void (*entry)(void*), cpu_reg_t* stack, size_t n_stack_words )
{
    int id = thread_new_id();
    if ( id >= 0 )
    {
        thread_t* thread = thread_state(id);
        if ( entry == NULL && stack == NULL )
        {
            thread->prio = THREAD_PRIO_MIN;
        }
        else
        {
             uint8_t* stack_uint8 = (uint8_t*)stack; 

            /* initialize the cpu state initial stack frame */
            cpu_state_t* cpu_state = (cpu_state_t*) &stack_uint8 [ (n_stack_words/sizeof(cpu_reg_t)) - sizeof(cpu_state_t) ];
            memset( cpu_state, 0, sizeof(cpu_state_t) );
    
            cpu_state->abi.ra = (cpu_reg_t)thread_exit;
            cpu_state->abi.pc = (cpu_reg_t)entry;
            cpu_state->abi.sp = (cpu_reg_t)cpu_state;

            /* initialize the initial thread state */
            thread->cpu_state  = cpu_state;
            thread->prio = THREAD_PRIO_SUSPEND;
        }
        strncpy( thread->name, name, THREAD_NAME_MAX );
    }
    return id;
}

int thread_set_prio ( int id, int8_t prio )
{
    thread_t* thread = thread_state(id);
    if ( thread )
    {
        thread->prio = prio;
        return id;
    }
    return -1;
}


static int thread_new_id( void )
{
    for(int id=0; id < THREAD_MAX; id++)
    {
        if ( scheduler_state.threads[id].prio == THREAD_PRIO_INACTIVE )
        {
            scheduler_state.threads[id].prio = THREAD_PRIO_SUSPEND;
            return id;
        }
    }
    return -1;
}

static cpu_reg_t next_context( void )
{
    for(int nThread=0; nThread < THREAD_MAX; nThread++)
    {
        scheduler_state.thread_id = ( scheduler_state.thread_id+1 >= THREAD_MAX ) ? 0 : scheduler_state.thread_id+1;
        if ( scheduler_state.threads[scheduler_state.thread_id].prio > 0 )
        {
            return scheduler_state.threads[scheduler_state.thread_id].cpu_state->abi.sp;
        }
    }
    return 0;
}


volatile __attribute__( ( naked ) ) void eclic_mtip_handler( void ) 
{
    cpu_systick_enter();
    
        cpu_push_state();
        
            systick_service();
            scheduler_service();

        cpu_pop_state();

    cpu_systick_exit();
}

volatile __attribute__( ( naked ) ) void eclic_msip_handler( void )
{
    cpu_systick_enter();
    
        cpu_push_state();
        
            thread_msip_clear();
            scheduler_service();

        cpu_pop_state();

    cpu_systick_exit();
}
