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

#ifndef _CUSTOM_STACK_OFFLOAD_H_
#define _CUSTOM_STACK_OFFLOAD_H_

#if ENABLE_STACK_OFFLOAD

#include <atheros_stack_offload.h>	 

#define IS_SOCKET_BLOCKED(index)  \
        isSocketBlocked((index))

/* Headroom definitions*/
#define UDP_HEADROOM 44 
#define TCP_HEADROOM 64
#define UDP6_HEADROOM 64
#define TCP6_HEADROOM 88
#define TCP6_HEADROOM_WITH_NO_OPTION 84

#define TX_PKT_OVERHEAD (TCP6_HEADROOM + sizeof(DRV_BUFFER) + sizeof(QOSAL_UINT32))

#if ZERO_COPY
extern A_NETBUF_QUEUE_T zero_copy_free_queue;
#endif


uint_32 isSocketBlocked(QOSAL_VOID* ctxt);

QOSAL_UINT32 custom_receive_tcpip(QOSAL_VOID *pCxt, QOSAL_VOID *pReq);
void txpkt_free(QOSAL_VOID* buffPtr);
uint_32 custom_send_tcpip
   (
      QOSAL_VOID*  			   pCxt,
         /* [IN] the Ethernet state structure */
      DRV_BUFFER_PTR              db_ptr,
         /* [IN] the packet to send */
      uint_32              size,
         /* [IN] total size of the packet */
      uint_32              frags,
         /* [IN] total fragments in the packet */
      uint_8*              header,   
      uint_32              header_size   
     
   );

QOSAL_VOID custom_free(QOSAL_VOID* buf);
QOSAL_VOID* custom_alloc(QOSAL_UINT32 size);
QOSAL_UINT32 get_total_pkts_buffered();
#if 0
QOSAL_INT32 t_ipconfig(ENET_CONTEXT_STRUCT_PTR enet_ptr, QOSAL_UINT32 mode,QOSAL_UINT32* ipv4_addr, QOSAL_UINT32* subnetMask, QOSAL_UINT32* gateway4);
QOSAL_INT32 t_ip6config(ENET_CONTEXT_STRUCT_PTR enet_ptr, QOSAL_UINT32 mode,IP6_ADDR_T *v6Global,IP6_ADDR_T *v6Local,
		    IP6_ADDR_T *v6DefGw,IP6_ADDR_T *v6GlobalExtd,QOSAL_INT32 *LinkPrefix, QOSAL_INT32 *GlbPrefix, QOSAL_INT32 *DefGwPrefix, QOSAL_INT32 *GlbPrefixExtd);
QOSAL_UINT32 custom_queue_empty(QOSAL_UINT32 index);
#endif
#endif //ENABLE_STACK_OFFLOAD
#endif
