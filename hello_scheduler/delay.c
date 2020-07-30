#include "delay.h"
#include "thread.h"

void delay_ms( uint32_t ms ) 
{
    thread_systick_t start = scheduler_state.systick; 
    while ( (scheduler_state.systick - start) < ms ) 
    { 
        thread_yield();
    }
}
