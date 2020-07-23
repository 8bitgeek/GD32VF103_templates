#include "hw_init.h"
#include "thread.h"

int main( void )
{
    hw_init();
    thread_init();

    for(;;)
        thread_yield();

    return 0;
}