//------------------------------------------------------------------------------
//  Flow control tamogatassal kiegeszitett stream buffer
//
//    File: MyFlowControlledStreamBuffer.h
//------------------------------------------------------------------------------
#ifndef MY_FLOWCONTROLLEDSTREAMBUFFER_H_
#define MY_FLOWCONTROLLEDSTREAMBUFFER_H_

#include "FreeRTOS.h"
#include "stream_buffer.h"


typedef enum
{
    MYFCSB_STATE__NORMAL = 0,
    MYFCSB_STATE__PAUSED
} MYFCSB_state_t;

typedef enum
{
    MYFCSB_EVENT__NONE = 0,
    MYFCSB_EVENT__HIGH_WATERMARK,
    MYFCSB_EVENT__LOW_WATERMARK
} MYFCSB_event_t;

// Magas ileltev alacyony kuszobok eseten hivodo callbackekre tipus definicio
typedef void (*MYFCSB_callback_t)(void* context);
//------------------------------------------------------------------------------
// Flow control tamogatott stream buffer konfiguracio
typedef struct
{
    // Stream buffer merete byteban
    size_t bufferSize;

    // Ha a bufferben levo adatmennyiseg eleri ezt az erteket, HIGH esemeny
    // keletkezik.
    // 0 = high watermark kikapcsolva, nem figyeli
    size_t highWatermark;


    // Ha a buffer korabban HIGH allapotban volt, es az adatmennyiseg erre vagy
    // ez ala csokken, akkor LOW esemeny keletkezik.
    //
    // HIGH watermark hasznalata eseten: low_watermark < high_watermark
    size_t lowWatermark;

    // HIGH/LOW adatmennyisegek eseten meghivott callbackek
    MYFCSB_callback_t on_highWatermark;
    MYFCSB_callback_t on_lowWatermark;
    // A callbackek szamara atadott tetszoleges adat
    void* userData;

    // A FreeRTOS StreamBuffer trigger level.
    // (Ez NEM a flow-control watermark.)
    // 0 eseten FreeRTOS default. (1)
    size_t triggerLevel;

} MYFCSB_Config_t;
//------------------------------------------------------------------------------
// Flow control tamogatott stream buffer handlere
typedef struct
{
    // FreeRTOS altal biztositott stream buffer handlere, melyet a sajat modul
    // wrappol, es kiegeszit a flow control vezerlessel.
    StreamBufferHandle_t streamBuffer;

    // Stream buffer merete byteban
    size_t bufferSize;

    // Ha a bufferben levo adatmennyiseg eleri ezt az erteket, HIGH esemeny
    // keletkezik.
    // 0 = high watermark kikapcsolva, nem figyeli
    size_t highWatermark;

    // Ha a buffer korabban HIGH allapotban volt, es az adatmennyiseg erre vagy
    // ez ala csokken, akkor LOW esemeny keletkezik.
    //
    // HIGH watermark hasznalata eseten: low_watermark < high_watermark
    size_t lowWatermark;

    // HIGH/LOW adatmennyisegek eseten meghivott callbackek
    MYFCSB_callback_t on_highWatermark;
    MYFCSB_callback_t on_lowWatermark;

    // A callbackek szamara atadott tetszoleges adat
    void* userData;

    // Aktualis flow control allapot
    volatile MYFCSB_state_t state;

} MYFCSB_t;
//------------------------------------------------------------------------------
// Flow control tamogatott stream buffer dinamikus letrehozasa
MYFCSB_t* MYFCSB_create(const MYFCSB_Config_t* config);

// Flow control tamogatott stream buffer statikus letrehozasa
MYFCSB_t* MYFCSB_createStatic(const MYFCSB_Config_t* config,
                              uint8_t* storage,
                              StaticStreamBuffer_t* streamBufferStorage,
                              MYFCSB_t* objectStorage);
// Dinamikusan letrehozott buffer megszuntetese
void MYFCSB_delete(MYFCSB_t* handle);


//==============================================================================
//  Task API
//==============================================================================
//
//  Ugyanazt a mukodest biztositja, mint a FreeRTOS StreamBuffer.
//  A send/receive utan automatikusan ellenorzi a watermarkot.
//  Ha state transition tortent, a megfelelo callback meghivodik.

// Adatok helyezese a stream bufferbe
size_t MYFCSB_send(MYFCSB_t* handle,
                   const void* data,
                   size_t dataLength,
                   TickType_t ticksToWait);

// Adatok olvasasa a bufferbol
size_t MYFCSB_receive(MYFCSB_t* handle,
                      void* data,
                      size_t bufferLength,
                      TickType_t ticksToWait);


//==============================================================================
//  ISR API
//==============================================================================
//
// FONTOS!:
// Ezek a fuggvenyek NEM hivjak meg a callbackeket ISR contextben.
//
//  Az event visszateresi ertekkent jelenik meg:
//      MYFCSB_EVENT_NONE
//      MYFCSB_EVENT_HIGH_WATERMARK
//      MYFCSB_EVENT_LOW_WATERMARK
//
//  Pelda:
//      MYFCSB_Event_t event;
//
//      event = MYFCSB_sendFromISR(...);
//      if (event == MYFCSB_EVENT_HIGH_WATERMARK)
//      {
//          xTaskNotifyFromISR(...);
//      }

// Adatok helyezese ISR alatt a stream bufferbe
size_t MYFCSB_sendFromISR(MYFCSB_t* handle,
                          const void* data,
                          size_t dataLength,
                          MYFCSB_event_t* flowControlEvents,
                          BaseType_t* higherPriorityTaskWoken);

// Adatok olvasasa ISR alatt a stream bufferbol
size_t MYFCSB_receiveFromISR(MYFCSB_t* handle,
                             void* buffer,
                             size_t bufferLength,
                             MYFCSB_event_t* flowControlEvents,
                             BaseType_t* higherPriorityTaskWoken);

//==============================================================================
//  A kapott eventhez tartozo callbacket meghivja, ha az event azt mondja
//  Task contextbol hivhato. ISR-ben NE hivd!
void MYFCSB_invokeEvent(MYFCSB_t* handle, MYFCSB_event_t event);

// Manualis update. (Kenyelmi fuggveny).
// - ellenorzi az aktualis bufferallapotot
// - ha state transition tortent, visszaadja az eventet
// Nem hiv callbacket.
MYFCSB_event_t MYFCSB_update(MYFCSB_t* handle);

// Bufferben talalhato byteok szamat adja vissza
size_t MYFCSB_bytesAvailable(const MYFCSB_t* handle);

// A bufferben talalhato szabad helyet adja vissza byteokban
size_t MYFCSB_spacesAvailable(const MYFCSB_t* handle);

// A buffer meretet byteban adja vissza
size_t MYFCSB_getBufferSize(const MYFCSB_t* handle);

// A buffer aktualis allapotat adja vissza
MYFCSB_state_t MYFCSB_getState(const MYFCSB_t* handle);

// A buffer altal wrappolt freeRTOS-es stream buffer handleret adja vissza.
StreamBufferHandle_t MYFCSB_getStreamBuffer(MYFCSB_t* handle);

// Buffer alaphelyzetbe allotasa
void MYFCSB_reset(MYFCSB_t* handle);

// Felso es also kuszob szintek beallitasa
BaseType_t MYFCSB_setWatermarks(MYFCSB_t* handle,
                                size_t lowWatermark,
                                size_t highWatermark);

// Trigger szint beallitasa
BaseType_t MYFCSB_setTriggerLevel(MYFCSB_t* handle, size_t triggerLevel);

// Ures buffer alalpot lekerdezese
BaseType_t MYFCSB_isEmpty(MYFCSB_t* handle);

// Teli buffer allapot lekerdezese
BaseType_t MYFCSB_isFull(MYFCSB_t* handle);
//------------------------------------------------------------------------------
#endif //MY_FLOWCONTROLLEDSTREAMBUFFER_H_
