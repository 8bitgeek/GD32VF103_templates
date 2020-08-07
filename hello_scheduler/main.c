#include "hw_init.h"
#include "led.h"
#include "thread.h"
#include "delay.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define EVER            ;;
#define DELAY           (250)
#define STACK_BYTES     (1024)
#define STACK_WORDS     STACK_BYTES / sizeof(cpu_reg_t)

static cpu_reg_t red_stack   [ STACK_WORDS ];
static cpu_reg_t green_stack [ STACK_WORDS ];

static int red_thread_handle=(-1);
static int green_thread_handle=(-1);

// created by thread_create(...)
static void thread_red(void* arg)
{
    for(EVER)
    {
        led_red(false);
        delay_ms(DELAY);
        led_red(true);
        delay_ms(DELAY);
    }

}

// created by thread_create(...)
static void thread_green(void* arg)
{
    for(EVER)
    {
        led_green(true);
        delay_ms(DELAY);
        led_green(false);
        delay_ms(DELAY);
    }

}

// main(...) thread
static void thread_blue(void* arg)
{
    for(EVER)
    {
        led_blue(false);
        delay_ms(DELAY*2);
        led_blue(true);
        delay_ms(DELAY*2);
    }
}

int main( void )
{
    hw_init();

    thread_init();
    
    red_thread_handle   = thread_create( "red",   thread_red,   red_stack,   STACK_BYTES );
    green_thread_handle = thread_create( "green", thread_green, green_stack, STACK_BYTES );

    thread_start( red_thread_handle );
    thread_start( green_thread_handle );

    thread_blue(NULL);

    return 0;
}