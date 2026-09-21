//------------------------------------------------------------------------------
//  Work kezeles
//
//    File: MyWork.h
//------------------------------------------------------------------------------
#ifndef MYWORK_H_
#define MYWORK_H_

#include "MyCommon.h"

typedef struct _MyWork MyWork_t;
typedef struct _MyWorkQueue MyWorkQueue_t;
typedef struct _MyWorkSync  MyWorkSync_t;

// Work altal hivott callback fuggveny tipusdefinicioja
typedef void (MyWorkHandler)(MyWork_t* work, void* callbackData);

//------------------------------------------------------------------------------
// Work szinkronizacios objektuma
struct _MyWorkSync
{
    // Varakozo objektumok lancolt listaja
    MyWorkSync_t* next;

    // Az objektum altal bezart semaphore, amira a hivo oldal varhat
    SemaphoreHandle_t semaphore;
    StaticSemaphore_t semaphoreBuffer;

    // true, ha az objektum varakozik
    bool waiting;
};
//------------------------------------------------------------------------------
// Egyetlen work leiroja
struct _MyWork
{
    // Workok lancolt lista kezelesehez pointerek
    MyWork_t* prev;
    MyWork_t* next;

    // A workhoz tartozo kezelo queue-re mutat
    MyWorkQueue_t* workQueue;
    // A work altal futtatott callback funkcio
    MyWorkHandler* handler;
    // A callback szamara atadhato tetszoleges parameter
    void* callbackData;

    // A work futasanak eloirt idopontja
    uint64_t expiry;

    // True, ha a work be van utemezve, es varja az idopontot, mikor futnia kell
    bool pending;
    // True, ha a work callbackja futtatas alatt van
    bool running;

    // A work befejezesere varakozok lancolt listajanak elso eleme.
    MyWorkSync_t* syncWaiters;
};
//------------------------------------------------------------------------------
// Workok kezeleset biztosito queue
struct _MyWorkQueue
{
    // Workok lancolt listajanak elso eleme
    MyWork_t* head;
    // Workok lancolt listajanak utolso eleme
    MyWork_t* tail;

    // A Work-hoz letrehozott statikus taszk  handlere
    TaskHandle_t taskHandle;
    StaticTask_t taskTcb;
};
//------------------------------------------------------------------------------
// Work queue inicializalasa
bool MyWork_queueInit(MyWorkQueue_t* workQueue,
                      StackType_t* taskStack,
                      uint32_t taskStackSize,
                      UBaseType_t taskPriority,
                      const char* taskName);

// Work letrehozasa
void MyWork_init(MyWork_t* work,
                 MyWorkQueue_t* workQueue,
                 MyWorkHandler* handler,
                 void* callbackData);

// Work azonnali inditasa
bool MyWork_submit(MyWork_t* work);

// Work inditasa megszakitasbol
bool MyWork_submitFromISR(MyWork_t* work,
                          BaseType_t* higherPriorityTaskWoken);

// Kesleltetett work beutemezese
bool MyWork_schedule(MyWork_t* work, TickType_t delay);

// Work utemezese megsazkitas alol
bool MyWork_scheduleFromISR(MyWork_t* work,
                            TickType_t delay,
                            BaseType_t* higherPriorityTaskWoken);

// Egy mar beutemezett work ujra utemezese
bool MyWork_reschedule(MyWork_t* work, TickType_t delay);

// Work ujra utemezese megszakitas alol
bool MyWork_rescheduleFromISR(MyWork_t* work,
                             TickType_t delay,
                             BaseType_t* higherPriorityTaskWoken);

// Work futtatasrol valo lemondas
bool MyWork_cancel(MyWork_t* work);

// Work torlese megszakitas alol
bool MyWork_cancelFromISR(MyWork_t* work, BaseType_t* higherPriorityTaskWoken);

// Szinkronizacios objektum inicializalasa
void MyWork_syncInit(MyWorkSync_t* sync);
// Work leallitasa. A rutin addig nem ter vissza, amig a work be nem fejezte
// a mukodest (amenyniben az fut.)
bool MyWork_cancelSync(MyWork_t* work, MyWorkSync_t* sync);

// A work varakozik (be van utemezve) allapot lekerdezese
bool MyWork_isPending(const MyWork_t *work);

// A work fut allapot lekerdezese
bool MyWork_isRunning(const MyWork_t *work);

// A work foglalt allapot lekerdezese
bool MyWork_isBusy(const MyWork_t *work);

// A work nyugalmi allapotban allapot lekerdezese
bool MyWork_isIdle(const MyWork_t *work);

#endif //MYWORK_H_
