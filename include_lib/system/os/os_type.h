#ifndef __OS_TYPE_H
#define __OS_TYPE_H


#define OS_TICKS_PER_SEC          100

#if defined CONFIG_UCOS_ENABLE
#include <os/ucos_ii.h>
#define portMAX_DELAY 0xffff
#define THREAD_NAME_MAX 64
typedef struct os_tcb StaticTask_t ;
typedef void *StaticQueue_t ;


#define  OS_EVENT_TYPE_UNUSED         0u
#define  OS_EVENT_TYPE_Q              1u
#define  OS_EVENT_TYPE_SEM            2u
#define  OS_EVENT_TYPE_MUTEX          3u

typedef struct {
    unsigned char OSEventType;
    int aa;
    void *bb;
    unsigned char value;
    unsigned char prio;
    unsigned short cc;
} OS_SEM, OS_MUTEX, OS_QUEUE;

typedef struct  {
    u8         OSFlagType;
    void         *OSFlagWaitList;
    u32      OSFlagFlags;
} OS_EVENT_GRP;


#elif defined CONFIG_FREE_RTOS_ENABLE

#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "FreeRTOS/task.h"

typedef StaticSemaphore_t OS_SEM, OS_MUTEX;
typedef StaticQueue_t OS_QUEUE;
typedef StaticEventGroup_t OS_EVENT_GRP;


#define THREAD_NAME_MAX configMAX_TASK_NAME_LEN


#else
#error "no_os_defined"
#endif





















#endif
