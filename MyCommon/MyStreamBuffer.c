//------------------------------------------------------------------------------
//  Sajat stream buffer, mely tamogatast ad a bemeneti stream adataramlasanak
//  kezelesere
//
//    File: MyStreamBuffer.c
//------------------------------------------------------------------------------
#include "MyStreamBuffer.h"
#include <string.h>

//------------------------------------------------------------------------------
//Stream buffer kezdeti inicializalasa es konfiguralasa
void MyStreamBuffer_init(MyStreamBuffer_t* buffer,
                         const MyStreamBuffer_Config_t* cfg)
{
    ASSERT(cfg);
    ASSERT(cfg->buffer);
    ASSERT(cfg->bufferSize);

    //Modul valtozoinak kezdeti torlese.
    memset(buffer, 0, sizeof(MyStreamBuffer_t));

    buffer->buffer=cfg->buffer;
    buffer->bufferSize=cfg->bufferSize;
    //Buffer vegere mutato pointer beallitasa
    buffer->bufferEnd=buffer->buffer + cfg->bufferSize;

    buffer->triggerLevel=1;

    //A pointerek a Reset() fuggvenyben be lesznek allitva
    MyStreamBuffer_reset(buffer);
}
//------------------------------------------------------------------------------
//Stream buffer uritese.
void MyStreamBuffer_reset(MyStreamBuffer_t* buffer)
{
    MY_ENTER_CRITICAL();

    //Iro, olvaso pointerek a buffer elejen
    buffer->readPtr=buffer->writePtr=buffer->buffer;

    //Byteok szama 0 a bufferben. A teljes szabad...
    buffer->bytesInBuffer=0;
    buffer->freeBytes=buffer->bufferSize;

    MY_LEAVE_CRITICAL();
}
//------------------------------------------------------------------------------
//Uj elem helyezese a stream bufferba megszakitasbol.
//Ha a buffer tele lenne, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_putByteFromIsr(MyStreamBuffer_t* buffer,
                                       uint8_t data,
                                       BaseType_t* xHigherPriorityTaskWoken)
{
    if (buffer->freeBytes==0) return kStatus_Fail;

    *buffer->writePtr++=data;

    if (buffer->writePtr>=buffer->bufferEnd)
    {   //A buffer vegere ert a pointer. Ugras az elejere (Circularis mukodes)
        buffer->writePtr=buffer->buffer;
    }

    //Szabad helyek szama csokken
    buffer->freeBytes--;
    //A bufferben levo byteok szama no
    buffer->bytesInBuffer++;

    if (buffer->bytesInBuffer >= buffer->triggerLevel)
    {   //Elerte a trigger szintet a bufferben talalhato byteok szama.
        //Ha van varakozo taszk, akkor azt ebreszteni kell.

        if (buffer->taskWaitingToReceive != NULL )
        {   //Van olyan taszk, mely varakozik adatokra. Jelezni kell neki, hogy
            //ebredhet!

            xTaskNotifyFromISR( buffer->taskWaitingToReceive,
                                0,
                                eNoAction,
                                xHigherPriorityTaskWoken );

            buffer->taskWaitingToReceive=NULL;
        }
    }

    return kStatus_Success;
}
//------------------------------------------------------------------------------
//Uj elem helyezese a bufferba normal futasbol
//Ha a buffer tele lenne, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_putByte(MyStreamBuffer_t* buffer, uint8_t data)
{
    MY_ENTER_CRITICAL();

    if (buffer->freeBytes==0)
    {
        MY_LEAVE_CRITICAL();
        return kStatus_Fail;
    }

    *buffer->writePtr++=data;

    if (buffer->writePtr>=buffer->bufferEnd)
    {   //A buffer vegere ert a pointer. Ugras az elejere (Circularis mukodes)
        buffer->writePtr=buffer->buffer;
    }

    //Szabad helyek szama csokken
    buffer->freeBytes--;
    //A bufferben levo byteok szama no
    buffer->bytesInBuffer++;

    MY_LEAVE_CRITICAL();

    if (buffer->bytesInBuffer >= buffer->triggerLevel)
    {   //Elerte a trigger szintet a bufferben talalhato byteok szama.
        //Ha van varakozo taszk, akkor azt ebreszteni kell.

        if (buffer->taskWaitingToReceive != NULL )
        {   //Van olyan taszk, mely varakozik adatokra. Jelezni kell neki, hogy
            //ebredhet!
            vTaskSuspendAll();
            xTaskNotify(buffer->taskWaitingToReceive, 0, eNoAction );
            buffer->taskWaitingToReceive=NULL;
            xTaskResumeAll();
        }
    }

    return kStatus_Success;
}
//------------------------------------------------------------------------------
//Uj elem olvasasa a bufferbol megszakitasban
//Ha a buffer ures, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_getByteFromIsr(MyStreamBuffer_t* buffer, uint8_t* data)
{
    if (buffer->bytesInBuffer==0) return kStatus_Fail;

    *data=*buffer->readPtr++;

    if (buffer->readPtr>=buffer->bufferEnd)
    {   //A buffer vegere ert a pointer. Ugras az elejere (Circularis mukodes)
        buffer->readPtr=buffer->buffer;
    }

    //Szabad helyek szama no
    buffer->freeBytes++;
    //A bufferben levo byteok szama csokken
    buffer->bytesInBuffer--;

    return kStatus_Success;
}
//------------------------------------------------------------------------------
//Uj elem olvasasa a bufferbol normal futasban
//Ha a buffer ures, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_getByte(MyStreamBuffer_t* buffer, uint8_t* data)
{
    status_t status;
    MY_ENTER_CRITICAL();
    status=MyStreamBuffer_getByteFromIsr(buffer, data);
    MY_LEAVE_CRITICAL();
    return status;
}
//------------------------------------------------------------------------------
//Bufferbol byte olvasasa. A rutin timeout ideig var, ha ures a buffer.
//Ha a timeout van, akkor kStatus_Fail hibaval ter vissza.
status_t MyStreamBuffer_receiveByte(MyStreamBuffer_t* buffer,
                                    uint8_t* data,
                                    uint32_t timeout)
{
    status_t status;
    status=MyStreamBuffer_getByte(buffer, data);
    if (status==kStatus_Fail)
    {   //Nem tudott adatot visszaadni. Varakozni kell...

        MY_ENTER_CRITICAL();

        xTaskNotifyStateClear(NULL);
        //Az aktualis taszk handlerenek megjegyzese. A buffert tolto fuggvenyek
        //ez alapjan lesznek kepesek ebreszteni a taszkot.
        buffer->taskWaitingToReceive=xTaskGetCurrentTaskHandle();

        MY_LEAVE_CRITICAL();

        //Varakozas, hogy a taszk jelzest kapjon.
        xTaskNotifyWait(0, 0, NULL, timeout);
        buffer->taskWaitingToReceive=NULL;

        //ujra probal visszaadni adatot.
        status=MyStreamBuffer_getByte(buffer, data);
    }

    return status;
}
//------------------------------------------------------------------------------
//Trigger szint beallitasa. Az adat olvaso fuggvenyek addig varakoznak, amig
//az itt megadott erteku adatbyte nem talalhato a bufferben.
void MyStreamBuffer_setTriggerLevel(MyStreamBuffer_t* buffer, uint32_t level)
{
    MY_ENTER_CRITICAL();
    buffer->triggerLevel=level;
    MY_LEAVE_CRITICAL();
}
//------------------------------------------------------------------------------
//Stream buffer olvasasa
uint32_t MyStreamBuffer_receive(MyStreamBuffer_t* buffer,
                                uint8_t* data,
                                uint32_t length,
                                uint32_t timeout)
{
    status_t status;
    status=kStatus_Fail;
    printf("MyStreamBuffer_receive()\n");

    uint32_t bytesInBuffer;

    MY_ENTER_CRITICAL();
    bytesInBuffer=buffer->bytesInBuffer;
    MY_LEAVE_CRITICAL();

    if (bytesInBuffer <= length)
    {   //Nincs annyi byte a bufferben, mint amit szeretnenek olvasni.

        if (timeout)
        {   //Van aloirva varakozasra lehetoseg. Varunk, hogy a bufferbe
            //legyen elgendo byte...


        }
    }



    return status;
}
//------------------------------------------------------------------------------
//Stream buffer irasa
uint32_t MyStreamBuffer_send(MyStreamBuffer_t* buffer,
                             const uint8_t* data,
                             uint32_t length)
{
    printf("MyStreamBuffer_send()\n");

    MY_ENTER_CRITICAL();
    if (buffer->freeBytes <= length)
    {   //Nincs eleg hely a bufferben. Csak annyit masol, amennyi hely van.
        length=buffer->freeBytes;
    }
    MY_LEAVE_CRITICAL();

    //Iro pointertol iras. (de max csak a buffer vegeig.)
    uint32_t firstLen=configMIN(buffer->bufferEnd - buffer->writePtr, length);
    memcpy(buffer->writePtr, data, firstLen);

    if (length > firstLen)
    {   //van meg mit masolni.

        //A maradek, meg masolando adatmennyiseg szamitasa
        uint32_t remainglength = length - firstLen;

        //A buffer elejetol kezdve maradek masolasa
        memcpy(buffer->buffer, &data[ firstLen ], remainglength);
    }

    MY_ENTER_CRITICAL();
    buffer->freeBytes -= length;
    buffer->bytesInBuffer += length;
    buffer->writePtr += length;
    if (buffer->writePtr >= buffer->bufferEnd)
    {
        //buffer->writePtr=buffer->buffer + (buffer->writePtr - buffer->bufferEnd);
        buffer->writePtr -= buffer->bufferSize;
    }
    MY_LEAVE_CRITICAL();


    if (buffer->bytesInBuffer >= buffer->triggerLevel)
    {   //Elerte a trigger szintet a bufferben talalhato byteok szama.
        //Ha van varakozo taszk, akkor azt ebreszteni kell.

        if (buffer->taskWaitingToReceive != NULL )
        {   //Van olyan taszk, mely varakozik adatokra. Jelezni kell neki, hogy
            //ebredhet!
            vTaskSuspendAll();
            xTaskNotify(buffer->taskWaitingToReceive, 0, eNoAction );
            buffer->taskWaitingToReceive=NULL;
            xTaskResumeAll();
        }
    }

    return length;
}
//------------------------------------------------------------------------------
//Stream buffer irasa megszakitas alol
uint32_t MyStreamBuffer_sendFromIsr(MyStreamBuffer_t* buffer,
                                    const uint8_t* data,
                                    uint32_t length,
                                    BaseType_t* xHigherPriorityTaskWoken)
{
    if (buffer->freeBytes <= length)
    {   //Nincs eleg hely a bufferben. Csak annyit masol, amennyi hely van.
        length=buffer->freeBytes;
    }

    //Iro pointertol iras. (de max csak a buffer vegeig.)
    uint32_t firstLen=configMIN(buffer->bufferEnd - buffer->writePtr, length);
    memcpy(buffer->writePtr, data, firstLen);

    if (length > firstLen)
    {   //van meg mit masolni.

        //A maradek, meg masolando adatmennyiseg szamitasa
        uint32_t remainglength = length - firstLen;

        //A buffer elejetol kezdve maradek masolasa
        memcpy(buffer->buffer, &data[ firstLen ], remainglength);
    }

    buffer->freeBytes -= length;
    buffer->bytesInBuffer += length;
    buffer->writePtr += length;
    if (buffer->writePtr >= buffer->bufferEnd)
    {
        //buffer->writePtr=buffer->buffer + (buffer->writePtr - buffer->bufferEnd);
        buffer->writePtr -= buffer->bufferSize;
    }

    if (buffer->bytesInBuffer >= buffer->triggerLevel)
    {   //Elerte a trigger szintet a bufferben talalhato byteok szama.
        //Ha van varakozo taszk, akkor azt ebreszteni kell.

        if (buffer->taskWaitingToReceive != NULL )
        {   //Van olyan taszk, mely varakozik adatokra. Jelezni kell neki, hogy
            //ebredhet!
            xTaskNotifyFromISR(buffer->taskWaitingToReceive,
                               0,
                               eNoAction,
                               xHigherPriorityTaskWoken);
            buffer->taskWaitingToReceive=NULL;

        }
    }

    return length;
}
//------------------------------------------------------------------------------
