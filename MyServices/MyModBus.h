//------------------------------------------------------------------------------
//  MyModBus service
//
//    File: MyModBus.h
//------------------------------------------------------------------------------
#ifndef MY_MODBUS_H_
#define MY_MODBUS_H_

#include "MyCommon.h"

//Olvasaskor ezt az erteket teszi azokra a regiszter helyekre, melyek nem
//leteznek
#ifndef MYMODBUS_FILL_VALUE
# define MYMODBUS_FILL_VALUE       0xFFFF
#endif

//A maximalisan kezelheto  keret hossza.
#define MAX_MODBUS_FRAME_LENGTH     256
//A legrovidebb mod busz uzenet hossza. (Addr, func, data, crcl, crch)
#define MODBUS_MIN_FRAME_LENGTH     5

#define MODBUS_BROADCAST_ADDRESS   0

//Modbus funkcio kodok
#define MODBUS_FC_READ_COILS                        0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS              0x02
#define MODBUS_FC_READ_HOLDING_REGISTERS            0x03
#define MODBUS_FC_READ_INPUT_REGISTERS              0x04
#define MODBUS_FC_WRITE_SINGLE_COIL                 0x05
#define MODBUS_FC_WRITE_SINGLE_REGISTER             0x06
#define MODBUS_FC_READ_EXCEPTION_STATUS             0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS              0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS          0x10
#define MODBUS_FC_REPORT_SLAVE_ID                   0x11
#define MODBUS_FC_MASK_WRITE_REGISTER               0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS          0x17

//modbusz protokol hibak
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION           0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS       0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE         0x03
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE    0x04
#define MODBUS_EXCEPTION_ACKNOWLEDGE                0x05
#define MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY       0x06
#define MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE       0x07
#define MODBUS_EXCEPTION_MEMORY_PARITY              0x08
#define MODBUS_EXCEPTION_NOT_DEFINED                0x09
#define MODBUS_EXCEPTION_GATEWAY_PATH               0x0A
#define MODBUS_EXCEPTION_GATEWAY_TARGET             0x0B

//Egyszerre maximum ennyi regisztert lehet kiolvasni. (A keretbe ennyi fer el.)
#define MODBUS_MAX_READABLE_REGISTZER               125


//------------------------------------------------------------------------------
// Hardver absztrakciok

//A modbus-hoz tartozo uart illetve I/O vonalak inicializalasara vagy
//deinicializalasara szolgalo callback rutin tipusa.
//Ha a deinit parameter true, akkor fel kell szabaditani az eroforrasokat.
//callbackData: a callbackhoz korabban megadott testszoleges adattag
typedef status_t MyModBus_periperalsInit_t(bool deinit, void* callbackData);

//Soros vonalon adatok kuldesehez tartozo callback rutinok felepitese.
//A fuggveny addig nem terhet vissza, amig a kiirando adat ki nem ment!
//msg: a kuldott uzenet
//msgLen: uzenet hossza
//callbackData: a callbackhoz korabban megadott testszoleges adattag
typedef status_t MyModBus_sendFunc_t(const uint8_t* msg,
                                     uint32_t msgLen,
                                     void* callbackData);
//------------------------------------------------------------------------------
#pragma pack(1)
//------------------------------------------------------------------------------
typedef struct
{
    uint16_t startingAddress;
    uint16_t quantityOfCoils;
} MyModBus_Req_ReadCoils_t;

typedef struct
{
    uint8_t  byteCount;
    uint8_t  coilStatus[1];
} MyModBus_Resp_ReadCoils_t;
//..............................................................................
typedef struct
{
    uint16_t startingAddress;
    uint16_t quantityOfInputs;
} MyModBus_Req_ReadDiscreteInputs_t;

typedef struct
{
    uint8_t  byteCount;
    uint8_t  inputStatus[1];
} MyModBus_Resp_ReadDiscreteInputs_t;
//..............................................................................
typedef struct
{
    uint16_t startingAddress;
    uint16_t quantityOfRegisters;
} MyModBus_Req_ReadHoldingRegisters_t;

typedef struct
{
    uint8_t  byteCount;
    uint16_t registerValue[1];
} MyModBus_Resp_ReadHoldingRegisters_t;
//..............................................................................
typedef struct
{
    uint16_t startingAddress;
    uint16_t quantityOfRegisters;
} MyModBus_Req_ReadInputRegisters_t;

typedef struct
{
    uint8_t  byteCount;
    uint16_t inputRegisters[1];
} MyModBus_Resp_ReadInputRegisters_t;
//..............................................................................
typedef struct
{
    uint16_t outputAddress;
    uint16_t outputValue;
} MyModBus_Req_WriteSingleCoil_t;

typedef struct
{
    uint16_t outputAddress;
} MyModBus_Resp_WriteSingleCoil_t;
//..............................................................................
typedef struct
{
    uint16_t registerAddress;
    uint16_t registerValue;
} MyModBus_Req_WriteSingleRegister_t;

typedef struct
{
    uint16_t registerAddress;
    uint16_t registerValue;
} MyModBus_Resp_WriteSingleRegister_t;
//..............................................................................
typedef struct
{
    uint16_t startingAddress;
    uint16_t quntity;
    uint8_t  byteCount;
    uint8_t  outputValues[1];
} MyModBus_Req_WriteMultipleCoils_t;

typedef struct
{
    uint16_t startingAddress;
    uint16_t quntity;
} MyModBus_Resp_WriteMultipleCoils_t;
//..............................................................................
typedef struct
{
    uint16_t startingAddress;
    uint16_t quntityOfRegisters;
    uint8_t  byteCount;
    uint16_t registerValues[1];
} MyModBus_Req_WriteMultipleRegisters_t;

typedef struct
{
    uint16_t startingAddress;
    uint16_t quntity;
} MyModBus_Resp_WriteMultipleRegisters_t;
//..............................................................................
//exception valasz frame
typedef struct
{
    uint8_t  exceptionCode;
} MyModBus_Resp_Exception_t;
//..............................................................................
//Keresk
typedef struct
{
    uint8_t functionCode;
    union
    {
        MyModBus_Req_ReadCoils_t                  readCoil;
        MyModBus_Req_ReadDiscreteInputs_t         readDiscretInputs;
        MyModBus_Req_ReadHoldingRegisters_t       readHoldingRegisters;
        MyModBus_Req_ReadInputRegisters_t         readInputRegistres;
        MyModBus_Req_WriteSingleCoil_t            writeSingleCoil;
        MyModBus_Req_WriteSingleRegister_t        writeSingleRegister;
        MyModBus_Req_WriteMultipleCoils_t         writeMultipleCoils;
        MyModBus_Req_WriteMultipleRegisters_t     writeMultipleRegisters;
    };
} MyModbus_request_PDUs_t;

//valaszok
typedef struct
{
    uint8_t functionCode;
    union
    {
        MyModBus_Resp_ReadCoils_t                 readCoil;
        MyModBus_Resp_ReadDiscreteInputs_t        readDiscretInputs;
        MyModBus_Resp_ReadHoldingRegisters_t      readHoldingRegisters;
        MyModBus_Resp_ReadInputRegisters_t        readInputRegistres;
        MyModBus_Resp_WriteSingleCoil_t           writeSingleCoil;
        MyModBus_Resp_WriteSingleRegister_t       writeSingleRegister;
        MyModBus_Resp_WriteMultipleCoils_t        writeMultipleCoils;
        MyModBus_Resp_WriteMultipleRegisters_t    writeMultipleRegisters;
        MyModBus_Resp_Exception_t                 exception;
    };
} MyModbus_response_PDUs_t;

typedef struct
{
    //Slave cime
    uint8_t         slaveAddr;

    //keresek/valaszok unioja
    union
    {
        MyModbus_request_PDUs_t   req;
        MyModbus_response_PDUs_t  resp;
    };
} MyModBus_ADU_t;

//------------------------------------------------------------------------------
//Be/ki meneti bufferek definialasara szolgalo szerkezet.
typedef union
{
    uint8_t buff[MAX_MODBUS_FRAME_LENGTH];
    struct
    {
        //Slave cime
        uint8_t         slaveAddr;

        //keresek/valaszok unioja
        union
        {
            MyModbus_request_PDUs_t    req;
            MyModbus_response_PDUs_t   resp;
        };
    };

} MyModbus_Buffer_t;
//------------------------------------------------------------------------------
#pragma pack()
//------------------------------------------------------------------------------
//Regiszter csoportra vonatkozo beallito fuggveny definicioja
#define MYMODBUS_APP_SET_FUNC(name)                 \
           status_t name (  uint16_t offset,        \
                            uint16_t* values,       \
                            uint16_t* quantity,     \
                            void* callbackData)
typedef MYMODBUS_APP_SET_FUNC(MyModBusApp_regSetFunc_t);


//Regiszter csoportra vonatkozo lekerdezo fuggveny definicioja
#define MYMODBUS_APP_GET_FUNC(name)                 \
           status_t name (  uint16_t offset,        \
                            uint16_t* dest,         \
                            uint16_t* quantity,     \
                            void* callbackData)
typedef MYMODBUS_APP_GET_FUNC(MyModBusApp_regGetFunc_t);
//------------------------------------------------------------------------------
//Modbusz cim tablazat egyes bejegyzeseinek felepitese
//A mod bus-on mutatott regiszter kiosztast allitjuk be segitsegevel. Az egyes
//bejegyzesek mutatjak a regiszter csoportokat.
typedef struct
{
    //regiszter csoport elso elemenek a cime
    uint16_t startAddress;
    //A regiszter csoport elemeinek a szama
    uint16_t qantity;
    //A csoportra vonatkozo beallito fuggveny
    MyModBusApp_regSetFunc_t* setFunc;
    //A csoportra vonatkozo lekerdezo fuggveny
    MyModBusApp_regGetFunc_t* getFunc;
} MyModBus_AddrTable_t;
//------------------------------------------------------------------------------
//Modul inicializalasakor atadando konfiguracios struktura
typedef struct
{    
    //Ha nem 0, akkor beallitja erre az eszkoz slave cimet. Kesobb ez a
    //MyModBus_setSlaveAddress() fuggvenynel feluldefinialhato
    uint8_t slaveAddress;

    //Regisztereket leiro tablazat
    const MyModBus_AddrTable_t* regTable;
    //A regiszter iro/olvaso callback funkciok szamara atadott tetszoleges adat
    void* regCallbackData;

    //Ha ennyi ideig nem erkezik ujabb karakter, akkor veszi ugy, hogy egy
    //teljes frame beerkezett. [ms]
    uint32_t rxTimeout;   

    //Periferiak inicializalasa/deinicializalasa callback
    MyModBus_periperalsInit_t* peripheralsInitFunc;

    //Soros porton adatok kuldeset biztosito callback
    MyModBus_sendFunc_t* sendFunc;

    //A Callbackek szamara atadott tetszolesges valtozo (user_data)
    void* callbackData;
} MyModbus_config_t;
//------------------------------------------------------------------------------
//ModBus valtozoi
typedef struct
{
    //Az eszkozunk slave cime
    uint8_t slaveAddress;
    //true, ha a cimunkre hallgathatunk. Ez az UID alapu egyeztetes utan valik
    //aktivva. Amig ez nem true, addig csak a broadcast cimre figyel.
    bool  slaveAddressActive;

    //true, ha egy csomag vetele kozben vagyunk
    bool inFrame;
    //keretbe talalhato byteokat szamolja
    uint16_t frameLength;
    //true, ha az uzenetet valami hiba miatt el kell majd dobni
    bool drop;

    //bemeneti adatbyteokat ebbe a bufferbe gyujtjuk    
    MyModbus_Buffer_t rxFrame;

    //exception kod. Ha ez nem 0, akkor a valaszban hiba keretet allit ossze,
    //melybe ez a kod lesz a tartalom.
    uint8_t exceptionCode;
    //Valasz uzenet hossza
    uint16_t responseLength;

    //kimeneti buffer. Ebbe allitjuk ossze a kuldendo adathalmazt
    MyModbus_Buffer_t txFrame;

    //Ha ennyi ideig nem erkezik ujabb karakter, akkor veszi ugy, hogy egy
    //teljes frame beerkezett.
    uint32_t rxTimeout;

    //Regisztereket leiro tablazat
    const MyModBus_AddrTable_t* regTable;
    //A regiszter iro/olvaso callback funkciok szamara atadott tetszoleges adat
    void* regCallbackData;

    //Soros porton adatok kuldeset biztosito callback
    MyModBus_sendFunc_t* sendFunc;
    //A Callbackek szamara atadott tetszolesges valtozo (user_data)
    void* callbackData;
} MyModBus_t;
//------------------------------------------------------------------------------
//service kezdeti inicializalasa
void MyModBus_init(MyModBus_t* this, const MyModbus_config_t* cfg);

//Sajat cim beallitasa
void MyModBus_setSlaveAddress(MyModBus_t* this, uint8_t slaveAddr);

//Uj karakter vetelekor hivando fuggveny, melyen keresztul atadasra kerul a
//vett karakter.
void MyModBus_feeding(MyModBus_t* this, uint8_t rxByte);

//Akkor kell hivodik, ha egy ideig nem kapott uj karaktert. Ez jelenti az egyes
//csomagok veget.
void MyModBus_rxTimeout(MyModBus_t* this);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MY_MODBUS_H_


