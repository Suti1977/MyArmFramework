//------------------------------------------------------------------------------
//  TLA2024 i2c buszos, 4 csatornas A/D konverter driver
//
//    File: TLA2024.h
//------------------------------------------------------------------------------
#ifndef TLA2024_H_
#define TLA2024_H_

#include "MyI2CM.h"

#define TLA2024_REG_CONFIG      0x01
#define TLA2024_REG_RESULT      0x00

//Statusz/Single-shot start (15)
typedef enum
{
    TLA2024_STATUS_BUSY=        (0 << 15),
    TLA2024_STATUS_START=       (1 << 15),
    TLA2024_STATUS_READY=       (1 << 15)
} TLA2024_status_t;
#define TLA2024_OS_MASK         (1 << 15)

//Bemeneti multiplexer konfiguracio (14:12)
typedef enum
{
//              AIN+ AIN-
    TLA2024_MUX_AIN0_AIN1 =     (0 << 12),
    TLA2024_MUX_AIN0_AIN3 =     (1 << 12),
    TLA2024_MUX_AIN1_AIN3 =     (2 << 12),
    TLA2024_MUX_AIN2_AIN3 =     (3 << 12),
    TLA2024_MUX_AIN0_GND  =     (4 << 12),
    TLA2024_MUX_AIN1_GND  =     (5 << 12),
    TLA2024_MUX_AIN2_GND  =     (6 << 12),
    TLA2024_MUX_AIN3_GND  =     (7 << 12),
} TLA2024_mux_t;
#define TLA2024_MUX_MASK        (7 << 12)


//Bemeneti feszultseg tartomany (11:9)
typedef enum
{
    TLA2024_RANGE_6_144 =       (0 << 9),
    TLA2024_RANGE_4_096 =       (1 << 9),
    TLA2024_RANGE_2_048 =       (2 << 9),
    TLA2024_RANGE_1_024 =       (3 << 9),
    TLA2024_RANGE_0_512 =       (4 << 9),
    TLA2024_RANGE_0_256 =       (5 << 9),
} TLA2024_range_t;
#define TLA2024_PGA_MASK        (7 << 9)


//Uzemmod   (8)
typedef enum
{
    TLA2024_MODE_CONTINUOUS =   (0 << 8),
    TLA2024_MODE_SINGLE_SHOT=   (1 << 8),
} TLA2024_mode_t;
#define TLA2024_MODE_MASK       (1 << 8)


//Mintaveteli sebesseg      (7:5)
typedef enum
{
    TLA2024_DR_128_SPS  =       (0 << 5),
    TLA2024_DR_250_SPS  =       (1 << 5),
    TLA2024_DR_490_SPS  =       (2 << 5),
    TLA2024_DR_920_SPS  =       (3 << 5),
    TLA2024_DR_1600_SPS =       (4 << 5),
    TLA2024_DR_2400_SPS =       (5 << 5),
    TLA2024_DR_3300_SPS =       (6 << 5),
} TLA2024_dataRate_t;
#define TLA2024_DR_MASK         (7 << 5)

//------------------------------------------------------------------------------
// TLA2024 driver valtozoi
typedef struct
{
    //Az eszkoz I2C-s eleresehez tartozo jellemzok. (busz, slave cim, stb...)
    MyI2CM_Device_t i2cDevice;
    uint16_t configShadow;
} TLA2024_t;
//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void TLA2024_create(TLA2024_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress);

//Az IC egy 16 bites regiszterenek irasa
status_t TLA2024_writeReg(TLA2024_t* dev, uint8_t address, uint16_t regValue);

//Az IC egy 16 bites regiszterenek olvasasa
status_t TLA2024_readReg(TLA2024_t* dev, uint8_t address, uint16_t* regValue);


//Eszkoz inicializalasa
status_t TLA2024_init(TLA2024_t* dev);

//Bemeneti multiplexer beallitasa (catorna kijelolese)
status_t TLA2024_setMux(TLA2024_t* dev, TLA2024_mux_t mux);

//Meresi taromany beallitasa
status_t TLA2024_setRange(TLA2024_t* dev, TLA2024_range_t range);

//Mintaveteli sebesseg beallitasa
status_t TLA2024_setDataRate(TLA2024_t* dev, TLA2024_dataRate_t dataRate);

//Uzemmod beallitasa
status_t TLA2024_setMode(TLA2024_t* dev, TLA2024_mode_t mode);

//Egyetlen mintavetel kezdemenyezese
status_t TLA2024_startSingleShot(TLA2024_t* dev);

//Foglaltsag lekerdezese. (Single shot modban)
status_t TLA2024_isBusy(TLA2024_t* dev, bool* busy);

//Mintavetel eredmenyenek kiolvasasa. (Raw)
status_t TLA2024_readResult(TLA2024_t* dev, int16_t* data);

//Mintavett adat feszultsegge konvertalasa.
float TLA2024_convertToVoltage(int16_t rawData, TLA2024_range_t range);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //TLA2024_H_
