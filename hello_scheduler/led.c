#include "led.h"
#include <gd32vf103.h>
#include <stdint.h>
#include <stdbool.h>

extern void led_green(bool state)
{
    if ( state )
        GPIOA->ODR &= ~( 0x1 << 1 );
    else
        GPIOA->ODR |=  ( 0x1 << 1 );
}

extern void led_red(bool state)
{
    if ( state )
        GPIOC->ODR &= ~( 0x1 << 13 );
    else
        GPIOC->ODR |=  ( 0x1 << 13 );
}

extern void led_blue(bool state)
{
    if ( state )
        GPIOA->ODR &= ~( 0x1 << 2 );
    else
        GPIOA->ODR |=  ( 0x1 << 2 );
}

