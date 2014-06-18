/*
 * Driver: dr_sd.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description:
 * TODO
 */

#include <inttypes.h>

#include "toSort/mmcsd_proto.h"
#include "toSort/hs_mmcsdlib.h"
#include "soc_AM335x.h"
#include "toSort/hs_mmcsd.h"
#include "../edma/edma_event.h"
#include "string.h"
#include "../interrupt/dr_interrupt.h"
#include "../timer/dr_timer.h"
#include "../edma/edma.h"
#include "../console/dr_console.h"
#include "cpu/hw_cpu.h"
#include "thirdParty/fatfs/src/ff.h"
#include "elf/dr_elfloader.h"

/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/

/* Delay */
#define delay TimerDelayDelay

/* Frequency */
#define HSMMCSD_IN_FREQ                96000000 /* 96MHz */
#define HSMMCSD_INIT_FREQ              400000   /* 400kHz */

#define HSMMCSD_CARD_DETECT_PINNUM     6

/* EDMA3 Event queue number. */
#define EVT_QUEUE_NUM                  0

/* EDMA3 Region Number. */
#define REGION_NUMBER                  0

/* Block size config */
#define HSMMCSD_BLK_SIZE               512
#define HSMMCSD_RW_BLK                 1

/* Global data pointers */
#define HSMMCSD_DATA_SIZE              512

/* GPIO instance related macros. */
#define GPIO_INST_BASE                 (SOC_GPIO_0_REGS)

/* MMCSD instance related macros. */
#define MMCSD_INST_BASE                (SOC_MMCHS_0_REGS)
#define MMCSD_INT_NUM                  (SYS_INT_MMCSD0INT)

/* EDMA instance related macros. */
#define EDMA_INST_BASE                 (SOC_EDMA30CC_0_REGS)
#define EDMA_COMPLTN_INT_NUM           (SYS_INT_EDMACOMPINT)
#define EDMA_ERROR_INT_NUM             (SYS_INT_EDMAERRINT)

/* EDMA Events */
#define MMCSD_TX_EDMA_CHAN             (EDMA3_CHA_MMCSD0_TX)
#define MMCSD_RX_EDMA_CHAN             (EDMA3_CHA_MMCSD0_RX)

/* MMU related macros. */
#define START_ADDR_OCMC                 0x40300000
#define START_ADDR_DDR                  0x80000000
#define START_ADDR_DEV                  0x44000000
#define NUM_SECTIONS_DDR                512
#define NUM_SECTIONS_DEV                960
#define NUM_SECTIONS_OCMC               1

/* SD card info structure */
mmcsdCardInfo sdCard;

/* SD Controller info structure */
mmcsdCtrlInfo  ctrlInfo;

/******************************************************************************
**                      FUNCTION PROTOTYPES
*******************************************************************************/
extern void HSMMCSDFsMount(unsigned int driveNum, void *ptr);
extern unsigned int HSMMCSDFsProcessCmdLine(void);
//extern int Cmd_help(int argc, char *argv[]);

/* EDMA callback function array */
static void (*cb_Fxn[EDMA3_NUM_TCC]) (unsigned int tcc, unsigned int status);

/******************************************************************************
**                      VARIABLE DEFINITIONS
*******************************************************************************/
/* Global flags for interrupt handling */
volatile unsigned int sdBlkSize = HSMMCSD_BLK_SIZE;
volatile unsigned int callbackOccured = 0;
volatile unsigned int xferCompFlag = 0;
volatile unsigned int dataTimeout = 0;
volatile unsigned int cmdCompFlag = 0;
volatile unsigned int cmdTimeout = 0;
volatile unsigned int errFlag = 0;


/******************************************************************************
**                          FUNCTION DEFINITIONS
*******************************************************************************/


/*
 * Check command status
 */

static unsigned int HSMMCSDCmdStatusGet(mmcsdCtrlInfo *ctrl)
{
    unsigned int status = 0;

    while ((cmdCompFlag == 0) && (cmdTimeout == 0));

    if (cmdCompFlag)
    {
        status = 1;
        cmdCompFlag = 0;
    }

    if (cmdTimeout)
    {
        status = 0;
        cmdTimeout = 0;
    }

    return status;
}

static unsigned int HSMMCSDXferStatusGet(mmcsdCtrlInfo *ctrl)
{
    unsigned int status = 0;
    volatile unsigned int timeOut = 0xFFFF;

    while ((xferCompFlag == 0) && (dataTimeout == 0));

    if (xferCompFlag)
    {
        status = 1;
        xferCompFlag = 0;
    }

    if (dataTimeout)
    {
        status = 0;
        dataTimeout = 0;
    }

    /* Also, poll for the callback */
    if (HWREG(ctrl->memBase + MMCHS_CMD) & MMCHS_CMD_DP)
    {
        while(callbackOccured == 0 && ((timeOut--) != 0));
        callbackOccured = 0;

        if(timeOut == 0)
        {
            status = 0;
        }
    }

    ctrlInfo.dmaEnable = 0;

    return status;
}

void HSMMCSDRxDmaConfig(void *ptr, unsigned int blkSize, unsigned int nblks)
{
    EDMA3CCPaRAMEntry paramSet;

    paramSet.srcAddr    = ctrlInfo.memBase + MMCHS_DATA;
    paramSet.destAddr   = (unsigned int)ptr;
    paramSet.srcBIdx    = 0;
    paramSet.srcCIdx    = 0;
    paramSet.destBIdx   = 4;
    paramSet.destCIdx   = (unsigned short)blkSize;
    paramSet.aCnt       = 0x4;
    paramSet.bCnt       = (unsigned short)blkSize/4;
    paramSet.cCnt       = (unsigned short)nblks;
    paramSet.bCntReload = 0x0;
    paramSet.linkAddr   = 0xffff;
    paramSet.opt        = 0;

    /* Set OPT */
    paramSet.opt |= ((MMCSD_RX_EDMA_CHAN << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);

    /* 1. Transmission complition interrupt enable */
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* 2. Read FIFO : SRC Constant addr mode */
    paramSet.opt |= (1 << 0);

    /* 3. SRC FIFO width is 32 bit */
    paramSet.opt |= (2 << 8);

    /* 4.  AB-Sync mode */
    paramSet.opt |= (1 << 2);

    /* configure PaRAM Set */
    EDMA3SetPaRAM(EDMA_INST_BASE, MMCSD_RX_EDMA_CHAN, &paramSet);

    /* Enable the transfer */
    EDMA3EnableTransfer(EDMA_INST_BASE, MMCSD_RX_EDMA_CHAN, EDMA3_TRIG_MODE_EVENT);
}

void HSMMCSDTxDmaConfig(void *ptr, unsigned int blkSize, unsigned int blks)
{
    EDMA3CCPaRAMEntry paramSet;

    paramSet.srcAddr    = (unsigned int)ptr;
    paramSet.destAddr   = ctrlInfo.memBase + MMCHS_DATA;
    paramSet.srcBIdx    = 4;
    paramSet.srcCIdx    = blkSize;
    paramSet.destBIdx   = 0;
    paramSet.destCIdx   = 0;
    paramSet.aCnt       = 0x4;
    paramSet.bCnt       = (unsigned short)blkSize/4;
    paramSet.cCnt       = (unsigned short)blks;
    paramSet.bCntReload = 0x0;
    paramSet.linkAddr   = 0xffff;
    paramSet.opt        = 0;

    /* Set OPT */
    paramSet.opt |= ((MMCSD_TX_EDMA_CHAN << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);

    /* 1. Transmission complition interrupt enable */
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* 2. Read FIFO : DST Constant addr mode */
    paramSet.opt |= (1 << 1);

    /* 3. DST FIFO width is 32 bit */
    paramSet.opt |= (2 << 8);

    /* 4.  AB-Sync mode */
    paramSet.opt |= (1 << 2);

    /* configure PaRAM Set */
    EDMA3SetPaRAM(EDMA_INST_BASE, MMCSD_TX_EDMA_CHAN, &paramSet);

    /* Enable the transfer */
    EDMA3EnableTransfer(EDMA_INST_BASE, MMCSD_TX_EDMA_CHAN, EDMA3_TRIG_MODE_EVENT);
}

static void HSMMCSDXferSetup(mmcsdCtrlInfo *ctrl, unsigned char rwFlag, void *ptr,
                             unsigned int blkSize, unsigned int nBlks)
{
    callbackOccured = 0;
    xferCompFlag = 0;

    if (rwFlag == 1)
    {
        HSMMCSDRxDmaConfig(ptr, blkSize, nBlks);
    }
    else
    {
        HSMMCSDTxDmaConfig(ptr, blkSize, nBlks);
    }

    ctrl->dmaEnable = 1;
    HSMMCSDBlkLenSet(ctrl->memBase, blkSize);
}


/*
** This function is used as a callback from EDMA3 Completion Handler.
*/
static void callback(unsigned int tccNum, unsigned int status)
{
    callbackOccured = 1;
    EDMA3DisableTransfer(EDMA_INST_BASE, tccNum, EDMA3_TRIG_MODE_EVENT);
}

static void Edma3CompletionIsr(void)
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int isIPR = 0;

    unsigned int indexl;
    unsigned int Cnt = 0;

    indexl = 1;

    isIPR = EDMA3GetIntrStatus(EDMA_INST_BASE);

    if(isIPR)
    {
        while ((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
        {
            indexl = 0u;
            pendingIrqs = EDMA3GetIntrStatus(EDMA_INST_BASE);

            while (pendingIrqs)
            {
                if((pendingIrqs & 1u) == TRUE)
                {
                    /**
                    * If the user has not given any callback function
                    * while requesting the TCC, its TCC specific bit
                    * in the IPR register will NOT be cleared.
                    */
                    /* here write to ICR to clear the corresponding IPR bits */

                    EDMA3ClrIntr(EDMA_INST_BASE, indexl);

                    if (cb_Fxn[indexl] != NULL)
                    {
                        (*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE);
                    }
                }
                ++indexl;
                pendingIrqs >>= 1u;
            }
            Cnt++;
        }
    }
}

static void Edma3CCErrorIsr(void)
{
    volatile unsigned int pendingIrqs;
    volatile unsigned int evtqueNum = 0;  /* Event Queue Num */
    volatile unsigned int isIPRH = 0;
    volatile unsigned int isIPR = 0;
    volatile unsigned int Cnt = 0u;
    volatile unsigned int index;

    pendingIrqs = 0u;
    index = 1u;

    isIPR  = EDMA3GetIntrStatus(EDMA_INST_BASE);
    isIPRH = EDMA3IntrStatusHighGet(EDMA_INST_BASE);

    if((isIPR | isIPRH ) || (EDMA3QdmaGetErrIntrStatus(EDMA_INST_BASE) != 0)
        || (EDMA3GetCCErrStatus(EDMA_INST_BASE) != 0))
    {
        /* Loop for EDMA3CC_ERR_HANDLER_RETRY_COUNT number of time,
         * breaks when no pending interrupt is found
         */
        while ((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT)
                    && (index != 0u))
        {
            index = 0u;

            if(isIPR)
            {
                   pendingIrqs = EDMA3GetErrIntrStatus(EDMA_INST_BASE);
            }
            else
            {
                   pendingIrqs = EDMA3ErrIntrHighStatusGet(EDMA_INST_BASE);
            }

            while (pendingIrqs)
            {
                   /*Process all the pending interrupts*/
                   if(TRUE == (pendingIrqs & 1u))
                   {
                      /* Write to EMCR to clear the corresponding EMR bits.
                       */
                        /*Clear any SER*/

                        if(isIPR)
                        {
                             EDMA3ClrMissEvt(EDMA_INST_BASE, index);
                        }
                        else
                        {
                             EDMA3ClrMissEvt(EDMA_INST_BASE, index + 32);
                        }
                   }
                   ++index;
                   pendingIrqs >>= 1u;
            }
            index = 0u;
            pendingIrqs = EDMA3QdmaGetErrIntrStatus(EDMA_INST_BASE);
            while (pendingIrqs)
            {
                /*Process all the pending interrupts*/
                if(TRUE == (pendingIrqs & 1u))
                {
                    /* Here write to QEMCR to clear the corresponding QEMR bits*/
                    /*Clear any QSER*/
                    EDMA3QdmaClrMissEvt(EDMA_INST_BASE, index);
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;


            pendingIrqs = EDMA3GetCCErrStatus(EDMA_INST_BASE);
            if (pendingIrqs != 0u)
            {
            /* Process all the pending CC error interrupts. */
            /* Queue threshold error for different event queues.*/
            for (evtqueNum = 0u; evtqueNum < SOC_EDMA3_NUM_EVQUE; evtqueNum++)
                {
                if((pendingIrqs & (1u << evtqueNum)) != 0u)
                {
                        /* Clear the error interrupt. */
                        EDMA3ClrCCErr(EDMA_INST_BASE, (1u << evtqueNum));
                    }
                }

            /* Transfer completion code error. */
            if ((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
            {
                EDMA3ClrCCErr(EDMA_INST_BASE,
                                      (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT));
            }
                ++index;
            }
            Cnt++;
        }
    }

}

static void HSMMCSDIsr(void)
{
    volatile unsigned int status = 0;

    status = HSMMCSDIntrStatusGet(ctrlInfo.memBase, 0xFFFFFFFF);

    HSMMCSDIntrStatusClear(ctrlInfo.memBase, status);

    if (status & HS_MMCSD_STAT_CMDCOMP)
    {
        cmdCompFlag = 1;
    }

    if (status & HS_MMCSD_STAT_ERR)
    {
        errFlag = status & 0xFFFF0000;

        if (status & HS_MMCSD_STAT_CMDTIMEOUT)
        {
            cmdTimeout = 1;
        }

        if (status & HS_MMCSD_STAT_DATATIMEOUT)
        {
            dataTimeout = 1;
        }
    }

    if (status & HS_MMCSD_STAT_TRNFCOMP)
    {
        xferCompFlag = 1;
    }
}


/*
** This function configures the AINTC to receive EDMA3 interrupts.
*/
static void EDMA3AINTCConfigure(void)
{
    /* Initializing the ARM Interrupt Controller. */
	IntControllerInit();

    /* Registering EDMA3 Channel Controller transfer completion interrupt.  */
	IntRegister(EDMA_COMPLTN_INT_NUM, Edma3CompletionIsr);

    /* Setting the priority for EDMA3CC completion interrupt in AINTC. */
   // IntPrioritySet(EDMA_COMPLTN_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Registering EDMA3 Channel Controller Error Interrupt. */
    IntRegister(EDMA_ERROR_INT_NUM, Edma3CCErrorIsr);

    /* Setting the priority for EDMA3CC Error interrupt in AINTC. */
   // IntPrioritySet(EDMA_ERROR_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enabling the EDMA3CC completion interrupt in AINTC. */
    IntHandlerEnable(EDMA_COMPLTN_INT_NUM);

    /* Enabling the EDMA3CC Error interrupt in AINTC. */
    IntHandlerEnable(EDMA_ERROR_INT_NUM);

    /* Registering HSMMC Interrupt handler */
    IntRegister(MMCSD_INT_NUM, HSMMCSDIsr);

    /* Setting the priority for EDMA3CC completion interrupt in AINTC. */
   // IntPrioritySet(MMCSD_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enabling the HSMMC interrupt in AINTC. */
    IntHandlerEnable(MMCSD_INT_NUM);

    /* Enabling IRQ in CPSR of ARM processor. */
    IntMasterIRQEnable();
}


/*
** Powering up, initializing and registering interrupts for EDMA.
*/

static void EDMA3Initialize(void)
{
    /* Initialization of EDMA3 */
    EDMA3Init(EDMA_INST_BASE, EVT_QUEUE_NUM);

    /* Configuring the AINTC to receive EDMA3 interrupts. */
    EDMA3AINTCConfigure();
}

static void HSMMCSDEdmaInit(void)
{
    /* Initializing the EDMA. */
    EDMA3Initialize();

    /* Request DMA Channel and TCC for MMCSD Transmit*/
    EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                        MMCSD_TX_EDMA_CHAN, MMCSD_TX_EDMA_CHAN,
                        EVT_QUEUE_NUM);

    /* Registering Callback Function for TX*/
    cb_Fxn[MMCSD_TX_EDMA_CHAN] = &callback;

    /* Request DMA Channel and TCC for MMCSD Receive */
    EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                        MMCSD_RX_EDMA_CHAN, MMCSD_RX_EDMA_CHAN,
                        EVT_QUEUE_NUM);

    /* Registering Callback Function for RX*/
    cb_Fxn[MMCSD_RX_EDMA_CHAN] = &callback;
}

/*
** Initialize the MMCSD controller structure for use
*/
static void HSMMCSDControllerSetup(void)
{
    ctrlInfo.memBase = MMCSD_INST_BASE;
    ctrlInfo.ctrlInit = HSMMCSDControllerInit;
    ctrlInfo.xferSetup = HSMMCSDXferSetup;
    ctrlInfo.cmdStatusGet = HSMMCSDCmdStatusGet;
    ctrlInfo.xferStatusGet = HSMMCSDXferStatusGet;
    /* Use the funciton HSMMCSDCDPinStatusGet() to use the card presence
       using the controller.
    */
    ctrlInfo.cardPresent = HSMMCSDCardPresent;
    ctrlInfo.cmdSend = HSMMCSDCmdSend;
    ctrlInfo.busWidthConfig = HSMMCSDBusWidthConfig;
    ctrlInfo.busFreqConfig = HSMMCSDBusFreqConfig;
    ctrlInfo.intrMask = (HS_MMCSD_INTR_CMDCOMP | HS_MMCSD_INTR_CMDTIMEOUT |
                            HS_MMCSD_INTR_DATATIMEOUT | HS_MMCSD_INTR_TRNFCOMP);
    ctrlInfo.intrEnable = HSMMCSDIntEnable;
    ctrlInfo.busWidth = (SD_BUS_WIDTH_1BIT | SD_BUS_WIDTH_4BIT);
    ctrlInfo.highspeed = 1;
    ctrlInfo.ocr = (SD_OCR_VDD_3P0_3P1 | SD_OCR_VDD_3P1_3P2);
    ctrlInfo.card = &sdCard;
    ctrlInfo.ipClk = HSMMCSD_IN_FREQ;
    ctrlInfo.opClk = HSMMCSD_INIT_FREQ;
    ctrlInfo.cdPinNum = HSMMCSD_CARD_DETECT_PINNUM;
    sdCard.ctrl = &ctrlInfo;

    callbackOccured = 0;
    xferCompFlag = 0;
    dataTimeout = 0;
    cmdCompFlag = 0;
    cmdTimeout = 0;
}

/**
 * Inits file system and checks for mounted sd
 */
int startFileSystem(void)
{
    volatile unsigned int i = 0;
    volatile unsigned int initFlg = 1;

    /* Initialize console for communication with the Host Machine */
    // ConsoleEnable(SOC_UART_0_REGS);

    /* Configure the EDMA clocks. */
    EDMAModuleClkConfig();

    /* Configure EDMA to service the HSMMCSD events. */
    HSMMCSDEdmaInit();

    /* Perform pin-mux for HSMMCSD pins. */
    HSMMCSDPinMuxSetup();

    /* Enable module clock for HSMMCSD. */
    HSMMCSDModuleClkConfig();

    /* Basic controller initializations */
    HSMMCSDControllerSetup();

    /* Initialize the MMCSD controller */
    MMCSDCtrlInit(&ctrlInfo);

    MMCSDIntEnable(&ctrlInfo);

    //CPUirqe();

    while(1)
    {
        if((HSMMCSDCardPresent(&ctrlInfo)) == 1)
        {
            if(initFlg)
            {
                HSMMCSDFsMount(0, &sdCard);
                initFlg = 0;
            }
            return 1;
        }
        else
        {
            delay(1);

            i = (i + 1) & 0xFFF;

            if(i %20 == 1)
            {
                 printf("FS: Please insert the card \n\r");
            }

            if(initFlg != 1)
            {
                 /* Reinitialize all the state variables */
                 callbackOccured = 0;
                 xferCompFlag = 0;
                 dataTimeout = 0;
                 cmdCompFlag = 0;
                 cmdTimeout = 0;

                 /* Initialize the MMCSD controller */
                 MMCSDCtrlInit(&ctrlInfo);

                 MMCSDIntEnable(&ctrlInfo);
            }

            initFlg = 1;
        }
    }
}

/* Defines size of the buffers that hold temporary data. */
#define DATA_BUF_SIZE   64 * (2 * 512)
static char g_cDataBuf[DATA_BUF_SIZE];

/**
 * Opens and reads file content
 */
void  getElfFile(uint8_t * dataBuf,DWORD size ,const char * path){

	FIL  fos;
	FRESULT result;
	WORD  read=0;
	unsigned int totalRead = 0;


	result = f_open(&fos, path,FA_READ);

	if(result != FR_OK){
		printf("FS: File could not be opened! FRESULT: %d\n", result);
		return;
	}

	/*result = f_stat(path, &fi);

		if(result != FR_OK){
			printf("FS: File could not be opened! FRESULT: %d", result);
			return 0;
		}
*/  do
	{
		totalRead += read;
		read = 0;
		result = f_read(&fos,g_cDataBuf, 65535 ,&read);


		if(result != FR_OK){
			printf("FS: File could not be read! FRESULT: %d\n", result);
			return;
		}
		memcpy(dataBuf + totalRead, g_cDataBuf, read);
	}
	 while(totalRead+read < size);

	result = f_close(&fos);

    if(result != FR_OK)
    {
    	printf("FS: File could not be closed! FRESULT: %d\n", result);
    	return;
    }
}


