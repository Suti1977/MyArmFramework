//------------------------------------------------------------------------------
//  Flow control tamogatassal kiegeszitett stream buffer
//
//    File: MyFlowControlledStreamBuffer.c
//------------------------------------------------------------------------------
#include "MyFlowControlledStreamBuffer.h"
#include <string.h>
#include "MyCommon.h"

//------------------------------------------------------------------------------
// Konfiguracio ellenorzese
static BaseType_t MYFCSB_configValid(const MYFCSB_Config_t* config)
{
    ASSERT(config);

    if (config->bufferSize == 0u)
    {
        return pdFALSE;
    }

    //  Ha a HIGH watermark engedelyezve van,
    //  akkor LOW-nak kisebbnek kell lennie...

    if ((config->highWatermark != 0u) &&
        (config->lowWatermark >= config->highWatermark))
    {
        return pdFALSE;
    }

    if ((config->highWatermark != 0u) &&
        (config->highWatermark > config->bufferSize))
    {
        return pdFALSE;
    }

    if (config->lowWatermark > config->bufferSize)
    {
        return pdFALSE;
    }

    return pdTRUE;
}
//------------------------------------------------------------------------------
// Az aktualis buffer occupancy alapjan meghatarozza, hogy tortent-e state
// transition.
static MYFCSB_event_t MYFCSB_updateState(MYFCSB_t* handle,
                                         size_t bytesAvailable)
{
    // NORMAL -> PAUSED
    if ((handle->state == MYFCSB_STATE__NORMAL) &&
        (handle->highWatermark != 0u) &&
        (bytesAvailable >= handle->highWatermark))
    {
        handle->state = MYFCSB_STATE__PAUSED;

        return MYFCSB_EVENT__HIGH_WATERMARK;
    }

    // PAUSED -> NORMAL
    if ((handle->state == MYFCSB_STATE__PAUSED) &&
        (bytesAvailable <= handle->lowWatermark))
    {
        handle->state = MYFCSB_STATE__NORMAL;

        return MYFCSB_EVENT__LOW_WATERMARK;
    }

    return MYFCSB_EVENT__NONE;
}
//------------------------------------------------------------------------------
// Flow control tamogatott stream buffer dinamikus letrehozasa
MYFCSB_t* MYFCSB_create(const MYFCSB_Config_t* config)
{
    MYFCSB_t* handle;

    // Konfiguracio ellenorzese
    if (MYFCSB_configValid(config) != pdTRUE)
    {
        return NULL;
    }

    // Sjat handlernek memoria foglalas
    handle = pvPortMalloc(sizeof(MYFCSB_t));
    if (handle == NULL)
    {
        return NULL;
    }
    memset(handle, 0, sizeof(MYFCSB_t));

    // FreeRTOS altal biztositott, wrappolni kivant stream buffer letrehozasa
    handle->streamBuffer = xStreamBufferCreate(config->bufferSize,
                                                config->triggerLevel);

    if (handle->streamBuffer == NULL)
    {
        vPortFree(handle);
        return NULL;
    }

    // Konfiguracio megjegyzese...
    handle->bufferSize = config->bufferSize;
    handle->highWatermark = config->highWatermark;
    handle->lowWatermark = config->lowWatermark;
    handle->on_highWatermark = config->on_highWatermark;
    handle->on_lowWatermark = config->on_lowWatermark;
    handle->userData = config->userData;

    // Mivel ez egy uj buffer, ezert NORMAL allapotban indul
    handle->state = MYFCSB_STATE__NORMAL;

    return handle;
}
//------------------------------------------------------------------------------
// Flow control tamogatott stream buffer statikus letrehozasa
MYFCSB_t* MYFCSB_createStatic(const MYFCSB_Config_t* config,
                              uint8_t* storage,
                              StaticStreamBuffer_t* streamBufferStorage,
                              MYFCSB_t* objectStorage)
{
    // Konfiguracio ellenorzese
    if (MYFCSB_configValid(config) != pdTRUE)
    {
        return NULL;
    }

    if ((storage == NULL) ||
        (streamBufferStorage == NULL) ||
        (objectStorage == NULL))
    {
        return NULL;
    }

    // Az objektumnak a hivo oldalon definialt buffer nullazasa
    memset(objectStorage, 0, sizeof(MYFCSB_t));

    // FreeRTOS altal biztositott, wrappolni kivant stream buffer letrehozasa
    // statikusan
    objectStorage->streamBuffer =
        xStreamBufferCreateStatic(config->bufferSize,
                                  config->triggerLevel,
                                  storage,
                                  streamBufferStorage);

    if (objectStorage->streamBuffer == NULL)
    {
        return NULL;
    }

    // Konfiguracio megjegyzese...
    objectStorage->bufferSize = config->bufferSize;
    objectStorage->highWatermark = config->highWatermark;
    objectStorage->lowWatermark = config->lowWatermark;
    objectStorage->on_highWatermark = config->on_highWatermark;
    objectStorage->on_lowWatermark = config->on_lowWatermark;
    objectStorage->userData = config->userData;

    // Mivel ez egy uj buffer, ezert NORMAL allapotban indul
    objectStorage->state = MYFCSB_STATE__NORMAL;

    return objectStorage;
}
//------------------------------------------------------------------------------
// Dinamikusan letrehozott buffer megszuntetese
void MYFCSB_delete(MYFCSB_t* handle)
{
    ASSERT(handle);

    if (handle->streamBuffer != NULL)
    {
        vStreamBufferDelete( handle->streamBuffer);
    }

    vPortFree(handle);
}
//------------------------------------------------------------------------------
//  A kapott eventhez tartozo callbacket meghivja, ha az event azt mondja
//  Task contextbol hivhato. ISR-ben NE hivd!
void MYFCSB_invokeEvent(MYFCSB_t* handle, MYFCSB_event_t event)
{
    ASSERT(handle);

    if (event == MYFCSB_EVENT__HIGH_WATERMARK)
    {
        // A felso kuszobszintet kell jelezni! Ha van callback, meghivja
        if (handle->on_highWatermark != NULL)
        {
            handle->on_highWatermark(handle->userData);
        }
    }
    else if (event == MYFCSB_EVENT__LOW_WATERMARK)
    {
        // A felso kuszobszintet kell jelezni! Ha van callback, meghivja
        if (handle->on_lowWatermark != NULL)
        {
            handle->on_lowWatermark(handle->userData);
        }
    }
}
//------------------------------------------------------------------------------
// Adatok helyezese a stream bufferbe
size_t MYFCSB_send(MYFCSB_t* handle,
                   const void* data,
                   size_t dataLength,
                   TickType_t ticksToWait)
{
    size_t sent;
    MYFCSB_event_t event;

    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    // FreeRTOS-es stream buffer-en keresztul kuldes
    sent = xStreamBufferSend(handle->streamBuffer,
                             data,
                             dataLength,
                             ticksToWait);

    // Ellenorzes, hogy sikerult-e kuldeni...
    if (sent != 0u)
    {
        // Sikeresen helyezett adatot a bufferbe. Ellenorzes, hogy a
        // flow control allapotot kell-e modositani. Ennek megfeleloen
        // allnak be az esemeny flagek.
        size_t available=xStreamBufferBytesAvailable(handle->streamBuffer);
        event = MYFCSB_updateState(handle, available);

        // Esemeny flagek ellenorzese es ha kell, flow control callback hivasa.
        MYFCSB_invokeEvent(handle, event);
    }

    return sent;
}
//------------------------------------------------------------------------------
// Adatok olvasasa a bufferbol
size_t MYFCSB_receive(MYFCSB_t* handle,
                      void* buffer,
                      size_t bufferLength,
                      TickType_t ticksToWait)
{
    size_t received;
    MYFCSB_event_t event;

    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    // FreeRTOS-es stream buffer-en keresztul olvasas
    received = xStreamBufferReceive(handle->streamBuffer,
                                    buffer,
                                    bufferLength,
                                    ticksToWait);

    if (received != 0u)
    {
        // Sikeresen olvasott. Ellenorzes, hogy a
        // flow control allapotot kell-e modositani. Ennek megfeleloen
        // allnak be az esemeny flagek.
        size_t available=xStreamBufferBytesAvailable(handle->streamBuffer);
        event = MYFCSB_updateState(handle, available);

        // Esemeny flagek ellenorzese es ha kell, flow control callback hivasa.
        MYFCSB_invokeEvent(handle, event);
    }

    return received;
}
//------------------------------------------------------------------------------
// Adatok helyezese ISR alatt a stream bufferbe
size_t MYFCSB_sendFromISR(MYFCSB_t* handle,
                          const void* data,
                          size_t dataLength,
                          MYFCSB_event_t* flowControlEvents,
                          BaseType_t* higherPriorityTaskWoken)
{
    size_t sent;
    size_t bytesAvailable;

    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    // FreeRTOS-es stream buffer-en keresztul kuldes
    sent = xStreamBufferSendFromISR(handle->streamBuffer,
                                    data,
                                    dataLength,
                                    higherPriorityTaskWoken);
    if (sent == 0u)
    {
        return MYFCSB_EVENT__NONE;
    }

    bytesAvailable = xStreamBufferBytesAvailable(handle->streamBuffer);

    // A flow control kezelesre vonatkozo esemenyek atadasa a hivo oldalnak.
    *flowControlEvents = MYFCSB_updateState(handle, bytesAvailable);

    return bytesAvailable;
}
//------------------------------------------------------------------------------
// Adatok olvasasa ISR alatt a stream bufferbol
size_t MYFCSB_receiveFromISR(MYFCSB_t* handle,
                             void* buffer,
                             size_t bufferLength,
                             MYFCSB_event_t* flowControlEvents,
                             BaseType_t* higherPriorityTaskWoken)
{
    size_t received;
    size_t bytesAvailable;

    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    // FreeRTOS-es stream buffer-en keresztul olvasas
    received = xStreamBufferReceiveFromISR(handle->streamBuffer,
                                           buffer,
                                           bufferLength,
                                           higherPriorityTaskWoken);
    if (received == 0u)
    {
        return MYFCSB_EVENT__NONE;
    }

    bytesAvailable = xStreamBufferBytesAvailable(handle->streamBuffer);

    // A flow control kezelesre vonatkozo esemenyek atadasa a hivo oldalnak.
    *flowControlEvents = MYFCSB_updateState(handle, bytesAvailable);

    return received;
}
//------------------------------------------------------------------------------
// Manualis update. (Kenyelmi fuggveny).
// - ellenorzi az aktualis bufferallapotot
// - ha state transition tortent, visszaadja az eventet
// Nem hiv callbacket.
MYFCSB_event_t MYFCSB_update(MYFCSB_t* handle)
{
    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    size_t bytesAvailable;
    bytesAvailable = xStreamBufferBytesAvailable(handle->streamBuffer);

    return MYFCSB_updateState(handle, bytesAvailable);
}
//------------------------------------------------------------------------------
// Bufferben talalhato byteok szamat adja vissza
size_t MYFCSB_bytesAvailable(const MYFCSB_t* handle)
{
    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    return xStreamBufferBytesAvailable(handle->streamBuffer);
}
//------------------------------------------------------------------------------
// A bufferben talalhato szabad helyet adja vissza byteokban
size_t MYFCSB_spacesAvailable(const MYFCSB_t* handle)
{
    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    return xStreamBufferSpacesAvailable(handle->streamBuffer);
}
//------------------------------------------------------------------------------
// A buffer meretet byteban adja vissza
size_t MYFCSB_getBufferSize(const MYFCSB_t* handle)
{
    ASSERT(handle);
    return handle->bufferSize;
}
//------------------------------------------------------------------------------
// A buffer aktualis allapotat adja vissza
MYFCSB_state_t MYFCSB_getState(const MYFCSB_t* handle)
{
    ASSERT(handle);

    return handle->state;
}
//------------------------------------------------------------------------------
// A buffer altal wrappolt freeRTOS-es stream buffer handleret adja vissza.
StreamBufferHandle_t MYFCSB_getStreamBuffer(MYFCSB_t* handle)
{
    ASSERT(handle);
    return handle->streamBuffer;
}
//------------------------------------------------------------------------------
// Buffer alaphelyzetbe allotasa
void MYFCSB_reset(MYFCSB_t* handle)
{
    ASSERT(handle);
    ASSERT(handle->streamBuffer);

    xStreamBufferReset(handle->streamBuffer);
    handle->state = MYFCSB_STATE__NORMAL;
}

//------------------------------------------------------------------------------
// Felso es also kuszob szintek beallitasa
BaseType_t MYFCSB_setWatermarks(MYFCSB_t* handle,
                                size_t lowWatermark,
                                size_t highWatermark)
{
    ASSERT(handle);

    if (highWatermark > handle->bufferSize)
    {
        return pdFAIL;
    }

    if (lowWatermark > handle->bufferSize)
    {
        return pdFAIL;
    }

    if ((highWatermark != 0u) &&
        (lowWatermark >= highWatermark))
    {
        return pdFAIL;
    }

    handle->lowWatermark = lowWatermark;
    handle->highWatermark = highWatermark;

    return pdPASS;
}
//------------------------------------------------------------------------------
// Trigger szint beallitasa
BaseType_t MYFCSB_setTriggerLevel(MYFCSB_t* handle, size_t triggerLevel)
{
    ASSERT(handle);
    ASSERT(handle->streamBuffer);
    return xStreamBufferSetTriggerLevel(handle->streamBuffer, triggerLevel);
}
//------------------------------------------------------------------------------
// Ures buffer alalpot lekerdezese
BaseType_t MYFCSB_isEmpty(MYFCSB_t* handle)
{
    ASSERT(handle);
    ASSERT(handle->streamBuffer);
    return xStreamBufferIsEmpty(handle->streamBuffer);
}
//------------------------------------------------------------------------------
// Teli buffer allapot lekerdezese
BaseType_t MYFCSB_isFull(MYFCSB_t* handle)
{
    ASSERT(handle);
    ASSERT(handle->streamBuffer);
    return xStreamBufferIsFull(handle->streamBuffer);
}
//------------------------------------------------------------------------------
