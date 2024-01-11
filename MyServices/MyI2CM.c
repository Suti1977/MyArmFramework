//------------------------------------------------------------------------------
//  I2C Master driver
//
//    File: MyI2CM.c
//------------------------------------------------------------------------------
#include "MyI2CM.h"
#include <string.h>
#include "MyHelpers.h"
#include <stdio.h>


static void MyI2CM_initPeri(MyI2CM_t* i2cm);
static void MyI2CM_disablePeri(MyI2CM_t* i2cm);
static void MyI2CM_busErrorResolver(MyI2CM_t* i2cm);
//------------------------------------------------------------------------------
//I2C master driver letrehozasa es konfiguralasa.
//Fontos! A config altal mutatott konfiguracionak permanensen a memoriaban
//kell maradnia!
void MyI2CM_create(MyI2CM_t* i2cm,
                   const MyI2CM_Config_t* config)
{
    ASSERT(i2cm);
    ASSERT(config);

    //Modul valtozoinak kezdeti torlese.
    memset(i2cm, 0, sizeof(MyI2CM_t));

    //A driver konfiguraciok megjegyzese
    i2cm->bitRate=config->bitRate;
    i2cm->maxRetry=config->maxRetry;
    i2cm->timeout=config->timeout;
    i2cm->hal=config->hal;
    i2cm->errorFunc=config->errorFunc;
    i2cm->errorFuncPrivData=config->errorFuncPrivData;

    //Az egyideju busz hozzaferest tobb taszk kozott kizaro mutex letrehozasa
    i2cm->busMutex=xSemaphoreCreateMutexStatic(&i2cm->busMutexBuffer);
    ASSERT(i2cm->busMutex);
}
//------------------------------------------------------------------------------
//I2C driver es eroforrasok felaszabditasa
void MyI2CM_destory(MyI2CM_t* i2cm)
{
    MyI2CM_deinit(i2cm);        
}
//------------------------------------------------------------------------------
//I2C Periferia inicializalasa/engedelyezese
void MyI2CM_init(MyI2CM_t* i2cm)
{
    //Sercom beallitasa I2C interfacenek megfeleloen, a kapott config alapjan.
    MyI2CM_initPeri(i2cm);
}
//------------------------------------------------------------------------------
//I2C Periferia tiltasa. HW eroforrasok tiltasa.
void MyI2CM_deinit(MyI2CM_t* i2cm)
{
    //I2C periferia tiltasa
    MyI2CM_disablePeri(i2cm);
}
//------------------------------------------------------------------------------
//I2C interfacehez tartozo sercom felkonfiguralasa
static void MyI2CM_initPeri(MyI2CM_t* i2cm)
{
    if (i2cm->hal->initDeinitFunc)
    {   //Periferia init
        i2cm->hal->initDeinitFunc(false, i2cm->hal->privData);
    }

    //Bit rata beallitasa
    MyI2CM_setBitRate(i2cm, i2cm->bitRate);
}
//------------------------------------------------------------------------------
static void MyI2CM_disablePeri(MyI2CM_t* i2cm)
{
    if (i2cm->hal->initDeinitFunc)
    {   //Periferia init
        i2cm->hal->initDeinitFunc(true, i2cm->hal->privData);
    }
}
//------------------------------------------------------------------------------
//I2C periferia resetelese
void MyI2CM_reset(MyI2CM_t* i2cm)
{
    if (i2cm->hal->resetFunc)
    {   //Periferia init
        i2cm->hal->resetFunc(i2cm->hal->privData);
    }
}
//------------------------------------------------------------------------------
//I2C busz sebesseg modositasa/beallitasa
status_t MyI2CM_setBitRate(MyI2CM_t* i2cm, uint32_t bitRate)
{    

    if (i2cm->hal->setBitrateFunc)
    {   //Periferia init
        i2cm->hal->setBitrateFunc(bitRate, i2cm->hal->privData);
    }

    i2cm->bitRate=bitRate;

    return kStatus_Success;
}
//------------------------------------------------------------------------------
//Busz hiba eseten hivhato rutin, melyben egy elore beregisztralt callback
//hivodhat, ha busz hibat, beragadt buszt kell helyre allitani.
static void MyI2CM_busErrorResolver(MyI2CM_t* i2cm)
{
    //I2C periferia tiltasa
    MyI2CM_deinit(i2cm);

    if (i2cm->hal->busErrorResolverFunc)
    {
        i2cm->hal->busErrorResolverFunc(i2cm->hal->privData);
    }

    //I2C periferia ujra inicializalasa
    MyI2CM_initPeri(i2cm);
}
//------------------------------------------------------------------------------
//I2C eszkoz letrehozasa
//Device: Az eszkoz leiroja
//I2C: Annak a busznak az I2CM driverenek handlere, melyre az eszkoz csatlakozik
//SlaveAddress: Eszkoz I2C slave cime a buszon
//Handler: Az I2C-s eszkozhoz tartozo driver handlere
void MyI2CM_createDevice(MyI2CM_Device_t* i2cDevice,
                         MyI2CM_t* i2cm,
                         uint8_t slaveAddress,
                         void* handler)
{
    i2cDevice->i2cm=(struct MyI2CM_t*) i2cm;
    i2cDevice->slaveAddress=slaveAddress;
    i2cDevice->handler=handler;
}
//------------------------------------------------------------------------------
//Eszkoz elerhetoseg tesztelese. Nincs adattartalom.
status_t MyI2CM_ackTest(MyI2CM_Device_t* i2cDevice)
{
    status_t status;

    //Adatatviteli blokk leirok listajnak osszeallitasa.
    //(Ez a stcaken marad, amig le nem megy a transzfer!)
    MyI2CM_xfer_t xferBlocks[]=
    {
        (MyI2CM_xfer_t){MYI2CM_FLAG_TX, NULL, 0}
    };

    //I2C mukodes kezdemenyezese.
    //(A rutin megvarja, amig befejezodik az eloirt folyamat!)
    status=MYI2CM_transfer(i2cDevice, xferBlocks, ARRAY_SIZE(xferBlocks));

    return status;
}
//------------------------------------------------------------------------------
//Atviteli leirok listaja alapjan atvitel vegrehajtasa
status_t MYI2CM_transfer(MyI2CM_Device_t* i2cDevice,
                         const MyI2CM_xfer_t* xferBlocks,
                         uint32_t itemCount)
{
    status_t status=kStatus_Success;
    MyI2CM_t* i2cm=(MyI2CM_t*) i2cDevice->i2cm;

    //Interfesz lefoglalasa a hivo taszk szamara
    xSemaphoreTake(i2cm->busMutex, portMAX_DELAY);



    uint32_t retryCnt=0;

    //Arbitacio vesztes/busz hiba eseten ujra probalkozik. (Addig ciklus....)
    while(1)
    {
        status=kStatus_Success;

        //Atvitel vegrehajtasa...
        if (i2cm->hal->transferFunc)
        {
            status=i2cm->hal->transferFunc(i2cDevice,
                                           xferBlocks,
                                           itemCount,
                                           i2cm->hal->privData);
        }

        if (status)
        {   //Van valami hiba

            if (status==kMyI2CMStatus_BusError)
            {   //Busz hibat detektalunk. Azt ha van beregisztralva kulso callback,
                //akkor megprobaljuk avval feloldani...
                printf("I2CM Bus error!\n");

                MyI2CM_busErrorResolver(i2cm);

                //adott szamuszor probalkozhat a busz helyreallitassal.
                retryCnt++;
                if (retryCnt < i2cm->maxRetry) continue;
                goto error;
            }

            if (status==kMyI2CMStatus_ArbitationLost)
            {   //Arbitacio veszetes. Ujra probalkozik...
                printf("I2C arbitation lost.\n");

                //A busz hiba feloldasi probalkozas szamlalo nullazhato. Minden
                //arbitacio vesztes helyes busz mukodest feltetelez.
                retryCnt=0;
                continue;
            }

            //Ha van beregisztralva error callback, akkor az meghivodik...
            if (i2cm->errorFunc)
            {
                i2cm->errorFunc(status, i2cm->errorFuncPrivData);
            }
        }

        //ha nincs hiba, akkor kilepes
        break;
    } //while(1);

error:

    //Interfeszt fogo mutex feloldasa
    xSemaphoreGive(i2cm->busMutex);

    return status;
}
//------------------------------------------------------------------------------
