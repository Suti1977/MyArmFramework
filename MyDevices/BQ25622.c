//------------------------------------------------------------------------------
//  BQ25622 Li-Ion akkumulator tolto IC driver
//
//    File: BQ25622.c
//------------------------------------------------------------------------------
#include "BQ25622.h"
#include <string.h>

//------------------------------------------------------------------------------
// I2C eszkoz letrehozasa a rendszerben
void BQ25622_create(BQ25622_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress)
{
    MyI2CM_createDevice(&dev->i2cDevice, i2c, slaveAddress, NULL);
}
//------------------------------------------------------------------------------
// Az IC egyetlen regiszterenek irasa
status_t BQ25622_writeReg(BQ25622_t* dev, uint8_t regAddress, uint8_t value)
{
    MyI2CM_xfer_t xferBlocks[] =
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &regAddress, 1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &value, 1}
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
//Az IC egyetlen regiszterenek byteos olvasasa
status_t BQ25622_readReg(BQ25622_t* dev, uint8_t regAddress, uint8_t* value)
{
    MyI2CM_xfer_t xferBlocks[] =
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &regAddress, 1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, value, 1}
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
// Az IC 16 bites regiszterenek olvasasa
status_t BQ25622_read2(BQ25622_t* dev, uint8_t regAddress, uint16_t* value)
{
    MyI2CM_xfer_t xferBlocks[] =
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &regAddress, 1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, (uint8_t*)value, 2}
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
// Az IC regiszterenek 16 bites irasa
status_t BQ25622_write2(BQ25622_t* dev, uint8_t regAddress, uint16_t value)
{
    MyI2CM_xfer_t xferBlocks[] =
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &regAddress, 1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, (uint8_t*)&value, 2}
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
// Eygetlen bit kiolvasasa
static status_t BQ25622_getBit(BQ25622_t* dev,
                               uint8_t reg,
                               uint8_t mask,
                               bool* value)
{
    uint8_t data = 0;
    status_t status;

    status = BQ25622_readReg(dev, reg, &data);
    if (status) return status;

    *value = (data & mask) != 0U;
    return kStatus_Success;
}
//------------------------------------------------------------------------------
static status_t BQ25622_getField(BQ25622_t* dev,
                                 uint8_t reg,
                                 uint8_t mask,
                                 uint8_t shift,
                                 uint8_t* value)
{
    uint8_t data = 0;
    status_t status;

    status = BQ25622_readReg(dev, reg, &data);
    if (status) return status;

    *value = (uint8_t)((data & mask) >> shift);
    return kStatus_Success;
}
//------------------------------------------------------------------------------
static status_t BQ25622_update16(BQ25622_t* dev,
                                 uint8_t regLSB,
                                 uint16_t mask,
                                 uint8_t shift,
                                 uint16_t code)
{
    uint16_t word = 0;

    status_t status = BQ25622_read2(dev, regLSB, &word);
    if (status) return status;

    word = (uint16_t)((word & (uint16_t)~mask) |
                      (uint16_t)((code << shift) & mask));
    return BQ25622_write2(dev, regLSB, word);
}
//------------------------------------------------------------------------------
static status_t BQ25622_readField16(BQ25622_t* dev,
                                    uint8_t regLSB,
                                    uint16_t mask,
                                    uint8_t shift,
                                    uint16_t scale,
                                    uint16_t* value)
{
    uint16_t word = 0;
    status_t status;

    status = BQ25622_read2(dev, regLSB, &word);
    if (status) return status;

    *value = (uint16_t)(((word & mask) >> shift) * scale);
    return kStatus_Success;
}
//------------------------------------------------------------------------------
static status_t BQ25622_update8(BQ25622_t* dev,
                                uint8_t reg,
                                uint8_t mask,
                                uint8_t shift,
                                uint8_t value)
{
    uint8_t oldValue = 0;

    status_t status = BQ25622_readReg(dev, reg, &oldValue);
    if (status) return status;

    oldValue = (uint8_t)((oldValue & (uint8_t)~mask) |
                         (uint8_t)((value << shift) & mask));
    return BQ25622_writeReg(dev, reg, oldValue);
}
//------------------------------------------------------------------------------
static status_t BQ25622_updateBit(BQ25622_t* dev,
                                  uint8_t reg,
                                  uint8_t mask,
                                  bool value)
{
    uint8_t oldValue = 0;
    status_t status = BQ25622_readReg(dev, reg, &oldValue);

    if (status) return status;

    if (value)
    {
        oldValue |= mask;
    }
    else
    {
        oldValue &= (uint8_t)~mask;
    }

    return BQ25622_writeReg(dev, reg, oldValue);
}
//------------------------------------------------------------------------------
static status_t BQ25622_readField(BQ25622_t* dev,
                                  uint8_t reg,
                                  uint8_t mask,
                                  uint8_t shift,
                                  uint8_t* value)
{
    uint8_t raw = 0;
    status_t status;

    status = BQ25622_readReg(dev, reg, &raw);
    if (status) return status;

    *value = (uint8_t)((raw & mask) >> shift);
    return status;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Chip szoftveres resetelese
status_t BQ25622_reset(BQ25622_t* dev)
{
    uint8_t value = 0;
    status_t status;

    status = BQ25622_readReg(dev, BQ25622_REG_CHARGER_CONTROL_2, &value);
    if (status) return status;

    value |= BQ25622_CTRL2_REG_RST;
    return BQ25622_writeReg(dev, BQ25622_REG_CHARGER_CONTROL_2, value);
}
//------------------------------------------------------------------------------
// Tolto aram korlat beallitasa. [mA]
// Rangge: 80mA - 3520mA , step 80mA
status_t BQ25622_setICHG(BQ25622_t* dev, int current_mA)
{
    uint16_t code;

    if ((current_mA < 80) || (current_mA > 3520))
    {
        return kStatus_OutOfRange;
    }

    code = (uint16_t)((uint16_t)current_mA / 80);
    return BQ25622_update16(dev,
                            BQ25622_REG_CHARGE_CURRENT_LSB,
                            BQ25622_ICHG_MASK,
                            BQ25622_ICHG_SHIFT,
                            code);
}
//------------------------------------------------------------------------------
// Tolto aram korlat lekerdezese [mA]
status_t BQ25622_getICHG(BQ25622_t* dev, uint16_t* current_mA)
{
    return BQ25622_readField16(dev,
                               BQ25622_REG_CHARGE_CURRENT_LSB,
                               BQ25622_ICHG_MASK,
                               BQ25622_ICHG_SHIFT,
                               80,
                               current_mA);
}
//------------------------------------------------------------------------------
// Maximalis tolto feszultseg beallitasa
// Range: 3500mV - 4800mV, step: 10mV
status_t BQ25622_setVREG(BQ25622_t* dev, int value)
{
    uint16_t code;

    if ((value < 3500) || (value > 4800))
    {
        return kStatus_OutOfRange;
    }

    code = (uint16_t)((uint16_t)value / 10);
    return BQ25622_update16(dev,
                            BQ25622_REG_CHARGE_VOLTAGE_LSB,
                            BQ25622_VREG_MASK,
                            BQ25622_VREG_SHIFT,
                            code);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVREG(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_readField16(dev,
                               BQ25622_REG_CHARGE_VOLTAGE_LSB,
                               BQ25622_VREG_MASK,
                               BQ25622_VREG_SHIFT,
                               10,
                               value);
}
//------------------------------------------------------------------------------
// Bemeneti aramkorlat beallitasa
status_t BQ25622_setIINDPM(BQ25622_t* dev, int value)
{
    uint16_t code;

    if ((value < 100) || (value > 3200))
    {
        return kStatus_OutOfRange;
    }

    code = (uint16_t)((uint16_t)value / 20);
    return BQ25622_update16(dev,
                            BQ25622_REG_INPUT_CURRENT_LSB,
                            BQ25622_IINDPM_MASK,
                            BQ25622_IINDPM_SHIFT,
                            code);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIINDPM(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_readField16(dev,
                               BQ25622_REG_INPUT_CURRENT_LSB,
                               BQ25622_IINDPM_MASK,
                               BQ25622_IINDPM_SHIFT,
                               20,
                               value);
}
//------------------------------------------------------------------------------
// Bemeneti feszultseg korlat beallitasa
// Range: 3800mV - 16800mV, step: 40mV
status_t BQ25622_setVINDPM(BQ25622_t* dev, int value)
{
    uint16_t code;

    if ((value < 3800) || (value > 16800))
    {
        return kStatus_OutOfRange;
    }

    code = (uint16_t)((uint16_t)value / 40);
    return BQ25622_update16(dev,
                            BQ25622_REG_INPUT_VOLTAGE_LSB,
                            BQ25622_VINDPM_MASK,
                            BQ25622_VINDPM_SHIFT,
                            code);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVINDPM(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_readField16(dev,
                               BQ25622_REG_INPUT_VOLTAGE_LSB,
                               BQ25622_VINDPM_MASK,
                               BQ25622_VINDPM_SHIFT,
                               40,
                               value);
}
//------------------------------------------------------------------------------
// Mimimalis VSYS feszultseg beallitsa
// Range: 2560mV - 3840mV step: 80mA
status_t BQ25622_setVSYSMIN(BQ25622_t* dev, int value)
{
    uint16_t code;

    if ((value < 2560) || (value > 3840))
    {
        return kStatus_OutOfRange;
    }

    code = (uint16_t)((uint16_t)value / 80);
    return BQ25622_update16(dev,
                            BQ25622_REG_MIN_SYSTEM_VOLTAGE_LSB,
                            BQ25622_VSYSMIN_MASK,
                            BQ25622_VSYSMIN_SHIFT,
                            code);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVSYSMIN(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_readField16(dev,
                               BQ25622_REG_MIN_SYSTEM_VOLTAGE_LSB,
                               BQ25622_VSYSMIN_MASK,
                               BQ25622_VSYSMIN_SHIFT,
                               80,
                               value);
}
//------------------------------------------------------------------------------
// Elotoltesi aram beallitasa
// Range: 20mA - 620mA. step: 20mA
status_t BQ25622_setIPRECHG(BQ25622_t* dev, int value)
{
    uint16_t code;

    if ((value < 20) || (value > 620))
    {
        return kStatus_OutOfRange;
    }

    code = (uint16_t)((uint16_t)value / 20);
    return BQ25622_update16(dev,
                            BQ25622_REG_PRECHARGE_LSB,
                            BQ25622_IPRECHG_MASK,
                            BQ25622_IPRECHG_SHIFT,
                            code);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIPRECHG(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_readField16(dev,
                               BQ25622_REG_PRECHARGE_LSB,
                               BQ25622_IPRECHG_MASK,
                               BQ25622_IPRECHG_SHIFT,
                               20,
                               value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setITERM(BQ25622_t* dev, int value)
{
    uint16_t code;

    if ((value < 10) || (value > 620))
    {
        return kStatus_OutOfRange;
    }

    code = (uint16_t)((uint16_t)value / 10);
    return BQ25622_update16(dev,
                            BQ25622_REG_TERMINATION_LSB,
                            BQ25622_ITERM_MASK,
                            BQ25622_ITERM_SHIFT,
                            code);
}
//------------------------------------------------------------------------------
status_t BQ25622_getITERM(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_readField16(dev,
                               BQ25622_REG_TERMINATION_LSB,
                               BQ25622_ITERM_MASK,
                               BQ25622_ITERM_SHIFT,
                               10,
                               value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargeControl0(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGE_CONTROL_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargeTimerControl(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGE_TIMER_CONTROL, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerControl1(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_CONTROL_1, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerControl2(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_CONTROL_2, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerControl4(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_CONTROL_4, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getNtcControl0(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_NTC_CONTROL_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerStatus0Reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_STATUS_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerStatus1Reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_STATUS_1, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getFaultStatus0Reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_FAULT_STATUS_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getAdcControl(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_ADC_CONTROL, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getAdcFunctionDisable0(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_ADC_FUNCTION_DISABLE_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTS_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_TS_DIS,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setEN_AUTO_INDET(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_EN_AUTO_INDET,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setFORCE_INDET(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_FORCE_INDET,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setEN_DCP_BIAS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_EN_DCP_BIAS,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTMR2X_EN(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_TMR2X_EN,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setEN_SAFETY_TMR(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_EN_SAFETY_TMR,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setPRECHG_TMR(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_PRECHG_TMR,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setCHG_TIMER(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_TIMER_CONTROL,
                             BQ25622_TIMER_CHG_TIMER,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setQ1_FULLON(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_CONTROL_0,
                             BQ25622_CTRL0_Q1_FULLON,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setQ4_FULLON(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_CONTROL_0,
                             BQ25622_CTRL0_Q4_FULLON,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTOPOFF_TMR(BQ25622_t* dev, int value)
{
    uint8_t code;

    switch (value)
    {
        case 0:  code = 0; break;
        case 17: code = 1; break;
        case 35: code = 2; break;
        case 52: code = 3; break;
        default:
            return kStatus_OutOfRange;
    }

    return BQ25622_update8(dev,
                           BQ25622_REG_CHARGE_CONTROL_0,
                           BQ25622_CTRL0_TOPOFF_TMR_MASK,
                           BQ25622_CTRL0_TOPOFF_TMR_SHIFT,
                           code);
}
//------------------------------------------------------------------------------
status_t BQ25622_setEN_CHG(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_1,
                             BQ25622_CTRL1_EN_CHG,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setWATCHDOG(BQ25622_t* dev, BQ25622_watchdog_t value)
{
    return BQ25622_update8(dev,
                           BQ25622_REG_CHARGER_CONTROL_1,
                           BQ25622_CTRL1_WATCHDOG_MASK,
                           BQ25622_CTRL1_WATCHDOG_SHIFT,
                           value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setVBUS_OVP(BQ25622_t* dev, int value)
{
    uint8_t code;

    if (value == 6)
    {
        code = 0;
    }
    else if (value == 18)
    {
        code = 1;
    }
    else
    {
        return kStatus_OutOfRange;
    }

    return BQ25622_update8(dev, BQ25622_REG_CHARGER_CONTROL_2,
                           BQ25622_CTRL2_VBUS_OVP, 0, code);
}
//------------------------------------------------------------------------------
status_t BQ25622_setCONV_STRN(BQ25622_t* dev, int value)
{
    if ((value < 0) || (value > 3))
    {
        return kStatus_OutOfRange;
    }
    return BQ25622_update8(dev,
                           BQ25622_REG_CHARGER_CONTROL_2,
                           BQ25622_CTRL2_CONV_STRN_MASK,
                           BQ25622_CTRL2_CONV_STRN_SHIFT,
                           (uint8_t)value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setCONV_FREQ(BQ25622_t* dev, int value)
{
    if ((value < 0) || (value > 3))
    {
        return kStatus_OutOfRange;
    }
    return BQ25622_update8(dev, BQ25622_REG_CHARGER_CONTROL_2,
                           BQ25622_CTRL2_CONV_FREQ_MASK,
                           BQ25622_CTRL2_CONV_FREQ_SHIFT,
                           (uint8_t)value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTREG(BQ25622_t* dev, int value)
{
    uint8_t code;

    if (value == 60)
    {
        code = 0;
    }
    else if (value == 120)
    {
        code = 1;
    }
    else
    {
        return kStatus_OutOfRange;
    }

    return BQ25622_update8(dev,
                           BQ25622_REG_CHARGER_CONTROL_2,
                           BQ25622_CTRL2_TREG,
                           6,
                           code);
}
//------------------------------------------------------------------------------
status_t BQ25622_setREG_RST(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_2,
                             BQ25622_CTRL2_REG_RST,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setEN_EXTLIM(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_4,
                             BQ25622_CTRL4_EN_EXTLIM,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTS_IGNORE(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_NTC_CONTROL_0,
                             BQ25622_NTC_TS_IGNORE,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setCONV_START(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_CONTROL,
                             BQ25622_ADC_CONV_START,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setCONV_RATE(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_CONTROL,
                             BQ25622_ADC_CONV_RATE,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setADC_SAMPLE(BQ25622_t* dev, int value)
{
    uint8_t code;

    switch (value)
    {
        case 12: code = 0; break;
        case 11: code = 1; break;
        case 10: code = 2; break;
        case 9:  code = 3; break;
        default:
            return kStatus_OutOfRange;
    }

    return BQ25622_update8(dev,
                           BQ25622_REG_ADC_CONTROL,
                           BQ25622_ADC_SAMPLE_MASK,
                           BQ25622_ADC_SAMPLE_SHIFT, code);
}
//------------------------------------------------------------------------------
status_t BQ25622_setADC_AVG(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_CONTROL,
                             BQ25622_ADC_AVG,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setADC_AVG_INIT(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_CONTROL,
                             BQ25622_ADC_AVG_INIT,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setVPMID_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                             BQ25622_ADC_DIS_VPMID,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTDIE_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                             BQ25622_ADC_DIS_TDIE,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTS_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                             BQ25622_ADC_DIS_TS,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setVSYS_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                             BQ25622_ADC_DIS_VSYS,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setVBAT_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                             BQ25622_ADC_DIS_VBAT,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setVBUS_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                            BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                            BQ25622_ADC_DIS_VBUS,
                            value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setIBAT_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                             BQ25622_ADC_DIS_IBAT,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setIBUS_ADC_DIS(BQ25622_t* dev, bool value)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_FUNCTION_DISABLE_0,
                             BQ25622_ADC_DIS_IBUS,
                             value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getWD_STAT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_STATUS_0,
                          BQ25622_STATUS0_WD_STAT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getSAFETY_TMR_STAT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_STATUS_0,
                          BQ25622_STATUS0_SAFETY_TMR_STAT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVINDPM_STAT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_STATUS_0,
                          BQ25622_STATUS0_VINDPM_STAT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIINDPM_STAT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_STATUS_0,
                          BQ25622_STATUS0_IINDPM_STAT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVSYS_STAT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_STATUS_0,
                          BQ25622_STATUS0_VSYS_STAT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getTREG_STAT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_STATUS_0,
                          BQ25622_STATUS0_TREG_STAT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getADC_DONE_STAT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_STATUS_0,
                          BQ25622_STATUS0_ADC_DONE_STAT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVBUS_STAT(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_getField(dev,
                            BQ25622_REG_CHARGER_STATUS_1,
                            BQ25622_STATUS1_VBUS_STAT_MASK,
                            BQ25622_STATUS1_VBUS_STAT_SHIFT,
                            value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCHG_STAT(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_getField(dev,
                            BQ25622_REG_CHARGER_STATUS_1,
                            BQ25622_STATUS1_CHG_STAT_MASK,
                            BQ25622_STATUS1_CHG_STAT_SHIFT,
                            value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getTS_STAT(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_getField(dev,
                            BQ25622_REG_FAULT_STATUS_0,
                            BQ25622_FAULT_TS_STAT_MASK,
                            0,
                            value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getTSHUT_FAULT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_FAULT_STATUS_0,
                          BQ25622_FAULT_TSHUT_FAULT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getOTG_FAULT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_FAULT_STATUS_0,
                          BQ25622_FAULT_OTG_FAULT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getSYS_FAULT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_FAULT_STATUS_0,
                          BQ25622_FAULT_SYS_FAULT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getBAT_FAULT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_FAULT_STATUS_0,
                          BQ25622_FAULT_BAT_FAULT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVBUS_FAULT(BQ25622_t* dev, bool* value)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_FAULT_STATUS_0,
                          BQ25622_FAULT_VBUS_FAULT,
                          value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIBAT(BQ25622_t* dev, int16_t* value)
{
    status_t status;

    uint16_t raw = 0;
    status = BQ25622_read2(dev, BQ25622_REG_IBAT_ADC_LSB, &raw);
    if (status) return status;

    raw &= 0xfffc;
    *value = (int16_t)raw;

    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getVBUSV(BQ25622_t* dev, uint16_t* value)
{
    uint16_t raw = 0;
    status_t status;

    status = BQ25622_read2(dev, BQ25622_REG_VBUS_ADC_LSB, &raw);
    if (status) return status;

    *value = (uint16_t)((((raw >> 2) & 0x1FFFU) * 397U) / 100U);
    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getBATV(BQ25622_t* dev, uint16_t* value)
{
    uint16_t raw = 0;
    status_t status;

    status = BQ25622_read2(dev, BQ25622_REG_VBAT_ADC_LSB, &raw);
    if (status) return status;

    *value = (uint16_t)((((raw >> 1) & 0x0FFFU) * 199U) / 100U);
    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getVPMID(BQ25622_t* dev, uint16_t* value)
{
    uint16_t raw = 0;
    status_t status;

    status = BQ25622_read2(dev, BQ25622_REG_VPMID_ADC_LSB, &raw);
    if (status) return status;

    *value = (uint16_t)((((raw >> 2) & 0x1FFFU) * 397U) / 100U);
    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getIBUS(BQ25622_t* dev, int16_t* value)
{
    uint16_t raw = 0;
    status_t status;

    status = BQ25622_read2(dev, BQ25622_REG_IBUS_ADC_LSB, &raw);
    if (status) return status;

    raw &= 0xfffe;
    *value = (int16_t)raw;

    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getVSYS(BQ25622_t* dev, uint16_t* value)
{
    uint16_t raw = 0;
    status_t status;

    status = BQ25622_read2(dev, BQ25622_REG_VSYS_ADC_LSB, &raw);
    if (status) return status;

    *value = (uint16_t)((((raw >> 1) & 0x0FFFU) * 199U) / 100U);
    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getTS(BQ25622_t* dev, float* value)
{
    uint16_t raw = 0;
    status_t status;

    status = BQ25622_read2(dev, BQ25622_REG_TS_ADC_LSB, &raw);
    if (status) return status;

    raw &= 0x0fff;
    *value = raw * 0.000961;
    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getTDIE(BQ25622_t* dev, float* value)
{
    uint16_t raw = 0;
    status_t status;

    status = BQ25622_read2(dev, BQ25622_REG_TDIE_ADC_LSB, &raw);
    if (status) return status;

    raw &= 0x0fff;
    *value = ((int16_t)raw) * 0.5f;
    return kStatus_Success;
}
//------------------------------------------------------------------------------
status_t BQ25622_getICHG_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_CHARGE_CURRENT_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVREG_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_CHARGE_VOLTAGE_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIINDPM_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_INPUT_CURRENT_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVINDPM_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_INPUT_VOLTAGE_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVSYSMIN_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_MIN_SYSTEM_VOLTAGE_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIPRE_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_PRECHARGE_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getITERM_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_TERMINATION_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCHG_TIMER_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGE_TIMER_CONTROL, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCTRL0_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGE_CONTROL_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCTRL1_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_CONTROL_1, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCTRL2_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_CONTROL_2, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCTRL4_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_CONTROL_4, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getNTC_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_NTC_CONTROL_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCHRG_STATUS_0_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_STATUS_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCHRG_STATUS_1_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_STATUS_1, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getFAULT_STATUS_0_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_FAULT_STATUS_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getADC_CTRL_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_ADC_CONTROL, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getADC_DIS_reg(BQ25622_t* dev, uint8_t* value)
{
    return BQ25622_readReg(dev, BQ25622_REG_ADC_FUNCTION_DISABLE_0, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIBAT_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_IBAT_ADC_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVBUSV_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_VBUS_ADC_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_getBATV_reg(BQ25622_t* dev, uint16_t* value)
{
    return BQ25622_read2(dev, BQ25622_REG_VBAT_ADC_LSB, value);
}
//------------------------------------------------------------------------------
status_t BQ25622_setTrickleCurrent(BQ25622_t* dev  , BQ25622_tricleCurrent_t tc)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_CONTROL_0,
                             BQ25622_CTRL0_ITRICKLE,
                             tc);
}
//------------------------------------------------------------------------------
status_t BQ25622_getTrickleCurrent(BQ25622_t* dev, BQ25622_tricleCurrent_t* tc)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGE_CONTROL_0,
                          BQ25622_CTRL0_ITRICKLE,
                          (bool*)tc);
}
//------------------------------------------------------------------------------
status_t BQ25622_setEnableTermination(BQ25622_t* dev, bool enable)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGE_CONTROL_0,
                             BQ25622_CTRL0_EN_TERM,
                             enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getEnableTermination(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGE_CONTROL_0,
                          BQ25622_CTRL0_EN_TERM,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_setVINDPMbatTrack(BQ25622_t* dev, bool enable)
{
    return BQ25622_updateBit(dev,
                           BQ25622_REG_CHARGE_CONTROL_0,
                           BQ25622_CTRL0_VINDPM_BAT_TRACK,
                           enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVINDPMbatTrack(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGE_CONTROL_0,
                          BQ25622_CTRL0_VINDPM_BAT_TRACK,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_setAutoBatteryDischarge(BQ25622_t* dev, bool enable)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_1,
                             BQ25622_CTRL1_EN_AUTO_IBATDIS,
                             enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getAutoBatteryDischarge(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_1,
                          BQ25622_CTRL1_EN_AUTO_IBATDIS,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_setForceBatteryDischarge(BQ25622_t* dev, bool enable)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_1,
                             BQ25622_CTRL1_FORCE_IBATDIS,
                             enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getForceBatteryDischarge(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_1,
                          BQ25622_CTRL1_FORCE_IBATDIS,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_setHighZ(BQ25622_t* dev, bool enable)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_1,
                             BQ25622_CTRL1_EN_HIZ,
                             enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getHighZ(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_1,
                          BQ25622_CTRL1_EN_HIZ,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_setForcePMIDDischarge(BQ25622_t* dev, bool enable)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_1,
                             BQ25622_CTRL1_FORCE_PMID_DIS,
                             enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getForcePMIDDischarge(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_1,
                          BQ25622_CTRL1_FORCE_PMID_DIS,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_resetWatchdog(BQ25622_t* dev)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_1,
                             BQ25622_CTRL1_WD_RST,
                             true);
}
//------------------------------------------------------------------------------
status_t BQ25622_setPeakBattDischarge(BQ25622_t* dev,
                                      BQ25622_peakDischargeCurrent_t peak)
{
    if (peak > 3U) return kStatus_OutOfRange;
    return BQ25622_update8(dev,
                           BQ25622_REG_CHARGER_CONTROL_4,
                           BQ25622_CTRL4_IBAT_PK_MASK,
                           BQ25622_CTRL4_IBAT_PK_SHIFT,
                           peak);
}
//------------------------------------------------------------------------------
status_t BQ25622_getPeakBattDischarge(BQ25622_t* dev,
                                      BQ25622_peakDischargeCurrent_t* peak)
{
    *peak=0;
    return BQ25622_readField(dev,
                             BQ25622_REG_CHARGER_CONTROL_4,
                             BQ25622_CTRL4_IBAT_PK_MASK,
                             BQ25622_CTRL4_IBAT_PK_SHIFT,
                             (uint8_t*)peak);
}
//------------------------------------------------------------------------------
status_t BQ25622_setVBatUVLO(BQ25622_t* dev, BQ25622_UVLO_t lowThreshold)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_CHARGER_CONTROL_4,
                             BQ25622_CTRL4_VBAT_UVLO,
                             lowThreshold);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVBatUVLO(BQ25622_t* dev, BQ25622_UVLO_t* lowThreshold)
{
    *lowThreshold=0;
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_4,
                          BQ25622_CTRL4_VBAT_UVLO,
                          (bool*)lowThreshold);
}
//------------------------------------------------------------------------------
status_t BQ25622_setChargeRate(BQ25622_t* dev, BQ25622_chargeRate_t rate)
{
    if (rate > 3U) return kStatus_OutOfRange;
    return BQ25622_update8(dev,
                           BQ25622_REG_CHARGER_CONTROL_4,
                           BQ25622_CTRL4_CHG_RATE_MASK,
                           BQ25622_CTRL4_CHG_RATE_SHIFT,
                           rate);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargeRate(BQ25622_t* dev, BQ25622_chargeRate_t* rate)
{
    *rate=0;
    return BQ25622_readField(dev,
                             BQ25622_REG_CHARGER_CONTROL_4,
                             BQ25622_CTRL4_CHG_RATE_MASK,
                             BQ25622_CTRL4_CHG_RATE_SHIFT,
                             (uint8_t*)rate);
}
//------------------------------------------------------------------------------
status_t BQ25622_setCoolThermistorCurrent(BQ25622_t* dev, uint8_t setting)
{
    if (setting > 3U) return kStatus_OutOfRange;
    return BQ25622_update8(dev,
                           BQ25622_REG_NTC_CONTROL_0,
                           BQ25622_NTC_COOL_MASK,
                           BQ25622_NTC_COOL_SHIFT,
                           setting);
}
//------------------------------------------------------------------------------
status_t BQ25622_getCoolThermistorCurrent(BQ25622_t* dev, uint8_t* setting)
{
    return BQ25622_readField(dev,
                             BQ25622_REG_NTC_CONTROL_0,
                             BQ25622_NTC_COOL_MASK,
                             BQ25622_NTC_COOL_SHIFT,
                             setting);
}
//------------------------------------------------------------------------------
status_t BQ25622_setWarmThermistorCurrent(BQ25622_t* dev, uint8_t setting)
{
    if (setting > 3U) return kStatus_OutOfRange;
    return BQ25622_update8(dev,
                           BQ25622_REG_NTC_CONTROL_0,
                           BQ25622_NTC_WARM_MASK,
                           BQ25622_NTC_WARM_SHIFT,
                           setting);
}
//------------------------------------------------------------------------------
status_t BQ25622_getWarmThermistorCurrent(BQ25622_t* dev, uint8_t* setting)
{
    return BQ25622_readField(dev,
                             BQ25622_REG_NTC_CONTROL_0,
                             BQ25622_NTC_WARM_MASK,
                             BQ25622_NTC_WARM_SHIFT,
                             setting);
}
//------------------------------------------------------------------------------
status_t BQ25622_setADCEnable(BQ25622_t* dev, bool enable)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_CONTROL,
                             BQ25622_ADC_CONV_START,
                             enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getADCEnable(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_ADC_CONTROL,
                          BQ25622_ADC_CONV_START,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_setADCOneShot(BQ25622_t* dev, bool oneShot)
{
    return BQ25622_updateBit(dev,
                             BQ25622_REG_ADC_CONTROL,
                             BQ25622_ADC_CONV_RATE,
                             oneShot);
}
//------------------------------------------------------------------------------
status_t BQ25622_getADCOneShot(BQ25622_t* dev, bool* oneShot)
{
    status_t status;
    bool continuous = false;
    status = BQ25622_getBit(dev,
                            BQ25622_REG_ADC_CONTROL,
                            BQ25622_ADC_CONV_RATE,
                            &continuous);
    if (status) return status;
    *oneShot = !continuous;
    return status;
}
//------------------------------------------------------------------------------
status_t BQ25622_getADCSampleRate(BQ25622_t* dev, uint8_t* sampleRate)
{
    return BQ25622_readField(dev,
                             BQ25622_REG_ADC_CONTROL,
                             BQ25622_ADC_SAMPLE_MASK,
                             BQ25622_ADC_SAMPLE_SHIFT,
                             sampleRate);
}
//------------------------------------------------------------------------------
status_t BQ25622_setEnableCharging(BQ25622_t* dev, bool enable)
{
    return BQ25622_setEN_CHG(dev, enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_getEnableCharging(BQ25622_t* dev, bool* enable)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_1,
                          BQ25622_CTRL1_EN_CHG,
                          enable);
}
//------------------------------------------------------------------------------
status_t BQ25622_setWatchdog(BQ25622_t* dev, uint8_t setting)
{
    return BQ25622_setWATCHDOG(dev, (int)setting);
}
//------------------------------------------------------------------------------
status_t BQ25622_getWatchdog(BQ25622_t* dev, uint8_t* setting)
{
    return BQ25622_readField(dev,
                             BQ25622_REG_CHARGER_CONTROL_1,
                             BQ25622_CTRL1_WATCHDOG_MASK,
                             BQ25622_CTRL1_WATCHDOG_SHIFT,
                             setting);
}
//------------------------------------------------------------------------------
status_t BQ25622_getThermalRegulation(BQ25622_t* dev, bool* temp120C)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_2,
                          BQ25622_CTRL2_TREG,
                          temp120C);
}
//------------------------------------------------------------------------------
status_t BQ25622_getConverterFrequency(BQ25622_t* dev, uint8_t* frequency)
{
    return BQ25622_readField(dev,
                             BQ25622_REG_CHARGER_CONTROL_2,
                             BQ25622_CTRL2_CONV_FREQ_MASK,
                             BQ25622_CTRL2_CONV_FREQ_SHIFT,
                             frequency);
}
//------------------------------------------------------------------------------
status_t BQ25622_getVBUSOvervoltage(BQ25622_t* dev, bool* highThreshold)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_CHARGER_CONTROL_2,
                          BQ25622_CTRL2_VBUS_OVP,
                          highThreshold);
}
//------------------------------------------------------------------------------
status_t BQ25622_getIgnoreThermistor(BQ25622_t* dev, bool* ignore)
{
    return BQ25622_getBit(dev,
                          BQ25622_REG_NTC_CONTROL_0,
                          BQ25622_NTC_TS_IGNORE,
                          ignore);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerStatus0(BQ25622_t* dev,
                                   BQ25622_chargerStatus0_t* stat)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_STATUS_0, &stat->raw);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerStatus1(BQ25622_t* dev,
                                   BQ25622_chargerStatus1_t* stat)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_STATUS_1, &stat->raw);
}
//------------------------------------------------------------------------------
status_t BQ25622_getFaultStatus0(BQ25622_t* dev, BQ25622_faultStatus0_t* stat)
{
    return BQ25622_readReg(dev, BQ25622_REG_FAULT_STATUS_0, &stat->raw);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerFlags0(BQ25622_t* dev, BQ25622_chargerFlag0_t* flags)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_FLAG_0, &flags->raw);
}
//------------------------------------------------------------------------------
status_t BQ25622_getChargerFlags1(BQ25622_t* dev, BQ25622_chargerFlag1_t* flags)
{
    return BQ25622_readReg(dev, BQ25622_REG_CHARGER_FLAG_1, &flags->raw);
}
//------------------------------------------------------------------------------
status_t BQ25622_getFaultFlags(BQ25622_t* dev, BQ25622_faultFlag0_t* flags)
{
    return BQ25622_readReg(dev, BQ25622_REG_FAULT_FLAG_0, &flags->raw);
}
//------------------------------------------------------------------------------
