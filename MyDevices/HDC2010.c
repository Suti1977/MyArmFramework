//------------------------------------------------------------------------------
//  Texas Instruments HDC2010 paratartalom es homerseklet mero driver

//
//    File: HDC2010.c
//------------------------------------------------------------------------------
#include "HDC2010.h"
#include <string.h>


//------------------------------------------------------------------------------
// I2C eszkoz letrehozasa a rendszerben
void HDC2010_create(HDC2010_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress)
{
    // I2C eleres letrehozasa
    MyI2CM_createDevice(&dev->i2cDevice, i2c, slaveAddress, NULL);
}
//------------------------------------------------------------------------------
// Az IC olvasasa
status_t HDC1080_read(HDC2010_t* dev,
                      uint8_t address,
                      uint8_t* buff,
                      uint8_t length)
{
    ASSERT(buff);

    status_t status;

    // regiszter cime (8 bites pointer)
    uint8_t cmd[1];
    cmd[0]= address;

    // Olvasas...
    // Adatatviteli blokk leirok listajnak osszeallitasa.
    // (Ez a stcaken marad, amig le nem megy a transzfer!)
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, cmd,  sizeof(cmd) },
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, buff, length           },
    };
    // I2C mukodes kezdemenyezese.
    // (A rutin megvarja, amig befejezodik az eloirt folyamat!)
    status=MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    return status;
}
//------------------------------------------------------------------------------
// Az IC egy 16 bites regiszterenek irasa
status_t HDC2010_writeReg16(HDC2010_t* dev, uint8_t address, uint16_t regValue)
{
    status_t status;

    // regiszter cime (8 bites pointer)
    uint8_t cmd[1];
    cmd[0]= address;

    // endian csere...
    //regValue= __builtin_bswap16(regValue);

    // Iras...
    // Adatatviteli blokk leirok listajnak osszeallitasa.
    // (Ez a stcaken marad, amig le nem megy a transzfer!)
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, cmd,                 sizeof(cmd) },
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, (uint8_t*)&regValue, 2           },
    };
    // I2C mukodes kezdemenyezese.
    // (A rutin megvarja, amig befejezodik az eloirt folyamat!)
    status=MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    return status;
}
//------------------------------------------------------------------------------
// Az IC egy 8 bites regiszterenek irasa
status_t HDC2010_writeReg8(HDC2010_t* dev, uint8_t address, uint8_t regValue)
{
    status_t status;

    // regiszter cime (8 bites pointer)
    uint8_t cmd[1];
    cmd[0]= address;

    // Iras...
    // Adatatviteli blokk leirok listajnak osszeallitasa.
    // (Ez a stcaken marad, amig le nem megy a transzfer!)
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, cmd,                 sizeof(cmd) },
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, (uint8_t*)&regValue, 1           },
    };
    // I2C mukodes kezdemenyezese.
    // (A rutin megvarja, amig befejezodik az eloirt folyamat!)
    status=MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    return status;
}
//------------------------------------------------------------------------------
// Szoftveres reset
status_t HDC2010_reset(HDC2010_t* dev)
{
    status_t status;
    uint8_t regValue;
    status=HDC1080_read(dev, HDC2010_RESET_DRDY_INT_CONF, &regValue, 1);
    if (status) return status;
    regValue |= HDC2010_RESET_DRDY_INT_CONF_SOFT_RES;
    return HDC2010_writeReg8(dev, HDC2010_RESET_DRDY_INT_CONF, regValue);
}
//------------------------------------------------------------------------------
// Szenzor konfiguralasa
status_t HDC2010_configure(HDC2010_t* dev, const HDC2010_config_t* config)
{
    status_t status;
    uint8_t regValue;
    status=HDC1080_read(dev, HDC2010_RESET_DRDY_INT_CONF, &regValue, 1);
    if (status) return status;

    regValue &= ~0x77;
    regValue |= config-> autoMeasurementRate;
    regValue |= config->drdyIntPinEnable ? 0x04 : 0x00;
    regValue |= config->intPol;
    regValue |= config->intMode;
    status=HDC2010_writeReg8(dev, HDC2010_RESET_DRDY_INT_CONF, regValue);
    if (status) return status;

    status=HDC1080_read(dev, HDC2010_MEASUREMENT_CONFIGURATION, &regValue, 1);
    if (status) return status;
    regValue &= ~0xf0;
    regValue |= config-> temperatureResolution;
    regValue |= config-> humidityResolution;
    status=HDC2010_writeReg8(dev, HDC2010_MEASUREMENT_CONFIGURATION, regValue);
    if (status) return status;

    return status;
}
//------------------------------------------------------------------------------
// Meres inditasa
status_t HDC2010_startMeasure(HDC2010_t* dev, bool humiMeasEnable)
{
    status_t status;
    uint8_t regValue;
    status=HDC1080_read(dev, HDC2010_MEASUREMENT_CONFIGURATION, &regValue, 1);
    if (status) return status;

    regValue &= 0xf8;

    if (humiMeasEnable==false) regValue |= 2;
    regValue |= 1;

    return HDC2010_writeReg8(dev, HDC2010_MEASUREMENT_CONFIGURATION, regValue);
}
//------------------------------------------------------------------------------
// Homerseklet lekerdezese
status_t HDC2010_readTemperature(HDC2010_t* dev, float* temperature)
{
    status_t status;
    uint16_t regValue=0;
    status=HDC1080_read(dev, HDC2010_TEMPERATURE_LOW, (uint8_t*)&regValue, 2);
    if (status) return status;

    *temperature = (float)regValue * 165 / 65536 - 40;

    return status;
}
//------------------------------------------------------------------------------
// Paratartalom lekerdezese
status_t HDC2010_readHumidity(HDC2010_t* dev, float* humidity)
{
    status_t status;
    uint16_t regValue=0;
    status=HDC1080_read(dev, HDC2010_HUMIDITY_LOW, (uint8_t*)&regValue, 2);
    if (status) return status;

    *humidity = (float)regValue / 65536 * 100;

    return status;
}
//------------------------------------------------------------------------------
// Gyarto azonosito lekerdezese
status_t HDC2010_readManufacturerId(HDC2010_t* dev, uint16_t* id)
{
    return HDC1080_read(dev, HDC2010_MANUFACTURER_ID_LOW, (uint8_t*)id, 2);
}
//------------------------------------------------------------------------------
// chip azonosito lekerdezese
status_t HDC2010_readDeviceId(HDC2010_t* dev, uint16_t* id)
{
    return HDC1080_read(dev, HDC2010_DEVICE_ID_LOW, (uint8_t*)id, 2);
}
//------------------------------------------------------------------------------
// Megszakitasok konfiguralasa
status_t HDC2010_configureInterrupts(HDC2010_t* dev,
                                     bool drdy_enable,
                                     bool th_enable,
                                     bool tl_enable,
                                     bool hh_enable,
                                     bool hl_enable)
{
    uint8_t regValue=0;
    if (drdy_enable) regValue |= 0x80;
    if (th_enable)   regValue |= 0x40;
    if (tl_enable)   regValue |= 0x20;
    if (hh_enable)   regValue |= 0x10;
    if (hl_enable)   regValue |= 0x08;
    return HDC2010_writeReg8(dev, HDC2010_INTERRUPT_ENABLE, regValue);
}
//------------------------------------------------------------------------------
// Futes be/ki kapcoslasa
status_t HDC2010_heaterEnbler(HDC2010_t* dev, bool enable)
{
    status_t status;
    uint8_t regValue;

    status=HDC1080_read(dev, HDC2010_RESET_DRDY_INT_CONF, &regValue, 1);
    if (status) return status;

    regValue &= ~ 0x08;
    if (enable) regValue |= 0x08;
    status=HDC2010_writeReg8(dev, HDC2010_RESET_DRDY_INT_CONF, regValue);
    if (status) return status;

    return status;
}
//------------------------------------------------------------------------------
// Homerseklet riasztasi kuszobok beallitasa
status_t HDC2010_setTemperatureThresholds(HDC2010_t* dev,
                                          float t_high,
                                          float t_low)
{
    status_t status;
    uint16_t regValue;

    if (t_low < -40) t_low=-40;
    if (t_high < -40) t_high=-40;
    if (t_low > 165) t_low=165;
    if (t_high > 165) t_high=165;

    regValue = (float)((t_high+40) * 256) / 165;
    status=HDC2010_writeReg8(dev, HDC2010_TEMP_THR_H, regValue);
    if (status) return status;

    regValue = (float)((t_low+40) * 256) / 165;
    status=HDC2010_writeReg8(dev, HDC2010_TEMP_THR_L, regValue);
    if (status) return status;

    return status;
}
//------------------------------------------------------------------------------
// Paratartalom riasztasi kuszobok beallitasa
status_t HDC2010_setHumidityThresholds(HDC2010_t* dev,
                                       float h_high,
                                       float h_low)
{
    status_t status;
    uint16_t regValue;

    if (h_low < 0) h_low=0;
    if (h_high < 0) h_high=0;
    if (h_low > 100) h_low=100;
    if (h_high > 100) h_high=100;

    regValue = (float)(h_high * 256) / 100;
    status=HDC2010_writeReg8(dev, HDC2010_RH_THR_H, regValue);
    if (status) return status;

    regValue = (float)(h_low * 256) / 100;
    status=HDC2010_writeReg8(dev, HDC2010_RH_THR_L, regValue);
    if (status) return status;

    return status;
}
//------------------------------------------------------------------------------
// Megszakitasi statusz regiszter olvasasa
status_t HDC2010_readStatus(HDC2010_t* dev, uint8_t* statusBits)
{
    return HDC1080_read(dev, HDC2010_INTERRUPT_DRDY, statusBits, 1);
}
//------------------------------------------------------------------------------
