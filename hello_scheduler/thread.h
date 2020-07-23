#ifndef _THREAD_H_
#define _THREAD_H_

#include <stdint.h>
#include <stddef.h>
#include "cpu.h"

#ifndef THREAD_MAX
    #define THREAD_MAX      10
#endif

#ifndef THREAD_NAME_MAX
    #define THREAD_NAME_MAX 8
#endif

#define thread_systick_t    uint32_t

#define THREAD_PRIO_INACTIVE    (-1)
#define THREAD_PRIO_SUSPEND     (0)
#define THREAD_PRIO_MIN         (1)
#define THREAD_PRIO_MAX         (127)

typedef struct thread
{
    int8_t              prio;
    char                name[THREAD_NAME_MAX+1];
    cpu_state_t*        cpu_state;
} thread_t;

typedef struct scheduler
{
    thread_systick_t    systick;
    thread_t            threads[THREAD_MAX];
    uint8_t             thread_id;
} scheduler_t;

extern scheduler_t scheduler_state;

#define thread_systick()    scheduler_state.systick

#define thread_stop(id)     thread_set_prio((id),THREAD_PRIO_SUSPEND)
#define thread_start(id)    thread_set_prio((id),THREAD_PRIO_MIN)

extern void thread_init     ( void );
extern int  thread_create   ( const char* name, void (*entry)(void*), void* stack, size_t stack_sz );
extern int  thread_set_prio ( int id, int8_t prio );
extern void thread_yield    ( void );

extern volatile __attribute__( ( naked ) ) void systick_isr( void );

#endif
