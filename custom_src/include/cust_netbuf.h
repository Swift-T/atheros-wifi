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
#ifndef _CUST_NETBUF_H_
#define _CUST_NETBUF_H_


#include <a_types.h>
#include <a_config.h>
#include <athdefs.h>
#include <netbuf.h>


#if !ENABLE_STACK_OFFLOAD

#include <pcb.h>

#define MAX_BUF_FRAGS 2 /* must match definition of PCB2 */
/* A_NATIVE_NETBUF and A_NATIVE_ORIG are used to point to upper-layer 
 *	netbuf structures. They are OS/TCP stack dependant. */
typedef PCB2 A_NATIVE_NETBUF;
typedef PCB A_NATIVE_ORIG;

/* A_NETBUF represents the data structure for a pReq object (packet)
 * 	throughout the driver. It is part of the custom implementation 
 *	because there may be more optimal ways to achieve the same results
 *	in other OS/systems. It is expected that any of the elements except
 *	for the A_TRANSPORT_OBJ can be replaced by some OS specific construct
 * 	thereby making more efficient use of resources. */
typedef struct _ath_netbuf {
    A_NATIVE_NETBUF native;	// DO NOT MOVE: must be first struct element
    QOSAL_UINT8			num_frags; // number of total fragments representing buffer
    A_NATIVE_ORIG   *native_orig; // the callers ptr passed in send() or NULL
    QOSAL_UINT8     	*head; // marks the start of head space
    QOSAL_UINT8     	*data; // marks the start of data space
    QOSAL_UINT8     	*tail; // marks the start of tail space
    QOSAL_UINT8     	*end;  // marks the end of packet.
    QOSAL_VOID			*queueLink; // used to enqueue packets
    A_TRANSPORT_OBJ  trans;       /* container object for netbuf elements */
} A_NETBUF, *A_NETBUF_PTR; 



QOSAL_VOID default_native_free_fn(PCB_PTR pcb_ptr);

#else



#define MAX_BUF_FRAGS 2 /* must match definition of PCB2 */
/*Fragment structure*/
typedef struct db_frag 
{
	QOSAL_UINT32 length;		//Length in bytes
	uint_8* payload;    //Pointer to data
} DB_FRAG, *DB_FRAG_PTR;

typedef struct drv_buffer 
{
	QOSAL_VOID* context;	//Generic placeholder, can be used to store context
	DB_FRAG bufFragment[1];		//data fragment
}DRV_BUFFER, *DRV_BUFFER_PTR ;

typedef struct min_db
{
    QOSAL_VOID*      context;	
    DB_FRAG      bufFragment[MAX_BUF_FRAGS];
} MIN_DB, *MIN_DB_PTR;

typedef struct tx_packet
{
   DRV_BUFFER db;
   QOSAL_UINT8    hdr[88];
   QOSAL_VOID*    a_netbuf_ptr;
} TX_PACKET, *TX_PACKET_PTR;

/* A_NATIVE_NETBUF and A_NATIVE_ORIG are used to point to upper-layer 
 *	netbuf structures. They are OS/TCP stack dependant. */
typedef MIN_DB A_NATIVE_NETBUF;
typedef DRV_BUFFER A_NATIVE_ORIG;


/* A_NETBUF represents the data structure for a pReq object (packet)
 * 	throughout the driver. It is part of the custom implementation 
 *	because there may be more optimal ways to achieve the same results
 *	in other OS/systems. It is expected that any of the elements except
 *	for the A_TRANSPORT_OBJ can be replaced by some OS specific construct
 * 	thereby making more efficient use of resources. */
typedef struct _ath_netbuf {
    A_NATIVE_NETBUF native;	// DO NOT MOVE: must be first struct element
    QOSAL_UINT8			num_frags; // number of total fragments representing buffer
    A_NATIVE_ORIG   *native_orig; // the callers ptr passed in send() or NULL
    QOSAL_UINT8     	*head; // marks the start of head space
    QOSAL_UINT8     	*data; // marks the start of data space
    QOSAL_UINT8     	*tail; // marks the start of tail space
    QOSAL_UINT8     	*end;  // marks the end of packet.
    QOSAL_UINT8         txPoolID; // If TX pools are used, assign ID here 
    QOSAL_UINT8         rxPoolID; // If rx pools are used, assign ID here
    QOSAL_VOID			*queueLink; // used to enqueue packets
    A_TRANSPORT_OBJ  trans;       /* container object for netbuf elements */
} A_NETBUF, *A_NETBUF_PTR; 



/* A_NETBUF_SET_TX_POOL - Set pool ID to indicate buffer is part of tx pool  */
#define A_NETBUF_SET_TX_POOL(bufPtr) \
    a_netbuf_set_tx_pool(bufPtr) 

/* A_NETBUF_SET_RX_POOL - Set pool ID to indicate buffer is part of RX pool  */
#define A_NETBUF_SET_RX_POOL(bufPtr) \
    a_netbuf_set_rx_pool(bufPtr)   

#define A_NETBUF_DEQUEUE_ADV(q, pkt) \
   a_netbuf_dequeue_adv((q), (pkt))

#define CUSTOM_PURGE_QUEUE(index) \
   a_netbuf_purge_queue((index)) 
 	         


QOSAL_VOID default_native_free_fn(QOSAL_VOID* pcb_ptr);
QOSAL_VOID a_netbuf_set_tx_pool(QOSAL_VOID* buffptr);
QOSAL_VOID a_netbuf_set_rx_pool(QOSAL_VOID* buffptr);
QOSAL_VOID* a_netbuf_dequeue_adv(A_NETBUF_QUEUE_T *q, QOSAL_VOID *pkt);
QOSAL_VOID a_netbuf_purge_queue(QOSAL_UINT32 index);
QOSAL_VOID* a_netbuf_reinit(QOSAL_VOID* netbuf_ptr, QOSAL_INT32 size);


#endif /*!ENABLE_STACK_OFFLOAD*/

/* the A_REQ_... definitions are used in calls to A_NETBUF_GET_ELEM and
 *	A_NETBUF_SET_ELEM.  They identify what element is required. The 
 *	developer can either adopt this implementation or create their
 *	own more suitable definitions as appropriate. */
#define A_REQ_ADDRESS trans.address
#define A_REQ_EPID trans.epid
#define A_REQ_STATUS trans.status
#define A_REQ_CREDITS trans.credits
#define A_REQ_CALLBACK trans.cb
#define A_REQ_COMMAND trans.cmd
#define A_REQ_LOOKAHEAD trans.lookahead
#define A_REQ_TRANSFER_LEN trans.transferLength

#define A_NETBUF_GET_ELEM(_obj, _e) ((A_NETBUF*)(_obj))->##_e
#define A_NETBUF_SET_ELEM(_obj, _e, _v) ((A_NETBUF*)(_obj))->##_e = (_v)

/* A_CLEAR_QUEUE_LINK - used by netbuf queue code to clear the link. */
#define A_CLEAR_QUEUE_LINK(_pReq) ((A_NETBUF*)(_pReq))->queueLink = NULL
/* A_ASSIGN_QUEUE_LINK - used by netbuf queue code to populate/connect the link. */
#define A_ASSIGN_QUEUE_LINK(_pReq, _pN) ((A_NETBUF*)(_pReq))->queueLink = (_pN)
/* A_GET_QUEUE_LINK - used by netbuf queue code to get the link. */
#define A_GET_QUEUE_LINK(_pReq) ((A_NETBUF*)(_pReq))->queueLink
/*
 * Network buffer support
 */

/* A_NETBUF_DECLARE - declare a NETBUF object on the stack */
#define A_NETBUF_DECLARE A_NETBUF
/* A_NETBUF_CONFIGURE - (re-)configure a NETBUF object */
#define A_NETBUF_CONFIGURE(bufPtr, buffer, headroom, length, size) \
	a_netbuf_configure((bufPtr), (buffer), (headroom), (length), (size))
/* A_NETBUF_ALLOC - allocate a NETBUF object from the "heap". */
#define A_NETBUF_ALLOC(size) \
    a_netbuf_alloc(size)
/* A_NETBUF_ALLOC_RAW - allocate a NETBUF object from the "heap". */
#define A_NETBUF_ALLOC_RAW(size) \
    a_netbuf_alloc_raw(size)
/* A_NETBUF_INIT - (re-)initialize a NETBUF object. */    
#define A_NETBUF_INIT(bufPtr, freefn, priv) \
	a_netbuf_init((bufPtr), (freefn), (priv))
/* A_NETBUF_FREE - release a NETBUF object back to the "heap". */	    
#define A_NETBUF_FREE(bufPtr) \
    a_netbuf_free(bufPtr)
/* A_NETBUF_DATA - Get a pointer to the front of valid/populated NETBUF memory. */
#define A_NETBUF_DATA(bufPtr) \
    a_netbuf_to_data(bufPtr)
/* A_NETBUF_LEN - Get the length of valid/populated NETBUF memory (total for all fragments). */
#define A_NETBUF_LEN(bufPtr) \
    a_netbuf_to_len(bufPtr)
/* A_NETBUF_PUSH - Adds #len bytes to the beginning of valid/populated NETBUF memory */
#define A_NETBUF_PUSH(bufPtr, len) \
    a_netbuf_push(bufPtr, len)
/* A_NETBUF_PUT - Adds #len bytes to the end of valid/populated NETBUF memory */
#define A_NETBUF_PUT(bufPtr, len) \
    a_netbuf_put(bufPtr, len)
/* A_NETBUF_TRIM - Removes #len bytes from the end of the valid/populated NETBUF memory */
#define A_NETBUF_TRIM(bufPtr,len) \
    a_netbuf_trim(bufPtr, len)
/* A_NETBUF_PULL - Removes #len bytes from the beginning valid/populated NETBUF memory */
#define A_NETBUF_PULL(bufPtr, len) \
    a_netbuf_pull(bufPtr, len)
/* A_NETBUF_HEADROOM - return the amount of space remaining to prepend content/headers */
#define A_NETBUF_HEADROOM(bufPtr)\
    a_netbuf_headroom(bufPtr)
/* A_NETBUF_TAILROOM - return the amount of space remaining to append content/trailers */
#define A_NETBUF_TAILROOM(bufPtr)\
	a_netbuf_tailroom(bufPtr)    
/* A_NETBUF_GET_FRAGMENT - Get the indexed fragment from the netbuf */
#define A_NETBUF_GET_FRAGMENT(bufPtr, index, pLen) \
	a_netbuf_get_fragment((bufPtr), (index), (pLen))

/* A_NETBUF_APPEND_FRAGMENT - Add a fragment to the end of the netbuf fragment list */
#define A_NETBUF_APPEND_FRAGMENT(bufPtr, frag, len) \
    a_netbuf_append_fragment((bufPtr), (frag), (len))

/* A_NETBUF_PUT_DATA - Add data to end of a buffer  */
#define A_NETBUF_PUT_DATA(bufPtr, srcPtr,  len) \
    a_netbuf_put_data(bufPtr, srcPtr, len) 

/*
 * OS specific network buffer access routines
 */
 
QOSAL_VOID a_netbuf_configure(QOSAL_VOID* buffptr, QOSAL_VOID *buffer, QOSAL_UINT16 headroom, QOSAL_UINT16 length, QOSAL_UINT16 size);
QOSAL_VOID a_netbuf_init(QOSAL_VOID* buffptr, QOSAL_VOID* freefn, QOSAL_VOID* priv);
QOSAL_VOID *a_netbuf_alloc(QOSAL_INT32 size);
QOSAL_VOID *a_netbuf_alloc_raw(QOSAL_INT32 size);
QOSAL_VOID a_netbuf_free(QOSAL_VOID *bufPtr);
QOSAL_VOID *a_netbuf_to_data(QOSAL_VOID *bufPtr);
QOSAL_UINT32 a_netbuf_to_len(QOSAL_VOID *bufPtr);
A_STATUS a_netbuf_push(QOSAL_VOID *bufPtr, QOSAL_INT32 len);
A_STATUS a_netbuf_push_data(QOSAL_VOID *bufPtr, QOSAL_UINT8 *srcPtr, QOSAL_INT32 len);
A_STATUS a_netbuf_put(QOSAL_VOID *bufPtr, QOSAL_INT32 len);
A_STATUS a_netbuf_put_data(QOSAL_VOID *bufPtr, QOSAL_UINT8 *srcPtr, QOSAL_INT32 len);
A_STATUS a_netbuf_pull(QOSAL_VOID *bufPtr, QOSAL_INT32 len);
QOSAL_VOID* a_netbuf_get_fragment(QOSAL_VOID *bufPtr, QOSAL_UINT8 index, QOSAL_INT32 *pLen);
QOSAL_VOID a_netbuf_append_fragment(QOSAL_VOID *bufPtr, QOSAL_UINT8 *frag, QOSAL_INT32 len);
A_STATUS a_netbuf_trim(QOSAL_VOID *bufPtr, QOSAL_INT32 len);
QOSAL_INT32 a_netbuf_headroom(QOSAL_VOID *bufPtr);
QOSAL_INT32 a_netbuf_tailroom(QOSAL_VOID *bufPtr);

#endif /* _CUST_NETBUF_H_ */