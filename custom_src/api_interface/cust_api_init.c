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
#include <a_osapi.h>
#include <a_types.h>
#include <driver_cxt.h>
#include <common_api.h>
#include <custom_wlan_api.h>
#include <wlan_api.h>

QOSAL_VOID *p_Global_Cxt = NULL;

/*The following pointers store the custom driver context used by QAPIs*/
QOSAL_VOID *p_Driver_Cxt[2];

#if MQX_VERSION == (402)
static uint_32 Custom_Api_Readmii(struct enet_context_struct * p_ctxt, uint_32 val, uint_32 * p_val, uint_32 val2);
static uint_32 Custom_Api_Writemii(struct enet_context_struct * p_ctxt,  uint_32 val1, uint_32 val2 , uint_32 val3);
#else
static bool Custom_Api_Readmii(struct enet_context_struct * p_ctxt, uint_32 val, uint_32 * p_val, uint_32 val2);
static bool Custom_Api_Writemii(struct enet_context_struct * p_ctxt,  uint_32 val1, uint_32 val2 , uint_32 val3);
#endif
static uint_32 Custom_Api_Join (struct enet_context_struct * p_ctxt, struct enet_mcb_struct * p_mcb);
static uint_32 Custom_Api_Rejoin(struct enet_context_struct * p_ctxt);
static uint_32 Custom_Api_Initialize(ENET_CONTEXT_STRUCT_PTR enet_ptr);
static uint_32 Custom_Api_Shutdown(struct enet_context_struct *enet_ptr);
extern uint_32 Custom_Api_Send(ENET_CONTEXT_STRUCT_PTR enet_ptr, PCB_PTR pcb_ptr, uint_32 size, uint_32 frags, uint_32 flags);
extern uint_32 Custom_Api_Mediactl( ENET_CONTEXT_STRUCT_PTR enet_ptr, uint_32 command_id, pointer inout_param);

uint_32 chip_state_ctrl(ENET_CONTEXT_STRUCT_PTR enet_ptr, uint_32 state);

const ENET_MAC_IF_STRUCT ATHEROS_WIFI_IF = {
    Custom_Api_Initialize,
    Custom_Api_Shutdown,
    Custom_Api_Send,
    Custom_Api_Readmii,
    Custom_Api_Writemii,
   #if BSPCFG_ENABLE_ENET_MULTICAST    
    Custom_Api_Join,
    Custom_Api_Rejoin,
    #endif
    Custom_Api_Mediactl   
};

#if MQX_VERSION == (402)
static uint_32 Custom_Api_Readmii(struct enet_context_struct * p_ctxt, uint_32 val, uint_32 * p_val, uint_32 val2)
#else
static bool Custom_Api_Readmii(struct enet_context_struct * p_ctxt, uint_32 val, uint_32 * p_val, uint_32 val2)
#endif
{
	/* NOTHING TO DO HERE */
	UNUSED_ARGUMENT(p_ctxt);
	UNUSED_ARGUMENT(val);
	UNUSED_ARGUMENT(p_val);
	UNUSED_ARGUMENT(val2);
	while(1){};
	return 0;
}
#if MQX_VERSION == (402)
static uint_32 Custom_Api_Writemii(struct enet_context_struct * p_ctxt,  uint_32 val1, uint_32 val2 , uint_32 val3)
#else
static bool Custom_Api_Writemii(struct enet_context_struct * p_ctxt,  uint_32 val1, uint_32 val2 , uint_32 val3)
#endif
{
	/* NOTHING TO DO HERE */
	UNUSED_ARGUMENT(p_ctxt);
	UNUSED_ARGUMENT(val1);
	UNUSED_ARGUMENT(val2);
	UNUSED_ARGUMENT(val3);
	while(1){};
	return 0;
}

static uint_32 Custom_Api_Join (struct enet_context_struct * p_ctxt, struct enet_mcb_struct * p_mcb)
{
	/* NOTHING TO DO HERE */
	UNUSED_ARGUMENT(p_ctxt);
	UNUSED_ARGUMENT(p_mcb);
	//while(1){};
	return 0;
}

static uint_32 Custom_Api_Rejoin(struct enet_context_struct * p_ctxt)
{
	/* NOTHING TO DO HERE */
	UNUSED_ARGUMENT(p_ctxt);
	//while(1){};
	return 0;
}



uint_32 chip_state_ctrl(ENET_CONTEXT_STRUCT_PTR enet_ptr, uint_32 state)
{
	uint_32 error = ENET_OK;
	
	if(state == 0){
		/* confirm that the driver is not already down */
		if(enet_ptr->MAC_CONTEXT_PTR != NULL)
		{
			/* shutdown chip and driver */
			error = Custom_Api_Shutdown(enet_ptr);
		}
	}else{
		/* confirm that the driver is not already up */
		if(enet_ptr->MAC_CONTEXT_PTR == NULL)
		{
			/* bring up chip and driver */
			error = Custom_Api_Initialize(enet_ptr);
		}
	}
	
	return error;
}

/*****************************************************************************/
/*  Custom_Api_Initialize - Entry point for MQX to initialize the Driver.
 *      ENET_CONTEXT_STRUCT_PTR enet_ptr - pointer to MQX ethernet object.
 * RETURNS: ENET_OK on success or ENET_ERROR otherwise. 
 *****************************************************************************/
static uint_32 Custom_Api_Initialize(ENET_CONTEXT_STRUCT_PTR enet_ptr)
{
    uint_32 error = ENET_OK;
    A_STATUS status;    
	    
    /* If it is the second device which is getting initialised, 
     * Fill the pointers and generate the MAC address from the 
     * first device
     */
    if(p_Global_Cxt)
    {
        ENET_CONTEXT_STRUCT_PTR tempenet_ptr = (ENET_CONTEXT_STRUCT_PTR)GET_DRIVER_CXT(p_Global_Cxt)->pUpperCxt[0];
        QOSAL_UINT8 devId;
    
        devId = enet_ptr->PARAM_PTR->ENET_IF->PHY_NUMBER;
        A_ASSERT(devId < WLAN_NUM_OF_DEVICES);
        /* Generating the MAC Address of the second interface using the first interface MAC */
        A_MEMCPY(enet_ptr->ADDRESS, tempenet_ptr->ADDRESS, ATH_MAC_LEN);
        enet_ptr->ADDRESS[0] = (((enet_ptr->ADDRESS[0]) ^ (1 << devId)) | 0x2);
        //enet_ptr->ADDRESS[0] = ((enet_ptr->ADDRESS[0])+ (devId << 4)) | 0x2;
        
        enet_ptr->MAC_CONTEXT_PTR = (pointer)p_Global_Cxt;
        GET_DRIVER_CXT(p_Global_Cxt)->pUpperCxt[devId] = enet_ptr;
        p_Driver_Cxt[1] = enet_ptr;
        return error;
    }

    do{        	               
        /* allocate the driver context and assign it to the enet_ptr mac_param */
        if(NULL == (p_Global_Cxt = (QOSAL_VOID*)A_MALLOC(sizeof(A_CUSTOM_DRIVER_CONTEXT), MALLOC_ID_CONTEXT))){
            error = ENET_ERROR;
            break;
        }

        A_MEMZERO(p_Global_Cxt, sizeof(A_CUSTOM_DRIVER_CONTEXT));
        /* alloc the driver common context */
        if(NULL == (GET_DRIVER_CXT(p_Global_Cxt)->pCommonCxt = A_MALLOC(sizeof(A_DRIVER_CONTEXT), MALLOC_ID_CONTEXT))){
            error = ENET_ERROR;
            break;
        }
                
        enet_ptr->MAC_CONTEXT_PTR = (pointer)p_Global_Cxt;
        /* initialize backwards pointers */
        GET_DRIVER_CXT(p_Global_Cxt)->pUpperCxt[0] = enet_ptr;    
        GET_DRIVER_CXT(p_Global_Cxt)->pDriverParam = enet_ptr->PARAM_PTR->MAC_PARAM;   
        /* create the 2 driver events. */    
     
		/* Api_InitStart() will perform any additional allocations that are done as part of 
		 * the common_driver initialization */
        if(A_OK != (status = Api_InitStart(p_Global_Cxt))){
            error = ENET_ERROR;
            break;
        }
		/* CreateDriverThread is a custom function to create or restart the driver thread.
		 * the bulk of the driver initialization is handled by the driver thread.
		 */
#if DRIVER_CONFIG_MULTI_TASKING	 
        if(A_OK != (status = Driver_CreateThread(p_Global_Cxt))){
        	error = ENET_ERROR;
        	break;
        }         
#else
		Driver_Init(p_Global_Cxt);
#endif        
		/* Api_InitFinish waits for wmi_ready event from chip. */
        if(A_OK != Api_InitFinish(p_Global_Cxt)){
        	error = ENET_ERROR;
        	break;
        }	 

        g_driverState = DRIVER_STATE_RUN;

        Api_WMIInitFinish(p_Global_Cxt);                        
    }while(0);
   
   	if(error != ENET_OK)
        {
   		if(p_Global_Cxt != NULL)
                {   			
   			A_FREE(GET_DRIVER_COMMON(p_Global_Cxt), MALLOC_ID_CONTEXT);
   			A_FREE(p_Global_Cxt, MALLOC_ID_CONTEXT);
   		}
   	}
        else
        {
            p_Driver_Cxt[0] = enet_ptr;
        }
   
#if 0       
if(g_totAlloc){
	A_PRINTF("init alloc: %d\n", g_totAlloc);	
	//for more information one can implement _mem_get_free() to 
	//determine how much free memory exists in the system pool.
}	
#endif
   
	Api_BootProfile(p_Global_Cxt, BOOT_PROFILE_DRIVE_READY);

    return error;
}


/*****************************************************************************/
/*  Custom_Api_Shutdown - Entry point for MQX to shutdown the Driver.
 *      ENET_CONTEXT_STRUCT_PTR enet_ptr - pointer to MQX ethernet object.
 * RETURNS: ENET_OK on success or ENET_ERROR otherwise. 
 *****************************************************************************/
static uint_32 Custom_Api_Shutdown(struct enet_context_struct *enet_ptr)
{	
	QOSAL_VOID *pCxt = enet_ptr->MAC_CONTEXT_PTR;
	
	if(pCxt != NULL){
		Api_DeInitStart(pCxt);
#if DRIVER_CONFIG_MULTI_TASKING
		Driver_DestroyThread(pCxt);		
#else
		Driver_DeInit(pCxt);
#endif		
		Api_DeInitFinish(pCxt);
		
		if(NULL != GET_DRIVER_COMMON(pCxt)){
			A_FREE(GET_DRIVER_COMMON(pCxt), MALLOC_ID_CONTEXT);
		}
   		
   		A_FREE(pCxt, MALLOC_ID_CONTEXT);
   		p_Global_Cxt = NULL;
   		enet_ptr->MAC_CONTEXT_PTR = NULL;
	}
	
	return (uint_32) ENET_OK;
}

/*****************************************************************************/
/*  Custom_Api_GetDriverCxt - return driver context based on device ID
 *      ENET_CONTEXT_STRUCT_PTR enet_ptr - pointer to MQX ethernet object.
 * RETURNS: ENET_OK on success or ENET_ERROR otherwise. 
 *****************************************************************************/
QOSAL_VOID* Custom_Api_GetDriverCxt(QOSAL_UINT8 device_id)
{
     return(p_Driver_Cxt[device_id]);
}