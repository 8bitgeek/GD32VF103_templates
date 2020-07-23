#include "hw_init.h"
#include "led.h"
#include "thread.h"
#include "delay.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DELAY           (250)
#define STACK_BYTES     (1024)

static cpu_reg_t red_stack[STACK_BYTES/sizeof(cpu_reg_t)];
static cpu_reg_t green_stack[STACK_BYTES/sizeof(cpu_reg_t)];

static int red_thread_handle=(-1);
static int green_thread_handle=(-1);

static void thread_red(void* arg)
{
    for(;;)
    {
        led_red(false);
        delay_ms(DELAY);
        led_red(true);
        delay_ms(DELAY);
    }

}

static void thread_green(void* arg)
{
    for(;;)
    {
        led_green(false);
        delay_ms(DELAY);
        led_green(true);
        delay_ms(DELAY);
    }

}

int main( void )
{
    hw_init();
    thread_init();

    led_red(false);
    led_green(false);
    led_blue(false);

    red_thread_handle = thread_create( "red", thread_red, red_stack, STACK_BYTES );
    green_thread_handle = thread_create( "green", thread_green, green_stack, STACK_BYTES );

    thread_start(red_thread_handle);
    thread_start(green_thread_handle);

    for(;;)
    {
        led_blue(false);
        delay_ms(DELAY);
        led_blue(true);
        delay_ms(DELAY);
    }

    return 0;
}