#include "thread.h"

int main( void )
{
    thread_init();

    for(;;)
        thread_yield();

    return 0;
}