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
//#include "mqx.h"
//#include "bsp.h"
//#include "enet.h"
//#include "enetprv.h"
#include <atheros_wifi_api.h>
#include "cust_netbuf.h"

#if ENABLE_STACK_OFFLOAD
#include "atheros_stack_offload.h"
#include "common_stack_offload.h"
#include "custom_stack_offload.h"
#include "atheros_wifi.h"


QOSAL_UINT32 Custom_Api_Send(ENET_CONTEXT_STRUCT_PTR enet_ptr, PCB_PTR pcb_ptr, uint_32 size, uint_32 frags, uint_32 flags);


QOSAL_UINT32 Custom_Api_Send(ENET_CONTEXT_STRUCT_PTR enet_ptr, PCB_PTR pcb_ptr, uint_32 size, uint_32 frags, uint_32 flags)
{
	return A_OK;
}

uint_32 custom_sock_is_pending(ATH_SOCKET_CONTEXT *sock_context)
{
    uint32 i, rtn;
    for(rtn = 0, i = 0; i <(SOCK_CMD_LAST/32+1); i++)
    { 
        rtn |= sock_context->sock_st_mask[i];
    }
    
    return rtn;
}

/*****************************************************************************/
/*  get_pending_pkts_buffered - 
 *   Returns number of pending packets buffered across all     socket queues.           
 *   RETURNS: number of packets 
 *****************************************************************************/

QOSAL_UINT32 get_holding_buffer()
{
	QOSAL_UINT8 index;
	QOSAL_UINT8 holdingBuffers = 0;
	QOSAL_UINT8 active_sock = 0;
	SOCKET_CONTEXT_PTR pcustctxt = NULL;
       for(index = 0; index < MAX_SOCKETS_SUPPORTED; index++)
	   {       
	        if(ath_sock_context[index]->handle!=0)
	         {			 
	             pcustctxt = GET_SOCKET_CONTEXT(ath_sock_context[index]);		  
                   active_sock++;  /*count the valid socket*/
                   if(A_NETBUF_QUEUE_SIZE(&(pcustctxt->rxqueue)) >=1)
		           holdingBuffers +=A_NETBUF_QUEUE_SIZE(&(pcustctxt->rxqueue)); /*count the number of pending buffer*/
	         }
         }

 /*   if more than 2 sockets hold 2+buffer, it maybe one socket hold multi buffer or multi socket hold at lease one buffer,
   *   so multisocket is enabled and hold server buffers. 
   */
/*if multiple socket is active , then return the total pending buffers, single socket won't check the pending sock.*/
	if(active_sock> 2)   
	  return holdingBuffers;
 	else 
	  return 0;			


}



/*****************************************************************************/
/*  custom_receive_tcpip - Called by Custom_DeliverFrameToNetworkStack to 
 *   deliver a data frame to the application thread. It unblocks a waiting 
 *   application thread and queues the packet in the queue of the corresponding 
 *   socket. If socket is not valid, the packet is dropped.
 *      QOSAL_VOID *pCxt - the driver context. 
 *      QOSAL_VOID *pReq - the packet.
 *****************************************************************************/
uint_32 custom_receive_tcpip(QOSAL_VOID *pCxt, QOSAL_VOID *pReq)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)pReq;	
    QOSAL_INT32 index = 0, handle=0;
    QOSAL_UINT8* data;
    SOCKET_CONTEXT_PTR pcustctxt = NULL;

    if(a_netbuf_ptr) {        
  	
		data = (QOSAL_UINT8*)A_NETBUF_DATA(a_netbuf_ptr);
	
    	/*Extract socket handle from custom header*/
    	handle = A_CPU2LE32(*((QOSAL_UINT32*)data));
    	
    	/*Get index*/
    	index = find_socket_context(handle, TRUE);
	if(index < 0 || index > MAX_SOCKETS_SUPPORTED)
	{
		last_driver_error = A_SOCKCXT_NOT_FOUND;
		A_NETBUF_FREE(pReq);
		return A_ERROR;
	}

        if(custom_sock_is_pending(ath_sock_context[index]) != 0){
            /* socket is pending a socket event so it cannot 
             * receive data packets */
            last_driver_error = A_SOCK_UNAVAILABLE;
            A_NETBUF_FREE(pReq);
            return A_ERROR;
        }
						
		UNBLOCK_SELECT(pCxt);
		pcustctxt = GET_SOCKET_CONTEXT(ath_sock_context[index]);

		//if(!IS_SOCKET_BLOCKED(ath_sock_context[index]))
		/*To-Do- Remove this once drop ligic is implemented in firmware*/ 
              /* For Mutiple Socket TCP Rx , there is a limitaion. In tcp rx , once tcp connection is established, fw will recevie data and send to host 
                *  without considering host have enough buffer or blocked.
                * if host buffers are all occupied by rx data from fw sometime, the tcp accept wmi event will be blocked in the driver layer.
                * and the api_accept will be blocked which will cause app layer qcom_recv can't  extract data from buffer and free buffer.
                * this lead deadlock . so we reserve one buffer for mutiple socket RX. This handle will solve the deadlock issue and also may lead
                * some pakcet dropped.
                */		
           /*more than 2 different socket has data that be buffered, be different origianl two single traffic */
	 if(get_holding_buffer()>= 2)	
		{
		   if(get_total_pkts_buffered() >= BSP_CONFIG_ATHEROS_PCB - 1)
		   	{
		   	   printf("%s dropping packets for %x\n", __FUNCTION__, handle);
			   A_NETBUF_FREE(pReq);
		   	}else
		     {
			   A_NETBUF_ENQUEUE(&(pcustctxt->rxqueue), a_netbuf_ptr);
			   UNBLOCK(ath_sock_context[index], RX_DIRECTION);  
		     }	   
		}
		else 
		{
		  if(get_total_pkts_buffered() >= BSP_CONFIG_ATHEROS_PCB )
		  {
			printf("%s dropping packets for %x\n", __FUNCTION__, handle);
			A_NETBUF_FREE(pReq);
		  }else
		   {
			A_NETBUF_ENQUEUE(&(pcustctxt->rxqueue), a_netbuf_ptr);
			UNBLOCK(ath_sock_context[index], RX_DIRECTION);  
		   }
		   
		}

    }
    return A_OK;
}

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
	ATH_PROMISCUOUS_CB prom_cb = (ATH_PROMISCUOUS_CB)(GET_DRIVER_CXT(pCxt)->promiscuous_cb);
    		
    if(GET_DRIVER_COMMON(pCxt)->promiscuous_mode){
        prom_cb(pReq);
    }else{
    	custom_receive_tcpip(pCxt,pReq);
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
uint_32 custom_send_tcpip
   (  
   	  QOSAL_VOID*              pCxt,    
      DRV_BUFFER_PTR       db_ptr,
         /* [IN] the packet to send */
      uint_32              size,
         /* [IN] total size of the packet */
      uint_32              frags,
         /* [IN] total fragments in the packet */
      QOSAL_UINT8*              header,   
      uint_32              header_size   
      
   )
{ 
    uint_32 error = A_OK;
    A_NETBUF* a_netbuf_ptr; 
    uint_32 total_hdr_size =  header_size;
    QOSAL_UINT8* payloadPtr = db_ptr->bufFragment[0].payload;
    
    /* create an atheros pcb and continue or fail. */
    do{    	
    	
      /*Get the netbuf pointer from TX packet*/
        a_netbuf_ptr = ((TX_PACKET_PTR)(payloadPtr - TX_PKT_OVERHEAD))->a_netbuf_ptr;

        a_netbuf_ptr->num_frags = (QOSAL_UINT8)frags;   

        A_NETBUF_PUT_DATA(a_netbuf_ptr, (QOSAL_UINT8*)header, total_hdr_size);
    	//ensure there is enough headroom to complete the tx operation
    	if (A_NETBUF_HEADROOM(a_netbuf_ptr) < sizeof(ATH_MAC_HDR) + sizeof(ATH_LLC_SNAP_HDR) +
            		sizeof(WMI_DATA_HDR) + HTC_HDR_LENGTH + WMI_MAX_TX_META_SZ) {
            error = A_NO_MEMORY;
            A_NETBUF_FREE(a_netbuf_ptr);
            break;   		
        }
    	//carry the original around until completion.
    	//it is freed by A_NETBUF_FREE  
    	a_netbuf_ptr->native_orig = db_ptr;       	
        
        if(A_OK != Api_DataTxStart(pCxt, (QOSAL_VOID*)a_netbuf_ptr)){
            error = A_ERROR;
            break;
    	} 	    
   	}while(0);
    
    if(error != A_OK){
		/* in the event of api failure, the original pcb should be returned to the caller un-touched
		 * and the a_netbuf_ptr should be freed */
		if(error != A_NO_MEMORY) 
			A_NETBUF_FREE((QOSAL_VOID*)a_netbuf_ptr);
	}          
    return error;
}

#endif
