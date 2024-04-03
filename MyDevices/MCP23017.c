//------------------------------------------------------------------------------
//  16 bites I2C buszos I/O bovito driver
//
//    File: MCP23017.c
//------------------------------------------------------------------------------
#include "MCP23017.h"
#include <string.h>


//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void MCP23017_create(MCP23017_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress)
{
    //I2C eleres letrehozasa
    MyI2CM_createDevice(&dev->i2cDevice, i2c, slaveAddress, NULL);
}
//------------------------------------------------------------------------------
//Az IC egy 8 bites regiszterenek irasa
status_t MCP23017_writeReg(MCP23017_t* dev, uint8_t address, uint8_t regValue)
{
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,  1},
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &regValue, 1},
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
//Az IC tobb 8 bites regiszterenek irasa
status_t MCP23017_writeMultipleRegs(MCP23017_t* dev,
                                   uint8_t address,
                                   const uint8_t* values,
                                   uint8_t length)
{
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,         1        },
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, (uint8_t*)values, length   },
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
//Az IC egy 8 bites regiszterenek olvasasa
status_t MCP23017_readReg(MCP23017_t* dev, uint8_t address, uint8_t* regValue)
{
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address, 1 },
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, regValue, 1 },
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}
//------------------------------------------------------------------------------
//Az IC tobb 8 bites regiszterenek olvasasa
status_t MCP23017_readMultipleRegs( MCP23017_t* dev,
                                    uint8_t address,
                                    uint8_t* regValues,
                                    uint8_t lengt)
{
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, &address,  1     },
        (MyI2CM_xfer_t){MYI2CM_FLAG_RX, regValues, lengt },
    };

    return MYI2CM_transfer(&dev->i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));
}

//------------------------------------------------------------------------------
//A maskban megadott bitek bealliatsa vagy torlese.
status_t MCP23017_changeBit(MCP23017_t* dev, uint8_t address, uint8_t mask, bool set)
{
    status_t status;
    uint8_t regValue;
    status=MCP23017_readReg(dev, address, &regValue);
    if (status) return status;

    if (set) regValue |= mask;
    else regValue &= ~mask;

    status=MCP23017_writeReg(dev, address, regValue);
    return status;
}
//------------------------------------------------------------------------------
//egyetlen 8 bites port irasa
status_t MCP23017_writePort8(MCP23017_t* dev, uint8_t portID, uint8_t data)
{
    return MCP23017_writeReg(dev, MCP23017_REG_OLAT_A + portID, data);
}
//------------------------------------------------------------------------------
//16 bites port irasa (A es B port egyutt)
status_t MCP23017_writePort16(MCP23017_t* dev, uint16_t data)
{
    return MCP23017_writeMultipleRegs(dev,
                                      MCP23017_REG_OLAT_A,
                                      (uint8_t*)&data,
                                      2);
}
//------------------------------------------------------------------------------
//egyetlen 8 bites port olvasasa
status_t MCP23017_readPort8(MCP23017_t* dev, uint8_t portID, uint8_t* data)
{
    return MCP23017_readReg(dev, MCP23017_REG_GPIO_A + portID, data);
}
//------------------------------------------------------------------------------
//16 bitesen portok olvasasa (A es B portok egyutt)
status_t MCP23017_readPort16(MCP23017_t* dev, uint8_t portID, uint16_t* data)
{
    return MCP23017_writeMultipleRegs(dev,
                                      MCP23017_REG_GPIO_A,
                                      (uint8_t*)data,
                                      2);
}
//------------------------------------------------------------------------------
//8 bites port adatiranyok beallitsa
status_t MCP23017_setDir8(MCP23017_t* dev, uint8_t portID, uint8_t directions)
{
    return MCP23017_writeReg(dev, MCP23017_REG_IODIR_A + portID, directions);
}
//------------------------------------------------------------------------------
//16 bites port adatiranyok beallitsa
status_t MCP23017_setDir16(MCP23017_t* dev, uint16_t directions)
{
    return MCP23017_writeMultipleRegs(dev,
                                      MCP23017_REG_IODIR_A,
                                      (uint8_t*)&directions,
                                      2);
}
//------------------------------------------------------------------------------
//8 bites port felhuzok beallitsa
status_t MCP23017_setPulls8(MCP23017_t* dev, uint8_t portID, uint8_t pullUps)
{
    return MCP23017_writeReg(dev, MCP23017_REG_GPPU_A + portID, pullUps);
}
//------------------------------------------------------------------------------
//16 bitesen portok felhuzoinak beallitsa
status_t MCP23017_setPulls16(MCP23017_t* dev, uint16_t pullUps)
{
    return MCP23017_writeMultipleRegs(dev,
                                      MCP23017_REG_GPPU_A,
                                      (uint8_t*)&pullUps,
                                      2);
}
//------------------------------------------------------------------------------
//8 bites port inverziok beallitsa
status_t MCP23017_setInputInversions8(MCP23017_t* dev,
                                 uint8_t portID,
                                 uint8_t inverted)
{
    return MCP23017_writeReg(dev, MCP23017_REG_IPOL_A + portID, inverted);
}
//------------------------------------------------------------------------------
//16 bitesen portokon inverziok beallitsa
status_t MCP23017_setInputInversions16(MCP23017_t* dev, uint16_t inverted)
{
    return MCP23017_writeMultipleRegs(dev,
                                      MCP23017_REG_IPOL_A,
                                      (uint8_t*)&inverted,
                                      2);
}
//------------------------------------------------------------------------------
//8 bites port modok beallitsa (Irany, felhuzasoko, inverziok)
status_t MCP23017_setMode8(MCP23017_t* dev,
                           uint8_t portID,
                           uint8_t directions,
                           uint8_t pullUps,
                           uint8_t inverted)
{
    status_t status;
    status=MCP23017_setDir8(dev, portID, directions);
    if (status) return status;
    status=MCP23017_setPulls8(dev, portID, pullUps);
    if (status) return status;
    status=MCP23017_setInputInversions8(dev, portID, inverted);
    return status;
}
//------------------------------------------------------------------------------
//16 bitesen port modok beallitsa (Irany, felhuzasoko, inverziok)
status_t MCP23017_setMode16(MCP23017_t* dev,
                           uint16_t directions,
                           uint16_t pullUps,
                           uint16_t inverted)
{
    status_t status;
    status=MCP23017_setDir16(dev, directions);
    if (status) return status;
    status=MCP23017_setPulls16(dev, pullUps);
    if (status) return status;
    status=MCP23017_setInputInversions16(dev, inverted);
    return status;
}
//------------------------------------------------------------------------------
//pin allapotanak lekerdezese
status_t MCP23017_readPin(MCP23017_t* dev, uint8_t pinId, bool* state)
{
    status_t status;
    uint8_t regValue;
    uint8_t regAddr;

    if (pinId <=7 )
    {   //0..7
        regAddr=MCP23017_REG_GPIO_A;
    } else
    {   //8..15
        regAddr=MCP23017_REG_GPIO_B;
        pinId -=8;
    }

    status=MCP23017_readReg(dev, regAddr, &regValue);
    if (status) return status;

    if (regValue & (1 << pinId)) *state=true; else *state=false;

    return status;
}
//------------------------------------------------------------------------------
//pin allapotanak beallitasa
status_t MCP23017_setPin(MCP23017_t* dev, uint8_t pinId, bool state)
{
    status_t status;
    uint8_t regValue;
    uint8_t regAddr;

    if (pinId <=7 )
    {   //0..7
        regAddr=MCP23017_REG_OLAT_A;
    } else
    {   //8..15
        regAddr=MCP23017_REG_OLAT_B;
        pinId -=8;
    }

    status=MCP23017_readReg(dev, regAddr, &regValue);
    if (status) return status;

    if (state) regValue |= (1 << pinId);
    else regValue &= ~(1 << pinId);

    status=MCP23017_writeReg(dev, regAddr, regValue);
    if (status) return status;

    return status;
}
//------------------------------------------------------------------------------
//pin modok beallitasa
status_t MCP23017_setPinMode(MCP23017_t* dev,
                             uint8_t pinId,
                             bool input,
                             bool pullUp,
                             bool invert,
                             bool initialState)
{
    status_t status;
    uint8_t regValue;
    uint8_t regAddr;
    uint8_t mask;
    uint8_t portId;

    if (pinId <=7 )
    {   //0..7
        portId=0;
        mask=(1 << pinId);
    } else
    {   //8..15
        portId=1;
        mask=(1 << (pinId-8));
    }

    //Kezdo port allapot beallitasa a LAT regiszterbe...
    status=MCP23017_changeBit(dev,
                              MCP23017_REG_OLAT_A + portId,
                              mask,
                              initialState);
    if (status) goto error;


    //Felhuzok beallitasa
    status=MCP23017_changeBit(dev,
                              MCP23017_REG_GPPU_A + portId,
                              mask,
                              pullUp);
    if (status) goto error;

    //bemenet inverzio beallitasa
    status=MCP23017_changeBit(dev,
                              MCP23017_REG_IPOL_A + portId,
                              mask,
                              invert);
    if (status) goto error;


    //Adatirany beallitasa
    status=MCP23017_changeBit(dev,
                              MCP23017_REG_IODIR_A + portId,
                              mask,
                              input);
    if (status) goto error;


error:
    return status;
}
//------------------------------------------------------------------------------
