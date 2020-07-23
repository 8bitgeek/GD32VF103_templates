#include "thread.h"
#include "cpu.h"
#include "gd32vf103.h"
#include "n200_func.h"
#include <string.h>

scheduler_t scheduler_state;

#define thread_mtime_clear()    *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME ) = 0
#define thread_state(id)        ((id) > 0 && (id) < THREAD_MAX) ? (&scheduler_state.threads[(id)]) : NULL

static int thread_new_id( void );

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

void thread_init( void )
{
    memset(&scheduler_state,0,sizeof(scheduler_state));
    for(int n=0; n < THREAD_MAX; n++)
        scheduler_state.threads[n].prio = THREAD_PRIO_INACTIVE;    
}

void thread_yield( void )
{
    // __WFI();
}

int thread_create( char* name, void (*entry)(void*), void* stack, size_t stack_sz )
{
    int id = thread_new_id();
    if ( id >= 0 )
    {
        thread_t* thread = thread_state(id);
        uint8_t* stack_uint8 = (uint8_t*)stack; 

        /* initialize the cpu state initial stack frame */
        cpu_state_t* cpu_state = (cpu_state_t*)&stack_uint8[stack_sz-sizeof(cpu_state_t)];
        memset( cpu_state, 0, sizeof(cpu_state_t) );
 
        cpu_state->abi.ra = (cpu_reg_t)entry;
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

__attribute__( ( interrupt ) )
void thread_systick_isr( void ) 
{
    cpu_push_state();
    ++scheduler_state.systick;
    thread_mtime_clear();
    cpu_pop_state();
    cpu_systick_exit();
}

