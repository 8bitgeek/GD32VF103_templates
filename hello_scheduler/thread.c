#include "thread.h"
#include "cpu.h"
#include "gd32vf103.h"
#include "n200_func.h"
#include "riscv_encoding.h"
#include <string.h>

#define thread_msip_set()           *( volatile uint8_t * )( TIMER_CTRL_ADDR + TIMER_MSIP ) = 0x01
#define thread_msip_clear()         *( volatile uint8_t * )( TIMER_CTRL_ADDR + TIMER_MSIP ) = 0x00
#define thread_mtime_clear()        *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME ) = 0
#define thread_state(id)            ((id) >= 0 && (id) < THREAD_MAX) ? (&scheduler_state.threads[(id)]) : NULL
#define systick_service()           ++scheduler_state.systick;                          \
                                    thread_mtime_clear();
#define thread_scheduler_service()  register cpu_reg_t context_sp;                      \
                                    if ( ( context_sp = thread_schedule_next() ) != 0 ) \
                                        cpu_wr_sp( context_sp )
#define thread_next_id()            (scheduler_state.thread_id = ( scheduler_state.thread_id+1 >= THREAD_MAX ) ? 0 : scheduler_state.thread_id+1)
#define thread_prio_clear()         scheduler_state.prio = 0;

scheduler_t scheduler_state;

static int       thread_new_id( void );
static cpu_reg_t thread_schedule_next ( void );
static void      thread_exit  ( void );


int thread_init( void )
{
    memset(&scheduler_state,0,sizeof(scheduler_state));
    for(int n=0; n < THREAD_MAX; n++)
        scheduler_state.threads[n].prio = THREAD_PRIO_INACTIVE;

    /* insert the 'main' thread into the scheduler */
    return thread_create( "main", NULL, NULL, NULL, 0 ); 
}


void thread_yield( void )
{
    /* trigger the timer "soft" interrupt */
    thread_msip_set();
}

static void thread_exit( void )
{
    // TODO - de-allocate, clean up
    // wait to die
    for(;;);
}

/**
 * @brief Create a new thread.
 * @param name A human readable name for the thread.
 * @param thread_fn A pointer to the entry point of the thread function.
 * @param arg This pointer will be passed as a parameter to the thread function.
 * @param stack Pointer to the base of the thread's program stack space on an even cpu_reg_t word boundary.
 * @param n_stack_words The number of cpu_reg_t words contained in the stack space.
 * @return a valid thread handle >= 0, or < 0 on failure.
 */
int thread_create( const char* name, void (*thread_fn)(void*), void* arg, cpu_reg_t* stack, size_t n_stack_words )
{
    int id = thread_new_id();
    if ( id >= 0 )
    {
        thread_t* thread = thread_state(id);
        if ( thread_fn == NULL && stack == NULL )
        {
            /* 'main' thread already has 'context' */
            thread->prio = THREAD_PRIO_MIN;
        }
        else
        {
             uint8_t* stack_uint8 = (uint8_t*)stack; 

            /* initialize the cpu state initial stack frame */
            cpu_state_t* cpu_state = (cpu_state_t*) &stack_uint8 [ (n_stack_words/sizeof(cpu_reg_t)) - sizeof(cpu_state_t) ];
            memset( cpu_state, 0, sizeof(cpu_state_t) );
    
            cpu_state->abi.a0 = (cpu_reg_t)arg;
            cpu_state->abi.ra = (cpu_reg_t)thread_exit;
            cpu_state->abi.pc = (cpu_reg_t)thread_fn;
            cpu_state->abi.sp = (cpu_reg_t)cpu_state;

            /* initialize the initial thread state */
            thread->cpu_state  = cpu_state;
            thread->prio = THREAD_PRIO_SUSPEND;
        }
        strncpy( thread->name, name, THREAD_NAME_MAX );
     }
    return id;
}

/**
 * @brief set a thread priority. < 0 is inactive, = 0 is active but suspended.
 *        > 0 indicates the maximum number of contiguous time slices the thread is allowed to get.
 * @param id The thread handle.
 * @param prio The thread priority -1 .. 127
 */
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

/**
 * @brief determine which thread gets this time slice.
 * @return the context (stack pointer) to the thread to allocate this time slice to.
 */
static cpu_reg_t thread_schedule_next( void )
{
    thread_t* thread;
            
    if ( --scheduler_state.prio > 0 )
    {
        return scheduler_state.threads[scheduler_state.thread_id].cpu_state->abi.sp;
    }
    else
    {
        for(int nThread=0; nThread < THREAD_MAX; nThread++)
        {
            if ( (thread = thread_state( thread_next_id() ))->prio > 0 )
            {
                scheduler_state.prio = thread->prio;
                return thread->cpu_state->abi.sp;
            }
        }
    }
    return 0;
}

/**
 * @brief timer interrupt, increment systick, and potentially switch thread context
 */
volatile __attribute__( ( naked ) ) void eclic_mtip_handler( void ) 
{
    cpu_systick_enter();
    
        cpu_push_state();
        
            systick_service();
            thread_scheduler_service();

        cpu_pop_state();

    cpu_systick_exit();
}

/**
 * @brief software interrupt, thread yield, give up remaining prio and switch context.
 */
volatile __attribute__( ( naked ) ) void eclic_msip_handler( void )
{
    cpu_systick_enter();
    
        cpu_push_state();
        
            thread_msip_clear();
            thread_prio_clear();
            thread_scheduler_service();

        cpu_pop_state();

    cpu_systick_exit();
}
