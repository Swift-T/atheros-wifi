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
//==============================================================================
// Author(s): ="Atheros"
//==============================================================================

#include <a_config.h>
#include <a_types.h>
#include <a_osapi.h>
#include <driver_cxt.h>
#include <common_api.h>
#include <custom_api.h>
#include <wmi_api.h>
#include <bmi.h>
#include <htc.h>
#include <wmi_host.h>
#if ENABLE_P2P_MODE
#include <wmi.h>
#include "p2p.h"
#endif
#include "mqx.h"
#include "bsp.h"
#include "enet.h"
#include "enetprv.h"
#include "atheros_wifi.h"
#include "enet_wifi.h"
#include "atheros_wifi_api.h"
#include "atheros_wifi_internal.h"
#include "atheros_stack_offload.h"
#include "dset_api.h"
#include "common_stack_offload.h"
#include "qcom_api.h"
#include "qcom_legacy.h"

extern A_VOID *p_Driver_Cxt[];
extern uint_32 Custom_Api_Mediactl( ENET_CONTEXT_STRUCT_PTR enet_ptr, uint_32 command_id, pointer inout_param);
extern uint_32 ath_ioctl_handler(ENET_CONTEXT_STRUCT_PTR enet_ptr, ATH_IOCTL_PARAM_STRUCT_PTR param_ptr);
extern A_STATUS qcom_set_deviceid( A_UINT16 id);


/*-----------------------------------------------------------------------------------------------------------------*/
A_STATUS depr_qcom_set_scan(A_UINT8 device_id)
{
    A_STATUS error = A_OK;
    if(qcom_set_deviceid(device_id) == A_ERROR){
        return A_ERROR;
    }
    if( Custom_Api_Mediactl(p_Driver_Cxt[device_id],ENET_SET_MEDIACTL_SCAN,NULL) != A_OK)
       error = A_ERROR;
  
    return error;
}

A_STATUS depr_qcom_ota_done()
{
    return((A_STATUS)custom_ota_done(p_Driver_Cxt[0], 1));
}



