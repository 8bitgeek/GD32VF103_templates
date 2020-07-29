#include "thread.h"
#include "cpu.h"
#include "gd32vf103.h"
#include "n200_func.h"
#include <string.h>

scheduler_t scheduler_state;

#define thread_mtime_clear()        *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME ) = 0

#define thread_state(id)            ((id) >= 0 && (id) < THREAD_MAX) ? (&scheduler_state.threads[(id)]) : NULL

#define systick_service()           ++scheduler_state.systick;      \
                                    thread_mtime_clear();

#define scheduler_service()         register cpu_reg_t context_sp;              \
                                    if ( ( context_sp = next_context() ) != 0 ) \
                                        cpu_load_sp( context_sp );

static void      thread_exit  ( void );
static int       thread_new_id( void );
static cpu_reg_t next_context ( void );

void thread_init( void )
{
    memset(&scheduler_state,0,sizeof(scheduler_state));
    for(int n=0; n < THREAD_MAX; n++)
        scheduler_state.threads[n].prio = THREAD_PRIO_INACTIVE;    
}

void thread_yield( void )
{
    __WFI();
}

static void thread_exit( void )
{
    // TODO - de-allocate, clean up
    // wait to die
    for(;;);
}

int thread_create( const char* name, void (*entry)(void*), void* stack, size_t stack_sz )
{
    int id = thread_new_id();
    if ( id >= 0 )
    {
        thread_t* thread = thread_state(id);
        uint8_t* stack_uint8 = (uint8_t*)stack; 

        /* initialize the cpu state initial stack frame */
        cpu_state_t* cpu_state = (cpu_state_t*)&stack_uint8[stack_sz-sizeof(cpu_state_t)];
        memset( cpu_state, 0, sizeof(cpu_state_t) );
 
        cpu_state->abi.ra = (cpu_reg_t)thread_exit;
        cpu_state->abi.pc = (cpu_reg_t)entry;
        cpu_state->abi.sp = (cpu_reg_t)cpu_state;

        /* initialize the initial thread state */
        thread->cpu_state  = cpu_state;
        thread->prio = THREAD_PRIO_SUSPEND;
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


volatile __attribute__( ( naked ) ) void systick_isr( void ) 
{
    cpu_systick_enter();
    
        cpu_push_state();

            systick_service();
            scheduler_service();
    
        cpu_pop_state();

    cpu_systick_exit();
}

