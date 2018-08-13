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
#include <driver_cxt.h>
#include <common_api.h>
#include <custom_wlan_api.h>
#include <hif_internal.h>
#include "spi_hcd_if.h"
#include <targaddrs.h>
#include <io_gpio.h>
#include <bsp.h>
#include "AR6002/hw2.0/hw/mbox_host_reg.h"
#include <atheros_wifi.h>
#include <stdlib.h> 
#include "atheros_wifi_api.h"


#define POWER_UP_DELAY (1)

#define HW_SPI_CAPS (HW_SPI_FRAME_WIDTH_8| \
					 HW_SPI_NO_DMA | \
					 HW_SPI_INT_EDGE_DETECT)      



QOSAL_VOID 
Custom_Hcd_EnableDisableSPIIRQ(QOSAL_VOID *pCxt, QOSAL_BOOL enable)
{
	if(enable == A_TRUE){
		ioctl(GET_DRIVER_CXT(pCxt)->customhcdcxt.int_cxt, GPIO_IOCTL_ENABLE_IRQ, NULL);  
	}else{
		ioctl(GET_DRIVER_CXT(pCxt)->customhcdcxt.int_cxt, GPIO_IOCTL_DISABLE_IRQ, NULL);  
	}
}


/*****************************************************************************/
/* Custom_Bus_InOutBuffer - This is the platform specific solution to 
 *  transfer a buffer on the SPI bus.  This solution is always synchronous
 *  regardless of sync param. The function will use the MQX fread and fwrite
 *  as appropriate.
 *      QOSAL_VOID * pCxt - the driver context.
 *      QOSAL_UINT8 *pBuffer - The buffer to transfer.
 *      QOSAL_UINT16 length - the length of the transfer in bytes.
 *      QOSAL_UINT8 doRead - 1 if operation is a read else 0.
 *      QOSAL_BOOL sync - TRUE is synchronous transfer is required else FALSE.
 *****************************************************************************/
A_STATUS 
Custom_Bus_InOutBuffer(QOSAL_VOID *pCxt,
                                    QOSAL_UINT8 *pBuffer,
									QOSAL_UINT16 length, 									
									QOSAL_UINT8 doRead,
                                    QOSAL_BOOL sync)
{
	A_STATUS status = A_OK;    
    
    UNUSED_ARGUMENT(sync);
    /* this function takes advantage of the SPI turbo mode which does not toggle the chip select
     * during the transfer.  Rather the chip select is asserted at the beginning of the transfer
     * and de-asserted at the end of the entire transfer via fflush(). */     
    if(doRead){	    
	    if(length != fread(pBuffer, 1, length, GET_DRIVER_CXT(pCxt)->customhcdcxt.spi_cxt)){
		    status = A_HARDWARE;    	    
	    }	    	
	}else{		
		if(length != fwrite(pBuffer, 1, (QOSAL_INT32)length, GET_DRIVER_CXT(pCxt)->customhcdcxt.spi_cxt)){
			status = A_HARDWARE;        	
		}
	} 
	
    return status;
}				


/*****************************************************************************/
/* Custom_Bus_Start_Transfer - This function is called by common layer prior
 *  to starting a new bus transfer. This solution merely sets up the SPI 
 *  mode as a precaution.
 *      QOSAL_VOID * pCxt - the driver context.     
 *      QOSAL_BOOL sync - TRUE is synchronous transfer is required else FALSE.
 *****************************************************************************/
A_STATUS 
Custom_Bus_StartTransfer(QOSAL_VOID *pCxt, QOSAL_BOOL sync)
{
    QOSAL_UINT32 param;
    A_STATUS status = A_OK;

    UNUSED_ARGUMENT(sync);
    //ensure SPI device is set to proper mode before issuing transfer
    param = SPI_CLK_POL_PHA_MODE3;
    /* set the mode in case the SPI is shared with another device. */
    if (SPI_OK != ioctl(GET_DRIVER_CXT(pCxt)->customhcdcxt.spi_cxt, IO_IOCTL_SPI_SET_MODE, &param)) 
    {        
        status = A_ERROR;        
    }

    return status;
}


/*****************************************************************************/
/* Custom_Bus_Complete_Transfer - This function is called by common layer prior
 *  to completing a bus transfer. This solution calls fflush to de-assert 
 *  the chipselect.
 *      QOSAL_VOID * pCxt - the driver context.     
 *      QOSAL_BOOL sync - TRUE is synchronous transfer is required else FALSE.
 *****************************************************************************/
A_STATUS 
Custom_Bus_CompleteTransfer(QOSAL_VOID *pCxt, QOSAL_BOOL sync)
{
    UNUSED_ARGUMENT(sync);
    fflush(GET_DRIVER_CXT(pCxt)->customhcdcxt.spi_cxt);
    return A_OK;
}



/*****************************************************************************/
/* Custom_Bus_InOut_Token - This is the platform specific solution to 
 *  transfer 4 or less bytes in both directions. The transfer must be  
 *  synchronous. This solution uses the MQX spi ioctl to complete the request.
 *      QOSAL_VOID * pCxt - the driver context.
 *      QOSAL_UINT32 OutToken - the out going data.
 *      QOSAL_UINT8 DataSize - the length in bytes of the transfer.
 *      QOSAL_UINT32 *pInToken - A Buffer to hold the incoming bytes. 
 *****************************************************************************/
A_STATUS 
Custom_Bus_InOutToken(QOSAL_VOID *pCxt,
                           QOSAL_UINT32        OutToken,
                           QOSAL_UINT8         DataSize,
                           QOSAL_UINT32    *pInToken) 
{   
    SPI_READ_WRITE_STRUCT  spi_buf;        
    A_STATUS status = A_OK;    
    
    spi_buf.BUFFER_LENGTH = (QOSAL_UINT32)DataSize; // data size if really a enum that is 1 less than number of bytes
     
    _DCACHE_FLUSH_MBYTES((pointer)&OutToken, sizeof(QOSAL_UINT32));
    
    spi_buf.READ_BUFFER = (char_ptr)pInToken;
    spi_buf.WRITE_BUFFER = (char_ptr)&OutToken;
            
    do{        
        if (A_OK != Custom_Bus_StartTransfer(pCxt, A_TRUE))
        {        
            status = A_HARDWARE;
            break;
        }
        //IO_IOCTL_SPI_READ_WRITE requires read && write buffers        
        if (SPI_OK != ioctl(GET_DRIVER_CXT(pCxt)->customhcdcxt.spi_cxt, IO_IOCTL_SPI_READ_WRITE, &spi_buf)) 
        {        
            status = A_HARDWARE;
            break;
        }
        //waits until interrupt based operation completes            
        Custom_Bus_CompleteTransfer(pCxt, A_TRUE);
        _DCACHE_INVALIDATE_MBYTES((pointer)pInToken, sizeof(QOSAL_UINT32));
    }while(0);
     	
    return status;                                                                        
}

A_STATUS 
Custom_HW_Init(QOSAL_VOID *pCxt)
{
    GPIO_PIN_STRUCT 	pins[2];
    A_CUSTOM_DRIVER_CONTEXT *pCustCxt = GET_DRIVER_CXT(pCxt);
    QOSAL_UINT32 baudrate, endian, input;
    ATHEROS_PARAM_WIFI_STRUCT *pParam;
    A_DRIVER_CONTEXT *pDCxt = GET_DRIVER_COMMON(pCxt);

            
#ifdef ATH_SPI_DMA
    {
        extern int _bsp_dspi_dma_setup(void);
        static int bsp_dspi_dma_ready = 0;
        if (!bsp_dspi_dma_ready)
        {
            _bsp_dspi_dma_setup();
            bsp_dspi_dma_ready = 1;
        }
    }
#endif
    pParam = GET_DRIVER_CXT(pCxt)->pDriverParam;
    
    /* If it is the second device which is getting initialised, 
     * Just return. The SPI interface has already opened by the first interface
     */
    if(pCustCxt->customhcdcxt.spi_cxt)
        return A_OK;
    
//    pCustCxt->spi_cxt = fopen(pParam->SPI_DEVICE, (pointer)SPI_FLAG_FULL_DUPLEX);
    pCustCxt->customhcdcxt.spi_cxt = fopen(pParam->SPI_DEVICE, (pointer)SPI_FLAG_HALF_DUPLEX);
        
    if(pCustCxt->customhcdcxt.spi_cxt == NULL){
        A_ASSERT(0);
    }

    ioctl(pCustCxt->customhcdcxt.spi_cxt, IO_IOCTL_SPI_GET_BAUD, &baudrate);
  
#define MAX_ALLOWED_BAUD_RATE 25000000
    
	if(baudrate > MAX_ALLOWED_BAUD_RATE){
		baudrate = MAX_ALLOWED_BAUD_RATE;
		ioctl(pCustCxt->customhcdcxt.spi_cxt, IO_IOCTL_SPI_SET_BAUD, &baudrate);
	}		
	
    ioctl(pCustCxt->customhcdcxt.spi_cxt, IO_IOCTL_SPI_GET_ENDIAN, &endian);
#if PSP_MQX_CPU_IS_KINETIS
	input = 1; //SPI_PUSHR_PCS(1 << 0); 
#elif PSP_MQX_CPU_IS_MCF52
    input = MCF5XXX_QSPI_QDR_QSPI_CS0; 
#else
#error Atheros wifi: unsupported Freescale chip     
#endif    
    if (SPI_OK != ioctl (pCustCxt->customhcdcxt.spi_cxt, IO_IOCTL_SPI_SET_CS, &input)) 
    {        
        A_ASSERT(0);
    }
    //SPI_DEVICE_LITTLE_ENDIAN or SPI_DEVICE_BIG_ENDIAN
    //IO_IOCTL_SPI_SET_CS
    /* fill the pins structure */
    pins[0] = pParam->INT_PIN;
    pins[1] = GPIO_LIST_END;        

    pCustCxt->customhcdcxt.int_cxt = fopen(pParam->GPIO_DEVICE, (char_ptr)&pins);

    if(NULL == pCustCxt->customhcdcxt.int_cxt){            
        A_ASSERT(0);
    }
        
    //if (IO_OK != ioctl(pCustCxt->customhcdcxt.int_cxt, GPIO_IOCTL_SET_IRQ_FUNCTION, (pointer)&Custom_HW_RegisterInterruptHandler )){        
        if (IO_OK != ioctl(pCustCxt->customhcdcxt.int_cxt, GPIO_IOCTL_SET_IRQ_FUNCTION, (pointer)&Custom_HW_InterruptHandler )){        
        A_ASSERT(0);
    }

#if WLAN_CONFIG_ENABLE_CHIP_PWD_GPIO
/* use gpio pin to reset the wifi chip as needed.
 * this service is required if during operation it
 * is desired to turn off the wifi chip. 
 */
	pins[0] = pParam->PWD_PIN;
    pins[1] = GPIO_LIST_END;     
	pCustCxt->customhcdcxt.pwd_cxt = fopen(pParam->PWD_DEVICE, (char_ptr)&pins);
	
	if(NULL == pCustCxt->customhcdcxt.pwd_cxt){            
          A_ASSERT(0);
        }
#endif        
        
    /* IT is necessary for the custom code to populate the following parameters
     * so that the common code knows what kind of hardware it is working with. */        
    pDCxt->spi_hcd.OperationalClock = baudrate;
    pDCxt->spi_hcd.PowerUpDelay = POWER_UP_DELAY;            
    pDCxt->spi_hcd.SpiHWCapabilitiesFlags = HW_SPI_CAPS;
    pDCxt->spi_hcd.MiscFlags |= MISC_FLAG_RESET_SPI_IF_SHUTDOWN;   
  
    return A_OK;
}

A_STATUS 
Custom_HW_DeInit(QOSAL_VOID *pCxt)
{
#if WLAN_CONFIG_ENABLE_CHIP_PWD_GPIO
	/* NOTE: if the gpio fails to retain its state after fclose
	 * then the wifi device will not remain in the off State.
	 */	
    fclose(GET_DRIVER_CXT(pCxt)->customhcdcxt.pwd_cxt);
    GET_DRIVER_CXT(pCxt)->customhcdcxt.pwd_cxt = NULL;
#endif       
	/* clean up resources initialized in Probe() */
    fclose(GET_DRIVER_CXT(pCxt)->customhcdcxt.spi_cxt);
    GET_DRIVER_CXT(pCxt)->customhcdcxt.spi_cxt = NULL;
    fclose(GET_DRIVER_CXT(pCxt)->customhcdcxt.int_cxt);
    GET_DRIVER_CXT(pCxt)->customhcdcxt.int_cxt = NULL;
    
#if BSPCFG_ENABLE_ADC_SENSOR
    GET_DRIVER_CXT(pCxt)->adc_cxt = NULL;
#endif     
    
    return A_OK;
}


void Custom_HW_InterruptHandler(CUSTOM_HW_INTR_CB HcdCallback, void *pContext) 
{  
        void *pHcdContext ;
	HcdCallback = Hcd_Irq; 
	pHcdContext = pContext;
        (*HcdCallback)(pHcdContext);       
}

QOSAL_VOID 
Custom_HW_PowerUpDown(QOSAL_VOID *pCxt, QOSAL_UINT32 powerUp)
{   
    QOSAL_UINT32 cmd = (powerUp)?  GPIO_IOCTL_WRITE_LOG1: GPIO_IOCTL_WRITE_LOG0;
    MQX_FILE_PTR                f=NULL;    
    if (IO_OK != ioctl (GET_DRIVER_CXT(pCxt)->customhcdcxt.pwd_cxt, cmd, NULL))
	{
		A_ASSERT(0);
	}
    
    {
      cmd = (powerUp)?  GPIO_PIN_STATUS_0: GPIO_PIN_STATUS_1;
      const uint_32 output_set[] = {
          BSP_ATHEROS_WIFI_GPIO_FET_PIN | cmd,
          GPIO_LIST_END
      };

      /* Open and set port TC as output to drive LEDs (LED10 - LED13) */
      f = fopen("gpio:write", (char_ptr) &output_set);
      cmd = (powerUp)?  GPIO_IOCTL_WRITE_LOG0: GPIO_IOCTL_WRITE_LOG1;

     if (f)
          ioctl(f, cmd, NULL);              
    
      fclose(f);      
      f=NULL;
      
      cmd = (powerUp)?  GPIO_PIN_STATUS_0: GPIO_PIN_STATUS_1;
      const uint_32 output1_set[] = {
          BSP_LED1 | cmd,
          GPIO_LIST_END
      };  
      
      f = fopen("gpio:write", (char_ptr) &output1_set);
      cmd = (powerUp)?  GPIO_IOCTL_WRITE_LOG0: GPIO_IOCTL_WRITE_LOG1;

       if (f)
            ioctl(f, cmd, NULL);              

      fclose(f);      
      f=NULL;
    }
#if 0
    {
      if(!powerUp)
      {
          //I2C
          GPIOE_PDOR |= 0x3;
          //A1 input
          GPIO_PDDR_REG(PTA_BASE_PTR) &= 0xfffffffd;
          //B18 output, high
          GPIO_PDDR_REG(PTB_BASE_PTR) |= 0x40000;
          GPIOB_PDOR |= 0x40000;
          //B2,B17
          PORT_PCR_REG(PORTB_BASE_PTR, 2) = PORT_PCR_MUX(0x1) | PORT_PCR_PE_MASK;
          PORT_PCR_REG(PORTB_BASE_PTR, 17) = PORT_PCR_MUX(0x1) | PORT_PCR_PE_MASK;
          GPIOB_PDDR |= 0x20004;
          GPIOB_PDOR &= 0xFFFDFFFB;
          
          //B3,B16
          PORT_PCR_REG(PORTB_BASE_PTR, 3) = PORT_PCR_MUX(0x5) | PORT_PCR_PE_MASK;
          PORT_PCR_REG(PORTB_BASE_PTR, 16) = PORT_PCR_MUX(0x5) | PORT_PCR_PE_MASK;
          //GPIOB_PDDR &= 0xFFFEFFF7;
      }else
      {
          //I2C
          GPIOE_PDOR &= 0xffffffc;
          //A1 input
          GPIO_PDDR_REG(PTA_BASE_PTR) |= 0x2;
          //B18 output, high
          GPIO_PDDR_REG(PTB_BASE_PTR) &= 0xfffbffff;
          GPIOB_PDOR &= 0xfffbffff;
          //B2,B17  UART0
          PORT_PCR_REG(PORTB_BASE_PTR, 2) = PORT_PCR_MUX(0x3) | PORT_PCR_PE_MASK;
          PORT_PCR_REG(PORTB_BASE_PTR, 17) = PORT_PCR_MUX(0x3) | PORT_PCR_PE_MASK;
          GPIOB_PDDR &= ~(0x20004);
          GPIOB_PDOR |= ~(0xFFFDFFFB);
          
          //B3,B16 UART0
          PORT_PCR_REG(PORTB_BASE_PTR, 3) = PORT_PCR_MUX(0x3) | PORT_PCR_PE_MASK;
          PORT_PCR_REG(PORTB_BASE_PTR, 16) = PORT_PCR_MUX(0x3) | PORT_PCR_PE_MASK;
          //GPIOB_PDDR &= 0xFFFEFFF7;
      }
    }
#endif     
}

#if 0   //TBD
QOSAL_VOID Custom_HW_RegisterInterruptHandler(CUSTOM_HW_INTR_CB *callback, QOSAL_VOID *pContext);
QOSAL_VOID Custom_HW_RegisterSuspendHandler(CUSTOM_HW_SUSPEND_CB *suspendCallback, QOSAL_VOID *pSuspContext);
QOSAL_VOID Custom_HW_RegisterResumeHandler(CUSTOM_HW_RESUME_CB *resumeCallback, QOSAL_VOID *pSuspContext);

QOSAL_VOID Custom_HW_RegisterInterruptHandler(CUSTOM_HW_INTR_CB *callback, QOSAL_VOID *pContext)
{
  //TBD
}

QOSAL_VOID Custom_HW_RegisterSuspendHandler(CUSTOM_HW_SUSPEND_CB *suspendCallback, QOSAL_VOID *pSuspContext) 
{
  //TBD
}

QOSAL_VOID Custom_HW_RegisterResumeHandler(CUSTOM_HW_RESUME_CB *resumeCallback, QOSAL_VOID *pSuspContext)
{
  //TBD
}

#endif  //#if 0 TBD