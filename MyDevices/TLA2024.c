//------------------------------------------------------------------------------
//  TLA2024 i2c buszos, 4 csatornas A/D konverter driver
//
//    File: TLA2024.c
//------------------------------------------------------------------------------
#include "TLA2024.h"
#include <string.h>

//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void TLA2024_create(TLA2024_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress)
{
    //I2C eleres letrehozasa
    MyI2CM_createDevice(&dev->i2cDevice, i2c, slaveAddress, NULL);
}
//------------------------------------------------------------------------------
//Az IC egy 16 bites regiszterenek irasa
status_t TLA2024_writeReg(TLA2024_t* dev, uint8_t address, uint16_t regValue)
{
    //Az IC big endian formaban fogadja az adatokat!
    regValue=__builtin_bswap16(regValue);

    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,            1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, (uint8_t*)&regValue, 2},
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
//Az IC egy 16 bites regiszterenek olvasasa
status_t TLA2024_readReg(TLA2024_t* dev, uint8_t address, uint16_t* regValue)
{
    volatile uint16_t temp;

    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,         1 },
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, (uint8_t*) &temp, 2 },
    };

    status_t status;
    status=MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    *regValue=__builtin_bswap16(temp);

    return status;
}
//------------------------------------------------------------------------------
//Eszkoz inicializalasa. Defaultok beallitasa.
status_t TLA2024_init(TLA2024_t* dev)
{
    dev->configShadow = 3
                        | TLA2024_DR_1600_SPS
                        | TLA2024_MODE_SINGLE_SHOT
                        | TLA2024_RANGE_2_048
                        | TLA2024_MUX_AIN0_AIN1;
    return TLA2024_writeReg(dev, TLA2024_REG_CONFIG, dev->configShadow);
}
//------------------------------------------------------------------------------
//Bemeneti multiplexer beallitasa (catorna kijelolese)
status_t TLA2024_setMux(TLA2024_t* dev, TLA2024_mux_t mux)
{
    dev->configShadow &= ~TLA2024_MUX_MASK;
    dev->configShadow |= mux;
    return TLA2024_writeReg(dev, TLA2024_REG_CONFIG, dev->configShadow);
}
//------------------------------------------------------------------------------
//Meresi taromany beallitasa
status_t TLA2024_setRange(TLA2024_t* dev, TLA2024_range_t range)
{
    dev->configShadow &= ~TLA2024_PGA_MASK;
    dev->configShadow |= range;
    return TLA2024_writeReg(dev, TLA2024_REG_CONFIG, dev->configShadow);
}
//------------------------------------------------------------------------------
//Mintaveteli sebesseg beallitasa
status_t TLA2024_setDataRate(TLA2024_t* dev, TLA2024_dataRate_t dataRate)
{
    dev->configShadow &= ~TLA2024_DR_MASK;
    dev->configShadow |= dataRate;
    return TLA2024_writeReg(dev, TLA2024_REG_CONFIG, dev->configShadow);
}
//------------------------------------------------------------------------------
//Uzemmod beallitasa
status_t TLA2024_setMode(TLA2024_t* dev, TLA2024_mode_t mode)
{
    dev->configShadow &= ~TLA2024_MODE_MASK;
    dev->configShadow |= mode;
    return TLA2024_writeReg(dev, TLA2024_REG_CONFIG, dev->configShadow);
}
//------------------------------------------------------------------------------
//Egyetlen mintavetel kezdemenyezese ha az eszkoz single-shot modban van.
status_t TLA2024_startSingleShot(TLA2024_t* dev)
{
    return TLA2024_writeReg(dev,
                            TLA2024_REG_CONFIG,
                            dev->configShadow | TLA2024_STATUS_START);
}
//------------------------------------------------------------------------------
//Foglaltsag lekerdezese. (Single shot modban)
status_t TLA2024_isBusy(TLA2024_t* dev, bool* busy)
{
    uint16_t regVal;
    status_t status=TLA2024_readReg(dev, TLA2024_REG_CONFIG, (uint16_t*)&regVal);
    if (status) return status;

    if ((regVal & TLA2024_OS_MASK) == TLA2024_STATUS_BUSY) *busy=true;
    else *busy=false;

    return status;
}
//------------------------------------------------------------------------------
//Mintavetel eredmenyenek kiolvasasa. (Raw)
status_t TLA2024_readResult(TLA2024_t* dev, int16_t* data)
{
    return TLA2024_readReg(dev, TLA2024_REG_RESULT, (uint16_t*)data);
}
//------------------------------------------------------------------------------
//Mintavett adat feszultsegge konvertalasa.
float TLA2024_convertToVoltage(int16_t rawData, TLA2024_range_t range)
{
    //Also 4 bit eldobasa, elojel kezelese...
    if (rawData & 0x8000)
    {   //negativ
        rawData >>= 4;
        rawData |= 0x8000;
    } else
    {   //pozitiv
        rawData >>= 4;
    }

    float coef;
    switch (range)
    {
        case TLA2024_RANGE_6_144: coef= 0.003000f; break;
        case TLA2024_RANGE_4_096: coef= 0.002000f; break;
        case TLA2024_RANGE_2_048: coef= 0.001000f; break;
        case TLA2024_RANGE_1_024: coef= 0.000500f; break;
        case TLA2024_RANGE_0_512: coef= 0.000250f; break;
        case TLA2024_RANGE_0_256: coef= 0.000125f; break;
        default:                  coef= 0.000000f; break;
    }

    return (float)rawData * coef;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
