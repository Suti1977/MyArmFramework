//------------------------------------------------------------------------------
//  Texas Instruments HDC2010 paratartalom es homerseklet mero driver

//
//    File: HDC2010.h
//------------------------------------------------------------------------------
#ifndef HDC2010_H_
#define HDC2010_H_

#include "MyI2CM.h"

#define HDC2010_TEMPERATURE_LOW             0x00
#define HDC2010_TEMPERATURE_HIGH            0x01
#define HDC2010_HUMIDITY_LOW                0x02
#define HDC2010_HUMIDITY_HIGH               0x03
#define HDC2010_INTERRUPT_DRDY              0x04
#define HDC2010_TEMPERATURE_MAX             0x05
#define HDC2010_HUMIDITY_MAX                0x06
#define HDC2010_INTERRUPT_ENABLE            0x07
#define HDC2010_TEMP_OFFSET_ADJUST          0x08
#define HDC2010_HUM_OFFSET_ADJUST           0x09
#define HDC2010_TEMP_THR_L                  0x0a
#define HDC2010_TEMP_THR_H                  0x0b
#define HDC2010_RH_THR_L                    0x0c
#define HDC2010_RH_THR_H                    0x0d
#define HDC2010_RESET_DRDY_INT_CONF         0x0e
#define HDC2010_MEASUREMENT_CONFIGURATION   0x0f
#define HDC2010_MANUFACTURER_ID_LOW         0xfc
#define HDC2010_MANUFACTURER_ID_HIGH        0xfd
#define HDC2010_DEVICE_ID_LOW               0xfe
#define HDC2010_DEVICE_ID_HIGH              0xff

#define HDC2010_RESET_DRDY_INT_CONF_SOFT_RES    0x80

#define HDC2010_MANUFACTURER_ID                 0x5449
#define HDC2010_DEVICE_ID                       0x07D0

#define HDC2010_STATUS_DRDY                 0x80
#define HDC2010_STATUS_TH                   0x40
#define HDC2010_STATUS_TL                   0x20
#define HDC2010_STATUS_HH                   0x10
#define HDC2010_STATUS_HL                   0x08

typedef enum
{
    HDC2010_TempResolution_14       = 0 << 6,
    HDC2010_TempResolution_11       = 1 << 6,
    HDC2010_TempResolution_9        = 2 << 6
} HDC2010_TempResolution_t;

typedef enum
{
    HDC2010_HumiResolution_14       = 0 << 4,
    HDC2010_HumiResolution_11       = 1 << 4,
    HDC2010_HumiResolution_9        = 2 << 4
} HDC2010_HumiResolution_t;

typedef enum
{
    HDC2010_AutoMeasRate_disable    =0 << 4,
    HDC2010_AutoMeasRate_2m         =1 << 4,
    HDC2010_AutoMeasRate_1m         =2 << 4,
    HDC2010_AutoMeasRate_10s        =3 << 4,
    HDC2010_AutoMeasRate_5s         =4 << 4,
    HDC2010_AutoMeasRate_1s         =5 << 4,
    HDC2010_AutoMeasRate_500ms      =6 << 4,
    HDC2010_AutoMeasRate_200ms      =7 << 4
} HDC2010_AutoMeasRate_t;

typedef enum
{
    HDC2010_IntPol_ActiveLow    =0,
    HDC2010_IntPol_ActiveHigh   =2,
} HDC2010_IntPol_t;

typedef enum
{
    HDC2010_IntMode_Level        =0,
    HDC2010_IntMode_Comparator   =1,
} HDC2010_IntMode_t;


typedef struct
{
    HDC2010_TempResolution_t temperatureResolution;
    HDC2010_HumiResolution_t humidityResolution;
    HDC2010_AutoMeasRate_t   autoMeasurementRate;
    bool drdyIntPinEnable;
    HDC2010_IntPol_t intPol;
    HDC2010_IntMode_t intMode;
} HDC2010_config_t;
//------------------------------------------------------------------------------
// HDC2010 driver valtozoi
typedef struct
{
    // Az eszkoz I2C-s eleresehez tartozo jellemzok. (busz, slave cim, stb...)
    MyI2CM_Device_t i2cDevice;
} HDC2010_t;
//------------------------------------------------------------------------------
// I2C eszkoz letrehozasa a rendszerben
void HDC2010_create(HDC2010_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress);

// Szenzor konfiguralasa
status_t HDC2010_configure(HDC2010_t* dev, const HDC2010_config_t* config);

// Szoftveres reset
status_t HDC2010_reset(HDC2010_t* dev);

// Meres inditasa
status_t HDC2010_startMeasure(HDC2010_t* dev, bool humiMeasEnable);

// Homerseklet lekerdezese
status_t HDC2010_readTemperature(HDC2010_t* dev, float* temperature);

// Paratartalom lekerdezese
status_t HDC2010_readHumidity(HDC2010_t* dev, float* humidity);

// Gyarto azonosito lekerdezese
status_t HDC2010_readManufacturerId(HDC2010_t* dev, uint16_t* id);

// chip azonosito lekerdezese
status_t HDC2010_readDeviceId(HDC2010_t* dev, uint16_t* id);

// Megszakitasok konfiguralasa
status_t HDC2010_configureInterrupts(HDC2010_t* dev,
                                     bool drdy_enable,
                                     bool th_enable,
                                     bool tl_enable,
                                     bool hh_enable,
                                     bool hl_enable);

// Futes be/ki kapcoslasa
status_t HDC2010_heaterEnbler(HDC2010_t* dev, bool enable);

// Homerseklet riasztasi kuszobok beallitasa
status_t HDC2010_setTemperatureThresholds(HDC2010_t* dev,
                                          float t_high,
                                          float t_low);

// Paratartalom riasztasi kuszobok beallitasa
status_t HDC2010_setHumidityThresholds(HDC2010_t* dev,
                                       float h_high,
                                       float h_low);

// Megszakitasi statusz regiszter olvasasa
status_t HDC2010_readStatus(HDC2010_t* dev, uint8_t* statusBits);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //HDC2010_H_
