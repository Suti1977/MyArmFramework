//------------------------------------------------------------------------------
//  H3LIS331DL 3 tengelyes gyorsulasmero driver
//
//    File: H3LIS331DL.h
//------------------------------------------------------------------------------
#ifndef H3LIS331DL_H_
#define H3LIS331DL_H_

#include "MyI2CM.h"

#define 	H3LIS331DL_REGADDR__WHO_AM_I        0x0F
#define 	H3LIS331DL_REGADDR__CTRL_REG1       0x20
#define 	H3LIS331DL_REGADDR__CTRL_REG2       0x21
#define 	H3LIS331DL_REGADDR__CTRL_REG3       0x22
#define 	H3LIS331DL_REGADDR__CTRL_REG4       0x23
#define 	H3LIS331DL_REGADDR__CTRL_REG5       0x24
#define 	H3LIS331DL_REGADDR__HP_FILTER_RESET 0x25
#define 	H3LIS331DL_REGADDR__REFERENCE       0x26
#define 	H3LIS331DL_REGADDR__STATUS_REG      0x27
#define 	H3LIS331DL_REGADDR__OUT_X_L         0x28
#define 	H3LIS331DL_REGADDR__OUT_X_H         0x29
#define 	H3LIS331DL_REGADDR__OUT_Y_L         0x2A
#define 	H3LIS331DL_REGADDR__OUT_Y_H         0x2B
#define 	H3LIS331DL_REGADDR__OUT_Z_L         0x2C
#define 	H3LIS331DL_REGADDR__OUT_Z_H         0x2D

#define 	H3LIS331DL_REGADDR__INT1_CFG        0x30
#define 	H3LIS331DL_REGADDR__INT1_SRC        0x31
#define 	H3LIS331DL_REGADDR__INT1_THS        0x32
#define 	H3LIS331DL_REGADDR__INT1_DURATION	0x33
#define 	H3LIS331DL_REGADDR__INT2_CFG        0x34
#define 	H3LIS331DL_REGADDR__INT2_SRC        0x35
#define 	H3LIS331DL_REGADDR__INT2_THS        0x36
#define 	H3LIS331DL_REGADDR__INT2_DURATION	0x37


//X, Y and Z-axis data overrun
#define      H3LIS331DL_STATUS_REG__ZYXOR       0x80
//Z-axis data overrun
#define      H3LIS331DL_STATUS_REG__ZOR         0x40
//Y-axis data overrun
#define      H3LIS331DL_STATUS_REG__YOR         0x20
//X-axis data overrun
#define      H3LIS331DL_STATUS_REG__XOR         0x10
//ZYXDA-axis new data available
#define      H3LIS331DL_STATUS_REG__ZYXDA       0x08
//Z-axis new data available
#define      H3LIS331DL_STATUS_REG__ZDA         0x04
//Y-axis new data available
#define      H3LIS331DL_STATUS_REG__YDA         0x02
//X-axis new data available
#define      H3LIS331DL_STATUS_REG__XDA         0x01

//Interrupt active
#define      H3LIS331DL_INT_SRC__IA             0x40
//Z high
#define      H3LIS331DL_INT_SRC__ZH             0x20
//Z Low
#define      H3LIS331DL_INT_SRC__ZL             0x10
//Y high
#define      H3LIS331DL_INT_SRC__YH             0x08
//Y Low
#define      H3LIS331DL_INT_SRC__YL             0x04
//X high
#define      H3LIS331DL_INT_SRC__XH             0x02
//X Low
#define      H3LIS331DL_INT_SRC__XL             0x01

//------------------------------------------------------------------------------
//Power mode
//H3LIS331DL_REGADDR__CTRL_REG1 [PM2, PM1, PM0]
typedef enum
{
    H3LIS331DL_POWER_MODE__POWER_DOWN       =0,
    H3LIS331DL_POWER_MODE__NORMAL_MODE      =1,
    H3LIS331DL_POWER_MODE__LOW_POWER_0_5Hz  =2,
    H3LIS331DL_POWER_MODE__LOW_POWER_1Hz    =3,
    H3LIS331DL_POWER_MODE__LOW_POWER_2Hz    =4,
    H3LIS331DL_POWER_MODE__LOW_POWER_5Hz    =5,
    H3LIS331DL_POWER_MODE__LOW_POWER_10Hz   =6,
} H3LIS331DL_powerMode_t;

//Data rate
//H3LIS331DL_REGADDR__CTRL_REG1 [DR1, DR0]
typedef enum
{
    H3LIS331DL_DATA_RATE__50Hz              =0,
    H3LIS331DL_DATA_RATE__100Hz             =1,
    H3LIS331DL_DATA_RATE__400Hz             =2,
    H3LIS331DL_DATA_RATE__1000Hz            =3,
} H3LIS331DL_dataRate_t;
//------------------------------------------------------------------------------
//High-pass-filter mode
//H3LIS331DL_REGADDR__CTRL_REG2 [HPM1, HPM2]
typedef enum
{
    H3LIS331DL_hpfMode__normal              =0,
    H3LIS331DL_hpfMode__reference           =1,
} H3LIS331DL_hpfMode_t;

//Filtered data selection
//H3LIS331DL_REGADDR__CTRL_REG2 [FDS]
typedef enum
{
    //internal filter bypassed
    H3LIS331DL_fds__bypass                  =0,
    //data from internal filter sent to output register
    H3LIS331DL_fds__enable                  =1,
} H3LIS331DL_fds_t;

//High-pass filter enabled for interrupt 1/2 source.
//H3LIS331DL_REGADDR__CTRL_REG2 [HPen1 / Hpen2]
typedef enum
{
    //internal filter bypassed
    H3LIS331DL_hpfEn__bypass                =0,
    //data from internal filter sent to output register
    H3LIS331DL_hpfEn__enable                =1,
} H3LIS331DL_hpfEn_t;

//High-pass filter cutoff frequency configuration. Default value: 00
//H3LIS331DL_REGADDR__CTRL_REG2 [HPCF1, HPCF0]
typedef enum
{
    H3LIS331DL_hpfCutoff__8                 =0,
    H3LIS331DL_hpfCutoff__16                =1,
    H3LIS331DL_hpfCutoff__32                =2,
    H3LIS331DL_hpfCutoff__64                =3,
} H3LIS331DL_hpfCutoff_t;
//                High-pass filter cutoff frequency configuration
//
//             Data rate:    50 Hz       100 Hz      400 Hz      1000 Hz
//H3LIS331DL_hpfCutoff__8       1           2           8           20
//H3LIS331DL_hpfCutoff__16      0.5         1           4           10
//H3LIS331DL_hpfCutoff__32      0.25        0.5         2           5
//H3LIS331DL_hpfCutoff__64      0.125       0.25        1           2.5
//------------------------------------------------------------------------------
//Interrupt active high, low. Default value: 0
//CTRL_REG3 [IHL]
typedef enum
{
    H3LIS331DL_interruptActiveLevel__high   =0,
    H3LIS331DL_interruptActiveLevel__low    =1,
} H3LIS331DL_interruptActiveLevel_t;

//Push-pull/open drain selection on interrupt pad. Default value 0.
//CTRL_REG3 [PP_OD]
typedef enum
{
    H3LIS331DL_interruptPadMode__pushPull   =0,
    H3LIS331DL_interruptPadMode__openDrain  =1,
} H3LIS331DL_interruptPadMode_t;

//Latch interrupt request on INT2_SRC register, with INT2_SRC register cleared
//by reading INT1_SRC / INT2_SRC itself. Default value: 0.
//CTRL_REG3 [LIR2 / LIR1]
typedef enum
{
    H3LIS331DL_latchInterruptReques__noLatch=0,
    H3LIS331DL_latchInterruptReques__latch  =1,
} H3LIS331DL_latchInterruptReques_t;

//Data signal on INT 1/2 pad control bits. Default value: 00.
//CTRL_REG3 [I1_CFG0,I1_CFG1 / I2_CFG0, I2_CFG1]
typedef enum
{
    //Interrupt 1 (2) source
    H3LIS331DL_interruptPadControl__source      =0,
    //Interrupt 1 source OR interrupt 2 source
    H3LIS331DL_latchInterruptReques__or         =1,
    //Data ready
    H3LIS331DL_latchInterruptReques__dataReady  =2,
    //Boot running
    H3LIS331DL_latchInterruptReques__bootRunning=3,
} H3LIS331DL_interruptConfig_t;
//------------------------------------------------------------------------------
//Block data update. Default value: 0
//CTRL_REG4 [BDU]
typedef enum
{
    //continuous update
    H3LIS331DL_blockDataUpdate__continuous      =0,
    //output registers not updated between MSB and LSB reading
    H3LIS331DL_blockDataUpdate__block           =1,
} H3LIS331DL_blockDataUpdate_t;

//Big/little endian data selection. Default value 0.
//CTRL_REG4 [BLE]
typedef enum
{
    H3LIS331DL_endian__little       =0,
    H3LIS331DL_endian__big          =1,
} H3LIS331DL_endian_t;


//Full scale selection. Default value: 00.
//CTRL_REG4 [FS1, FS0]
typedef enum
{
    H3LIS331DL_fullScale__100g       =0,
    H3LIS331DL_fullScale__200g       =1,
    H3LIS331DL_fullScale__400g       =3,
} H3LIS331DL_fullScale_t;

//Full scale selection. Default value: 00.
//CTRL_REG4 [SIM]
typedef enum
{
    H3LIS331DL_spiInterfaceMode__4Wire  =0,
    H3LIS331DL_spiInterfaceMode__3Wire  =1,
} H3LIS331DL_spiInterfaceMode_t;
//------------------------------------------------------------------------------
//AND/OR combination of interrupt events. Default value: 0
//INT1_CFG/INT2_CFG [AOI]
typedef enum
{
    H3LIS331DL_interruptCombination__or     =0,
    H3LIS331DL_interruptCombination__and    =1,
} H3LIS331DL_interruptCombination_t;
//------------------------------------------------------------------------------
//H3LIS331DL driver valtozoi
typedef struct
{
    //Az eszkoz I2C-s eleresehez tartozo jellemzok. (busz, slave cim, stb...)
    MyI2CM_Device_t i2cDevice;
} H3LIS331DL_t;
//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void H3LIS331DL_create(H3LIS331DL_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress);


//Az IC olvasasa
status_t H3LIS331DL_read(H3LIS331DL_t* dev,
                      uint8_t address,
                      uint8_t* buff,
                      uint8_t length);

//Az IC egy 8 bites regiszterenek olvasasa
status_t H3LIS331DL_readReg(H3LIS331DL_t* dev, uint8_t address, uint8_t* regValue);

//Az IC egy 8 bites regiszterenek irasa
status_t H3LIS331DL_writeReg(H3LIS331DL_t* dev, uint8_t address, uint8_t regValue);


//WHO_AM_I regiszter lekerdezese.
//H3LIS331DL chip eseten 0x32-t kell visszaadnia
static inline status_t H3LIS331DL_whoAmI(H3LIS331DL_t* dev, uint8_t* regValue)
{
    return H3LIS331DL_readReg(dev, H3LIS331DL_REGADDR__WHO_AM_I, regValue);
}

//Power mode beallitasa
status_t H3LIS331DL_setPowerMode(H3LIS331DL_t* dev,
                                 H3LIS331DL_powerMode_t powerMode);

//kiemneti adatsebesseg beallitasa
status_t H3LIS331DL_setDataRate(H3LIS331DL_t* dev,
                                H3LIS331DL_dataRate_t dataRate);

//X, Y Z iranyok engedelyezese/tiltsa
status_t H3LIS331DL_axisEnabler(H3LIS331DL_t* dev,
                                bool xEn,
                                bool yEn,
                                bool zEn);

status_t H3LIS331DL_rebootMemoryContent(H3LIS331DL_t* dev);

//High pass filter beallitasa
status_t H3LIS331DL_setHighPassFilter(H3LIS331DL_t* dev,
                                      H3LIS331DL_hpfMode_t hpfMode,
                                      H3LIS331DL_fds_t fds,
                                      H3LIS331DL_hpfEn_t HPen2,
                                      H3LIS331DL_hpfEn_t HPen1,
                                      H3LIS331DL_hpfCutoff_t HPFC);


//Megszakitas generalas beallitasai
status_t H3LIS331DL_setInterruptControl(H3LIS331DL_t* dev,
                                        H3LIS331DL_interruptActiveLevel_t activelevel,
                                        H3LIS331DL_interruptPadMode_t padMode,
                                        H3LIS331DL_latchInterruptReques_t lir1,
                                        H3LIS331DL_latchInterruptReques_t lir2,
                                        H3LIS331DL_interruptConfig_t i1Cfg,
                                        H3LIS331DL_interruptConfig_t i2Cfg);

//Adat konfiguracio
status_t H3LIS331DL_setDataConfig(H3LIS331DL_t* dev,
                                   H3LIS331DL_blockDataUpdate_t bdu,
                                   H3LIS331DL_endian_t endian,
                                   H3LIS331DL_fullScale_t fullScale,
                                   H3LIS331DL_spiInterfaceMode_t sim);

//Sleep-to-wake function enabler
status_t H3LIS331DL_setSleepToWake(H3LIS331DL_t* dev, bool enabled);

//Dummy register. Reading at this address zeroes instantaneously the content of
//the internal high-pass filter. If the high-pass filter is enabled, all three
//axes are instantaneously set to 0 g. This allows the settling time of the
//high-pass filter to be overcome.
status_t H3LIS331DL_hpFilterReset(H3LIS331DL_t* dev);

//Status regiszter lekerdezese
//H3LIS331DL_STATUS_REG__ maszkok alkalmazhatok a visszakapott regiszter adatra
static inline status_t H3LIS331DL_getStatusReg(H3LIS331DL_t* dev,
                                                uint8_t* regValue)
{
    return H3LIS331DL_readReg(dev, H3LIS331DL_REGADDR__STATUS_REG, regValue);
}

typedef struct
{
    H3LIS331DL_interruptCombination_t aoi;
    bool xHiEn;
    bool xLoEn;

    bool yHiEn;
    bool yLoEn;

    bool zHiEn;
    bool zLoEn;

} H3LIS331DL_interruptCfg_t;

//Kivalasztott interrupt kimenet konfiguralasa
status_t H3LIS331DL_setInterrupt(H3LIS331DL_t* dev,
                                 uint8_t itSelect,
                                 const H3LIS331DL_interruptCfg_t* cfg);

//Kivalasztott interrupthoz tartozo threshold ertek beallitasa
status_t H3LIS331DL_setInterruptThreshold(H3LIS331DL_t* dev,
                                          uint8_t itSelect,
                                          uint8_t ths);

//Kivalasztott interrupthoz tartozo minimalis kitartas, ami felett int. general
status_t H3LIS331DL_setInterruptDurartion(H3LIS331DL_t* dev,
                                          uint8_t itSelect,
                                          uint8_t duration);

//Kivalasztott interrupthoz tartozo intetrupt statusz regiszter kiolvasasa
//H3LIS331DL_INT_SRC__ maszkok alkalmazhatok a visszakapott regiszter adatra
status_t H3LIS331DL_getInterruptStatus(H3LIS331DL_t* dev,
                                       uint8_t itSelect,
                                       uint8_t* regValue);

static inline float H3LIS331DL_from_fs100_to_g(int16_t lsb)
{
    //return ((float)lsb * 0.003f);
    return ((float)((int32_t)lsb * 49) / 1000.0f);
}

static inline float H3LIS331DL_from_fs200_to_g(int16_t lsb)
{
    return ((float)((int32_t)lsb * 98) / 1000.0f);
}

static inline float H3LIS331DL_from_fs400_to_g(int16_t lsb)
{
    return ((float)((int32_t)lsb * 195) / 1000.0f);
}

//status_t H3LIS331DL_
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //H3LIS331DL_H_
