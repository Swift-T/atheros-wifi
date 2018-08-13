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

#include <osal.h>

#if FREE_RTOS

xEventHandle Driver_Wake_Event_handle ;
xEventHandle User_Wake_Event_handle ;
xEventHandle SockSelect_Wake_Event_handle ;
#endif //FREE_RTOS



/******************************************************************************/
/* OS Layer Wrapper function implementation */
/******************************************************************************/
/****************
 * OS Task API's
 ****************/

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
QOSAL_VOID qosal_start()
{
#if MQX_LITE 
    _mqxlite();
#endif
}

/* Description:
 * This function is used to get the OS return/error code
 * 
 * Params: OS error code
 *
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
 */
QOSAL_STATUS qosal_get_error_code(QOSAL_UINT32 os_err)
{
    QOSAL_STATUS osal_ret;    
    
    switch(os_err)
    {
#if MQX_LITE || MQX_RTOS    
    case MQX_INVALID_TASK_ID:
        osal_ret = QOSAL_INVALID_TASK_ID; 
        break;
    
    case MQX_INVALID_PARAMETER:
        osal_ret = QOSAL_INVALID_PARAMETER; 
        break;
        
    case MQX_INVALID_POINTER:
        osal_ret = QOSAL_INVALID_POINTER; 
        break;
        
    case MQX_EINVAL:
        osal_ret = QOSAL_ALREADY_EXISTS; 
        break;
        
    case MQX_LWEVENT_INVALID:
        osal_ret = QOSAL_INVALID_EVENT; 
        break;
        
    case LWEVENT_WAIT_TIMEOUT:
        osal_ret = QOSAL_EVENT_TIMEOUT; 
        break;
        
    case MQX_INVALID_COMPONENT_BASE:
        osal_ret = QOSAL_INVALID_MUTEX; 
        break;
        
    case MQX_EDEADLK:
        osal_ret = QOSAL_TASK_ALREADY_LOCKED; 
        break;
        
    case MQX_EBUSY:
        osal_ret = QOSAL_MUTEX_ALREADY_LOCKED; 
        break;  
        
    case MQX_OUT_OF_MEMORY:
        osal_ret = QOSAL_OUT_OF_MEMORY; 
        break;
        
    case MQX_OK:
        osal_ret = QOSAL_OK;
        break;
    
    default:
        osal_ret = QOSAL_ERROR;        
#elif FREE_RTOS
    case pdPASS:
        osal_ret = QOSAL_SUCCESS;
        break;
        
    case pdFAIL:
        osal_ret = QOSAL_ERROR;
        break;
        
    default:
        osal_ret = QOSAL_ERROR;
#endif
    }
    return osal_ret;
}


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
                           QOSAL_BOOL auto_start 
                        )
{  
    QOSAL_STATUS osal_ret = QOSAL_OK;
    QOSAL_UINT32 os_ret;
    
#if MQX_LITE     
    QOSAL_UINT16 processor_number = 0;
    QOSAL_UINT32 template_index = 2; //Give task ID #defines--need to be modified
    QOSAL_VOID* stack_ptr;
    
    task_id = _task_create_at(processor_number, // proc number always 0
                         template_index, // template index
                         (QOSAL_UINT32)param, // task input param
                         stack_ptr,     //task stack          
                         stack_size);   //task stack size    
    
#elif MQX_RTOS 

    TASK_TEMPLATE_STRUCT task_template;
        
    A_MEMZERO((QOSAL_UCHAR*) &task_template, sizeof(task_template));
    task_template.TASK_NAME          = task_name; 
    task_template.TASK_PRIORITY      = task_priority;
    task_template.TASK_STACKSIZE     = stack_size;
    task_template.TASK_ADDRESS       = (TASK_FPTR)Task;
    task_template.CREATION_PARAMETER = (QOSAL_UINT32)param;
    
    if (!auto_start)
   {    
      *task_handle = _task_create_blocked(0, 0, (QOSAL_UINT32)&task_template);
   
        // Caller has to call qosal_task_resume()
                                    // to put the task into ready queue.
   }
   else
   {
       *task_handle = _task_create(0, 0, (QOSAL_UINT32)&task_template);   
   }
   if(task_handle == NULL)
     {
        return QOSAL_ERROR;
     }
        
#elif FREE_RTOS 
    os_ret = xTaskCreate((TaskFunction_t)Task,   /* pointer to the task entry code */
                       task_name,         /* task name */
                       stack_size,        /* task stack size */
                       param,             /* parameters to pass */
                       task_priority,     /* task priority - lower no= lower priority */
                       task_handle);    /* handle by which task can be referenced */
    
    if(os_ret == pdFAIL)
     {
        return QOSAL_ERROR;
     }
    osal_ret = qosal_get_error_code(os_ret);
#endif
    
    return osal_ret;
}

/* 
 * Description: This function is used to get the current priority of the task 
 * 
 * Params: Task handle          
 *         Task priority 
 * 
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
 */
QOSAL_STATUS qosal_task_get_priority(qosal_task_handle task_handle, 
                                        QOSAL_UINT32 *priority_ptr)
{
    QOSAL_STATUS osal_ret = QOSAL_OK;    
    
#if MQX_LITE || MQX_RTOS
    QOSAL_UINT32 os_ret = 0;
    
    os_ret = _task_get_priority(task_handle, priority_ptr);    
    osal_ret = qosal_get_error_code(os_ret);
#elif FREE_RTOS
    task_handle = xTaskGetCurrentTaskHandle();
    *priority_ptr = uxTaskPriorityGet(task_handle);    
#endif  
    
    return osal_ret;
}

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
                                     QOSAL_VOID *priority_ptr)
{
    QOSAL_STATUS osal_ret = QOSAL_OK;    
    
#if MQX_LITE || MQX_RTOS
    QOSAL_UINT32 os_ret = 0;
    
    os_ret = _task_set_priority(task_handle, new_priority, (QOSAL_UINT32 *)&priority_ptr);        
    osal_ret = qosal_get_error_code(os_ret);
#elif FREE_RTOS
    UNUSED_ARGUMENT(priority_ptr);
     vTaskPrioritySet(task_handle, new_priority);   
#endif
    
    return osal_ret;    
}

/* 
 * Description: This function is used to get the current task ID
 * 
 * Params: None
 *          
 * Returns: Task handle
 */
qosal_task_handle qosal_task_get_handle(QOSAL_VOID)
{
    qosal_task_handle handle = 0;
    
#if MQX_LITE || MQX_RTOS
    handle = _task_get_id();
#elif FREE_RTOS
    handle = xTaskGetCurrentTaskHandle();
#endif
    
    return handle;
}

/* 
 * Description: This function is used to destroy the task
 * 
 * Params: task_handle
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
 */
QOSAL_STATUS qosal_task_destroy(qosal_task_handle task_handle)
{
    QOSAL_STATUS osal_ret = QOSAL_OK;    
    
#if MQX_LITE || MQX_RTOS    
    QOSAL_UINT32 os_ret = 0;    
    os_ret = _task_destroy(task_handle);
    osal_ret = qosal_get_error_code(os_ret);
#endif
    
    return osal_ret;
}

/* 
 * Description: This function is used to suspend the active task
 * 
 * Params: Task handle
 *          
 * Returns: None  
 */
QOSAL_VOID qosal_task_suspend(qosal_task_handle *task_handle)
{    
#if MQX_LITE || MQX_RTOS
    _task_block();
#elif FREE_RTOS
    vTaskSuspend(task_handle);    
#endif    
    
    return;
}
   
/* 
 * Description: This function is used to unblock the task
 * 
 * Params: Task handle
 *          
 * Returns: None 
 */

QOSAL_VOID qosal_task_resume(qosal_task_handle *task_handle)
{
#if MQX_LITE || MQX_RTOS
    _task_ready(task_handle);
#elif FREE_RTOS
    vTaskResume(task_handle);    
#endif 
    
    return;    
}

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
QOSAL_VOID qosal_malloc_init(QOSAL_VOID)
{
#if MQX_LITE
  // MQX-Lite memory management is done inside the driver
#elif MQX_RTOS || FREE_RTOS
    //Not needed
#endif
}

/*
 * Description: This function is used to get the memory block size
 *
 * Params: Address, Size
 *          
 * Returns: Size of memory block 
 */
QOSAL_UINT32 qosal_get_size(QOSAL_VOID* addr)
{
  QOSAL_UINT32 size = 0;

#if MQX_RTOS  
    size = _get_size(addr);    
#endif 
	
    return size;
}

/*
 * Description: This function is used for allocating the memory block of 
 * requested size
 *
 * Params: Size
 *          
 * Returns: Address of allocatated memory block
 */
QOSAL_VOID* qosal_malloc(QOSAL_UINT32 size)
{
    QOSAL_VOID* addr = NULL;
    
#if FREE_RTOS
    addr = pvPortMalloc(size);

#elif MQX_LITE
    // MQX-Lite memory allocation is done inside the driver
#elif MQX_RTOS
    QOSAL_UINT32 len;
    addr =  _mem_alloc_system_zero((QOSAL_UINT32)size);
#endif
    
    return addr;
}

/*
 * Description: This function is used for freeing of the memory block
 *
 * Params: Address
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
*/
/*Clear a memory pool*/
QOSAL_STATUS  qosal_free(QOSAL_VOID* addr)
{
   QOSAL_STATUS osal_ret = QOSAL_OK;   
    
#if FREE_RTOS    
    vPortFree(addr);
#elif MQX_LITE        
    // MQX-Lite memory free is done inside the driver   
#elif MQX_RTOS    
    QOSAL_UINT32 os_ret;
    
    os_ret = _mem_free(addr);   
    osal_ret = qosal_get_error_code(os_ret);
#endif
    
    return osal_ret;
}

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
QOSAL_VOID qosal_msec_delay(QOSAL_ULONG mSec)
{
#if MQX_LITE || MQX_RTOS
    signed long time = (mSec*(_time_get_ticks_per_sec())) >> 10;

    time = (time)? time : 1;    
    _time_delay_ticks(time);    
#endif
    
    return;
}

/*
 * Description: This function is used for delay the OS task for specific time 
 * in micro secs
 *
 * Params: uSec
 *          
 * Returns: None 
*/

QOSAL_VOID qosal_usec_delay(QOSAL_UINT32 uSec)
{  
#if MQX_LITE
    QOSAL_LONG time = uSec >> 10;
    time = (time)? time : 1;    
    time = (time*(_time_get_ticks_per_sec())) >> 10;
    time = (time)? time : 1;    
    _time_delay_ticks(time);

#elif MQX_RTOS
    //avoids division and modulo.
    QOSAL_INT32 time = uSec>>10; // divide by 1024
    // account for difference between 1024 and 1000 with 
    // acceptable margin for error
    time += (time * 24 > 500)? 1:0;
    // capture round off between usec and msec
    time += (uSec - (time * 1000) > 500)? 1:0;
    //input must be sub 500 so we simply wait 1msec
    if(time == 0) time += 1;
    //current implementation relys on msec sleep
    _time_delay(time);   
    
#elif FREE_RTOS
    unsigned long xUnblockPeriod = uSec/1000, xCurrentTime = xTaskGetTickCount();   
    if(!xUnblockPeriod)
    {
        xUnblockPeriod = 1;
    }
    vTaskDelayUntil(&xCurrentTime, xUnblockPeriod);
#endif
    
    return;
}

/*
 * Description: This function is used to get absolute time in ticks 
 *
 * Params: count
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
 */

QOSAL_STATUS qosal_time_get_ticks(QOSAL_UINT32 *count)
{ 
    QOSAL_STATUS osal_ret = QOSAL_OK;
    
#if MQX_RTOS || MQX_LITE
    _time_get_ticks((MQX_TICK_STRUCT_PTR)count);    
#elif FREE_RTOS        
    *count = xTaskGetTickCount();     
#endif    
    
    return osal_ret;
}

/*
 * Description: This function is used to get time per sec  
 *
 * Params: None
 *          
 * Returns: Time ticks per sec
 */
QOSAL_ULONG qosal_time_get_ticks_per_sec(QOSAL_VOID)
{
    QOSAL_ULONG val = 0;
    
#if MQX_LITE || MQX_RTOS  
  val = _time_get_ticks_per_sec();  
#endif
    
  return val;   
}

/*
 * Description: This function is used flushing the data cache.
 *
 * Params: None
 *          
 * Returns: None
*/
QOSAL_VOID qosal_dcache_flush(QOSAL_VOID)
{
#if MQX_RTOS
  _DCACHE_FLUSH();
#endif  
}

/*
 * Description: This function is used nvalidating all the data cache entries.
 *
 * Params: None
 *          
 * Returns: None
*/
QOSAL_VOID qosal_dcache_invalidate(QOSAL_VOID)
{
#if MQX_RTOS
  _DCACHE_INVALIDATE();
#endif
}
  
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
QOSAL_VOID qosal_intr_disable (QOSAL_VOID)
{
#if MQX_LITE || MQX_RTOS
    _int_disable();
#elif FREE_RTOS
    portDISABLE_INTERRUPTS();       
#endif
}

/*
 * Description: This function is used for enabling the external MCU interrupts
 *
 * Params: None
 *          
 * Returns: None
 */
QOSAL_VOID qosal_intr_enable (QOSAL_VOID)
{
#if MQX_LITE || MQX_RTOS
    _int_enable();    
#elif FREE_RTOS
    portENABLE_INTERRUPTS(); 
#endif
}

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

QOSAL_UINT32 qosal_wait_for_event(qosal_event_handle event_ptr, 
                                   QOSAL_UINT32 bitsToWaitFor, 
                                   QOSAL_UINT32 all, 
                                   QOSAL_UINT32 var1, 
                                   QOSAL_UINT32 ticksToWait)
{   
    QOSAL_UINT32 os_ret;
    
#if MQX_LITE || MQX_RTOS
    event_ptr->AUTO = 1; // with 0 value it doesn't work
    os_ret = _lwevent_wait_ticks(event_ptr, bitsToWaitFor, all, 
                              ticksToWait);
#elif FREE_RTOS        
    /*os_ret =  xEventGroupWaitBits(event_ptr, 
                               bitsToWaitFor, 
                               (const BaseType_t)all, 
                               (const BaseType_t) var1, 
                               ticksToWait); */   
    os_ret =  xEventGroupGetBits( (const EventGroupHandle_t)event_ptr );    
    //printf("WFE ... %d\n", ret);  
#endif  
    return os_ret;
}

/*
 * Description: This function is used for set an event 
 *
 * Params: Event pointer, Bits to Set
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_set_event(qosal_event_handle event_ptr, QOSAL_UINT32 bitsToSet)
{
    QOSAL_STATUS osal_ret = QOSAL_OK;    
    QOSAL_UINT32 os_ret = 0;
    
#if MQX_LITE || MQX_RTOS
    os_ret = _lwevent_set(event_ptr, bitsToSet);    
#elif FREE_RTOS
    os_ret = xEventGroupSetBits(event_ptr, (const EventBits_t) bitsToSet);    
#endif  
    
    osal_ret = qosal_get_error_code(os_ret);
    return osal_ret;
}

/*
 * Description: This function is used for creating an event
 *
 * Params: Event pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
//QOSAL_STATUS
QOSAL_EVT_RET
qosal_create_event(qosal_event_handle event_ptr)
{        
#if MQX_LITE || MQX_RTOS
    QOSAL_STATUS osal_ret = QOSAL_OK;
    QOSAL_UINT32 os_ret = 0;
    
    os_ret  = _lwevent_create(event_ptr, 0);            
    osal_ret = qosal_get_error_code(os_ret);
    return osal_ret;
#elif FREE_RTOS           
    event_ptr = xEventGroupCreate();
    return event_ptr;
#endif    
}

/*
 * Description: This function is used for setting auto clearing of event bits in event group
 *
 * Params: Event pointer, flags
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_set_event_auto_clear(qosal_event_handle event_ptr, QOSAL_UINT32 flags)
{    
    QOSAL_STATUS osal_ret = QOSAL_OK;    
    
#if MQX_LITE || MQX_RTOS  
    QOSAL_UINT32 os_ret = 0;
    
    os_ret = _lwevent_set_auto_clear(event_ptr, flags);        
    osal_ret = qosal_get_error_code(os_ret);    
#elif FREE_RTOS
    //Not needed
#endif    
    
    return osal_ret;
}

/*
 * Description: This function is used for clearing the event 
 *
 * Params: Event pointer, BitsToClear
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
*/
QOSAL_STATUS qosal_clear_event(qosal_event_handle event_ptr, QOSAL_UINT32 bitsToClear)
{
    QOSAL_STATUS osal_ret;
    QOSAL_UINT32 os_ret;
    
#if MQX_LITE || MQX_RTOS
    os_ret = _lwevent_clear(event_ptr, bitsToClear);    
#elif FREE_RTOS    
    os_ret = xEventGroupClearBits(event_ptr, (const EventBits_t)bitsToClear);    
#endif
    
    osal_ret = qosal_get_error_code(os_ret);
    return osal_ret;
}

/*
 * Description:  This function is used for deleting an event   
 *
 * Params: Event pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_delete_event(qosal_event_handle event_ptr)
{
    QOSAL_STATUS osal_ret = QOSAL_OK;    
    
#if MQX_LITE || MQX_RTOS
    QOSAL_UINT32 os_ret = 0;
    
    os_ret = _lwevent_destroy(event_ptr);
    osal_ret = qosal_get_error_code(os_ret);
#elif FREE_RTOS
    vEventGroupDelete(event_ptr);
#endif
    
    return osal_ret;
}

/*****************************************************************************
 *
 * Task Synchronization APIs (Mutex)
 *
 *****************************************************************************/
/*
 * Description: This function is used for initialization of the mutex 
 *
 * Params: Mutex pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_mutex_init(qosal_mutex_handle mutex_ptr)
{
    QOSAL_STATUS osal_ret = QOSAL_OK;
        
#if MQX_LITE || MQX_RTOS
    QOSAL_UINT32 os_ret = 0;
    
    os_ret = _mutex_init(mutex_ptr, NULL);
    osal_ret = qosal_get_error_code(os_ret);
#elif FREE_RTOS
    // 
#endif
    
    return osal_ret;
}

/*
 * Description: This function is used for aquiring the mutex lock
 *
 * Params: Mutex pointer, tick_count
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_mutex_acquire(qosal_mutex_handle mutex_lock, QOSAL_ULONG tick_count)
{
    QOSAL_STATUS osal_ret;
    QOSAL_UINT32 os_ret;
    
#if MQX_LITE || MQX_RTOS
    UNUSED_ARGUMENT(tick_count);
    os_ret = _mutex_lock(mutex_lock);
#elif FREE_RTOS
    os_ret = xSemaphoreTake(mutex_lock, tick_count);
#endif
    
    osal_ret = qosal_get_error_code(os_ret);
    return osal_ret; 
}

/*
 * Description: This function is used for releasing the mutex lock
 *
 * Params: Mutex pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail
 */
QOSAL_STATUS qosal_mutex_release(qosal_mutex_handle mutex_ptr)
{
    QOSAL_STATUS osal_ret;
    QOSAL_UINT32 os_ret;
    
#if MQX_LITE || MQX_RTOS
    os_ret = _mutex_unlock(mutex_ptr);
#elif FREE_RTOS        
    os_ret = xSemaphoreGive(mutex_ptr);    
#endif
    
    osal_ret = qosal_get_error_code(os_ret);
    return osal_ret;
}

/*
 * Description: This function is used for deleting the mutex 
 *
 * Params: Mutex pointer
 *          
 * Returns: QOSAL_OK if Success, QOSAL_ERROR if Fail 
*/
QOSAL_STATUS qosal_mutex_destroy(qosal_mutex_handle mutex_ptr)
{
    QOSAL_STATUS osal_ret = QOSAL_OK;    
    
#if MQX_LITE || MQX_RTOS
    QOSAL_UINT32 os_ret = 0;
    
    os_ret = _mutex_destroy((MUTEX_STRUCT_PTR)mutex_ptr);  
    osal_ret = qosal_get_error_code(os_ret);
#endif
    return osal_ret;
}

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
QOSAL_VOID qosal_time_delay(QOSAL_UINT32 msec)
{    
#if MQX_RTOS 
    _time_delay(msec);
#elif MQX_LITE
   //
#elif FREE_RTOS
     #define qosal_time_delay(msecs) \
              do{\
                portTickType xCurrentTime = xTaskGetTickCount(); \
                vTaskDelayUntil(&xCurrentTime, (msecs));\
              }while(0)    
#endif
    return;
}

/*
 * Description: This function is used to get the elapsed time from tick time
 *
 * Params: Ptr to Time struct
 *          
 * Returns: None
*/
QOSAL_VOID qosal_time_get_elapsed(TIME_STRUCT* time)
{  
#if MQX_RTOS || MQX_LITE
    QOSAL_UINT32 ticks_per_sec = _time_get_ticks_per_sec();    
    MQX_TICK_STRUCT tick_struct;   
    
    _time_get_elapsed_ticks(&tick_struct);
    
    time->SECONDS = tick_struct.TICKS[0] / ticks_per_sec;
    time->MILLISECONDS = (tick_struct.TICKS[0] - (time->SECONDS * ticks_per_sec)) * 1000 / ticks_per_sec;   
#elif FREE_RTOS
    if(time)
    {
      if(xTaskGetTickCount() > 0)  
      {     
        time->SECONDS = xTaskGetTickCount()/1000;      
        time->MILLISECONDS = xTaskGetTickCount()%1000;            
      }
      else
      {
         printf("ERROR:%s:time=%d%d\r\n",__func__,time->SECONDS,time->MILLISECONDS);        
      }
    }
    else
    {
      printf("ERROR:%s:Null pointer\r\n",__func__);      
    }
#endif
    return;
}

/*********************************************************************
 *                 Kernel Log APIs
*********************************************************************/                    
/*
 * Description: This function is used to Creates the kernel logs
 *
* Params: size    : size of the log, 
           flags  :   1 (When the log is full, oldest entries are overwritten.)
-                     0 (When the log is full, no more entries are written; default.)
 *          
* Returns: QOSAL_OK for success, QOSAL_ERROR for failure 
*/
QOSAL_UINT32 qosal_klog_create(QOSAL_UINT32 size, QOSAL_UINT32 flags)
{
    QOSAL_UINT32 os_ret = 0;
    
#if MQX_RTOS  || MQX_LITE
    os_ret = _klog_create(size, flags);   
#endif   
    
    return os_ret;
}  

/*
 * Description: Controls logging in kernel log
 *
* Params:   bit_mask    :    Which bits of the kernel log control variable to modify.
           set_bits     :   TRUE ((Bits set in bit_mask are set in the control variable)
-                           FALSE (Bits set in bit_mask are cleared in the control variable)
 *          
* Returns: None
*/

QOSAL_VOID qosal_klog_control(QOSAL_UINT32 bit_mask, QOSAL_BOOL set_bits)
{
#if MQX_RTOS || MQX_LITE
  _klog_control(bit_mask , set_bits);
#endif 
}

/*
 * Description: Displays the oldest entry in kernel log and delete this entry
 *
* Params: None
 *          
* Returns: QOSAL_OK for success, QOSAL_ERROR for failure 
*/

QOSAL_BOOL qosal_klog_dispaly(QOSAL_VOID)
{
    QOSAL_BOOL  val = 0;
    
#if MQX_RTOS || MQX_LITE
   val = _klog_display();
#endif 
    
   return val;
}
/*
 * Description: This function is used to Creates the kernel logs
 *
* Params: size    :   size of the log, 
          flags   :   1 (When the log is full, oldest entries are overwritten.)
-                     0 (When the log is full, no more entries are written; default.)
          ptr     :   Where in memory is the log to start
 *          
* Returns: QOSAL_OK for success, QOSAL_ERROR for failure 
*/

QOSAL_UINT32 qosal_klog_create_at(QOSAL_UINT32 max_size, QOSAL_UINT32 flags, QOSAL_VOID* ptr)
{
    QOSAL_UINT32 os_ret = 0;
    
#if MQX_RTOS || MQX_LITE
    os_ret = _klog_create_at(max_size, flags, ptr);    
#endif   

    return os_ret;
}


/*******************************************************************************/
