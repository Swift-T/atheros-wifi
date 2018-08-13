// Copyright (c) 2017 Qualcomm Technologies, Inc.
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) 
// provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived 
// from this software without specific prior written permission.
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef _OSAL_H_
#define _OSAL_H_

#include <string.h>

/*
 * Global macros
 */
#define MQX_LITE        0
#define NO_OS           0
#define FREE_RTOS       0
#define MQX_RTOS        1

#if MQX_LITE
#include <mqxlite.h>
#include <mutex.h>
#include <lwevent.h>
#endif

#if MQX_RTOS 
#include <psptypes.h>
#include <athdefs.h>
#include <string.h>
#include <mqx.h>
#include <queue.h>
#include <mutex.h>
#include <lwevent.h>
#include <bsp.h>
#include "enet.h"
#include "enetprv.h"
#include "enet_wifi.h"
#include <a_osapi.h>
//#include <klog.h>
#endif

#if FREE_RTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <portmacro.h>
#include <athdefs.h>
#include <a_types.h>
#include <event_groups.h>
#endif

#define QOSAL_INT8              int8_t
#define QOSAL_INT16             int16_t
#define QOSAL_INT32             int32_t
#define QOSAL_UINT8             uint8_t
#define QOSAL_UINT16            uint16_t
#define QOSAL_UINT32            uint32_t
#define QOSAL_BOOL              int32_t
#define QOSAL_CHAR              char
#define QOSAL_UCHAR             uint8_t
#define QOSAL_VOID              void 
#define QOSAL_CONST             const
#define QOSAL_ULONG             uint32_t
#define QOSAL_LONG              int32_t
#define QOSAL_TRUE              ((QOSAL_BOOL)1)
#define QOSAL_FALSE             ((QOSAL_BOOL)0)


#if FREE_RTOS
typedef int8_t                  A_INT8;
typedef int16_t                 A_INT16;
typedef int32_t                 A_INT32;
typedef uint8_t                 A_UINT8;
typedef uint16_t                A_UINT16;
typedef uint32_t                A_UINT32;
typedef int32_t                 A_BOOL;
typedef char                    A_CHAR;
typedef uint8_t                 A_UCHAR;
typedef void*                   pointer;

#define boolean     QOSAL_BOOL
#define  uint_8  uint8_t
#define   int_8   int8_t
#define uint_16 uint16_t
#define  int_16  int16_t
#define uint_32 uint32_t
#define  int_32  int32_t
#define uint_64 uint64_t
#define  int_64  int64_t

#endif // FREERTOS

/* QOSAL Error codes */
typedef enum 
{
  QOSAL_ERROR = -1,
  QOSAL_OK = 0,
  QOSAL_SUCCESS = 1,
  QOSAL_INVALID_TASK_ID = 2,
  QOSAL_INVALID_PARAMETER = 3,
  QOSAL_INVALID_POINTER = 4,
  QOSAL_ALREADY_EXISTS = 5,
  QOSAL_INVALID_EVENT = 6,
  QOSAL_EVENT_TIMEOUT = 7,
  QOSAL_INVALID_MUTEX = 8,
  QOSAL_TASK_ALREADY_LOCKED = 9,
  QOSAL_MUTEX_ALREADY_LOCKED = 10,  
  QOSAL_OUT_OF_MEMORY = 11,
} QOSAL_STATUS;

/* Priority setting */
#define QOSAL_DRIVER_TASK_PRIORITY     QOSAL_TASK_PRIORITY_HIGHEST
  
#if MQX_RTOS || MQX_LITE
#define QOSAL_TASK_PRIORITY_LOWEST              11       
#define QOSAL_TASK_PRIORITY_LOWER               10
#define QOSAL_TASK_PRIORITY_MEDIUM              9
#define QOSAL_TASK_PRIORITY_HIGH                8
#define QOSAL_TASK_PRIORITY_HIGHER              7
#define QOSAL_TASK_PRIORITY_HIGHEST             6  
#elif FREE_RTOS
#define QOSAL_TASK_PRIORITY_LOWEST              0
#define QOSAL_TASK_PRIORITY_ABOVE_LOWEST        1
#define QOSAL_TASK_PRIORITY_BELOW_LOWER         2
#define QOSAL_TASK_PRIORITY_LOWER               3
#define QOSAL_TASK_PRIORITY_ABOVE_LOWER         4
#define QOSAL_TASK_PRIORITY_MEDIUM              5
#define QOSAL_TASK_PRIORITY_ABOVE_MEDIUM        6
#define QOSAL_TASK_PRIORITY_BELOW_HIGH          7
#define QOSAL_TASK_PRIORITY_HIGH                8
#define QOSAL_TASK_PRIORITY_ABOVE_HIGH          9
#define QOSAL_TASK_PRIORITY_HIGHER              10
#define QOSAL_TASK_PRIORITY_HIGHEST             11
#endif

#if MQX_RTOS || MQX_LITE 
#define QOSAL_EVENT_STRUCT              LWEVENT_STRUCT
#define QOSAL_TASK_DESTRUCTION          MQX_TASK_DESTRUCTION
#define QOSAL_TASK_ID                   _task_id
#define QOSAL_NULL_TASK_ID              MQX_NULL_TASK_ID  
#define QOSAL_TICK_STRUCT               MQX_TICK_STRUCT
#define QOSAL_TICK_STRUCT_PTR           MQX_TICK_STRUCT_PTR
#define QOSAL_EVENT_AUTO_CLEAR          LWEVENT_AUTO_CLEAR
#define _mqx_uint_ptr                   uint_32_ptr

#elif FREE_RTOS

#define QOSAL_NULL_TASK_ID              0
/* Mutual Exclusion */
typedef xSemaphoreHandle                A_MUTEX_T;
#define QOSAL_EVENT_STRUCT              QOSAL_UINT8
#define QOSAL_TICK_STRUCT               NULL
#define QOSAL_TICK_STRUCT_PTR           void*
#define QOSAL_TASK_ID                   QOSAL_UINT32
#define UNUSED_ARGUMENT(arg)            ((void)arg)
#define QOSAL_EVENT_AUTO_CLEAR          0x01
 typedef struct time_struct
{
   /* The number of seconds in the time.  */
   uint32_t     SECONDS;
   /* The number of milliseconds in the time. */
   uint32_t     MILLISECONDS;
} TIME_STRUCT;

#define _PTR_      *

/* A_MEM -- macros that should be mapped to OS/STDLIB equivalent functions */
#define A_MEMCPY(dst, src, len)         memcpy((QOSAL_UINT8 *)(dst), (src), (len))
#define A_MEMZERO(addr, len)            memset((addr), 0, (len))
#define A_MEMCMP(addr1, addr2, len)     memcmp((addr1), (addr2), (len))

#endif //MQX_RTOS

#if MQX_LITE || MQX_RTOS
typedef uint_32         qosal_task_handle; /* what a task_id looks like  */
typedef LWEVENT_STRUCT_PTR         qosal_event_handle;
typedef MUTEX_STRUCT_PTR           qosal_mutex_handle;
typedef MUTEX_ATTR_STRUCT_PTR      QOSAL_MUTEX_ATTR;
typedef void*                      qosal_sema_handle;  
typedef QOSAL_STATUS               QOSAL_EVT_RET;
#elif FREE_RTOS
/*Global Handlers type defination*/
typedef void *          qosal_sem_handle;
typedef void *	        qosal_queue_handle;
typedef void * 	        qosal_task_handle;
typedef EventGroupHandle_t          qosal_event_handle;
typedef QueueHandle_t               qosal_mutex_handle;//TBD
typedef void*                       qosal_sema_handle; //TBD    
typedef QOSAL_VOID*                 QOSAL_EVT_RET;

/* Defines the prototype to which task functions must conform. */
typedef void (*pTask_entry)( void * );

xEventHandle Driver_Wake_Event_handle ;
xEventHandle User_Wake_Event_handle ;
xEventHandle SockSelect_Wake_Event_handle ;

#endif  //MQX_LITE || MQX_RTOS

#if MQX_LITE
#if 0 //TBD -- QCA will provide more updates on this
#if ENABLE_P2P_MODE
#define OS_MAX_SOCKETS_SUPPORTED                (2)
#else
#define OS_MAX_SOCKETS_SUPPORTED                (8)
#endif

#define OS_AR4100_BUFFER_SIZE                   1664
#define OS_ATH_MAX_SCAN_BUFFERS                 (12)

#endif // #if 0 //TBD

void assert_func_dbg(char const *, QOSAL_UINT32);
#define B_ASSERT(expr) if(!(expr)) do{assert_func_dbg(__FUNCTION__, __LINE__);}while(0)

/*Global Structure*/
typedef struct time_struct
{

   /* The number of seconds in the time.  */
   uint_32     SECONDS;

   /* The number of milliseconds in the time. */
   uint_32     MILLISECONDS;

} TIME_STRUCT;

#endif  //#if MQX_LITE

/*
*This universal OS Layer function is used to start the OS
*/
/* Description:
 * This function is used to start the RTOS 
 * 
 * Params: None
 *
 * Returns: None
 */
QOSAL_VOID qosal_start();

/* 
 * Description: This function is used for creating an RTOS task 
 * 
 * Params: Task entry function
 *         Function arguments
 *         Task name
 *         Stack size 
 *         Task priority 
 *         Task handle
 *         Blocking Flag
 * Returns:  QOSAL_OK if Success, QOSAL_ERROR if Fail 
 */
QOSAL_STATUS qosal_task_create
                        (
                           QOSAL_VOID  Task(QOSAL_UINT32), 
                           char *task_name, 
                           int stack_size, QOSAL_VOID *param, 
                           unsigned long task_priority, 
                           qosal_task_handle *task_handle,
                           QOSAL_BOOL blocked  
                        );

/* 
 * Description: This function is used to get the current priority of the task 
 * 
 * Params: Task handle          
 *         Task priority 
 * 
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
 */
QOSAL_STATUS qosal_task_get_priority(qosal_task_handle task_handle, 
                                    QOSAL_UINT32 *priority_ptr);


/* 
 * Description: This function is used to set the current priority of the task 
 * 
 * Params: Task handle          
 *         Old priority 
 *         New priority
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
 */
QOSAL_STATUS qosal_task_set_priority(qosal_task_handle task_handle, 
                                     QOSAL_UINT32 new_priority, 
                                     QOSAL_VOID *priority_ptr);

/* 
 * Description: This function is used to get the current task ID
 * 
 * Params: None
 *          
 * Returns: Task handle
 */
qosal_task_handle qosal_task_get_handle(QOSAL_VOID);

/* 
 * Description: This function is used to destroy the task
 * 
 * Params: task_hanlde
 *          
 * Returns: QOSAL_OK on Success, QOSAL_ERROR on fail
 */
QOSAL_STATUS qosal_task_destroy(qosal_task_handle task_handle);

/* 
 * Description: This function is used to suspend the active task
 * 
 * Params: Task handle
 *          
 * Returns: None
 */
QOSAL_VOID qosal_task_suspend(qosal_task_handle *task_handle);

/* 
 * Description: This function is used to unblock the task
 * 
 * Params: Task handle
 *          
 * Returns: None
 */
QOSAL_VOID qosal_task_resume(qosal_task_handle *task_handle);

/******************************************************************************
 *
 * Memory Management APIs
 *
 *****************************************************************************/
/*
 * Description: This function is used for initializing the memory blocks
 *
 * Params: None
 *          
 * Returns: None
 */
QOSAL_VOID qosal_malloc_init(QOSAL_VOID);

/*
 * Description: This function is used to get the memory block size
 *
 * Params: Address, Size
 *          
 * Returns: Size of memory block 
 */
QOSAL_UINT32 qosal_get_size(QOSAL_VOID* addr);

/*
 * Description: This function is used for allocating the memory block of 
 * requested size
 *
 * Params: Size
 *          
 * Returns: Address of allocatated memory block
 */
QOSAL_VOID* qosal_malloc(QOSAL_UINT32 size);

/*
 * Description: This function is used for freeing of the memory block
 *
 * Params: Address
 *          
 * Returns: None
*/
/*Clear a memory pool*/
QOSAL_STATUS  qosal_free(QOSAL_VOID* addr);

#define QOSAL_MALLOC(size)           qosal_malloc(size)
#define QOSAL_FREE(addr)             qosal_free(addr)

/******************************************************************************
 * Timer Ticks APIs   
 *****************************************************************************/
/*
 * Description: This function is used for delay the OS task for specific time 
 * in milli secs
 *
 * Params: msec
 *          
 * Returns: None 
*/
QOSAL_VOID qosal_msec_delay(QOSAL_ULONG mSec);

/*
 * Description: This function is used for delay the OS task for specific time 
 * in micro secs
 *
 * Params: uSec
 *          
 * Returns: None 
*/
QOSAL_VOID qosal_usec_delay(QOSAL_UINT32 uSec);

/*
 * Description: This function is used to count timer ticks 
 *
 * Params: count
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
*/

QOSAL_STATUS qosal_time_get_ticks(QOSAL_UINT32 *count);


/*
 * Description: This function is used to get time per sec  
 *
 * Params: None
 *          
 * Returns: Time ticks per sec
 */
QOSAL_ULONG qosal_time_get_ticks_per_sec(QOSAL_VOID);

/*
 * Description: This function is used nvalidating all the data cache entries.
 *
 * Params: None
 *          
 * Returns: None
*/
QOSAL_VOID qosal_dcache_invalidate();

/*
 * Description: This function is used flushing the data cache.
 *
 * Params: None
 *          
 * Returns: None
*/
QOSAL_VOID qosal_dcache_flush();

/*
 * Description: This function is used for time delay 
 *
 * Params: Delay value
 *          
 * Returns: None
 */
QOSAL_VOID qosal_time_delay_ticks(QOSAL_ULONG val);

/******************************************************************************
*
* Interrupt Control APIs
*
******************************************************************************/
/*
 * Description: This function is used for disabling the external MCU interrupts
 *
 * Params: None
 *          
 * Returns: None
 */
QOSAL_VOID qosal_intr_disable (QOSAL_VOID);

/*
 * Description: This function is used for enabling the external MCU interrupts
 *
 * Params: None
 *          
 * Returns: None
 */
QOSAL_VOID qosal_intr_enable (QOSAL_VOID);


/*****************************************************************************
*
* Event Handling APIs
*
******************************************************************************/
/*
 * Description: This function is used for waiting for an event 
 *
 * Params: Event pointer, Bits to Wait for, Ticks to Wait for
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_UINT32  qosal_wait_for_event(qosal_event_handle event_ptr, 
                                   QOSAL_UINT32 bitsToWaitFor, 
                                   QOSAL_UINT32 all, 
                                   QOSAL_UINT32 var1, 
                                   QOSAL_UINT32 ticksToWait);
/*
 * Description: This function is used for set an event 
 *
 * Params: Event pointer, Bits to Set
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_set_event(qosal_event_handle event_ptr, QOSAL_UINT32 bitsToSet);


/*
 * Description: This function is used for creating an event
 *
 * Params: Event pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_EVT_RET qosal_create_event(qosal_event_handle event_ptr);

/*
 * Description: This function is used for setting auto clearing of event bits in event group
 *
 * Params: Event pointer, flags
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_set_event_auto_clear(qosal_event_handle event_ptr, QOSAL_UINT32 flags);

/*
 * Description: This function is used for clearing the event 
 *
 * Params: Event pointer, BitsToClear
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
*/
QOSAL_STATUS qosal_clear_event(qosal_event_handle event_ptr, QOSAL_UINT32 bitsToClear);

/*
 * Description:  This function is used for deleting an event   
 *
 * Params: Event pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_delete_event(qosal_event_handle event_ptr);


/*****************************************************************************
 *
 * Task Synchronization APIs (Mutex)
 *
 *****************************************************************************/
/*
 * Description: This function is used for initialization of the mutex 
 *
 * Params: Mutex pointer, Attr_ptr
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */

QOSAL_STATUS qosal_mutex_init(qosal_mutex_handle mutex_ptr);
                              

/*
 * Description: This function is used for aquiring the mutex lock
 *
 * Params: Mutex pointer, tick_count
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_mutex_acquire(qosal_mutex_handle mutex_lock, QOSAL_ULONG tick_count);

/*
 * Description: This function is used for releasing the mutex lock
 *
 * Params: Mutex pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_mutex_release(qosal_mutex_handle mutex_ptr);

/*
 * Description: This function is used for deleting the mutex 
 *
 * Params: Mutex pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
*/
QOSAL_STATUS qosal_mutex_destroy(qosal_mutex_handle mutex_ptr);

/*****************************************************************************
 *
 * Time delay APIs
 *
 *****************************************************************************/
/*
 * Description: This function is used to delay for given time ms 
 *
 * Params: msec
 *          
 * Returns: None
*/
QOSAL_VOID qosal_time_delay(QOSAL_UINT32 msec);

/*
 * Description: This function is used to get the elapsed time from tick time
 *
 * Params: ptr
 *          
 * Returns: None
*/
QOSAL_VOID qosal_time_get_elapsed(TIME_STRUCT* time);
   
/********************************************************************
*               Task Synchronization APIs (Semaphores)
********************************************************************/
//TBD 

QOSAL_STATUS qosal_sema_init(qosal_sema_handle *sem_ptr, 
                              QOSAL_UINT32 maxCount);
QOSAL_STATUS qosal_sema_get(qosal_sema_handle *sem_lock, QOSAL_ULONG tick_count);

QOSAL_STATUS qosal_sema_put(qosal_sema_handle *sem_lock);

QOSAL_STATUS qosal_sema_destroy(qosal_sema_handle *sem_lock);

#if 0 //TBD
   
/*********************************************************************
*                  Kernel Log APIs
*********************************************************************/  

QOSAL_UINT32 qosal_klog_create(QOSAL_UINT32 size, QOSAL_UINT32 flags);   
QOSAL_VOID qosal_klog_control(QOSAL_UINT32 size ,QOSAL_BOOL flags);
QOSAL_BOOL qosal_klog_dispaly(QOSAL_VOID);
QOSAL_UINT32 qosal_klog_create_at(QOSAL_UINT32 max_size, QOSAL_UINT32 flags, QOSAL_VOID* ptr);
/*****************************************************************************/
#endif

#endif          //_OSAL_H_
