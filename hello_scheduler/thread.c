#include "thread.h"
#include "hw_init.h"
#include "cpu.h"
#include "gd32vf103.h"
#include "n200_func.h"
#include <string.h>

scheduler_t scheduler_state;

#define thread_mtime_clear()    *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME ) = 0

void thread_init( void )
{
    memset(&scheduler_state,0,sizeof(scheduler_state));
    
    for(int n=0; n < THREAD_MAX; n++)
        scheduler_state.threads[n].prio = THREAD_PRIO_INACTIVE;
    
    hw_init();
}

void thread_yield( void )
{
    __WFI();
}

int thread_create( char* name, void (*entry)(void*), void* stack, size_t stack_sz, int8_t prio )
{
    return -1;
}

__attribute__( ( interrupt ) ) 
void thread_systick_isr( void ) 
{
    cpu_push_state();
    ++scheduler_state.systick;
    thread_mtime_clear();
    cpu_pop_state();
}
