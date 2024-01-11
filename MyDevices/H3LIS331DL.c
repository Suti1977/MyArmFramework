//------------------------------------------------------------------------------
//  H3LIS331DL 3 tengelyes gyorsulasmero driver
//
//    File: H3LIS331DL.c
//------------------------------------------------------------------------------
#include "H3LIS331DL.h"

//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void H3LIS331DL_create(H3LIS331DL_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress)
{
    //I2C eleres letrehozasa
    MyI2CM_createDevice(&dev->i2cDevice, i2c, slaveAddress, NULL);
}
//------------------------------------------------------------------------------
//Az IC olvasasa
status_t H3LIS331DL_read(H3LIS331DL_t* dev,
                      uint8_t address,
                      uint8_t* buff,
                      uint8_t length)
{
    ASSERT(buff);

    status_t status;
    //Ha 1-nel tobb byteot kell olvasni, akkor az MSB=1 jelenti az auto
    //inkrementet.
    if (length > 1) address |=0x80;

    //Adatatviteli blokk leirok listajnak osszeallitasa.
    //(Ez a stcaken marad, amig le nem megy a transzfer!)
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,  1 },
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, buff, length },
    };
    //I2C mukodes kezdemenyezese.
    //(A rutin megvarja, amig befejezodik az eloirt folyamat!)
    status=MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    return status;
}
//------------------------------------------------------------------------------
//Az IC egy 8 bites regiszterenek olvasasa
status_t H3LIS331DL_readReg(H3LIS331DL_t* dev, uint8_t address, uint8_t* regValue)
{
    status_t status;

    //Adatatviteli blokk leirok listajnak osszeallitasa.
    //(Ez a stcaken marad, amig le nem megy a transzfer!)
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,  1 },
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, regValue,  1 },
    };
    //I2C mukodes kezdemenyezese.
    //(A rutin megvarja, amig befejezodik az eloirt folyamat!)
    status=MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    return status;
}
//------------------------------------------------------------------------------
//Az IC egy 16 bites regiszterenek irasa
status_t H3LIS331DL_writeReg(H3LIS331DL_t* dev, uint8_t address, uint8_t regValue)
{
    status_t status;

    //Adatatviteli blokk leirok listajnak osszeallitasa.
    //(Ez a stcaken marad, amig le nem megy a transzfer!)
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,            1 },
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, (uint8_t*)&regValue, 1 },
    };
    //I2C mukodes kezdemenyezese.
    //(A rutin megvarja, amig befejezodik az eloirt folyamat!)
    status=MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    return status;
}
//------------------------------------------------------------------------------
//Power mode beallitasa
status_t H3LIS331DL_setPowerMode(H3LIS331DL_t* dev,
                                 H3LIS331DL_powerMode_t powerMode)
{
    status_t status;
    uint8_t regValue;
    status=H3LIS331DL_readReg(dev, H3LIS331DL_REGADDR__CTRL_REG1, &regValue);
    if (status) goto error;
    regValue &=~0xE0;
    regValue |= powerMode << 5;
    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG1, regValue);
error:
    return status;
}
//------------------------------------------------------------------------------
//kiemneti adatsebesseg beallitasa
status_t H3LIS331DL_setDataRate(H3LIS331DL_t* dev,
                                H3LIS331DL_dataRate_t dataRate)
{
    status_t status;
    uint8_t regValue;
    status=H3LIS331DL_readReg(dev, H3LIS331DL_REGADDR__CTRL_REG1, &regValue);
    if (status) goto error;
    regValue &=~0x18;
    regValue |= dataRate << 3;
    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG1, regValue);
error:
    return status;
}
//------------------------------------------------------------------------------
//X, Y Z iranyok engedelyezese/tiltsa
status_t H3LIS331DL_axisEnabler(H3LIS331DL_t* dev,
                                bool xEn,
                                bool yEn,
                                bool zEn)
{
    status_t status;
    uint8_t regValue;
    status=H3LIS331DL_readReg(dev, H3LIS331DL_REGADDR__CTRL_REG1, &regValue);
    if (status) goto error;
    regValue &=~0x03;
    regValue |= xEn | (yEn << 1) | (zEn << 2);
    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG1, regValue);
error:
    return status;
}
//------------------------------------------------------------------------------
status_t H3LIS331DL_rebootMemoryContent(H3LIS331DL_t* dev)
{
    status_t status;
    uint8_t regValue;
    status=H3LIS331DL_readReg(dev, H3LIS331DL_REGADDR__CTRL_REG2, &regValue);
    if (status) goto error;
    regValue |= 0x80;   //BOOT
    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG2, regValue);
error:
    return status;
}
//------------------------------------------------------------------------------
//High pass filter beallitasa
status_t H3LIS331DL_setHighPassFilter(H3LIS331DL_t* dev,
                                      H3LIS331DL_hpfMode_t hpfMode,
                                      H3LIS331DL_fds_t fds,
                                      H3LIS331DL_hpfEn_t HPen2,
                                      H3LIS331DL_hpfEn_t HPen1,
                                      H3LIS331DL_hpfCutoff_t HPFC)
{
    status_t status;
    uint8_t regValue;
    regValue = (hpfMode << 5) |
               (fds << 4) |
               (HPen2 << 3) |
               (HPen1 << 2)  |
               HPFC;
    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG2, regValue);
    return status;
}
//------------------------------------------------------------------------------
//Megszakitas generalas beallitasai
status_t H3LIS331DL_setInterruptControl(H3LIS331DL_t* dev,
                                        H3LIS331DL_interruptActiveLevel_t activelevel,
                                        H3LIS331DL_interruptPadMode_t padMode,
                                        H3LIS331DL_latchInterruptReques_t lir1,
                                        H3LIS331DL_latchInterruptReques_t lir2,
                                        H3LIS331DL_interruptConfig_t i1Cfg,
                                        H3LIS331DL_interruptConfig_t i2Cfg)
{
    status_t status;
    uint8_t regValue;
    regValue = (activelevel << 7) |
               (padMode << 6) |
               (lir2  << 5) |
               (i2Cfg << 3) |
               (lir1  << 2) |
               (i1Cfg << 0);

    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG3, regValue);
    return status;
}
//------------------------------------------------------------------------------
//Adat konfiguracio
status_t H3LIS331DL_setDataConfig(H3LIS331DL_t* dev,
                                   H3LIS331DL_blockDataUpdate_t bdu,
                                   H3LIS331DL_endian_t endian,
                                   H3LIS331DL_fullScale_t fullScale,
                                   H3LIS331DL_spiInterfaceMode_t sim)
{
    status_t status;
    uint8_t regValue;
    regValue = (bdu << 7) |
               (endian << 6) |
               (fullScale  << 4) |
               (sim << 0);

    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG4, regValue);
    return status;
}
//------------------------------------------------------------------------------
//Sleep-to-wake function enabler
status_t H3LIS331DL_setSleepToWake(H3LIS331DL_t* dev, bool enabled)
{
    status_t status;
    uint8_t regValue;
    regValue = enabled ? 0x03 : 0x00;
    status=H3LIS331DL_writeReg(dev, H3LIS331DL_REGADDR__CTRL_REG5, regValue);
    return status;
}
//------------------------------------------------------------------------------
//Dummy register. Reading at this address zeroes instantaneously the content of
//the internal high-pass filter. If the high-pass filter is enabled, all three
//axes are instantaneously set to 0 g. This allows the settling time of the
//high-pass filter to be overcome.
status_t H3LIS331DL_hpFilterReset(H3LIS331DL_t* dev)
{
    status_t status;
    uint8_t regValue;
    status=H3LIS331DL_readReg(dev,
                              H3LIS331DL_REGADDR__HP_FILTER_RESET,
                              &regValue);
    return status;
}
//------------------------------------------------------------------------------
//Kivalasztott interrupt kimenet konfiguralasa
status_t H3LIS331DL_setInterrupt(H3LIS331DL_t* dev,
                                 uint8_t itSelect,
                                 const H3LIS331DL_interruptCfg_t* cfg)
{
    status_t status;
    uint8_t regValue;
    regValue=   (cfg->aoi   << 7) |
                (cfg->zHiEn << 5) |
                (cfg->zLoEn << 4) |
                (cfg->yHiEn << 3) |
                (cfg->yLoEn << 2) |
                (cfg->xHiEn << 1) |
                (cfg->xLoEn << 0);

    status=H3LIS331DL_writeReg(dev,
                              itSelect ? H3LIS331DL_REGADDR__INT2_CFG :
                                         H3LIS331DL_REGADDR__INT1_CFG,
                              regValue);
    return status;
}
//------------------------------------------------------------------------------
//Kivalasztott interrupthoz tartozo threshold ertek beallitasa
status_t H3LIS331DL_setInterruptThreshold(H3LIS331DL_t* dev,
                                          uint8_t itSelect,
                                          uint8_t ths)
{
    status_t status;
    uint8_t regValue;
    regValue=   ths & 0x7f;
    status=H3LIS331DL_writeReg(dev,
                              itSelect ? H3LIS331DL_REGADDR__INT2_THS :
                                         H3LIS331DL_REGADDR__INT1_THS,
                              regValue);
    return status;
}
//------------------------------------------------------------------------------
//Kivalasztott interrupthoz tartozo minimalis kitartas, ami felett int. general
status_t H3LIS331DL_setInterruptDurartion(H3LIS331DL_t* dev,
                                          uint8_t itSelect,
                                          uint8_t duration)
{
    status_t status;
    uint8_t regValue;
    regValue=   duration & 0x7f;
    status=H3LIS331DL_writeReg(dev,
                              itSelect ? H3LIS331DL_REGADDR__INT2_DURATION :
                                         H3LIS331DL_REGADDR__INT1_DURATION,
                              regValue);
    return status;
}
//------------------------------------------------------------------------------
//Kivalasztott interrupthoz tartozo intetrupt statusz regiszter kiolvasasa
//H3LIS331DL_INT_SRC__ maszkok alkalmazhatok a visszakapott regiszter adatra
status_t H3LIS331DL_getInterruptStatus(H3LIS331DL_t* dev,
                                       uint8_t itSelect,
                                       uint8_t* regValue)
{
    status_t status;
    status=H3LIS331DL_readReg(dev,
                              itSelect ? H3LIS331DL_REGADDR__INT2_SRC :
                                         H3LIS331DL_REGADDR__INT1_SRC,
                              regValue);
    return status;
}
//------------------------------------------------------------------------------
