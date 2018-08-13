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
/**HEADER********************************************************************
*
*
*
* $FileName: dset.h$
* $Version : 3.8.40.0$
* $Date    : May-28-2014$
*
* Comments:
*
*   This file contains the defines, externs and data
*   structure definitions required by application
*   programs in order to use the Ethernet packet driver.
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
*END************************************************************************/
#ifndef __dset_api_h__
#define __dset_api_h__

/*--------------------------------------------------------------------------*/
/*                        
**                            CONSTANT DEFINITIONS
*/

#define			MAX_HOST_DSET_SIZE				8

#define			INVALID_DSET_ID					0

typedef  enum  __DSET_OP  {
		DSET_OP_CREATE  = 1,
		DSET_OP_OPEN  = 2,
		DSET_OP_READ  = 3,
		DSET_OP_WRITE  = 4,
		DSET_OP_COMMIT = 5,
		DSET_OP_CLOSE  = 6,
		DSET_OP_SIZE   = 7,
		DSET_OP_DELETE  = 8,
} DSET_OP;

#define    MAX_DSET_BUFF_SIZE  1500

/*--------------------------------------------------------------------------*/
/*                        
**                            TYPE DEFINITIONS
*/

typedef void  (*dset_callback_fn_t)(uint32_t status, void *pv);

typedef struct host_api_dset_struct {
   QOSAL_UINT32     dset_id;         /* dset id    */
   QOSAL_UINT32     media_id;
   QOSAL_UINT32     flags;
   QOSAL_UINT32     offset;          /* dset offset            */
   QOSAL_UINT32     length;          /* dset length            */
   QOSAL_UINT32     left_len;        /* for large read/write   */
   dset_callback_fn_t  cb;
   void		   *cb_args;
   uint_8      done_flag;
   uint_8      *data_ptr;        /* dset buffer address */
} HOST_DSET_HANDLE;

//typedef struct host_dset_item {
//   QOSAL_UINT32     dset_id;         /* dset id    */
//   QOSAL_UINT32     length;          /* dset length            */
//} HOST_DSET_ITEM;

/*--------------------------------------------------------------------------*/
/*                        
**                            PROTOTYPES AND GLOBAL EXTERNS
*/

extern HOST_DSET_HANDLE   dset_handles[];

HOST_DSET_HANDLE *dset_find_handle(uint32_t dset_id);

QOSAL_UINT32    qcom_dset_create(HOST_DSET_HANDLE **pDsetHandle, uint32_t dset_id, QOSAL_UINT32 media_id, uint32_t length, uint32_t flags, dset_callback_fn_t create_cb, void *callback_arg);
QOSAL_UINT32    qcom_dset_open(HOST_DSET_HANDLE **pDsetHandle, uint32_t dset_id, uint32_t flags, dset_callback_fn_t open_cb, void *callback_arg);
QOSAL_UINT32    qcom_dset_write(HOST_DSET_HANDLE *pDsetHandle, uint8_t *buffer, uint32_t length, uint32_t offset, uint32_t flags, dset_callback_fn_t write_cb, void *callback_arg);
QOSAL_UINT32    qcom_dset_read(HOST_DSET_HANDLE *pDsetHandle, uint8_t *buffer, uint32_t length, uint32_t offset, dset_callback_fn_t  read_cb, void *callback_arg);
QOSAL_UINT32    qcom_dset_commit(HOST_DSET_HANDLE *pDsetHandle, dset_callback_fn_t  read_cb, void *callback_arg);
QOSAL_UINT32    qcom_dset_close(HOST_DSET_HANDLE *pDsetHandle, dset_callback_fn_t  read_cb, void *callback_arg);
QOSAL_UINT32    qcom_dset_size(HOST_DSET_HANDLE *pDsetHandle, dset_callback_fn_t  read_cb, void *callback_arg);
QOSAL_UINT32    qcom_dset_delete(uint32_t dset_id, uint32_t flags, dset_callback_fn_t  read_cb, void *callback_arg);

#endif