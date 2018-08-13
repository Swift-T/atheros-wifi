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
#include <netbuf.h>
#include "atheros_wifi_api.h"

#define AR6000_DATA_OFFSET    64

QOSAL_UINT8 g_driverState = DRIVER_STATE_INVALID;


#if (!ENABLE_STACK_OFFLOAD)


QOSAL_VOID default_native_free_fn(PCB_PTR pcb_ptr)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)pcb_ptr;
    /* head is guaranteed to always point to the
     * start of the original buffer */
    if(a_netbuf_ptr->head)
        A_FREE(a_netbuf_ptr->head, MALLOC_ID_NETBUFF);
        
    if(a_netbuf_ptr->native_orig)
        a_netbuf_ptr->native_orig->FREE(a_netbuf_ptr->native_orig);
        
    A_FREE(a_netbuf_ptr, MALLOC_ID_NETBUFF_OBJ);    
}


typedef void (*_pcb_free_fn)(PCB_PTR pcb_ptr);

QOSAL_VOID
a_netbuf_init(QOSAL_VOID* buffptr, QOSAL_VOID* freefn, QOSAL_VOID* priv)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)buffptr;
	
	a_netbuf_ptr->native.FREE = (_pcb_free_fn)freefn;
	a_netbuf_ptr->native.PRIVATE = priv;
	a_netbuf_ptr->tail = a_netbuf_ptr->data = (pointer)((QOSAL_UINT32)a_netbuf_ptr->head + AR6000_DATA_OFFSET);
	a_netbuf_ptr->native.FRAG[0].FRAGMENT = a_netbuf_ptr->head;
	a_netbuf_ptr->native.FRAG[0].LENGTH = (QOSAL_UINT32)a_netbuf_ptr->end - (QOSAL_UINT32)a_netbuf_ptr->head;
	
}

QOSAL_VOID *
a_netbuf_alloc(QOSAL_INT32 size)
{
    A_NETBUF* a_netbuf_ptr;
    
    do{    
	    a_netbuf_ptr = A_MALLOC(sizeof(A_NETBUF), MALLOC_ID_NETBUFF_OBJ);
	    
	    if(a_netbuf_ptr == NULL){
	    	break;
	    }
	    A_MEMZERO(a_netbuf_ptr, sizeof(A_NETBUF));
	        
	    a_netbuf_ptr->native.FREE = default_native_free_fn;
	    
	    a_netbuf_ptr->native.FRAG[0].LENGTH = AR6000_DATA_OFFSET + size;
	    a_netbuf_ptr->native.FRAG[0].FRAGMENT = A_MALLOC((QOSAL_INT32)(a_netbuf_ptr->native.FRAG[0].LENGTH), MALLOC_ID_NETBUFF);
	    
	    if(a_netbuf_ptr->native.FRAG[0].FRAGMENT == NULL){
	    	A_FREE(a_netbuf_ptr, MALLOC_ID_NETBUFF_OBJ);
	    	a_netbuf_ptr = NULL;
	    	break;
	    }
	    
	    a_netbuf_ptr->head = a_netbuf_ptr->native.FRAG[0].FRAGMENT;
	    a_netbuf_ptr->end = (pointer)((QOSAL_UINT32)a_netbuf_ptr->native.FRAG[0].FRAGMENT + a_netbuf_ptr->native.FRAG[0].LENGTH);
	    // reserve head room
	    a_netbuf_ptr->tail = a_netbuf_ptr->data = (pointer)((QOSAL_UINT32)a_netbuf_ptr->head + AR6000_DATA_OFFSET);
	    A_MEMZERO(&a_netbuf_ptr->trans, sizeof(A_TRANSPORT_OBJ));     
    }while(0);
    
    return ((QOSAL_VOID *)a_netbuf_ptr);
}

/*
 * Allocate an NETBUF w.o. any encapsulation requirement.
 */
QOSAL_VOID *
a_netbuf_alloc_raw(QOSAL_INT32 size)
{
    A_NETBUF* a_netbuf_ptr=NULL;
    
    do{
	    a_netbuf_ptr = A_MALLOC(sizeof(A_NETBUF), MALLOC_ID_NETBUFF_OBJ);
	    
	    if(a_netbuf_ptr == NULL){
	    	break;
	    }
	    
	    A_MEMZERO(a_netbuf_ptr, sizeof(A_NETBUF));
	        
	    a_netbuf_ptr->native.FREE = default_native_free_fn;
	    a_netbuf_ptr->native.FRAG[0].LENGTH = (QOSAL_UINT32)size;
	    a_netbuf_ptr->native.FRAG[0].FRAGMENT = 
	    	A_MALLOC((QOSAL_INT32)a_netbuf_ptr->native.FRAG[0].LENGTH, MALLOC_ID_NETBUFF);
	    
	    
	    if(a_netbuf_ptr->native.FRAG[0].FRAGMENT == NULL){
	    	A_FREE(a_netbuf_ptr, MALLOC_ID_NETBUFF_OBJ);
	    	a_netbuf_ptr = NULL;
	    	break;
	    }
	    
	    a_netbuf_ptr->head = a_netbuf_ptr->tail = a_netbuf_ptr->data = a_netbuf_ptr->native.FRAG[0].FRAGMENT;
	    a_netbuf_ptr->end = (pointer)((QOSAL_UINT32)a_netbuf_ptr->native.FRAG[0].FRAGMENT + (QOSAL_UINT32)a_netbuf_ptr->native.FRAG[0].LENGTH);
        A_MEMZERO(&a_netbuf_ptr->trans, sizeof(A_TRANSPORT_OBJ)); 
    }while(0);
    
    return ((QOSAL_VOID *)a_netbuf_ptr);
}

QOSAL_VOID
a_netbuf_free(QOSAL_VOID* buffptr)
{
    ((A_NATIVE_ORIG*)buffptr)->FREE(((A_NATIVE_ORIG*)buffptr));
}

/* returns the length in bytes of the entire packet.  The packet may 
 * be composed of multiple fragments or it may be just a single buffer.
 */
QOSAL_UINT32
a_netbuf_to_len(QOSAL_VOID *bufPtr)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    QOSAL_UINT32 len = (QOSAL_UINT32)a_netbuf_ptr->tail - (QOSAL_UINT32)a_netbuf_ptr->data;
    QOSAL_UINT16 i = 0;
    
    if(a_netbuf_ptr->num_frags) {
    	if(a_netbuf_ptr->native_orig != NULL){ 
	    	/* count up fragments */
	    	for(i=0 ; i<a_netbuf_ptr->num_frags ; i++){
	    		len += a_netbuf_ptr->native_orig->FRAG[i].LENGTH;
	    	}
	    }else{
	    	/* used for special case native's from ATH_TX_RAW */
	    	for(i=1 ; i<=a_netbuf_ptr->num_frags ; i++){
	    		len += a_netbuf_ptr->native.FRAG[i].LENGTH;	
	    	}
	    }	
    }
    
    return len;
}

/* returns a buffer fragment of a packet.  If the packet is not 
 * fragmented only index == 0 will return a buffer which will be 
 * the whole packet. pLen will hold the length of the buffer in 
 * bytes.
 */
QOSAL_VOID*
a_netbuf_get_fragment(QOSAL_VOID *bufPtr, QOSAL_UINT8 index, QOSAL_INT32 *pLen)
{
	void* pBuf = NULL;
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
	
	if(0==index){
		pBuf = a_netbuf_to_data(bufPtr);
		*pLen = (QOSAL_INT32)((QOSAL_UINT32)a_netbuf_ptr->tail - (QOSAL_UINT32)a_netbuf_ptr->data);
	}else if(a_netbuf_ptr->num_frags >= index) {
		if(a_netbuf_ptr->native_orig){
			/* additional fragments are held in native_orig. this is only
			 * used for tx packets received from the TCP stack. */
			pBuf = a_netbuf_ptr->native_orig->FRAG[index-1].FRAGMENT;
			*pLen = (QOSAL_INT32)a_netbuf_ptr->native_orig->FRAG[index-1].LENGTH;	
		}else{
			/* used for special case native's from ATH_TX_RAW */
			pBuf = a_netbuf_ptr->native.FRAG[index].FRAGMENT;
			*pLen = (QOSAL_INT32)a_netbuf_ptr->native.FRAG[index].LENGTH;
		}	
	}
	
	return pBuf;	
}

/* fills the spare bufFragment position for the net buffer */
QOSAL_VOID
a_netbuf_append_fragment(QOSAL_VOID *bufPtr, QOSAL_UINT8 *frag, QOSAL_INT32 len)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    /* the first bufFragment is used to hold the built-in buffer. subsequent
     * entries if any can be used to append additional buffers. hence 
     * deduct 1 from MAX_BUF_FRAGS */
    if(a_netbuf_ptr->num_frags < MAX_BUF_FRAGS-1){
        a_netbuf_ptr->num_frags++;
        a_netbuf_ptr->native.FRAG[a_netbuf_ptr->num_frags].FRAGMENT = frag;
        a_netbuf_ptr->native.FRAG[a_netbuf_ptr->num_frags].LENGTH = len;        
    }else{
        A_ASSERT(0);
    }   
}





#else
#include "common_stack_offload.h"
#include "custom_stack_offload.h"


QOSAL_VOID a_netbuf_set_tx_pool(QOSAL_VOID* buffptr);
QOSAL_VOID a_netbuf_set_rx_pool(QOSAL_VOID* buffptr);
QOSAL_VOID a_netbuf_free_tx_pool(QOSAL_VOID* buffptr);
QOSAL_VOID a_netbuf_free_rx_pool(QOSAL_VOID* buffptr);
QOSAL_VOID Driver_ReportRxBuffStatus(QOSAL_VOID *pCxt, QOSAL_BOOL status);


QOSAL_VOID default_native_free_fn(QOSAL_VOID* pcb_ptr)
{

    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)pcb_ptr;
    /* head is guaranteed to always point to the
     * start of the original buffer */
    if(a_netbuf_ptr->head){
        A_FREE(a_netbuf_ptr->head, MALLOC_ID_NETBUFF);
    }
    if(a_netbuf_ptr->native_orig){
        txpkt_free(a_netbuf_ptr->native_orig);
    }
    A_FREE(a_netbuf_ptr, MALLOC_ID_NETBUFF_OBJ);    
    
}

QOSAL_VOID* a_netbuf_dequeue_adv(A_NETBUF_QUEUE_T *q, QOSAL_VOID *pkt)
{        
    A_NETBUF* curr, *prev;
    
    if(q->head == NULL) return (QOSAL_VOID*)NULL;
    
    curr = (A_NETBUF*)q->head;
    
    while(curr != NULL)
    {
    	if((A_NETBUF*)curr->data == pkt)
    	{
    		/*Match found*/
    		if(curr == (A_NETBUF*)q->head)
    		{
    			/*Match found at head*/
    			q->head = curr->queueLink;
    			break;
    		}
    		else
    		{
    			/*Match found at intermediate node*/
    			prev->queueLink = curr->queueLink;
    			if(q->tail == curr)
    			{
    				/*Last node*/
    				q->tail = prev;
    			}
    			break;   			
    		}
    	}    	
    	prev = curr;
    	curr = curr->queueLink;     	
    }

	if(curr != NULL)
	{
		q->count--;
    	curr->queueLink = NULL;    	
	}
	return (QOSAL_VOID*)curr;    
}




QOSAL_VOID a_netbuf_purge_queue(QOSAL_UINT32 index)
{
	SOCKET_CONTEXT_PTR pcustctxt;
	A_NETBUF_QUEUE_T *q;
	A_NETBUF* temp;
	
	pcustctxt = GET_SOCKET_CONTEXT(ath_sock_context[index]);
	
	q = &pcustctxt->rxqueue;
	
	while(q->count)
	{
		temp = q->head;
		q->head = A_GET_QUEUE_LINK(temp); 
		A_CLEAR_QUEUE_LINK(temp);
		A_NETBUF_FREE(temp);
		q->count--;	
	}
	q->head = q->tail = NULL;
}



QOSAL_VOID
a_netbuf_init(QOSAL_VOID* buffptr, QOSAL_VOID* freefn, QOSAL_VOID* priv)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)buffptr;
	
//	a_netbuf_ptr->native.FREE = (_pcb_free_fn)freefn;
	a_netbuf_ptr->native.context = priv;
	a_netbuf_ptr->tail = a_netbuf_ptr->data = (pointer)((QOSAL_UINT32)a_netbuf_ptr->head + AR6000_DATA_OFFSET);
	a_netbuf_ptr->native.bufFragment[0].payload = a_netbuf_ptr->head;
	a_netbuf_ptr->native.bufFragment[0].length = (QOSAL_UINT32)a_netbuf_ptr->end - (QOSAL_UINT32)a_netbuf_ptr->head;
#if DRIVER_CONFIG_IMPLEMENT_RX_FREE_QUEUE 
	a_netbuf_ptr->rxPoolID = 1;
#endif	
}

QOSAL_VOID *
a_netbuf_alloc(QOSAL_INT32 size)
{
    A_NETBUF* a_netbuf_ptr;
    
    do{    
	    a_netbuf_ptr = A_MALLOC(sizeof(A_NETBUF), MALLOC_ID_NETBUFF_OBJ);
	    
	    if(a_netbuf_ptr == NULL){
	    	break;
	    }
	    A_MEMZERO(a_netbuf_ptr, sizeof(A_NETBUF));	       
	    
	    a_netbuf_ptr->native.bufFragment[0].length = AR6000_DATA_OFFSET + size;
	    a_netbuf_ptr->native.bufFragment[0].payload = A_MALLOC((QOSAL_INT32)(a_netbuf_ptr->native.bufFragment[0].length), MALLOC_ID_NETBUFF);
	    
	    if(a_netbuf_ptr->native.bufFragment[0].payload == NULL){
	    	A_FREE(a_netbuf_ptr, MALLOC_ID_NETBUFF_OBJ);
	    	a_netbuf_ptr = NULL;
	    	break;
	    }
	    
	    a_netbuf_ptr->head = a_netbuf_ptr->native.bufFragment[0].payload;
	    a_netbuf_ptr->end = (pointer)((QOSAL_UINT32)a_netbuf_ptr->native.bufFragment[0].payload + a_netbuf_ptr->native.bufFragment[0].length);
	    // reserve head room
	    a_netbuf_ptr->tail = a_netbuf_ptr->data = (pointer)((QOSAL_UINT32)a_netbuf_ptr->head + AR6000_DATA_OFFSET);
	    A_MEMZERO(&a_netbuf_ptr->trans, sizeof(A_TRANSPORT_OBJ));
	    /*Init pool IDs*/
	    a_netbuf_ptr->txPoolID = 0;
	    a_netbuf_ptr->rxPoolID = 0;     
    }while(0);
    
    return ((QOSAL_VOID *)a_netbuf_ptr);
}



/*****************************************************************************/
/*  a_netbuf_reinit - called from custom free after TX over SPI is completed.
            It is only called in "Blocking TX" mode where we reuse the same 
            netbuf. 
            Tail, data pointers within netbuf may have moved during previous TX, 
            reinit the netbuf so it can be used again. No allocation is done here.

 * RETURNS: reinitialized netbuf pointer for success and NULL for failure 
 *****************************************************************************/
QOSAL_VOID* a_netbuf_reinit(QOSAL_VOID* netbuf_ptr, QOSAL_INT32 size)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)(netbuf_ptr);
    
    do{    	    
	    a_netbuf_ptr->native.bufFragment[0].length = AR6000_DATA_OFFSET + size;
	   	    
	    a_netbuf_ptr->head = a_netbuf_ptr->native.bufFragment[0].payload;
	    a_netbuf_ptr->end = (pointer)((QOSAL_UINT32)a_netbuf_ptr->native.bufFragment[0].payload + a_netbuf_ptr->native.bufFragment[0].length);
	    // reserve head room
	    a_netbuf_ptr->tail = a_netbuf_ptr->data = (pointer)((QOSAL_UINT32)a_netbuf_ptr->head + AR6000_DATA_OFFSET);
	    A_MEMZERO(&a_netbuf_ptr->trans, sizeof(A_TRANSPORT_OBJ));
	    /*Init pool IDs*/
	    a_netbuf_ptr->txPoolID = 0;
	    a_netbuf_ptr->rxPoolID = 0;     
    }while(0);
    
    return ((QOSAL_VOID *)a_netbuf_ptr);
}


QOSAL_VOID a_netbuf_set_rx_pool(QOSAL_VOID* buffptr)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)buffptr;
	a_netbuf_ptr->rxPoolID = 1;
}


QOSAL_VOID a_netbuf_set_tx_pool(QOSAL_VOID* buffptr)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)buffptr;
	a_netbuf_ptr->txPoolID = 1;
}

/*
 * Allocate an NETBUF w.o. any encapsulation requirement.
 */
QOSAL_VOID *
a_netbuf_alloc_raw(QOSAL_INT32 size)
{
    A_NETBUF* a_netbuf_ptr=NULL;
    
    do{
	    a_netbuf_ptr = A_MALLOC(sizeof(A_NETBUF), MALLOC_ID_NETBUFF_OBJ);
	    
	    if(a_netbuf_ptr == NULL){
	    	break;
	    }
	    
	    A_MEMZERO(a_netbuf_ptr, sizeof(A_NETBUF));
	        
	   // a_netbuf_ptr->native.FREE = default_native_free_fn;
	    a_netbuf_ptr->native.bufFragment[0].length = (QOSAL_UINT32)size;
	    a_netbuf_ptr->native.bufFragment[0].payload= 
	    	A_MALLOC((QOSAL_INT32)a_netbuf_ptr->native.bufFragment[0].length, MALLOC_ID_NETBUFF);
	    
	    
	    if(a_netbuf_ptr->native.bufFragment[0].payload == NULL){
	    	A_FREE(a_netbuf_ptr, MALLOC_ID_NETBUFF_OBJ);
	    	a_netbuf_ptr = NULL;
	    	break;
	    }
	    
	    a_netbuf_ptr->head = a_netbuf_ptr->tail = a_netbuf_ptr->data = a_netbuf_ptr->native.bufFragment[0].payload;
	    a_netbuf_ptr->end = (pointer)((QOSAL_UINT32)a_netbuf_ptr->native.bufFragment[0].payload + (QOSAL_UINT32)a_netbuf_ptr->native.bufFragment[0].length);
        A_MEMZERO(&a_netbuf_ptr->trans, sizeof(A_TRANSPORT_OBJ)); 
    }while(0);
    
    return ((QOSAL_VOID *)a_netbuf_ptr);
}

QOSAL_VOID
a_netbuf_free(QOSAL_VOID* buffptr)
{
   A_NETBUF* a_netbuf_ptr = (A_NETBUF*)buffptr;
   if(a_netbuf_ptr->rxPoolID != 0)
   {
   		a_netbuf_free_rx_pool(buffptr);
   		return;
   }
   else if(a_netbuf_ptr->txPoolID != 0)
   {
   		a_netbuf_free_tx_pool(buffptr);
   		return;
   }
   else
   {
     if(a_netbuf_ptr->native_orig){
          /*This was a TX packet, do not free netbuf here as it may be reused*/
	  txpkt_free(a_netbuf_ptr);
     }
     else{ 
     /*For all other packets, free the netbuf*/  
         if(a_netbuf_ptr->head){
              A_FREE(a_netbuf_ptr->head, MALLOC_ID_NETBUFF);
         }
     
	 A_FREE(a_netbuf_ptr, MALLOC_ID_NETBUFF_OBJ);    
     }
   }  
}

QOSAL_VOID
a_netbuf_free_rx_pool(QOSAL_VOID* buffptr)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)buffptr;
	A_CUSTOM_DRIVER_CONTEXT *pCxt = (A_CUSTOM_DRIVER_CONTEXT *)a_netbuf_ptr->native.context;
    
    RXBUFFER_ACCESS_ACQUIRE((QOSAL_VOID*)pCxt);	    
    //A_NETBUF_ENQUEUE(&(pCxt->rxFreeQueue), a_netbuf_ptr);
    A_NETBUF_ENQUEUE(&(GET_DRIVER_COMMON(pCxt)->rxFreeQueue), a_netbuf_ptr);
    Driver_ReportRxBuffStatus((QOSAL_VOID*)pCxt, A_TRUE);	    
    RXBUFFER_ACCESS_RELEASE((QOSAL_VOID*)pCxt);

}

QOSAL_VOID
a_netbuf_free_tx_pool(QOSAL_VOID* buffptr)
{
	
}
/* returns the length in bytes of the entire packet.  The packet may 
 * be composed of multiple fragments or it may be just a single buffer.
 */
QOSAL_UINT32
a_netbuf_to_len(QOSAL_VOID *bufPtr)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    QOSAL_UINT32 len = (QOSAL_UINT32)a_netbuf_ptr->tail - (QOSAL_UINT32)a_netbuf_ptr->data;
    QOSAL_UINT16 i = 0;
    
    if(a_netbuf_ptr->num_frags) {
    	if(a_netbuf_ptr->native_orig != NULL){ 
	    	/* count up fragments */
	    	for(i=0 ; i<a_netbuf_ptr->num_frags ; i++){
	    		len += a_netbuf_ptr->native_orig->bufFragment[i].length;
	    	}
	    }else{
	    	/* used for special case native's from ATH_TX_RAW */
	    	for(i=1 ; i<=a_netbuf_ptr->num_frags ; i++){
	    		len += a_netbuf_ptr->native.bufFragment[i].length;	
	    	}
	    }	
    }
    
    return len;
}


/* returns a buffer fragment of a packet.  If the packet is not 
 * fragmented only index == 0 will return a buffer which will be 
 * the whole packet. pLen will hold the length of the buffer in 
 * bytes.
 */
QOSAL_VOID*
a_netbuf_get_fragment(QOSAL_VOID *bufPtr, QOSAL_UINT8 index, QOSAL_INT32 *pLen)
{
	void* pBuf = NULL;
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
	
	if(0==index){
		pBuf = a_netbuf_to_data(bufPtr);
		*pLen = (QOSAL_INT32)((QOSAL_UINT32)a_netbuf_ptr->tail - (QOSAL_UINT32)a_netbuf_ptr->data);
	}else if(a_netbuf_ptr->num_frags >= index) {
		if(a_netbuf_ptr->native_orig){
			/* additional fragments are held in native_orig. this is only
			 * used for tx packets received from the TCP stack. */
			pBuf = a_netbuf_ptr->native_orig->bufFragment[index-1].payload;
			*pLen = (QOSAL_INT32)a_netbuf_ptr->native_orig->bufFragment[index-1].length;	
		}else{
			/* used for special case native's from ATH_TX_RAW */
			pBuf = a_netbuf_ptr->native.bufFragment[index].payload;
			*pLen = (QOSAL_INT32)a_netbuf_ptr->native.bufFragment[index].length;
		}	
	}
	
	return pBuf;	
}

/* fills the spare bufFragment position for the net buffer */
QOSAL_VOID
a_netbuf_append_fragment(QOSAL_VOID *bufPtr, QOSAL_UINT8 *frag, QOSAL_INT32 len)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    /* the first bufFragment is used to hold the built-in buffer. subsequent
     * entries if any can be used to append additional buffers. hence 
     * deduct 1 from MAX_BUF_FRAGS */
    if(a_netbuf_ptr->num_frags < MAX_BUF_FRAGS-1){
        a_netbuf_ptr->num_frags++;
        a_netbuf_ptr->native.bufFragment[a_netbuf_ptr->num_frags].payload = frag;
        a_netbuf_ptr->native.bufFragment[a_netbuf_ptr->num_frags].length = len;        
    }else{
        A_ASSERT(0);
    }   
}

#endif /*!ENABLE_STACK_OFFLOAD*/


QOSAL_VOID a_netbuf_enqueue(A_NETBUF_QUEUE_T *q, QOSAL_VOID *pReq)
{
//    QOSAL_UINT8   i;

    qosal_intr_disable(); 
    if (q->head == NULL) {
        q->head = pReq;
    } else {
    	A_ASSIGN_QUEUE_LINK(q->tail, pReq);
        //((A_NETBUF*)q->tail)->queueLink = (A_NETBUF*)pReq;
    }

    q->tail = pReq;
    A_CLEAR_QUEUE_LINK(pReq);
    //((A_NETBUF*)pkt)->queueLink = NULL;
    q->count++;
    qosal_intr_enable();
}

QOSAL_VOID a_netbuf_prequeue(A_NETBUF_QUEUE_T *q, QOSAL_VOID *pReq)
{
    if(q->head == NULL){
        q->tail = pReq;
    }
    A_ASSIGN_QUEUE_LINK(pReq, q->head);
    //((A_NETBUF*)pkt)->queueLink = q->head;
    q->head = pReq;
    q->count++;
}

QOSAL_VOID *a_netbuf_dequeue(A_NETBUF_QUEUE_T *q)
{
    QOSAL_VOID* pReq;
    

    if(q->head == NULL) return (QOSAL_VOID*)NULL;
    qosal_intr_disable();
    pReq = q->head;

    if(q->tail == q->head){
        q->tail = q->head = NULL;
    }else{
    	q->head = A_GET_QUEUE_LINK(pReq);
        //q->head = (QOSAL_VOID*)(curr->queueLink);
    }

    q->count--;
    A_CLEAR_QUEUE_LINK(pReq);
    //curr->queueLink = NULL;
    qosal_intr_enable();
    return (QOSAL_VOID*)pReq;
}


QOSAL_VOID a_netbuf_queue_init(A_NETBUF_QUEUE_T *q)
{
    q->head = q->tail = NULL;
    q->count = 0;
}



QOSAL_VOID
a_netbuf_configure(QOSAL_VOID* buffptr, QOSAL_VOID *buffer, QOSAL_UINT16 headroom, QOSAL_UINT16 length, QOSAL_UINT16 size)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)buffptr;
	
	A_MEMZERO(a_netbuf_ptr, sizeof(A_NETBUF));
	
	if(buffer != NULL){
	    a_netbuf_ptr->head = buffer;
        a_netbuf_ptr->data = &(((QOSAL_UINT8*)buffer)[headroom]);
	    a_netbuf_ptr->tail = &(((QOSAL_UINT8*)buffer)[headroom+length]);
        a_netbuf_ptr->end = &(((QOSAL_UINT8*)buffer)[size]);
    }
}




QOSAL_VOID *
a_netbuf_to_data(QOSAL_VOID *bufPtr)
{
    return (((A_NETBUF*)bufPtr)->data);
}

/*
 * Add len # of bytes to the beginning of the network buffer
 * pointed to by bufPtr
 */
A_STATUS
a_netbuf_push(QOSAL_VOID *bufPtr, QOSAL_INT32 len)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    
    if((QOSAL_UINT32)a_netbuf_ptr->data - (QOSAL_UINT32)a_netbuf_ptr->head < len) 
    {
    	A_ASSERT(0);
    }
    
    a_netbuf_ptr->data = (pointer)(((QOSAL_UINT32)a_netbuf_ptr->data) - len);    

    return A_OK;
}

/*
 * Add len # of bytes to the beginning of the network buffer
 * pointed to by bufPtr and also fill with data
 */
A_STATUS
a_netbuf_push_data(QOSAL_VOID *bufPtr, QOSAL_UINT8 *srcPtr, QOSAL_INT32 len)
{
    a_netbuf_push(bufPtr, len);
    A_MEMCPY(((A_NETBUF*)bufPtr)->data, srcPtr, (QOSAL_UINT32)len);

    return A_OK;
}

/*
 * Add len # of bytes to the end of the network buffer
 * pointed to by bufPtr
 */
A_STATUS
a_netbuf_put(QOSAL_VOID *bufPtr, QOSAL_INT32 len)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    
    if((QOSAL_UINT32)a_netbuf_ptr->end - (QOSAL_UINT32)a_netbuf_ptr->tail < len) {
    	A_ASSERT(0);
    }
    
    a_netbuf_ptr->tail = (pointer)(((QOSAL_UINT32)a_netbuf_ptr->tail) + len);
    
    return A_OK;
}

/*
 * Add len # of bytes to the end of the network buffer
 * pointed to by bufPtr and also fill with data
 */
A_STATUS
a_netbuf_put_data(QOSAL_VOID *bufPtr, QOSAL_UINT8 *srcPtr, QOSAL_INT32 len)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    void *start = a_netbuf_ptr->tail;
    
    a_netbuf_put(bufPtr, len);
    A_MEMCPY(start, srcPtr, (QOSAL_UINT32)len);

    return A_OK;
}





/*
 * Returns the number of bytes available to a a_netbuf_push()
 */
QOSAL_INT32
a_netbuf_headroom(QOSAL_VOID *bufPtr)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    
    return (QOSAL_INT32)((QOSAL_UINT32)a_netbuf_ptr->data - (QOSAL_UINT32)a_netbuf_ptr->head);
}

QOSAL_INT32
a_netbuf_tailroom(QOSAL_VOID *bufPtr)
{
	A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    
    return (QOSAL_INT32)((QOSAL_UINT32)a_netbuf_ptr->end - (QOSAL_UINT32)a_netbuf_ptr->tail);	
}

/*
 * Removes specified number of bytes from the beginning of the buffer
 */
A_STATUS
a_netbuf_pull(QOSAL_VOID *bufPtr, QOSAL_INT32 len)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    
    if((QOSAL_UINT32)a_netbuf_ptr->tail - (QOSAL_UINT32)a_netbuf_ptr->data < len) {
    	A_ASSERT(0);
    }

    a_netbuf_ptr->data = (pointer)((QOSAL_UINT32)a_netbuf_ptr->data + len);

    return A_OK;
}

/*
 * Removes specified number of bytes from the end of the buffer
 */
A_STATUS
a_netbuf_trim(QOSAL_VOID *bufPtr, QOSAL_INT32 len)
{
    A_NETBUF* a_netbuf_ptr = (A_NETBUF*)bufPtr;
    
    if((QOSAL_UINT32)a_netbuf_ptr->tail - (QOSAL_UINT32)a_netbuf_ptr->data < len) {
    	A_ASSERT(0);
    }

    a_netbuf_ptr->tail = (pointer)((QOSAL_UINT32)a_netbuf_ptr->tail - len);

    return A_OK;
}





