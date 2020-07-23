#ifndef _THREAD_H_
#define _THREAD_H_

#include <stdint.h>
#include "cpu.h"

#ifndef THREAD_MAX
    #define THREAD_MAX      10
#endif

#ifndef THREAD_NAME_MAX
    #define THREAD_NAME_MAX 8
#endif

#define thread_systick_t    uint32_t

typedef struct thread
{
    char                name[THREAD_NAME_MAX+1];
    cpu_state_t*        cpu_state;
} thread_t;

typedef struct scheduler
{
    thread_systick_t    systick;
    thread_t            threads[THREAD_MAX];
    thread_t*           thread;
} scheduler_t;

extern scheduler_t scheduler_state;

#define thread_systick_isr  eclic_mtip_handler
#define thread_systick()    scheduler_state.systick

extern void thread_init  ( void );
extern void thread_yield ( void );

#endif
