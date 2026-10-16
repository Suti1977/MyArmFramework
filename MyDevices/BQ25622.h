//-----------------------------------------------------------------------------
//  BQ25622 Li-Ion akkumulator tolto IC driver
//
//    File: BQ25622.h
//------------------------------------------------------------------------------
#ifndef BQ25622_H_
#define BQ25622_H_

#include <stdbool.h>
#include <stdint.h>
#include "MyI2CM.h"

//------------------------------------------------------------------------------
#define BQ25622_I2C_ADDRESS                 0x6B

#define BQ25622_REG_CHARGE_CURRENT_LSB      0x02
#define BQ25622_REG_CHARGE_CURRENT_MSB      0x03
#define BQ25622_REG_CHARGE_VOLTAGE_LSB      0x04
#define BQ25622_REG_CHARGE_VOLTAGE_MSB      0x05
#define BQ25622_REG_INPUT_CURRENT_LSB       0x06
#define BQ25622_REG_INPUT_CURRENT_MSB       0x07
#define BQ25622_REG_INPUT_VOLTAGE_LSB       0x08
#define BQ25622_REG_INPUT_VOLTAGE_MSB       0x09
#define BQ25622_REG_INPUT_IOTG_LSB          0x0a
#define BQ25622_REG_INPUT_IOTG_MSB          0x0b
#define BQ25622_REG_INPUT_VOTG_LSB          0x0c
#define BQ25622_REG_INPUT_VOTG_MSB          0x0d
#define BQ25622_REG_MIN_SYSTEM_VOLTAGE_LSB  0x0E
#define BQ25622_REG_MIN_SYSTEM_VOLTAGE_MSB  0x0F
#define BQ25622_REG_PRECHARGE_LSB           0x10
#define BQ25622_REG_PRECHARGE_MSB           0x11
#define BQ25622_REG_TERMINATION_LSB         0x12
#define BQ25622_REG_TERMINATION_MSB         0x13
#define BQ25622_REG_CHARGE_CONTROL_0        0x14
#define BQ25622_REG_CHARGE_TIMER_CONTROL    0x15
#define BQ25622_REG_CHARGER_CONTROL_1       0x16
#define BQ25622_REG_CHARGER_CONTROL_2       0x17
#define BQ25622_REG_CHARGER_CONTROL_3       0x18
#define BQ25622_REG_CHARGER_CONTROL_4       0x19
#define BQ25622_REG_NTC_CONTROL_0           0x1A
#define BQ25622_REG_NTC_CONTROL_1           0x1B
#define BQ25622_REG_NTC_CONTROL_2           0x1C

#define BQ25622_REG_CHARGER_STATUS_0        0x1D
#define BQ25622_REG_CHARGER_STATUS_1        0x1E
#define BQ25622_REG_FAULT_STATUS_0          0x1f
#define BQ25622_REG_CHARGER_FLAG_0          0x20
#define BQ25622_REG_CHARGER_FLAG_1          0x21
#define BQ25622_REG_FAULT_FLAG_0            0x22

#define BQ25622_REG_CHAREGR_MASK_0          0x23
#define BQ25622_REG_CHAREGR_MASK_1          0x24
#define BQ25622_REG_FAULT_MASK_0            0x25

#define BQ25622_REG_ADC_CONTROL             0x26
#define BQ25622_REG_ADC_FUNCTION_DISABLE_0  0x27
#define BQ25622_REG_IBUS_ADC_LSB            0x28
#define BQ25622_REG_IBUS_ADC_MSB            0x29
#define BQ25622_REG_IBAT_ADC_LSB            0x2A
#define BQ25622_REG_IBAT_ADC_MSB            0x2B
#define BQ25622_REG_VBUS_ADC_LSB            0x2C
#define BQ25622_REG_VBUS_ADC_MSB            0x2D
#define BQ25622_REG_VPMID_ADC_LSB           0x2E
#define BQ25622_REG_VPMID_ADC_MSB           0x2F
#define BQ25622_REG_VBAT_ADC_LSB            0x30
#define BQ25622_REG_VBAT_ADC_MSB            0x31
#define BQ25622_REG_VSYS_ADC_LSB            0x32
#define BQ25622_REG_VSYS_ADC_MSB            0x33
#define BQ25622_REG_TS_ADC_LSB              0x34
#define BQ25622_REG_TS_ADC_MSB              0x35
#define BQ25622_REG_TDIE_ADC_LSB            0x36
#define BQ25622_REG_TDIE_ADC_MSB            0x37
#define BQ25622_REG_PART_INFORMATION        0x38

#define BQ25622_ICHG_SHIFT                  6   // Adatlap szerint 6:11
#define BQ25622_ICHG_MASK                   (0x3F << BQ25622_ICHG_SHIFT)

#define BQ25622_VREG_SHIFT                  3
#define BQ25622_VREG_MASK                   (0x1FF << BQ25622_VREG_SHIFT)

#define BQ25622_IINDPM_SHIFT                4
#define BQ25622_IINDPM_MASK                 (0xFF << BQ25622_IINDPM_SHIFT)

#define BQ25622_VINDPM_SHIFT                5
#define BQ25622_VINDPM_MASK                 (0x1FF << BQ25622_VINDPM_SHIFT)

#define BQ25622_VSYSMIN_SHIFT               6
#define BQ25622_VSYSMIN_MASK                (0x3F << BQ25622_VSYSMIN_SHIFT)

#define BQ25622_IPRECHG_SHIFT               4
#define BQ25622_IPRECHG_MASK                (0x1F << BQ25622_IPRECHG_SHIFT)
#define BQ25622_ITERM_SHIFT                 3
#define BQ25622_ITERM_MASK                  (0x3F << BQ25622_ITERM_SHIFT)

#define BQ25622_CTRL0_VRECHG                1
#define BQ25622_CTRL0_VINDPM_BAT_TRACK      (1U << 1)
#define BQ25622_CTRL0_EN_TERM               (1U << 2)
#define BQ25622_CTRL0_TOPOFF_TMR_SHIFT      3
#define BQ25622_CTRL0_TOPOFF_TMR_MASK       (0x03 << BQ25622_CTRL0_TOPOFF_TMR_SHIFT)
#define BQ25622_CTRL0_ITRICKLE              (1U << 5)
#define BQ25622_CTRL0_Q4_FULLON             (1U << 6)
#define BQ25622_CTRL0_Q1_FULLON             (1U << 7)

#define BQ25622_TIMER_TS_DIS                (1U << 0)
#define BQ25622_TIMER_EN_AUTO_INDET         (1U << 1)
#define BQ25622_TIMER_FORCE_INDET           (1U << 2)
#define BQ25622_TIMER_EN_DCP_BIAS           (1U << 3)
#define BQ25622_TIMER_TMR2X_EN              (1U << 4)
#define BQ25622_TIMER_EN_SAFETY_TMR         (1U << 5)
#define BQ25622_TIMER_PRECHG_TMR            (1U << 6)
#define BQ25622_TIMER_CHG_TIMER             (1U << 7)

#define BQ25622_CTRL1_WATCHDOG_SHIFT        0
#define BQ25622_CTRL1_WATCHDOG_MASK         (0x03 << BQ25622_CTRL1_WATCHDOG_SHIFT)
#define BQ25622_CTRL1_WD_RST                (1U << 2)
#define BQ25622_CTRL1_FORCE_PMID_DIS        (1U << 3)
#define BQ25622_CTRL1_EN_HIZ                (1U << 4)
#define BQ25622_CTRL1_EN_CHG                (1U << 5)
#define BQ25622_CTRL1_FORCE_IBATDIS         (1U << 6)
#define BQ25622_CTRL1_EN_AUTO_IBATDIS       (1U << 7)

#define BQ25622_CTRL2_VBUS_OVP              (1U << 0)
#define BQ25622_CTRL2_CONV_STRN_SHIFT       2
#define BQ25622_CTRL2_CONV_STRN_MASK        (0x03 << BQ25622_CTRL2_CONV_STRN_SHIFT)
#define BQ25622_CTRL2_CONV_FREQ_SHIFT       4
#define BQ25622_CTRL2_CONV_FREQ_MASK        (0x03 << BQ25622_CTRL2_CONV_FREQ_SHIFT)
#define BQ25622_CTRL2_TREG                  (1U << 6)
#define BQ25622_CTRL2_REG_RST               (1U << 7)

#define BQ25622_CTRL4_CHG_RATE_SHIFT        0
#define BQ25622_CTRL4_CHG_RATE_MASK         (0x03U << BQ25622_CTRL4_CHG_RATE_SHIFT)
#define BQ25622_CTRL4_EN_EXTLIM             (1U << 2)
#define BQ25622_CTRL4_VBAT_UVLO             (1U << 5)
#define BQ25622_CTRL4_IBAT_PK_SHIFT         6
#define BQ25622_CTRL4_IBAT_PK_MASK          (0x03U << BQ25622_CTRL4_IBAT_PK_SHIFT)

#define BQ25622_NTC_COOL_SHIFT              0
#define BQ25622_NTC_COOL_MASK               (0x03U << BQ25622_NTC_COOL_SHIFT)
#define BQ25622_NTC_WARM_SHIFT              2
#define BQ25622_NTC_WARM_MASK               (0x03U << BQ25622_NTC_WARM_SHIFT)
#define BQ25622_NTC_TS_IGNORE               (1U << 7)

#define BQ25622_STATUS0_WD_STAT             (1U << 0)
#define BQ25622_STATUS0_SAFETY_TMR_STAT     (1U << 1)
#define BQ25622_STATUS0_VINDPM_STAT         (1U << 2)
#define BQ25622_STATUS0_IINDPM_STAT         (1U << 3)
#define BQ25622_STATUS0_VSYS_STAT           (1U << 4)
#define BQ25622_STATUS0_TREG_STAT           (1U << 5)
#define BQ25622_STATUS0_ADC_DONE_STAT       (1U << 6)
#define BQ25622_STATUS1_VBUS_STAT_SHIFT     0
#define BQ25622_STATUS1_VBUS_STAT_MASK      (0x07 << BQ25622_STATUS1_VBUS_STAT_SHIFT)
#define BQ25622_STATUS1_CHG_STAT_SHIFT      3
#define BQ25622_STATUS1_CHG_STAT_MASK       (0x03 << BQ25622_STATUS1_CHG_STAT_SHIFT)

#define BQ25622_FAULT_TS_STAT_MASK          0x07
#define BQ25622_FAULT_TSHUT_FAULT           (1U << 3)
#define BQ25622_FAULT_OTG_FAULT             (1U << 4)
#define BQ25622_FAULT_SYS_FAULT             (1U << 5)
#define BQ25622_FAULT_BAT_FAULT             (1U << 6)
#define BQ25622_FAULT_VBUS_FAULT            (1U << 7)

#define BQ25622_ADC_AVG_INIT                (1U << 2)
#define BQ25622_ADC_AVG                     (1U << 3)
#define BQ25622_ADC_SAMPLE_SHIFT            4
#define BQ25622_ADC_SAMPLE_MASK             (0x03 << BQ25622_ADC_SAMPLE_SHIFT)
#define BQ25622_ADC_CONV_RATE               (1U << 6)
#define BQ25622_ADC_CONV_START              (1U << 7)

#define BQ25622_ADC_DIS_VPMID               (1U << 0)
#define BQ25622_ADC_DIS_TDIE                (1U << 1)
#define BQ25622_ADC_DIS_TS                  (1U << 2)
#define BQ25622_ADC_DIS_VSYS                (1U << 3)
#define BQ25622_ADC_DIS_VBAT                (1U << 4)
#define BQ25622_ADC_DIS_VBUS                (1U << 5)
#define BQ25622_ADC_DIS_IBAT                (1U << 6)
#define BQ25622_ADC_DIS_IBUS                (1U << 7)

typedef struct
{
    MyI2CM_Device_t i2cDevice;
} BQ25622_t;
//------------------------------------------------------------------------------
// I2C eszkoz letrehozasa a rendszerben
void BQ25622_create(BQ25622_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress);

// Az IC regiszterenek 8 bites irasa
status_t BQ25622_writeReg(BQ25622_t* dev, uint8_t regAddress, uint8_t value);

// Az IC regiszterenek 8 bites olvasasa
status_t BQ25622_readReg(BQ25622_t* dev, uint8_t regAddress, uint8_t* value);

// Az IC regiszterenek 16 bites olvasasa
status_t BQ25622_read2(BQ25622_t* dev, uint8_t regLSB, uint16_t* value);

// Az IC regiszterenek 16 bites irasa
status_t BQ25622_write2(BQ25622_t* dev, uint8_t regLSB, uint16_t value);

// Chip szoftveres resetelese
status_t BQ25622_reset(BQ25622_t* dev);

// Tolto aram korlat beallitasa. [mA]
// Range: 80mA - 3520mA , step 80mA
status_t BQ25622_setICHG(BQ25622_t* dev, int current_mA);
status_t BQ25622_getICHG(BQ25622_t* dev, uint16_t* current_mA);

// Maximalis tolto feszultseg beallitasa
// Range: 3500mV - 4800mV, step: 10mV
status_t BQ25622_setVREG(BQ25622_t* dev, int mV);
status_t BQ25622_getVREG(BQ25622_t* dev, uint16_t* mV);

// Bemeneti aramkorlat beallitasa
// Range: 100mA - 3200mA, step: 20mA
status_t BQ25622_setIINDPM(BQ25622_t* dev, int mA);
status_t BQ25622_getIINDPM(BQ25622_t* dev, uint16_t* mA);

// Bemeneti feszultseg korlat beallitasa
// Range: 3800mV - 16800mV, step: 40mV
status_t BQ25622_setVINDPM(BQ25622_t* dev, int mV);
status_t BQ25622_getVINDPM(BQ25622_t* dev, uint16_t* mV);

// Mimimalis VSYS feszultseg beallitsa
// Range: 2560mV - 3840mV step: 80mA
status_t BQ25622_setVSYSMIN(BQ25622_t* dev, int mV);
status_t BQ25622_getVSYSMIN(BQ25622_t* dev, uint16_t* mV);

// Elotoltesi aram beallitasa
// Range: 20mA - 620mA. step: 20mA
status_t BQ25622_setIPRECHG(BQ25622_t* dev, int mA);
status_t BQ25622_getIPRECHG(BQ25622_t* dev, uint16_t* mA);

// Toltes kikapcsolasi aram beallitasa. Ha a tolto aram ez ala aszint ala megy,
// akkor a toltes kikapcsol.
// Range: 10mA - 620mA. step: 10mA
status_t BQ25622_setITERM(BQ25622_t* dev, int mA);
status_t BQ25622_getITERM(BQ25622_t* dev, uint16_t* mA);

status_t BQ25622_getChargeControl0(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getChargeTimerControl(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getChargerControl1(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getChargerControl2(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getChargerControl4(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getNtcControl0(BQ25622_t* dev, uint8_t* value);


status_t BQ25622_getChargerStatus0Reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getChargerStatus1Reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getFaultStatus0Reg(BQ25622_t* dev, uint8_t* value);

status_t BQ25622_getAdcControl(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getAdcFunctionDisable0(BQ25622_t* dev, uint8_t* value);

status_t BQ25622_setTS_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setEN_AUTO_INDET(BQ25622_t* dev, bool value);
status_t BQ25622_setFORCE_INDET(BQ25622_t* dev, bool value);
status_t BQ25622_setEN_DCP_BIAS(BQ25622_t* dev, bool value);
status_t BQ25622_setTMR2X_EN(BQ25622_t* dev, bool value);
status_t BQ25622_setEN_SAFETY_TMR(BQ25622_t* dev, bool value);
status_t BQ25622_setPRECHG_TMR(BQ25622_t* dev, bool value);
status_t BQ25622_setCHG_TIMER(BQ25622_t* dev, bool value);
status_t BQ25622_setQ1_FULLON(BQ25622_t* dev, bool value);
status_t BQ25622_setQ4_FULLON(BQ25622_t* dev, bool value);
status_t BQ25622_setTOPOFF_TMR(BQ25622_t* dev, int value);
status_t BQ25622_setEN_CHG(BQ25622_t* dev, bool value);

typedef enum
{
    BQ25622_watchdog_disabled =0,
    BQ25622_watchdog_50s      =1,
    BQ25622_watchdog_100s     =2,
    BQ25622_watchdog_200s     =3
} BQ25622_watchdog_t;

status_t BQ25622_setWATCHDOG(BQ25622_t* dev, BQ25622_watchdog_t value);

status_t BQ25622_setVBUS_OVP(BQ25622_t* dev, int value);
status_t BQ25622_setCONV_STRN(BQ25622_t* dev, int value);
status_t BQ25622_setCONV_FREQ(BQ25622_t* dev, int value);
status_t BQ25622_setTREG(BQ25622_t* dev, int value);
status_t BQ25622_setREG_RST(BQ25622_t* dev, bool value);

// Enable the external ILIM pin input current regulation
// 0b = Disabled
// 1b = Enabled (default)
status_t BQ25622_setEN_EXTLIM(BQ25622_t* dev, bool value);

status_t BQ25622_setTS_IGNORE(BQ25622_t* dev, bool value);
status_t BQ25622_setCONV_START(BQ25622_t* dev, bool value);
status_t BQ25622_setCONV_RATE(BQ25622_t* dev, bool value);
status_t BQ25622_setADC_SAMPLE(BQ25622_t* dev, int value);
status_t BQ25622_setADC_AVG(BQ25622_t* dev, bool value);
status_t BQ25622_setADC_AVG_INIT(BQ25622_t* dev, bool value);

status_t BQ25622_setVPMID_ADC_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setTDIE_ADC_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setTS_ADC_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setVSYS_ADC_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setVBAT_ADC_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setVBUS_ADC_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setIBAT_ADC_DIS(BQ25622_t* dev, bool value);
status_t BQ25622_setIBUS_ADC_DIS(BQ25622_t* dev, bool value);

status_t BQ25622_getWD_STAT(BQ25622_t* dev, bool* value);
status_t BQ25622_getSAFETY_TMR_STAT(BQ25622_t* dev, bool* value);
status_t BQ25622_getVINDPM_STAT(BQ25622_t* dev, bool* value);
status_t BQ25622_getIINDPM_STAT(BQ25622_t* dev, bool* value);
status_t BQ25622_getVSYS_STAT(BQ25622_t* dev, bool* value);
status_t BQ25622_getTREG_STAT(BQ25622_t* dev, bool* value);
status_t BQ25622_getADC_DONE_STAT(BQ25622_t* dev, bool* value);
status_t BQ25622_getVBUS_STAT(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getCHG_STAT(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getTS_STAT(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getTSHUT_FAULT(BQ25622_t* dev, bool* value);
status_t BQ25622_getOTG_FAULT(BQ25622_t* dev, bool* value);
status_t BQ25622_getSYS_FAULT(BQ25622_t* dev, bool* value);
status_t BQ25622_getBAT_FAULT(BQ25622_t* dev, bool* value);
status_t BQ25622_getVBUS_FAULT(BQ25622_t* dev, bool* value);

status_t BQ25622_getICHG_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getVREG_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getIINDPM_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getVINDPM_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getVSYSMIN_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getIPRE_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getITERM_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getCHG_TIMER_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getCTRL0_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getCTRL1_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getCTRL2_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getCTRL4_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getNTC_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getCHRG_STATUS_0_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getCHRG_STATUS_1_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getFAULT_STATUS_0_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getADC_CTRL_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getADC_DIS_reg(BQ25622_t* dev, uint8_t* value);
status_t BQ25622_getIBAT_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getVBUSV_reg(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getBATV_reg(BQ25622_t* dev, uint16_t* value);

status_t BQ25622_getIBAT(BQ25622_t* dev, int16_t* value);
status_t BQ25622_getVBUSV(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getBATV(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getIBUS(BQ25622_t* dev, int16_t* value);
status_t BQ25622_getVPMID(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getVSYS(BQ25622_t* dev, uint16_t* value);
status_t BQ25622_getTS(BQ25622_t* dev, float* value);
status_t BQ25622_getTDIE(BQ25622_t* dev, float* value);


typedef enum
{
    BQ25622_tricleCurrent_20mA=0,
    BQ25622_tricleCurrent_80mA=1,
} BQ25622_tricleCurrent_t;

status_t BQ25622_setTrickleCurrent(BQ25622_t* dev, BQ25622_tricleCurrent_t tc);
status_t BQ25622_getTrickleCurrent(BQ25622_t* dev, BQ25622_tricleCurrent_t* tc);

status_t BQ25622_setEnableTermination(BQ25622_t* dev, bool enable);
status_t BQ25622_getEnableTermination(BQ25622_t* dev, bool* enable);

status_t BQ25622_setVINDPMbatTrack(BQ25622_t* dev, bool enable);
status_t BQ25622_getVINDPMbatTrack(BQ25622_t* dev, bool* enable);

status_t BQ25622_setAutoBatteryDischarge(BQ25622_t* dev, bool enable);
status_t BQ25622_getAutoBatteryDischarge(BQ25622_t* dev, bool* enable);

status_t BQ25622_setForceBatteryDischarge(BQ25622_t* dev, bool enable);
status_t BQ25622_getForceBatteryDischarge(BQ25622_t* dev, bool* enable);

status_t BQ25622_setHighZ(BQ25622_t* dev, bool enable);
status_t BQ25622_getHighZ(BQ25622_t* dev, bool* enable);

status_t BQ25622_setForcePMIDDischarge(BQ25622_t* dev, bool enable);
status_t BQ25622_getForcePMIDDischarge(BQ25622_t* dev, bool* enable);

status_t BQ25622_resetWatchdog(BQ25622_t* dev);

typedef enum
{
    BQ25622_peakDischargeCurrent_6A  = 2,
    BQ25622_peakDischargeCurrent_12A = 3,
} BQ25622_peakDischargeCurrent_t;
status_t BQ25622_setPeakBattDischarge(BQ25622_t* dev,
                                      BQ25622_peakDischargeCurrent_t peak);
status_t BQ25622_getPeakBattDischarge(BQ25622_t* dev,
                                      BQ25622_peakDischargeCurrent_t* peak);

typedef enum
{
    // 0b = VBAT_UVLO 2.2V, VBAT_SHORT 2.05V (default)
    BQ25622_UVLO_2_2V  = 0,
    // 1b = VBAT_UVLO 1.8V, VBAT_SHORT 1.85V
    BQ25622_UVLO_1_8V  = 1,
} BQ25622_UVLO_t;
status_t BQ25622_setVBatUVLO(BQ25622_t* dev, BQ25622_UVLO_t lowThreshold);
status_t BQ25622_getVBatUVLO(BQ25622_t* dev, BQ25622_UVLO_t* lowThreshold);


typedef enum
{
    BQ25622_chargeRate_1C = 0,
    BQ25622_chargeRate_2C = 1,
    BQ25622_chargeRate_4C = 2,
    BQ25622_chargeRate_6C = 3,
} BQ25622_chargeRate_t;

status_t BQ25622_setChargeRate(BQ25622_t* dev, BQ25622_chargeRate_t rate);
status_t BQ25622_getChargeRate(BQ25622_t* dev, BQ25622_chargeRate_t* rate);

status_t BQ25622_setCoolThermistorCurrent(BQ25622_t* dev, uint8_t setting);
status_t BQ25622_getCoolThermistorCurrent(BQ25622_t* dev, uint8_t* setting);
status_t BQ25622_setWarmThermistorCurrent(BQ25622_t* dev, uint8_t setting);
status_t BQ25622_getWarmThermistorCurrent(BQ25622_t* dev, uint8_t* setting);

status_t BQ25622_setADCEnable(BQ25622_t* dev, bool enable);
status_t BQ25622_getADCEnable(BQ25622_t* dev, bool* enable);
status_t BQ25622_setADCOneShot(BQ25622_t* dev, bool oneShot);
status_t BQ25622_getADCOneShot(BQ25622_t* dev, bool* oneShot);
status_t BQ25622_getADCSampleRate(BQ25622_t* dev, uint8_t* sampleRate);

status_t BQ25622_setEnableCharging(BQ25622_t* dev, bool enable);
status_t BQ25622_getEnableCharging(BQ25622_t* dev, bool* enable);

status_t BQ25622_setWatchdog(BQ25622_t* dev, uint8_t setting);
status_t BQ25622_getWatchdog(BQ25622_t* dev, uint8_t* setting);
status_t BQ25622_getThermalRegulation(BQ25622_t* dev, bool* temp120C);
status_t BQ25622_getConverterFrequency(BQ25622_t* dev, uint8_t* frequency);
status_t BQ25622_getVBUSOvervoltage(BQ25622_t* dev, bool* highThreshold);
status_t BQ25622_getIgnoreThermistor(BQ25622_t* dev, bool* ignore);

#pragma pack(1)
typedef union
{
    struct
    {
        // I2C watch dog timer status
        // 0b = Normal
        // 1b = WD timer expired
        uint8_t wd          :1;

        // Fast charge, trickle charge and pre-charge timer status
        // 0b = Normal
        // 1b = Safety timer expired
        uint8_t safetyTmr   :1;

        // VINDPM status (forward mode) or VOTG status (OTG
        // mode, backup mode)
        // 0b = Normal
        // 1b = In VINDPM regulation or VOTG regulation
        uint8_t vindpm      :1;

        // In forward mode, indicates that either IINDPM
        // regulation is active or ILIM pin regulation is active
        // In OTG mode, indicates that IOTG regulation is active
        // 0b = Normal
        // 1b = In IINDPM/ILIM regulation or IOTG regulation
        uint8_t iindpm      :1;

        // VSYS Regulation Status (forward mode)
        // 0b = Not in VSYSMIN regulation (BAT>VSYSMIN)
        // 1b = In VSYSMIN regulation (BAT<VSYSMIN)
        uint8_t vsys        :1;

        // IC Thermal regulation status
        // 0b = Normal
        // 1b = Device in thermal regulation
        uint8_t treg        :1;

        // ADC Conversion Status (in one-shot mode only)
        // Note: Always reads 0 in continuous mode
        // 0b = Conversion not complete
        // 1b = Conversion complete
        uint8_t adcDone     :1;

        uint8_t reserved    :1;
    };
    uint8_t raw;
} BQ25622_chargerStatus0_t;
#pragma pack()

status_t BQ25622_getChargerStatus0(BQ25622_t* dev,
                                   BQ25622_chargerStatus0_t* stat);

// Charge Status bits
// 00b = Not Charging or Charge Terminated
// 01b = Trickle Charge, Pre-charge or Fast charge (CC mode)
// 10b = Taper Charge (CV mode)
// 11b = Top-off Timer Active Charging
typedef enum
{
    BQ25622_chgStatus_notCharging = 0,
    BQ25622_chgStatus_cc          = 1,
    BQ25622_chgStatus_cv          = 2,
    BQ25622_chgStatus_topOff      = 3,
} BQ25622_chgStatus_t;

// VBUS status bits
// 000b = Not powered from VBUS
// 100b = Unknown Adapter (default IINDPM setting)
// 111b = In boost OTG mode
typedef enum
{
    BQ25622_vBusStatus_notPowered = 0,
    BQ25622_vBusStatus_adapter    = 4,
    BQ25622_vBusStatus_otgMode    = 7,
} BQ25622_vBusStatus_t;

#pragma pack(1)
typedef union
{
    struct
    {
        // VBUS status bits
        BQ25622_vBusStatus_t vbusStatus : 3;
        // Charge Status bits
        BQ25622_chgStatus_t chgStatus   : 2;
        uint8_t reserved                : 3;
    };
    uint8_t raw;
} BQ25622_chargerStatus1_t;
#pragma pack()

status_t BQ25622_getChargerStatus1(BQ25622_t* dev,
                                   BQ25622_chargerStatus1_t* stat);


// The TS temperature zone.
// 000b = TS_NORMAL
// 001b = TS_COLD or TS_OTG_COLD or TS resistor string power rail is not available.
// 010b = TS_HOT or TS_OTG_HOT
// 011b = TS_COOL
// 100b = TS_WARM
// 101b = TS_PRECOOL
// 110b = TS_PREWARM
// 111b = TS pin bias reference fault
typedef enum
{
    BQ25622_tsStatus_normal     = 0,
    BQ25622_tsStatus_cold       = 1,
    BQ25622_tsStatus_hot        = 2,
    BQ25622_tsStatus_otg_hot    = 2,
    BQ25622_tsStatus_cool       = 3,
    BQ25622_tsStatus_warm       = 4,
    BQ25622_tsStatus_precool    = 5,
    BQ25622_tsStatus_prewarm    = 6,
    BQ25622_tsStatus_fault      = 7,
} BQ25622_tsStatus_t;

#pragma pack(1)
typedef union
{
    struct
    {
        BQ25622_tsStatus_t tsStatus : 3;

        // IC temperature shutdown status
        // 0b = Normal
        // 1b = Device in thermal shutdown protection
        uint8_t tshut               : 1;

        // Reverse-current or undervoltage or overvoltage fault
        // detected at PMID or VBUS during boost OTG
        // 0b = Normal
        // 1b = Reverse-current fault or PMID or VBUS in over
        // voltage or under voltage during OTG
        uint8_t otgFault            : 1;

        // VSYS under voltage and over voltage status
        // 0b = Normal
        // 1b = SYS in SYS short circuit or over voltage
        uint8_t vsysFault           : 1;

        // BAT fault status, IBAT OCP and VBAT OVP
        // 0b = Normal
        // 1b = Device in battery over current protection or battery overvoltage
        //      protection
        uint8_t batFault           : 1;

        // VBUS fault status, VBUS OVP and sleep comparator
        // 0b = Normal
        // 1b = Device not switching due to over voltage protection or sleep
        //      comparator
        uint8_t vbusFault           : 1;
    };
    uint8_t raw;
} BQ25622_faultStatus0_t;
#pragma pack()

status_t BQ25622_getFaultStatus0(BQ25622_t* dev, BQ25622_faultStatus0_t* stat);

#pragma pack(1)
typedef union
{
    struct
    {
        // I2C watchdog timer flag
        // 0b = Normal
        // 1b = WD timer signal rising edge detected
        uint8_t wd              : 1;

        // Fast charge, trickle charge and pre-charge timer flag
        // 0b = Normal
        // 1b = Fast charge timer expired rising edge detected
        uint8_t safetyTmr       : 1;

        // VINDPM or VOTG flag
        // 0b = Normal
        // 1b = VINDPM or VOTG regulation signal rising edge detected
        uint8_t vindpm         : 1;

        // Indicates that either the IINDPM regulation loop, ILIM pin regulation
        // or IOTG regulation loop has been entered.
        // 0b = Normal
        // 1b = IINDPM, ILIM or IOTG regulation signal rising edge detected
        uint8_t iindpm          : 1;

        // VSYS min regulation flag
        // 0b = Normal
        // 1b = Entered or existed VSYS min regulation
        uint8_t vsys           : 1;

        // IC Thermal regulation flag
        // 0b = Normal
        // 1b = TREG signal rising threshold detected
        uint8_t treg           : 1;

        // ADC conversion flag (only in one-shot mode)
        // 0b = Conversion not completed
        // 1b = Conversion completed
        uint8_t adcDone         : 1;

        uint8_t reserved        : 1;
    };
    uint8_t raw;
} BQ25622_chargerFlag0_t;
#pragma pack()

status_t BQ25622_getChargerFlags0(BQ25622_t* dev, BQ25622_chargerFlag0_t* flags);

#pragma pack(1)
typedef union
{
    struct
    {
        // VBUS status flag
        // 0b = Normal
        // 1b = VBUS status changed
        uint8_t vbus        : 1;

        uint8_t reserved1   : 2;

        // Charge status flag
        // 0b = Normal
        // 1b = Charge status changed
        uint8_t chg         : 1;

        uint8_t reserved2   : 4;
    };
    uint8_t raw;
} BQ25622_chargerFlag1_t;
#pragma pack()

status_t BQ25622_getChargerFlags1(BQ25622_t* dev, BQ25622_chargerFlag1_t* flags);

#pragma pack(1)
typedef union
{
    struct
    {
        // TS status flag
        // 0b = Normal
        // 1b = A change to TS status was detected
        uint8_t ts          : 1;

        uint8_t reserved1   : 2;

        // IC thermal shutdown flag
        // 0b = Normal
        // 1b = TS shutdown signal rising threshold detected
        uint8_t tshut       : 1;

        // OTG PMID and VBUS reverse-current, under voltage and over voltage flag
        // 0b = Normal
        // 1b = Stopped OTG due to reverse-current fault, PMID under voltage or
        //      over voltage fault
        uint8_t otgFault    : 1;

        // VSYS over voltage and SYS short flag
        // 0b = Normal
        // 1b = Stopped switching due to system over-voltage or SYS short fault
        uint8_t sysFault    : 1;

        // IBAT over-current and VBAT over-voltage flag
        // 0b = Normal
        // 1b = Entered battery discharged OCP or VBAT OVP
        uint8_t batFault    : 1;

        // VBUS over-voltage or sleep flag
        // 0b = Normal
        // 1b = Entered VBUS OVP or sleep
        uint8_t vbusFault   : 1;
    };
    uint8_t raw;
} BQ25622_faultFlag0_t;
#pragma pack()

status_t BQ25622_getFaultFlags(BQ25622_t* dev, BQ25622_faultFlag0_t* flags);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //BQ25622_H_
