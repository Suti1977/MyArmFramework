//------------------------------------------------------------------------------
//  BQ24195 Li-Ion akkumulator tolto
//
//    File: BQ24195.c
//------------------------------------------------------------------------------
//STUFF:
//https://www.ti.com/lit/gpn/bq24195
#include "BQ24195.h"

//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void BQ24195_create(BQ24195_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress)
{
    //I2C eleres letrehozasa
    MyI2CM_createDevice(&dev->i2cDevice, i2c, slaveAddress, NULL);
}
//------------------------------------------------------------------------------
//Az IC egyetlen regiszterenek irasa
status_t BQ24195_writeReg(BQ24195_t* dev, uint8_t regAddress, uint8_t value)
{
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &regAddress,  1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &value,       1},
    };
    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
//Az IC tobb regiszterenek olvasasa
status_t BQ24195_readReg(BQ24195_t* dev, uint8_t regAddress, uint8_t* value)
{
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &regAddress,  1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, value,        1},
    };
    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
//Set HiZ state
status_t BQ24195_setHiZ(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG00, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG00_EN_HIZ;
    if (enable) regData |=BQ24195_REG00_EN_HIZ;
    return BQ24195_writeReg(dev, BQ24195_REG00, regData);
}
//------------------------------------------------------------------------------
//Set input voltage limit.
//valid range: 3800mV-5080mV (step: 80mV)
status_t BQ24195_setInputVoltageLimit(BQ24195_t* dev, uint16_t limit_mV)
{
    if (limit_mV < 3880) limit_mV=3880; else
    if (limit_mV > 5080) limit_mV=5080;
    limit_mV = (limit_mV - 3880) / 80;
    limit_mV <<= BQ24195_REG00_VINDPM_SHIFT;

    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG00, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG00_VINDPM_MASK;
    regData |= limit_mV;
    return BQ24195_writeReg(dev, BQ24195_REG00, regData);
}
//------------------------------------------------------------------------------
//Set input current limit.
status_t BQ24195_setInputCurrentLimit(BQ24195_t* dev,
                                      BQ24195_inputCurrentLimit_t limit)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG00, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG00_INLIM_MASK;
    regData |= limit;
    return BQ24195_writeReg(dev, BQ24195_REG00, regData);
}
//------------------------------------------------------------------------------
//Reset
status_t BQ24195_reset(BQ24195_t* dev)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG01, &regData);
    if (status) return status;
    regData |= BQ24195_REG01_REG_RESET;
    status=BQ24195_writeReg(dev, BQ24195_REG01, regData);
    if (status) return status;

    //pici varakozas, hogy az eszkoz reseteljen
    vTaskDelay(1);

    return status;
}
//------------------------------------------------------------------------------
//I2C Watchdog timer reset
status_t BQ24195_resetWatchdog(BQ24195_t* dev)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG01, &regData);
    if (status) return status;
    regData |= BQ24195_REG01_WDOG_RESET;
    return BQ24195_writeReg(dev, BQ24195_REG01, regData);
}
//------------------------------------------------------------------------------
//Set mimium system voltage
//valid range: 3000mV-3700mV (step: 100mV)
status_t BQ24195_setMinimumSysVoltage(BQ24195_t* dev,
                                      uint16_t minSys_mV)
{
    if (minSys_mV < 3000) minSys_mV=3000; else
    if (minSys_mV > 3700) minSys_mV=3700;
    minSys_mV = (minSys_mV - 3000) / 100;
    minSys_mV <<= BQ24195_REG01_SYS_MINV_SHIFT;

    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG01, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG01_SYS_MINV_MASK;
    regData |= minSys_mV;
    return BQ24195_writeReg(dev, BQ24195_REG01, regData);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//TODO:  BIT5 es BIT4 nincs meg kezelve!!!!
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Set charge current
//valid range: 512mA-4544mA (step: 64mA)
status_t BQ24195_setChargeCurrent(BQ24195_t* dev,
                                  uint16_t chargeCurrent_mA)
{
    if (chargeCurrent_mA <  512) chargeCurrent_mA=512; else
    if (chargeCurrent_mA > 4544) chargeCurrent_mA=4544;
    chargeCurrent_mA = (chargeCurrent_mA - 512) / 64;
    chargeCurrent_mA <<= BQ24195_REG02_ICHG_SHIFT;

    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG02, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG02_ICHG_MASK;
    regData |= chargeCurrent_mA;
    return BQ24195_writeReg(dev, BQ24195_REG02, regData);
}
//------------------------------------------------------------------------------
//set 20% current limit.
status_t BQ24195_setForce_20PCT(BQ24195_t* dev, bool _20ptc)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG02, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG02_FORCE_20PCT;
    if (_20ptc) regData |=BQ24195_REG02_FORCE_20PCT;
    return BQ24195_writeReg(dev, BQ24195_REG02, regData);
}
//------------------------------------------------------------------------------
//Set precharge current
//valid range: 128mA-2048mA (step: 128mA)
status_t BQ24195_setPreChargeCurrent(BQ24195_t* dev,
                                     uint16_t preChargeCurrent_mA)
{
    if (preChargeCurrent_mA <  128) preChargeCurrent_mA=128; else
    if (preChargeCurrent_mA > 2048) preChargeCurrent_mA=2048;
    preChargeCurrent_mA = (preChargeCurrent_mA - 512) / 128;
    preChargeCurrent_mA <<= BQ24195_REG03_IPRECHG_SHIFT;

    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG03, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG03_IPRECHG_MASK;
    regData |= preChargeCurrent_mA;
    return BQ24195_writeReg(dev, BQ24195_REG03, regData);
}
//------------------------------------------------------------------------------
//Set termination current
//valid range: 128mA-2048mA (step: 128mA)
status_t BQ24195_setPrechargeCurrent(BQ24195_t* dev,
                                     uint16_t prechargeCurrent_mA)
{
    if (prechargeCurrent_mA <  128) prechargeCurrent_mA=128; else
    if (prechargeCurrent_mA > 2048) prechargeCurrent_mA=2048;
    prechargeCurrent_mA = (prechargeCurrent_mA - 512) / 128;
    prechargeCurrent_mA <<= BQ24195_REG03_IPRECHG_SHIFT;

    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG03, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG03_IPRECHG_MASK;
    regData |= prechargeCurrent_mA;
    return BQ24195_writeReg(dev, BQ24195_REG03, regData);
}
//------------------------------------------------------------------------------
//Set voltage
//valid range: 3504-4400mA (step: 16mV)
status_t BQ24195_setVoltage(BQ24195_t* dev, uint16_t voltage_mV)
{
    if (voltage_mV < 3504) voltage_mV=3504; else
    if (voltage_mV > 4400) voltage_mV=4400;
    voltage_mV = (voltage_mV - 3504) / 16;
    voltage_mV <<= BQ24195_REG04_VREG_SHIFT;

    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG04, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG04_VREG_MASK;
    regData |= voltage_mV;
    return BQ24195_writeReg(dev, BQ24195_REG04, regData);
}
//------------------------------------------------------------------------------
//Set Battery Precharge to Fast Charge Threshold
status_t BQ24195_setPrechargeToFastChargeThreshold(BQ24195_t* dev,
                                                   BQ24195_battLowVoltage_t v)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG04, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG04_BATLOWV;
    regData |= v;
    return BQ24195_writeReg(dev, BQ24195_REG04, regData);
}
//------------------------------------------------------------------------------
//Set Battery Recharge Threshold (below battery regulation voltage)
status_t BQ24195_setRechargeThreshold(BQ24195_t* dev,
                                      BQ24195_rechargeVoltage_t v)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG04, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG04_VRECHG;
    regData |= v;
    return BQ24195_writeReg(dev, BQ24195_REG04, regData);
}
//------------------------------------------------------------------------------
//enable/disable charging termination
status_t BQ24195_setChargingTermination(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG05, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG05_EN_TERM;
    if (enable) regData |=BQ24195_REG05_EN_TERM;
    return BQ24195_writeReg(dev, BQ24195_REG05, regData);
}
//------------------------------------------------------------------------------
//Set Termination Indicator Threshold
status_t BQ24195_setTerminationIndicator(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG05, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG05_TERM_STAT;
    if (enable) regData |=BQ24195_REG05_TERM_STAT;
    return BQ24195_writeReg(dev, BQ24195_REG05, regData);
}
//------------------------------------------------------------------------------
//Set I2C watchdog timer
status_t BQ24195_setWatchdogTimer(BQ24195_t* dev,
                                  BQ24195_watchdogTime_t time)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG05, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG05_WATCHDOG_MASK;
    regData |= time;
    return BQ24195_writeReg(dev, BQ24195_REG05, regData);
}
//------------------------------------------------------------------------------
//Enable/disable safety timer
status_t BQ24195_setSafetyTimer(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG05, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG05_EN_TIMER;
    if (enable) regData |=BQ24195_REG05_EN_TIMER;
    return BQ24195_writeReg(dev, BQ24195_REG05, regData);
}
//------------------------------------------------------------------------------
//Set fast charge timer
status_t BQ24195_setFastChargeTimer(BQ24195_t* dev,
                                    BQ24195_fastChargeTime_t time)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG05, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG05_CHG_TIMER_MASK;
    regData |= time;
    return BQ24195_writeReg(dev, BQ24195_REG05, regData);
}
//------------------------------------------------------------------------------
//set Thermal Regulation Threshold
status_t BQ24195_setThermalRegulationThreshold(BQ24195_t* dev,
                                    BQ24195_thermalRegulationThreshold_t th)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG06, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG06_TREG_MASK;
    regData |= th;
    return BQ24195_writeReg(dev, BQ24195_REG06, regData);
}
//------------------------------------------------------------------------------
//Force DPDM detection
status_t BQ24195_setDpdmDetection(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG07, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG07_DPDM_EN;
    if (enable) regData |=BQ24195_REG07_DPDM_EN;
    return BQ24195_writeReg(dev, BQ24195_REG07, regData);
}
//------------------------------------------------------------------------------
//Safety Timer Setting during Input DPM and Thermal Regulation
status_t BQ24195_setSafetyTimer2x(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG07, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG07_TMR2X_EN;
    if (enable) regData |=BQ24195_REG07_TMR2X_EN;
    return BQ24195_writeReg(dev, BQ24195_REG07, regData);
}
//------------------------------------------------------------------------------
//Force BATFET Off
status_t BQ24195_batFetDisable(BQ24195_t* dev, bool disable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG07, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG07_BATFET_DISABLE;
    if (disable) regData |=BQ24195_REG07_BATFET_DISABLE;
    return BQ24195_writeReg(dev, BQ24195_REG07, regData);
}
//------------------------------------------------------------------------------
//INT on CHRG_FAULT enable
status_t BQ24195_setChargeFaultInterrupt(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG07, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG07_INT_MASK1;
    if (enable) regData |=BQ24195_REG07_INT_MASK1;
    return BQ24195_writeReg(dev, BQ24195_REG07, regData);
}
//------------------------------------------------------------------------------
//INT on BAT_FAULT enable
status_t BQ24195_setBatteryFaultInterrupt(BQ24195_t* dev, bool enable)
{
    status_t status;
    uint8_t regData;
    status=BQ24195_readReg(dev, BQ24195_REG07, &regData);
    if (status) return status;
    regData &= ~BQ24195_REG07_INT_MASK0;
    if (enable) regData |=BQ24195_REG07_INT_MASK0;
    return BQ24195_writeReg(dev, BQ24195_REG07, regData);
}
//------------------------------------------------------------------------------
//Read system status
status_t BQ24195_readSystemStatus(BQ24195_t* dev,
                                  BQ24195_systemStatus_t* sysStatus)
{
    status_t status;
    return BQ24195_readReg(dev, BQ24195_REG08, (uint8_t*) sysStatus);
}
//------------------------------------------------------------------------------
//Read fault status
status_t BQ24195_readFaultInfo(BQ24195_t* dev,
                               BQ24195_faultInfo_t* faultInfo)
{
    status_t status;
    return BQ24195_readReg(dev, BQ24195_REG09, (uint8_t*) faultInfo);
}
//------------------------------------------------------------------------------
