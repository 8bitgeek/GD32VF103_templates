#include "hw_init.h"
#include "led.h"
#include "thread.h"
#include "delay.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DEC_DELAY       (10)
#define MIN_DELAY       (10)
#define MAX_DELAY       (200)

#define EVER            ;;
#define STACK_BYTES     (1024)
#define STACK_WORDS     STACK_BYTES / sizeof(cpu_reg_t)

static cpu_reg_t red_stack   [ STACK_WORDS ];
static cpu_reg_t green_stack [ STACK_WORDS ];
static cpu_reg_t blue_stack  [ STACK_WORDS ];

static int red_thread_handle   = (-1);
static int green_thread_handle = (-1);
static int blue_thread_handle  = (-1);
static int main_thread_handle  = (-1);

static void sweep_delay(int* delay);

static void run_red  (void* arg);
static void run_green(void* arg);
static void run_blue (void* arg);
static void run_main (void* arg);


static void sweep_delay(int* delay)
{
    *delay = *delay - DEC_DELAY;
    if ( *delay < MIN_DELAY )
        *delay = MAX_DELAY;
} 


static void run_red(void* arg)
{
    int* delay = (int*)arg;
    for(EVER)
    {
        led_red(false);
        delay_ms(*delay);
        led_red(true);
        delay_ms(*delay);
    }

}

static void run_green(void* arg)
{
    int* delay = (int*)arg;
    for(EVER)
    {
        led_green(true);
        delay_ms(*delay);
        led_green(false);
        delay_ms(*delay);
    }

}

static void run_blue(void* arg)
{
    int* delay = (int*)arg;
    for(EVER)
    {
        led_blue(false);
        delay_ms((*delay)*2);
        led_blue(true);
        delay_ms((*delay)*2);
    }
}


// 'main' thread - sweeps the delay
static void run_main(void* arg)
{
    int* delay = (int*)arg;
    for(EVER)
    {
        sweep_delay(delay);
        delay_ms((*delay)*2);
    }
}


int main( void )
{
    int delay = MAX_DELAY;

    hw_init();

    main_thread_handle  = thread_init();
    red_thread_handle   = thread_create( "red",   run_red,   &delay, red_stack,   STACK_WORDS );
    green_thread_handle = thread_create( "green", run_green, &delay, green_stack, STACK_WORDS );
    blue_thread_handle  = thread_create( "blue",  run_blue,  &delay, blue_stack,  STACK_WORDS );

    thread_start( red_thread_handle );
    thread_start( green_thread_handle );
    thread_start( blue_thread_handle );

    run_main(&delay);

    return 0;
}