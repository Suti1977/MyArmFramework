//------------------------------------------------------------------------------
//  Sajat stream buffer, mely tamogatast ad a bemeneti stream adataramlasanak
//  kezelesere
//
//    File: MySteramBuffer.h
//------------------------------------------------------------------------------
#ifndef MYSTREAMBUFFER_H_
#define MYSTREAMBUFFER_H_

#if USE_FREERTOS
#include "MyCommon.h"
#include "MyAtomic.h"
//------------------------------------------------------------------------------
//Buffert inicailizalo konfiguracios parameterek
typedef struct
{
    //A buffer szamara foglalt memoria, es annak hossza
    uint8_t* buffer;
    uint32_t bufferSize;
} MyStreamBuffer_Config_t;
//------------------------------------------------------------------------------
//MyStreamBuffer valtozoi
typedef struct
{
    //A korabban foglalt buffer kezdocime
    uint8_t*    buffer;
    //A buffer vegere mutato pointer
    uint8_t*    bufferEnd;
    //A buffer merete
    uint32_t    bufferSize;

    //Varakozo taszk azonositoja, mely alapjan a taszk ebresztheto.
    TaskHandle_t taskWaitingToReceive;

    //A buffert iro pointer
    uint8_t*    writePtr;
    //buffert olvaso pointer
    uint8_t*    readPtr;

    //A bufferben talalhato byteok szama
    uint32_t    bytesInBuffer;
    //A bufferben talalhato szabad byteok szama
    uint32_t    freeBytes;

    //Legalabb enyni adatbyte-nak kell lennie a bufferben, hogy a varakozo
    //taszk fel legyen ebresztve
    uint32_t triggerLevel;
} MyStreamBuffer_t;
//------------------------------------------------------------------------------
//Stream buffer kezdeti inicializalasa es konfiguralasa
void MyStreamBuffer_init(MyStreamBuffer_t* buffer,
                         const MyStreamBuffer_Config_t* cfg);

//buffer uritese.
void MyStreamBuffer_reset(MyStreamBuffer_t* buffer);

//Uj elem helyezese a bufferba megszakitasbol.
//Ha a buffer tele lenne, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_putByteFromIsr(MyStreamBuffer_t* buffer,
                                       uint8_t data,
                                       BaseType_t* xHigherPriorityTaskWoken);

//Uj elem helyezese a bufferba normal futasbol
//Ha a buffer tele lenne, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_putByte(MyStreamBuffer_t* buffer, uint8_t data);

//Uj elem olvasasa a bufferbol megszakitasban
//Ha a buffer ures, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_getByteFromIsr(MyStreamBuffer_t* buffer, uint8_t* data);

//Uj elem olvasasa a bufferbol normal futasban
//Ha a buffer ures, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_getByte(MyStreamBuffer_t* buffer, uint8_t* data);

//Stream buffer olvasasa
uint32_t MyStreamBuffer_receive(MyStreamBuffer_t* buffer,
                                uint8_t* data,
                                uint32_t length,
                                uint32_t timeout);

//Stream buffer irasa
uint32_t MyStreamBuffer_send(MyStreamBuffer_t* buffer,
                             const uint8_t* data,
                             uint32_t length);

//Stream buffer irasa megszakitas alol
uint32_t MyStreamBuffer_sendFromIsr(MyStreamBuffer_t* buffer,
                                    const uint8_t* data,
                                    uint32_t length,
                                    BaseType_t* xHigherPriorityTaskWoken);

//Bufferbol byte olvasasa. A rutin timeout ideig var, ha ures a buffer.
//Ha a timeout van, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_receiveByte(MyStreamBuffer_t* buffer,
                                    uint8_t* data,
                                    uint32_t timeout);

//Szabad helyek szamanak lekerdezese megszakitasi rutinban
static inline uint32_t MyStreamBuffer_getFreeFromIsr(MyStreamBuffer_t* buffer)
{
    return buffer->freeBytes;
}

//A bufferban levo byteok szamanak lekerdezese megszakitasi rutinban
static inline uint32_t MyStreamBuffer_getAvailableFromIsr(MyStreamBuffer_t* buffer)
{
    return buffer->bytesInBuffer;
}


//Szabad helyek szamanak lekerdezese
static inline uint32_t MyStreamBuffer_getFree(MyStreamBuffer_t* buffer)
{
    uint32_t Ret;
    MY_ENTER_CRITICAL();
    Ret=MyStreamBuffer_getFreeFromIsr(buffer);
    MY_LEAVE_CRITICAL();
    return Ret;
}

//A bufferban levo byteok szamanak lekerdezese
static inline uint32_t MyStreamBuffer_getAvailable(MyStreamBuffer_t* buffer)
{
    uint32_t Ret;
    MY_ENTER_CRITICAL();
    Ret=MyStreamBuffer_getAvailableFromIsr(buffer);
    MY_LEAVE_CRITICAL();
    return Ret;
}

//Trigger szint beallitasa. Az adat olvaso fuggvenyek addig varakoznak, amig
//az itt megadott erteku adatbyte nem talalhato a bufferben.
void MyStreamBuffer_setTriggerLevel(MyStreamBuffer_t* buffer, uint32_t level);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif // #if USE_FREERTOS
#endif //MYSTREAMBUFFER_H_
