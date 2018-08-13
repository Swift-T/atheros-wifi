//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
//==============================================================================
// Author(s): ="Atheros"
//==============================================================================
#include <a_config.h>
#include <a_types.h>
#include <a_osapi.h>

#include <custom_wlan_api.h>
#include <common_api.h>
#include <netbuf.h>

#include "enet.h"
#include "enetprv.h"
#include "enet_wifi.h"
#include "atheros_wifi_api.h"
#include "atheros_wifi_internal.h"



ATH_CUSTOM_INIT_T ath_custom_init = 
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,	
	NULL,
	NULL,
	A_FALSE,
	A_FALSE,
};

ATH_CUSTOM_MEDIACTL_T ath_custom_mediactl = 
{
	NULL
};

ATH_CUSTOM_HTC_T ath_custom_htc = 
{
	NULL,
	NULL
};

static QOSAL_VOID 
Custom_FreeRxRequest(A_NATIVE_NETBUF *native_ptr)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)native_ptr;
#if (!ENABLE_STACK_OFFLOAD)      
#if DRIVER_CONFIG_IMPLEMENT_RX_FREE_QUEUE    
    A_CUSTOM_DRIVER_CONTEXT *pCxt = (A_CUSTOM_DRIVER_CONTEXT *)a_netbuf_ptr->native.PRIVATE;
    	
	if(_mem_get_type(a_netbuf_ptr) == MEM_TYPE_ATHEROS_PERSIST_RX_PCB) {    
	    RXBUFFER_ACCESS_ACQUIRE((QOSAL_VOID*)pCxt);	    
	    A_NETBUF_ENQUEUE(&(GET_DRIVER_COMMON(pCxt)->rxFreeQueue), a_netbuf_ptr);			   
	    Driver_ReportRxBuffStatus((QOSAL_VOID*)pCxt, A_TRUE);	    
	    RXBUFFER_ACCESS_RELEASE((QOSAL_VOID*)pCxt);
    }else{
    	A_ASSERT(0);
    }
#else
	default_native_free_fn((PCB_PTR)a_netbuf_ptr);
#endif
#endif
}


/*****************************************************************************/
/*  Custom_GetRxRequest - Attempts to get an RX Buffer + RX Request object 
 *	 for a pending RX packet.  If an Rx Request cannot be acquired then this
 *	 function calls Driver_ReportRxBuffStatus(pCxt, A_FALSE) to prevent the 
 *	 driver from repeating the request until a buffer becomes available. 
 *      QOSAL_VOID *pCxt - the driver context.
 *		QOSAL_UINT16 length - length of request in bytes.
 *****************************************************************************/
QOSAL_VOID *
Custom_GetRxRequest(QOSAL_VOID *pCxt, QOSAL_UINT16 length)
{    	
    QOSAL_VOID *pReq;
    
	do{						    	    									    	    	
	    RXBUFFER_ACCESS_ACQUIRE(pCxt);
        {
#if DRIVER_CONFIG_IMPLEMENT_RX_FREE_QUEUE        
	        pReq = A_NETBUF_DEQUEUE(&(GET_DRIVER_COMMON(pCxt)->rxFreeQueue));	    		    		    	
    	    
		    if(A_NETBUF_QUEUE_EMPTY(&(GET_DRIVER_COMMON(pCxt)->rxFreeQueue))){
			    Driver_ReportRxBuffStatus(pCxt, A_FALSE);
		    }
#else
			pReq = A_NETBUF_ALLOC(length);
			if(pReq == NULL){
				Driver_ReportRxBuffStatus(pCxt, A_FALSE);	
			}
#endif		    
        }		
	    RXBUFFER_ACCESS_RELEASE(pCxt); 
	    
	    if(pReq != NULL) {
	    	/* init pcb */
    		A_NETBUF_INIT(pReq, (QOSAL_VOID*)Custom_FreeRxRequest, pCxt);	     	
    		A_ASSERT(length <= A_NETBUF_TAILROOM(pReq)); 	    	
    	}else{
    		//should never happen thanks to HW_ReportRxBuffStatus
	    	A_ASSERT(0);	    	
    	}    	    		    	   	    
    }while(0);
    
    return pReq;        
}


/*****************************************************************************/
/*  Custom_Driver_ContextDeInit - Frees any allocations performed by 
 *	 Custom_Driver_ContextInit.
 *      QOSAL_VOID *pCxt - the driver context.
 *****************************************************************************/
QOSAL_VOID 
Custom_Driver_ContextDeInit(QOSAL_VOID *pCxt)
{
	QOSAL_VOID *pReq;
	
	while(GET_DRIVER_COMMON(pCxt)->rxBufferStatus == A_TRUE){
		pReq = Custom_GetRxRequest(pCxt, 10);
		/* call default free function */
		default_native_free_fn((PCB_PTR)pReq);	
	}
	
	if(GET_DRIVER_COMMON(pCxt)->tempStorage != NULL){
		A_FREE(GET_DRIVER_COMMON(pCxt)->tempStorage,0);
		GET_DRIVER_COMMON(pCxt)->tempStorage = NULL;
	}
}


/*****************************************************************************/
/*  Custom_Driver_ContextInit - Allocates and initializes driver context 
 *	 elements including pre-allocated rx buffers. This function will call
 *	 A_ASSERT for any failure. Look to Custom_Driver_ContextDeInit to undo
 *	 any allocations performed by this function.
 *      QOSAL_VOID *pCxt - the driver context.
 *****************************************************************************/
A_STATUS 
Custom_Driver_ContextInit(QOSAL_VOID *pCxt)
{	
	ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)GET_DRIVER_CXT(pCxt)->pUpperCxt[0];
	QOSAL_VOID *pReq;
    QOSAL_UINT32 tempStorageLen = 0;
     A_DRIVER_CONTEXT* pDCxt = GET_DRIVER_COMMON(pCxt);
    
    /*Allocate the temporary storage buffer. This may be shared by multiple modules. 
      If store recall is enabled, it may use this buffer for storing target data.
      Will also be shared by scan module to store scan results*/
    tempStorageLen = max((STORE_RECALL_BUF_SIZE), (ATH_MAX_SCAN_BUFFERS*sizeof(ENET_SCAN_INFO)));
    tempStorageLen = max(tempStorageLen, (ATH_MAX_SCAN_BUFFERS*sizeof(ATH_SCAN_EXT)));
    
    if(tempStorageLen)
    {
       if(NULL == (pDCxt->tempStorage = A_MALLOC(tempStorageLen,0))){        
         return A_NO_MEMORY;
       }
       pDCxt->tempStorageLength = tempStorageLen;
    }
    else
    {
       pDCxt->tempStorage = NULL;
       pDCxt->tempStorageLength = 0;
    }
    
	/* Prepare buffer for the scan results, reuse tempStorage. */
    GET_DRIVER_COMMON(pCxt)->pScanOut =  pDCxt->tempStorage;
    A_ASSERT(GET_DRIVER_COMMON(pCxt)->pScanOut != NULL);
    GET_DRIVER_COMMON(pCxt)->securityType = ENET_MEDIACTL_SECURITY_TYPE_NONE;  
#if DRIVER_CONFIG_IMPLEMENT_RX_FREE_QUEUE     
	{  
		QOSAL_UINT16 numRxBuffers, i;
		QOSAL_VOID *pReq;	  
		/* allocate rx objects and buffers */
		A_NETBUF_QUEUE_INIT(&GET_DRIVER_COMMON(pCxt)->rxFreeQueue);
		/* NUM_RX_PCBS should be == BSP_CONFIG_ATHEROS_PCB which
		 * is defined in atheros_wifi.h
		 */	 
		numRxBuffers = enet_ptr->PARAM_PTR->NUM_RX_PCBS;
		//pre-allocate rx buffers
		for(i=0 ; i< numRxBuffers; i++){
		
			pReq = A_NETBUF_ALLOC_RAW(AR4100_MAX_RX_MESSAGE_SIZE);
	    		
			A_ASSERT(pReq != NULL);  
#if ENABLE_STACK_OFFLOAD	
			/*Set rx pool ID so that the buffer is freed to the RX pool*/		
			A_NETBUF_SET_RX_POOL(pReq);
#endif					    	
	    	/* mqx allows us to set a memtype to help identify buffers */	
	    	_mem_set_type(pReq, MEM_TYPE_ATHEROS_PERSIST_RX_PCB);
		    A_NETBUF_ENQUEUE(&GET_DRIVER_COMMON(pCxt)->rxFreeQueue, pReq);
		    
	    }
	}
#endif    

    /* init the common->rxBufferStatus to true to indicate that rx buffers are
     * ready for use. */
    pDCxt->rxBufferStatus = A_TRUE;
    GET_DRIVER_COMMON(pCxt)->driverShutdown = A_FALSE;
    
    return A_OK;
}





