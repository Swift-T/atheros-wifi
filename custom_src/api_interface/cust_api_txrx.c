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
#include <common_api.h>
#include <custom_wlan_api.h>
#include <htc.h>
#include <atheros_wifi_api.h>

#if (!ENABLE_STACK_OFFLOAD)

uint_32 
Custom_Api_Send(ENET_CONTEXT_STRUCT_PTR enet_ptr, PCB_PTR pcb_ptr, uint_32 size, uint_32 frags, uint_32 flags);

/*****************************************************************************/
/*  Custom_DeliverFrameToNetworkStack - Called by API_RxComplete to 
 *   deliver a data frame to the network stack. This code will perform 
 *   platform specific operations.
 *      QOSAL_VOID *pCxt - the driver context. 
 *      QOSAL_VOID *pReq - the packet.
 *****************************************************************************/
QOSAL_VOID 
Custom_DeliverFrameToNetworkStack(QOSAL_VOID *pCxt, QOSAL_VOID *pReq)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)pReq;
    ENET_CONTEXT_STRUCT_PTR enet_ptr = (ENET_CONTEXT_STRUCT_PTR)GET_DRIVER_CXT(pCxt)->pUpperCxt[0];
	ENET_ECB_STRUCT_PTR RxECB;
	QOSAL_UINT32 len;		
	A_DRIVER_CONTEXT *pDCxt = GET_DRIVER_COMMON(pCxt);
    ATH_PROMISCUOUS_CB prom_cb = (ATH_PROMISCUOUS_CB)(GET_DRIVER_CXT(pCxt)->promiscuous_cb);
    
    if(a_netbuf_ptr) {    	        
    	len = A_NETBUF_LEN(pReq);
    	_DCACHE_INVALIDATE_MBYTES(A_NETBUF_DATA(pReq), len);
        
        if(pDCxt->promiscuous_mode){				    	
	    	if(prom_cb){
	    		/* send frame to callback function rather than an ENET_receiver */  
				a_netbuf_ptr->native.FRAG[0].LENGTH = len;
		    	a_netbuf_ptr->native.FRAG[0].FRAGMENT = A_NETBUF_DATA(pReq);
	    		prom_cb((QOSAL_VOID*)&(a_netbuf_ptr->native));			
	    	}else{
	    		A_NETBUF_FREE(pReq);
	    	}
        }else if((RxECB = ENET_find_receiver(enet_ptr, (ENET_HEADER_PTR) A_NETBUF_DATA(pReq), &len))!= NULL){
        	// Call the receiver's service function to pass the PCB to the receiver   
        	a_netbuf_ptr->native.FRAG[0].LENGTH = len;
	    	a_netbuf_ptr->native.FRAG[0].FRAGMENT = A_NETBUF_DATA(pReq);	    			    		    		    	
    		RxECB->SERVICE((PCB_PTR)&(a_netbuf_ptr->native),RxECB->PRIVATE);    				
    	}else{    	
            /* failed to find a receiver for this data packet. */
    		A_NETBUF_FREE(pReq);
    	}    
    }   
}

/*****************************************************************************/
/*  Custom_Api_Send - Entry point for MQX driver interface to send a packet.
 *	 This is specific to MQX. This function is NOT called from within the 
 *	 driver.
 *      ENET_CONTEXT_STRUCT_PTR  enet_ptr - the MQX driver context.
 *		PCB_PTR              pcb_ptr - the MQX packet object.
 *		uint_32              size - the length in bytes of pcb_ptr.
 *		uint_32              frags - the number of fragments in pcb_ptr.
 *		uint_32              flags - optional flags for transmit.      
 *****************************************************************************/
uint_32 
Custom_Api_Send
   (
      ENET_CONTEXT_STRUCT_PTR  enet_ptr,
         /* [IN] the Ethernet state structure */
      PCB_PTR              pcb_ptr,
         /* [IN] the packet to send */
      uint_32              size,
         /* [IN] total size of the packet */
      uint_32              frags,
         /* [IN] total fragments in the packet */
      uint_32              flags
         /* [IN] optional flags, zero = default */
   )
{ 
    uint_32 error = ENET_OK;
    A_NETBUF* a_netbuf_ptr;
    UNUSED_ARGUMENT(flags);
    UNUSED_ARGUMENT(size);
#if 0    
if(g_sampleTotAlloc != g_totAlloc){
	A_PRINTF("send alloc: %d\n", g_totAlloc);
	g_sampleTotAlloc = g_totAlloc;
}
#endif    
    /* create an atheros pcb and continue or fail. */
    do{    	
    	/* provide enough space in the top buffer to store the 14 byte 
    	 * header which will be copied from its position in the original
    	 * buffer. this will allow wmi_dix_2_dot3() to function properly.
    	 */
		if((a_netbuf_ptr = (A_NETBUF*)A_NETBUF_ALLOC(sizeof(ATH_MAC_HDR))) == NULL){    	
    		error = ENETERR_ALLOC_PCB;
    		break;
    	}
    
		a_netbuf_ptr->num_frags = (QOSAL_UINT8)frags;   
		/* HACK: copy the first part of the fragment to the new buf in order to allow 
		 * wmi_dix_2_dot3() to function properly. */
		A_ASSERT(pcb_ptr->FRAG[0].LENGTH >= sizeof(ATH_MAC_HDR));
		A_NETBUF_PUT_DATA(a_netbuf_ptr, (QOSAL_UINT8*)pcb_ptr->FRAG[0].FRAGMENT, sizeof(ATH_MAC_HDR));
		/*(QOSAL_CHAR*)pcb_ptr->FRAG[0].FRAGMENT += sizeof(ATH_MAC_HDR);*/
                pcb_ptr->FRAG[0].FRAGMENT = (QOSAL_UCHAR*)((QOSAL_UINT32)pcb_ptr->FRAG[0].FRAGMENT + sizeof(ATH_MAC_HDR));
		pcb_ptr->FRAG[0].LENGTH -= sizeof(ATH_MAC_HDR);

    	//ensure there is enough headroom to complete the tx operation
    	if (A_NETBUF_HEADROOM(a_netbuf_ptr) < sizeof(ATH_MAC_HDR) + sizeof(ATH_LLC_SNAP_HDR) +
            		sizeof(WMI_DATA_HDR) + HTC_HDR_LENGTH + WMI_MAX_TX_META_SZ) {
            error = ENETERR_ALLOC_PCB;            
    		break;   		
        }
    	//carry the original around until completion.
    	//it is freed by A_NETBUF_FREE  
    	a_netbuf_ptr->native_orig = pcb_ptr;       
    	
  		if(A_OK != Api_DataTxStart((QOSAL_VOID*)enet_ptr->MAC_CONTEXT_PTR, (QOSAL_VOID*)a_netbuf_ptr)){
    		error = ENET_ERROR;
    		break;
    	}
    	    
   	}while(0);
           
	if(error != ENET_OK){
		/* in the event of api failure, the original pcb should be returned to the caller un-touched
		 * and the a_netbuf_ptr should be freed */
		a_netbuf_ptr->native_orig = NULL;
		A_NETBUF_FREE((QOSAL_VOID*)a_netbuf_ptr);
	}           
           
    return error;
}

#endif