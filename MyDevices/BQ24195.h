//------------------------------------------------------------------------------
//  BQ24195 Li-Ion akkumulator tolto
//
//    File: BQ24195.h
//------------------------------------------------------------------------------
#ifndef BQ24195_H_
#define BQ24195_H_

#include "MyI2CM.h"

#define BQ24195_I2C_ADDRESS     0x6B

#define BQ24195_REG00     0x00
#define BQ24195_REG01     0x01
#define BQ24195_REG02     0x02
#define BQ24195_REG03     0x03
#define BQ24195_REG04     0x04
#define BQ24195_REG05     0x05
#define BQ24195_REG06     0x06
#define BQ24195_REG07     0x07
#define BQ24195_REG08     0x08
#define BQ24195_REG09     0x09
#define BQ24195_REG0A     0x0a

#define BQ24195_REG00_EN_HIZ               (1 << 7)

#define BQ24195_REG00_VINDPM_SHIFT          3
#define BQ24195_REG00_VINDPM_MASK           (0xf << BQ24195_REG00_VINDPM_SHIFT)

#define BQ24195_REG00_INLIM_SHIFT           0
#define BQ24195_REG00_INLIM_MASK            (0x7 << BQ24195_REG00_INLIM_SHIFT)

typedef enum
{
    BQ24195_inputCurrentLimit_100mA  = (0x0 << BQ24195_REG00_INLIM_SHIFT),
    BQ24195_inputCurrentLimit_150mA  = (0x1 << BQ24195_REG00_INLIM_SHIFT),
    BQ24195_inputCurrentLimit_500mA  = (0x2 << BQ24195_REG00_INLIM_SHIFT),
    BQ24195_inputCurrentLimit_900mA  = (0x3 << BQ24195_REG00_INLIM_SHIFT),
    BQ24195_inputCurrentLimit_1000mA = (0x4 << BQ24195_REG00_INLIM_SHIFT),
    BQ24195_inputCurrentLimit_1500mA = (0x5 << BQ24195_REG00_INLIM_SHIFT),
    BQ24195_inputCurrentLimit_2000mA = (0x6 << BQ24195_REG00_INLIM_SHIFT),
    BQ24195_inputCurrentLimit_3000mA = (0x7 << BQ24195_REG00_INLIM_SHIFT),
} BQ24195_inputCurrentLimit_t;

#define BQ24195_REG01_REG_RESET             (1 << 7)

#define BQ24195_REG01_WDOG_RESET            (1 << 6)

#define BQ24195_REG01_SYS_MINV_SHIFT        1
#define BQ24195_REG01_SYS_MINV_MASK         (7 << BQ24195_REG01_SYS_MINV_SHIFT)

#define BQ24195_REG01_OTG_CONFIG            (1 << 5)

#define BQ24195_REG01_CHG_CONFIG            (1 << 4)

#define BQ24195_REG02_ICHG_SHIFT            2
#define BQ24195_REG02_ICHG_MASK             (0x3f << BQ24195_REG02_ICHG_SHIFT)

#define BQ24195_REG02_FORCE_20PCT           (1 << 0)

#define BQ24195_REG03_IPRECHG_SHIFT         4
#define BQ24195_REG03_IPRECHG_MASK          (0xf << BQ24195_REG03_IPRECHG_SHIFT)

#define BQ24195_REG04_VREG_SHIFT            2
#define BQ24195_REG04_VREG_MASK             (0x3f << BQ24195_REG04_VREG_SHIFT)

#define BQ24195_REG04_BATLOWV               (1 << 1)
#define BQ24195_REG04_VRECHG                (1 << 0)

typedef enum
{
    BQ24195_battLowVoltage_2800mV=0,
    BQ24195_battLowVoltage_3000mV=BQ24195_REG04_BATLOWV,
} BQ24195_battLowVoltage_t;

typedef enum
{
    BQ24195_rechargeVoltage_100mV=0,
    BQ24195_rechargeVoltage_300mV=BQ24195_REG04_VRECHG,
} BQ24195_rechargeVoltage_t;

#define BQ24195_REG05_EN_TERM               (1 << 7)

#define BQ24195_REG05_TERM_STAT             (1 << 6)

#define BQ24195_REG05_WATCHDOG_SHIFT        4
#define BQ24195_REG05_WATCHDOG_MASK         (3 << BQ24195_REG05_WATCHDOG_SHIFT)

typedef enum
{
    BQ24195_watchdogTime_disable = (0 << BQ24195_REG05_WATCHDOG_SHIFT),
    BQ24195_watchdogTime_40sec   = (1 << BQ24195_REG05_WATCHDOG_SHIFT),
    BQ24195_watchdogTime_80sec   = (2 << BQ24195_REG05_WATCHDOG_SHIFT),
    BQ24195_watchdogTime_160sec  = (3 << BQ24195_REG05_WATCHDOG_SHIFT),
} BQ24195_watchdogTime_t;

#define BQ24195_REG05_EN_TIMER              (1 << 3)

#define BQ24195_REG05_CHG_TIMER_SHIFT       1
#define BQ24195_REG05_CHG_TIMER_MASK        (3 << BQ24195_REG05_CHG_TIMER_SHIFT)

typedef enum
{
    BQ24195_fastChargeTime_5h  = (0 << BQ24195_REG05_CHG_TIMER_SHIFT),
    BQ24195_fastChargeTime_8h  = (1 << BQ24195_REG05_CHG_TIMER_SHIFT),
    BQ24195_fastChargeTime_12h = (2 << BQ24195_REG05_CHG_TIMER_SHIFT),
    BQ24195_fastChargeTime_20h = (3 << BQ24195_REG05_CHG_TIMER_SHIFT)
} BQ24195_fastChargeTime_t;


#define BQ24195_REG06_TREG_SHIFT            0
#define BQ24195_REG06_TREG_MASK             (3 << BQ24195_REG06_TREG_SHIFT)
typedef enum
{
    BQ24195_thermalRegulationThreshold_60C  = (0 << BQ24195_REG06_TREG_SHIFT),
    BQ24195_thermalRegulationThreshold_80C  = (1 << BQ24195_REG06_TREG_SHIFT),
    BQ24195_thermalRegulationThreshold_100C = (2 << BQ24195_REG06_TREG_SHIFT),
    BQ24195_thermalRegulationThreshold_110C = (3 << BQ24195_REG06_TREG_SHIFT),
} BQ24195_thermalRegulationThreshold_t;


#define BQ24195_REG07_DPDM_EN               (1 << 7)

#define BQ24195_REG07_TMR2X_EN              (1 << 6)

#define BQ24195_REG07_BATFET_DISABLE        (1 << 5)

#define BQ24195_REG07_INT_MASK1             (1 << 1)

#define BQ24195_REG07_INT_MASK0             (1 << 0)


typedef enum
{
    //00 – Unknown (no input, or DPDM detection incomplete)
    BQ24195_vBusStat_unknown=0,
    //01 – USB host
    BQ24195_vBusStat_UsbHost=1,
    //10 – Adapterport
    BQ24195_vBusStat_adapter=2,
    //11 – OTG
    BQ24195_vBusStat_OTG=3
} BQ24195_vBusStat_t;

typedef enum
{
    //00 – Not Charging
    BQ24195_chrgStat_notCharging=0,
    //01 – Pre-charge (<VBATLOWV)
    BQ24195_chrgStat_preCharge=1,
    //10 – Fast Charging
    BQ24195_chrgStat_fastCharging=2,
    //11 – Charge Termination Done
    BQ24195_chrgStat_chargeTerminationDone=3
} BQ24195_chrgStat_t;

typedef union
{

    struct
    {
        //0 – Not in VSYSMIN regulation (BAT > VSYSMIN),
        //1 – In VSYSMIN regulation (BAT < VSYSMIN)
        uint8_t vSysStat            :1;
        //0 – Normal, 1 – In Thermal Regulation
        uint8_t thermalRegulation   :1;
        //0 – Not Power Good, 1 – Power Good
        uint8_t powerGood           :1;
        //0 – Not DPM, 1 – VINDPM or IINDPM
        uint8_t dpmStat             :1;
        //BQ24195_chrgStat_t
        BQ24195_chrgStat_t chrgStat :2;
        //BQ24195_vBusStat_t
        BQ24195_vBusStat_t vbusStat :2;
    };
    uint8_t reg;
} BQ24195_systemStatus_t;




typedef enum
{
    //00 – Normal
    BQ24195_chrgFault_normal=0,
    //01 – Input fault (VBUS OVP or VBAT < VBUS < 3.8 V)
    BQ24195_chrgFault_inputFault=1,
    //10 - Thermal shutdown
    BQ24195_chrgFault_thermalShutdown=2,
    //11 – Charge Safety Timer Expiration
    BQ24195_chrgFault_safetyTimerExpiration=3,
} BQ24195_chrgFault_t;

typedef union
{
    struct
    {
        //000 – Normal, 101 – Cold, 110 – Hot
        uint8_t ntcFault            :3;
        //0 – Normal, 1 – BATOVP
        uint8_t batteryFault        :1;
        //BQ24195_chrgFault_t
        BQ24195_chrgFault_t chrgFault :2;
        //Reserved
        uint8_t reserved1           :1;
        //0 – Normal, 1- Watchdog timer expiration
        uint8_t watchdogFault       :1;
    };
    uint8_t reg;
} BQ24195_faultInfo_t;


//------------------------------------------------------------------------------
//BQ24195 driver valtozoi
typedef struct
{
    //Az eszkoz I2C-s eleresehez tartozo jellemzok. (busz, slave cim, stb...)
    MyI2CM_Device_t i2cDevice;
} BQ24195_t;
//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void BQ24195_create(BQ24195_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress);

//Az IC egyetlen regiszterenek irasa
status_t BQ24195_writeReg(BQ24195_t* dev, uint8_t regAddress, uint8_t value);

//Az IC tobb regiszterenek olvasasa
status_t BQ24195_readReg(BQ24195_t* dev, uint8_t regAddress, uint8_t* value);

//Set HiZ state
status_t BQ24195_setHiZ(BQ24195_t* dev, bool enable);

//Set input voltage limit.
//valid range: 3800mV-5080mV (step: 80mV)
status_t BQ24195_setInputVoltageLimit(BQ24195_t* dev, uint16_t limit_mV);

//Set input current limit.
status_t BQ24195_setInputCurrentLimit(BQ24195_t* dev,
                                      BQ24195_inputCurrentLimit_t limit);
//Reset
status_t BQ24195_reset(BQ24195_t* dev);

//I2C Watchdog timer reset
status_t BQ24195_resetWatchdog(BQ24195_t* dev);

//Set mimium system voltage
//valid range: 3000mV-3700mV (step: 100mV)
status_t BQ24195_setMinimumSysVoltage(BQ24195_t* dev,
                                      uint16_t minSys_mV);

//Set charge current
//valid range: 512mA-4544mA (step: 64mA)
status_t BQ24195_setChargeCurrent(BQ24195_t* dev,
                                  uint16_t chargeCurrent_mA);

//set 20% current limit.
status_t BQ24195_setForce_20PCT(BQ24195_t* dev, bool _20ptc);

//Set precharge current
//valid range: 128mA-2048mA (step: 128mA)
status_t BQ24195_setPreChargeCurrent(BQ24195_t* dev,
                                     uint16_t preChargeCurrent_mA);

//Set termination current
//valid range: 128mA-2048mA (step: 128mA)
status_t BQ24195_setPrechargeCurrent(BQ24195_t* dev,
                                     uint16_t prechargeCurrent_mA);

//Set voltage
//valid range: 3504-4400mA (step: 16mV)
status_t BQ24195_setVoltage(BQ24195_t* dev, uint16_t voltage_mV);

//Set Battery Precharge to Fast Charge Threshold
status_t BQ24195_setPrechargeToFastChargeThreshold(BQ24195_t* dev,
                                                   BQ24195_battLowVoltage_t v);

//Set Battery Recharge Threshold (below battery regulation voltage)
status_t BQ24195_setRechargeThreshold(BQ24195_t* dev,
                                      BQ24195_rechargeVoltage_t v);

//enable/disable charging termination
status_t BQ24195_setChargingTermination(BQ24195_t* dev, bool enable);

//Set Termination Indicator Threshold
status_t BQ24195_setTerminationIndicator(BQ24195_t* dev, bool enable);

//Set I2C watchdog timer
status_t BQ24195_setWatchdogTimer(BQ24195_t* dev,
                                  BQ24195_watchdogTime_t time);

//Enable/disable safety timer
status_t BQ24195_setSafetyTimer(BQ24195_t* dev, bool enable);

//Set fast charge timer
status_t BQ24195_setFastChargeTimer(BQ24195_t* dev,
                                    BQ24195_fastChargeTime_t time);

//set Thermal Regulation Threshold
status_t BQ24195_setThermalRegulationThreshold(BQ24195_t* dev,
                                    BQ24195_thermalRegulationThreshold_t th);

//Force DPDM detection
status_t BQ24195_setDpdmDetection(BQ24195_t* dev, bool enable);

//Safety Timer Setting during Input DPM and Thermal Regulation
status_t BQ24195_setSafetyTimer2x(BQ24195_t* dev, bool enable);

//Force BATFET Off
status_t BQ24195_batFetDisable(BQ24195_t* dev, bool disable);

//INT on CHRG_FAULT enable
status_t BQ24195_setChargeFaultInterrupt(BQ24195_t* dev, bool enable);

//INT on BAT_FAULT enable
status_t BQ24195_setBatteryFaultInterrupt(BQ24195_t* dev, bool enable);

//Read system status
status_t BQ24195_readSystemStatus(BQ24195_t* dev,
                                  BQ24195_systemStatus_t* sysStatus);
//Read fault status
status_t BQ24195_readFaultInfo(BQ24195_t* dev,
                               BQ24195_faultInfo_t* faultInfo);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //BQ24195_H_
