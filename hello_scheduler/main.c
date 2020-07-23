#include "hw_init.h"
#include "thread.h"
#include "delay.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DELAY   (250)

static void led_green(bool state);
static void led_red(bool state);
static void led_blue(bool state);


static void led_green(bool state)
{
    if ( state )
        GPIOA->ODR &= ~( 0x1 << 1 );
    else
        GPIOA->ODR |=  ( 0x1 << 1 );
}

static void led_red(bool state)
{
    if ( state )
        GPIOC->ODR &= ~( 0x1 << 13 );
    else
        GPIOC->ODR |=  ( 0x1 << 13 );
}

static void led_blue(bool state)
{
    if ( state )
        GPIOA->ODR &= ~( 0x1 << 2 );
    else
        GPIOA->ODR |=  ( 0x1 << 2 );
}

int main( void )
{
    hw_init();
    thread_init();

    led_green(false);
    led_red(false);
    led_blue(false);

    for(;;)
    {
        led_blue(false);
        delay_ms(DELAY);
        led_blue(true);
        delay_ms(DELAY);
    }

    return 0;
}