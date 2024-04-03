//------------------------------------------------------------------------------
//  16 bites I2C buszos I/O bovito driver
//
//    File: MCP23017.h
//------------------------------------------------------------------------------
#ifndef MCP23017_H_
#define MCP23017_H_

#include "MyI2CM.h"

#define MCP23017_REG_IODIR_A	0x00
#define MCP23017_REG_IODIR_B	0x01
#define MCP23017_REG_IPOL_A     0x02
#define MCP23017_REG_IPOL_B     0x03
#define MCP23017_REG_GPINTEN_A  0x04
#define MCP23017_REG_GPINTEN_B  0x05
#define MCP23017_REG_DEFVAL_A   0x06
#define MCP23017_REG_DEFVAL_B   0x07
#define MCP23017_REG_INTCON_A   0x08
#define MCP23017_REG_INTCON_B   0x09
#define MCP23017_REG_IOCON      0x0A
#define MCP23017_REG_GPPU_A     0x0C
#define MCP23017_REG_GPPU_B     0x0D
#define MCP23017_REG_INTF_A     0x0E
#define MCP23017_REG_INTF_B     0x0F
#define MCP23017_REG_INTCAP_A   0x10
#define MCP23017_REG_INTCAP_B   0x11
#define MCP23017_REG_GPIO_A     0x12
#define MCP23017_REG_GPIO_B     0x13
#define MCP23017_REG_OLAT_A     0x14
#define MCP23017_REG_OLAT_B     0x15

//------------------------------------------------------------------------------
//MCP23017 driver valtozoi
typedef struct
{
    //Az eszkoz I2C-s eleresehez tartozo jellemzok. (busz, slave cim, stb...)
    MyI2CM_Device_t i2cDevice;
} MCP23017_t;
//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa a rendszerben
void MCP23017_create(MCP23017_t* dev, MyI2CM_t* i2c, uint8_t slaveAddress);

//Az IC egy 8 bites regiszterenek irasa
status_t MCP23017_writeReg(MCP23017_t* dev, uint8_t address, uint8_t regValue);

//Az IC tobb 8 bites regiszterenek irasa
status_t MCP23017_writeMultipleRegs(MCP23017_t* dev,
                                   uint8_t address,
                                   const uint8_t* values,
                                   uint8_t length);

//Az IC egy 8 bites regiszterenek olvasasa
status_t MCP23017_readReg(MCP23017_t* dev, uint8_t address, uint8_t* regValue);

//Az IC tobb 8 bites regiszterenek olvasasa
status_t MCP23017_readMultipleRegs( MCP23017_t* dev,
                                    uint8_t address,
                                    uint8_t* regValues,
                                    uint8_t lengt);

//egyetlen 8 bites port irasa
status_t MCP23017_writePort8(MCP23017_t* dev, uint8_t portID, uint8_t data);

//16 bites port irasa (A es B port egyutt)
status_t MCP23017_writePort16(MCP23017_t* dev, uint16_t data);

//egyetlen 8 bites port olvasasa
status_t MCP23017_readPort8(MCP23017_t* dev, uint8_t portID, uint8_t* data);

//16 bitesen portok olvasasa (A es B portok egyutt)
status_t MCP23017_readPort16(MCP23017_t* dev, uint8_t portID, uint16_t* data);

//8 bites port adatiranyok beallitsa
status_t MCP23017_setDir8(MCP23017_t* dev, uint8_t portID, uint8_t directions);

//16 bites port adatiranyok beallitsa
status_t MCP23017_setDir16(MCP23017_t* dev, uint16_t directions);

//8 bites port felhuzok beallitsa
status_t MCP23017_setPulls8(MCP23017_t* dev, uint8_t portID, uint8_t pullUps);

//16 bitesen portok felhuzoinak beallitsa
status_t MCP23017_setPulls16(MCP23017_t* dev, uint16_t pullUps);

//8 bites port inverziok beallitsa
status_t MCP23017_setInputInversions8(MCP23017_t* dev,
                                 uint8_t portID,
                                 uint8_t inverted);

//16 bitesen portokon inverziok beallitsa
status_t MCP23017_setInputInversions16(MCP23017_t* dev, uint16_t inverted);

//8 bites port modok beallitsa (Irany, felhuzasoko, inverziok)
status_t MCP23017_setMode8(MCP23017_t* dev,
                           uint8_t portID,
                           uint8_t directions,
                           uint8_t pullUps,
                           uint8_t inverted);

//16 bitesen port modok beallitsa (Irany, felhuzasoko, inverziok)
status_t MCP23017_setMode16(MCP23017_t* dev,
                           uint16_t directions,
                           uint16_t pullUps,
                           uint16_t inverted);

//pin allapotanak lekerdezese
status_t MCP23017_readPin(MCP23017_t* dev, uint8_t pinId, bool* state);

//pin allapotanak beallitasa
status_t MCP23017_setPin(MCP23017_t* dev, uint8_t pinId, bool state);

//pin modok beallitasa
status_t MCP23017_setPinMode(MCP23017_t* dev,
                             uint8_t pinId,
                             bool input,
                             bool pullUp,
                             bool invert,
                             bool initialState);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MCP23017_H_
