/*
 * Driver: gpio.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 14.03.2014
 * Description: 
 * TODO
 */

#include <hw_cm_per.h>
#include <soc_AM335x.h>
#include "dr_gpio.h"
#include <hw_gpio.h>

#define HWREG(x)	(*((volatile unsigned int *)(x)))

void GPIO1ModuleClkConfig(void)
{
    /* Configuring L3 Interface Clocks. */

    /* Writing to MODULEMODE field of CM_PER_L3_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) |=
          CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) &
           CM_PER_L3_CLKCTRL_MODULEMODE));

    /* Writing to MODULEMODE field of CM_PER_L3_INSTR_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) |=
          CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) &
           CM_PER_L3_INSTR_CLKCTRL_MODULEMODE));

    /* Writing to CLKTRCTRL field of CM_PER_L3_CLKSTCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) |=
          CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    /* Waiting for CLKTRCTRL field to reflect the written value. */
    while(CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
           CM_PER_L3_CLKSTCTRL_CLKTRCTRL));

    /* Writing to CLKTRCTRL field of CM_PER_L3S_CLKSTCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) |=
          CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    /*Waiting for CLKTRCTRL field to reflect the written value. */
    while(CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
           CM_PER_L3S_CLKSTCTRL_CLKTRCTRL));

    /* Writing to MODULEMODE field in CM_PER_OCPWP_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_CLKCTRL) |=
          CM_PER_OCPWP_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_OCPWP_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_CLKCTRL) &
           CM_PER_OCPWP_CLKCTRL_MODULEMODE));

    /* Writing to CLKTRCTRL field of CM_PER_OCPWP_L3_CLKSTCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) |=
          CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    /*Waiting for CLKTRCTRL field to reflect the written value. */
    while(CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
           CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL));


    /* Checking fields for necessary values.  */

    /* Waiting for IDLEST field in CM_PER_L3_CLKCTRL register to be set to 0x0. */
    while((CM_PER_L3_CLKCTRL_IDLEST_FUNC << CM_PER_L3_CLKCTRL_IDLEST_SHIFT)!=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKCTRL) &
           CM_PER_L3_CLKCTRL_IDLEST));

    /*
    ** Waiting for IDLEST field in CM_PER_L3_INSTR_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_L3_INSTR_CLKCTRL_IDLEST_FUNC <<
           CM_PER_L3_INSTR_CLKCTRL_IDLEST_SHIFT)!=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_INSTR_CLKCTRL) &
           CM_PER_L3_INSTR_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_L3_GCLK field in CM_PER_L3_CLKSTCTRL register to
    ** attain the desired value.
    */
    while(CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3_CLKSTCTRL) &
           CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK));

    /*
    ** Waiting for STBYST bit in CM_PER_OCPWP_CLKCTRL register to attain
    ** the desired value.
    */
    /* while((CM_PER_OCPWP_CLKCTRL_STBYST_FUNC <<
           CM_PER_OCPWP_CLKCTRL_STBYST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_CLKCTRL) &
            CM_PER_OCPWP_CLKCTRL_STBYST)); */

    /*
    ** Waiting for IDLEST field in CM_PER_OCPWP_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_OCPWP_CLKCTRL_IDLEST_FUNC <<
           CM_PER_OCPWP_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_CLKCTRL) &
            CM_PER_OCPWP_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_OCPWP_L3_GCLK field in CM_PER_OCPWP_L3_CLKSTCTRL
    ** register to attain the desired value.
    */
    while(CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
           CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK));


    /*
    ** Waiting for CLKACTIVITY_L3S_GCLK field in CM_PER_L3S_CLKSTCTRL register
    ** to attain the desired value.
    */
    while(CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L3S_CLKSTCTRL) &
          CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK));

    /* Configuring L4 Interface Clocks. */

    /* Writing to MODULEMODE field of CM_PER_L4LS_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKCTRL) |=
          CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKCTRL) &
           CM_PER_L4LS_CLKCTRL_MODULEMODE));

    /* Writing to CLKTRCTRL field of CM_PER_L4LS_CLKSTCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) |=
          CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP;

    /* Waiting for CLKTRCTRL field to reflect the written value. */
    while(CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL));

    /* Verifying if other configurations are correct. */

    /*
    ** Waiting for IDLEST field in CM_PER_L4LS_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_L4LS_CLKCTRL_IDLEST_FUNC <<
           CM_PER_L4LS_CLKCTRL_IDLEST_SHIFT) !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKCTRL) &
           CM_PER_L4LS_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_L4LS_GCLK bit in CM_PER_L4LS_CLKSTCTRL register
    ** to attain the desired value.
    */
    while(CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_L4LS_GCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_L4LS_GCLK));

    /*
    ** Waiting for CLKACTIVITY_OCPWP_L4_GCLK bit in CM_PER_OCPWP_L3_CLKSTCTRL
    ** register to attain the desired value.
    */
    /* while(CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_OCPWP_L3_CLKSTCTRL) &
          CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK)); */

    /* Performing configurations for GPIO1 instance. */

    /* Writing to MODULEMODE field of CM_PER_GPIO1_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |=
          CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_GPIO1_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) &
           CM_PER_GPIO1_CLKCTRL_MODULEMODE));
    /*
    ** Writing to OPTFCLKEN_GPIO_1_GDBCLK bit in CM_PER_GPIO1_CLKCTRL
    ** register.
    */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) |=
          CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK;

    /*
    ** Waiting for OPTFCLKEN_GPIO_1_GDBCLK bit to reflect the desired
    ** value.
    */
    while(CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) &
           CM_PER_GPIO1_CLKCTRL_OPTFCLKEN_GPIO_1_GDBCLK));

    /*
    ** Waiting for IDLEST field in CM_PER_GPIO1_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_GPIO1_CLKCTRL_IDLEST_FUNC <<
           CM_PER_GPIO1_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO1_CLKCTRL) &
            CM_PER_GPIO1_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_GPIO_1_GDBCLK bit in CM_PER_L4LS_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_1_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_1_GDBCLK));
}


unsigned int GPIO1Pin23PinMuxSetup(void)
{

	int status=FALSE;

	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(7))=CONTROL_CONF_MUXMODE(7);
            status = TRUE;


    return status;
}

/**
 * \brief  This API is used to enable the GPIO module. When the GPIO module
 *         is enabled, the clocks to the module are not gated.
 *
 * \param  baseAdd    The memory address of the GPIO instance being used
 *
 * \return None
 *
 * \note   Enabling the GPIO module is a primary step before any other
 *         configurations can be done.
 */

void GPIOModuleEnable(unsigned int baseAdd)
{
    /* Clearing the DISABLEMODULE bit in the Control(CTRL) register. */
    HWREG(baseAdd + GPIO_CTRL) &= ~(GPIO_CTRL_DISABLEMODULE);
}

void GPIOModuleReset(unsigned int baseAdd)
{
    /*
    ** Setting the SOFTRESET bit in System Configuration register.
    ** Doing so would reset the GPIO module.
    */
    HWREG(baseAdd + GPIO_SYSCONFIG) |= (GPIO_SYSCONFIG_SOFTRESET);

    /* Waiting until the GPIO Module is reset.*/
    while(!(HWREG(baseAdd + GPIO_SYSSTATUS) & GPIO_SYSSTATUS_RESETDONE));
}

void GPIODirModeSet(unsigned int baseAdd,
                    unsigned int pinNumber,
                    unsigned int pinDirection)
{
    /* Checking if pin is required to be an output pin. */
    if(GPIO_DIR_OUTPUT == pinDirection)
    {
        HWREG(baseAdd + GPIO_OE) &= ~(1 << pinNumber);
    }
    else
    {
        HWREG(baseAdd + GPIO_OE) |= (1 << pinNumber);
    }
}

void GPIOPinWrite(unsigned int baseAdd,
                  unsigned int pinNumber,
                  unsigned int pinValue)
{
    if(GPIO_PIN_HIGH == pinValue)
    {
        HWREG(baseAdd + GPIO_SETDATAOUT) = (1 << pinNumber);
    }
    else
    {
        HWREG(baseAdd + GPIO_CLEARDATAOUT) = (1 << pinNumber);
    }
}



