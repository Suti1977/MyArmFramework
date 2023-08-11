//------------------------------------------------------------------------------
//  I2C Master driver
//
//    File: MyI2CM.h
//------------------------------------------------------------------------------
#ifndef MY_I2CM_H_
#define MY_I2CM_H_


#include "MyCommon.h"

#ifndef kStatusGroup_MyI2CM
#define kStatusGroup_MyI2CM 400
#endif

struct MyI2CM_t_;
//------------------------------------------------------------------------------
//Adatblokk atvitel iranyai, egyeb flagek
typedef enum
{
    //Adat kiiras a buszon
    MYI2CM_FLAG_TX=0,
    //Adat olvasas a buszrol. (A logika kihasznalja, hogy az RX=1!!!)
    MYI2CM_FLAG_RX=1,
    //10 bites cimzes eloirasa
    MYI2CM_FLAG_TENBIT=2,
} MyI2CM_flag_t;
//------------------------------------------------------------------------------
//I2C- buszon torteno adatblokk tranzakcios leiro. Ilyenekbol tobb felsorolhato,
//melyet a driver egymas utan hajt vegre. Egy-egy ilyen leiro adat irast, vagy
//olvasast is eloirhat.
typedef struct
{
    //A blokk iranya, es egyeb flagek
    MyI2CM_flag_t  flags;
    //Az adatok helyere/celteruletere mutat
    uint8_t*       buffer;
    //A blokkban mozgatni kivant adatbyteok szama
    uint32_t       length;
} MyI2CM_xfer_t;
#define MYI2CM_DIR_MASK     1
//------------------------------------------------------------------------------
//Statusz kodok
enum
{
    kMyI2CMStatus_= MAKE_STATUS(kStatusGroup_MyI2CM, 0),
    //A slave nem ACK-zott
    kMyI2CMStatus_NACK,
    //Hiba a buszon. Peldaul GND-re huzza valami.
    kMyI2CMStatus_BusError,
    //Arbitacio elvesztese
    kMyI2CMStatus_ArbitationLost,
    //Low timeout
    kMyI2CMStatus_LowTimeout,
    //periferia LEN error
    kMyI2CMStatus_LenError,
    //SCL Low Timeout
    kMyI2CMStatus_SclLowTimeout,
    //Master SCL Low Extend Timeout
    kMyI2CMStatus_MasterSclExtendTimeout,
    //Slave SCL Low Extend Timeout
    kMyI2CMStatus_SlaveSclLowExtendTimeout,
    //Hibas leiro
    kMyI2CMStatus_InvalidTransferDescriptor,
    //Foglalt a driver
    kMyI2CMStatus_Busy,
    //Overrin
    kMyI2CMStatus_Overrun,

    //Olyan hibakodok, melyek itt nem szerepelnek, de az egyes implementaciok
    //a sajat kodjukat ehhez adhatjak hozza.
    kMyI2CMStatus_customErrorCodes,
};
//------------------------------------------------------------------------------
//  Altalanos I2C buszra kotott eszkozok elerese
//  Egy-egy ilyen strukturaban tarolodnak azok az informaciok, melyek egy eszkoz
//  eleresehez szuksegesek. Ilyen peldaul, hogy melyik I2C buszon talalhato,
//  illetve, hogy mi az I2C cime...
typedef struct
{
    //A hozza tartozo I2C driver handlerere mutat
    struct MyI2CM_t*   i2cm;
    //Az eszkoz slave cime a buszon 7 bites
    uint8_t     slaveAddress;
    //Az eszkozhoz tartozo driver valtozoira mutat. Ezt minden driver eseten
    //sajat tipusanak megfeleloen kell kasztolni.
    void*       handler;
} MyI2CM_Device_t;
//------------------------------------------------------------------------------
//Hardveres periferia init/deinit
typedef status_t MyI2CM_hal_initDeinitFunc_t(bool deinit, void* privData);
//Adatatvitelis sebesseg beallitasa
typedef status_t MyI2CM_hal_setBitRateFunc_t(uint32_t newBitRate, void* privData);
//I2C transzfert megvalosito fuggveny
typedef status_t MYI2CM_hal_transfer_t(MyI2CM_Device_t* i2cDevice,
                                    const MyI2CM_xfer_t* xferBlocks,
                                    uint32_t itemCount,
                                    void* privData);
//I2C irast biztosito fuggveny
typedef status_t MyI2CM_hal_writeFunc_t(MyI2CM_Device_t* device, uint8_t* data, uint32_t length, void* privData);
//I2C olvasast biztosito fuggveny
typedef status_t MyI2CM_hal_readFunc_t(MyI2CM_Device_t* device, uint8_t* data, uint32_t length, void* privData);
//Periferia reset fuggveny
typedef status_t MyI2CM_hal_reset_t(void* privData);
//I2C busz hiba eseten hivott callback, melyben a beragadt busz feloldasa
//megoldhato.
typedef void MyI2CM_hal_busErrorResolverFunc_t(void* privData);
//------------------------------------------------------------------------------
//hardver absztrakcios reteggel valo kapcsolat
typedef struct
{
    //Hardveres periferia init/deinit
    MyI2CM_hal_initDeinitFunc_t* initDeinitFunc;
    //Adatatvitelis sebesseg beallitasa
    MyI2CM_hal_setBitRateFunc_t* setBitrateFunc;
    //I2C transzfert megvalosito fuggveny
    MYI2CM_hal_transfer_t* transferFunc;
    //I2C irast biztosito fuggveny
    MyI2CM_hal_writeFunc_t* writeFunc;
    //I2C olvasast biztosito fuggveny
    MyI2CM_hal_readFunc_t* readFunc;
    //Periferia reset fuggveny
    MyI2CM_hal_reset_t* resetFunc;
    //I2C busz hiba eseten hivott callback, melyben a beragadt busz feloldasa
    //megoldhato.
    MyI2CM_hal_busErrorResolverFunc_t* busErrorResolverFunc;

    //A hal szamara atadhato tetszoleges adat, melyet a callbackek megkapnak
    void* privData;
} MyI2CM_hal_t;
//------------------------------------------------------------------------------
//I2C driver hibaja eseten hivott callback funkcio definicioja
typedef void MyI2CM_errorFunc_t(status_t errorCode, void* privData);
//------------------------------------------------------------------------------
//I2C periferia konfiguracios parameterei, melyet az MyI2CM_Init() fuggvenynek
//adunk at initkor.
typedef struct
{
    //Busz frekvencia/sebesseg initkor
    uint32_t bitRate;
    //I2C atvitel timeout
    uint32_t timeout;
    //I2C hiba eseten maximalis ujraprobalkozasok szama
    uint32_t maxRetry;

    //Hardver absztrakcios reteg parameterei.
    //A megadott konfiguracionak permanensen a memoriaban kell maradnia!
    const MyI2CM_hal_t* hal;

    //Hiba eseten meghivodo callback fuggveny
    MyI2CM_errorFunc_t* errorFunc;
    //Hiba eseten meghivodo callback fuggvenynek atadando tetszoleges adat
    void* errorFuncPrivData;
} MyI2CM_Config_t;
//------------------------------------------------------------------------------
//MyI2CM valtozoi
typedef struct MyI2CM_t_
{
    //Hardver absztrakcios reteg parameterei.
    //A megadott konfiguracionak permanensen a memoriaban kell maradnia!
    const MyI2CM_hal_t* hal;

    //A taszkok kozotti busz hozzaferest kizaro mutex.
    SemaphoreHandle_t   busMutex;
    StaticSemaphore_t   busMutexBuffer;

    //A busz aktualis sebessege
    uint32_t    bitRate;
    //I2C atvitel timeout
    uint32_t timeout;
    //I2C hiba eseten maximalis ujraprobalkozasok szama
    uint32_t maxRetry;

    //Hiba eseten meghivodo callback fuggveny
    MyI2CM_errorFunc_t* errorFunc;
    //Hiba eseten meghivodo callback fuggvenynek atadando tetszoleges adat
    void* errorFuncPrivData;
} MyI2CM_t;
//------------------------------------------------------------------------------
//I2C master driver letrehozasa es konfiguralasa.
//Fontos! A config altal mutatott konfiguracionak permanensen a memoriaban
//kell maradnia!
void MyI2CM_create(MyI2CM_t* i2cm,
                   const MyI2CM_Config_t* config);

//I2C driver es eroforrasok felaszabditasa
void MyI2CM_destory(MyI2CM_t* i2cm);

//I2C Periferia inicializalasa/engedelyezese
void MyI2CM_init(MyI2CM_t* i2cm);
//I2C Periferia tiltasa. HW eroforrasok tiltasa.
void MyI2CM_deinit(MyI2CM_t* i2cm);

//I2C periferia resetelese
void MyI2CM_reset(MyI2CM_t* i2cm);


//I2C eszkoz letrehozasa
//i2c_device: A letrehozando eszkoz leiroja
//i2c: Annak a busznak az I2CM driverenek handlere, melyre az eszkoz csatlakozik
//slave_address: Eszkoz I2C slave cime a buszon
//handler: Az I2C-s eszkozhoz tartozo driver handlere
void MyI2CM_createDevice(MyI2CM_Device_t* i2cDevice,
                         MyI2CM_t* i2cm,
                         uint8_t slaveAddress,
                         void* handler);

//Atviteli leirok listaja alapjan atvitel vegrehajtasa
status_t MYI2CM_transfer(MyI2CM_Device_t* i2cDevice,
                         const MyI2CM_xfer_t* xferBlocks,
                         uint32_t itemCount);

//Eszkoz elerhetoseg tesztelese. Nincs adattartalom.
status_t MyI2CM_ackTest(MyI2CM_Device_t* i2cDevice);

//I2C busz sebesseg modositasa/beallitasa
status_t MyI2CM_setBitRate(MyI2CM_t* i2cDevice, uint32_t bitRate);

//------------------------------------------------------------------------------
#endif //MY_I2CM_H_
