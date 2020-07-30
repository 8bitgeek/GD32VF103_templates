#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

extern void led_green(bool state);
extern void led_red(bool state);
extern void led_blue(bool state);

#endif
