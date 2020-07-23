#include "delay.h"
#include "thread.h"

void delay_ms( uint32_t ms ) 
{
    thread_systick_t start = thread_systick(); 
    while ( thread_systick() - start < ms ) 
    { 
        thread_yield();
    }
}
