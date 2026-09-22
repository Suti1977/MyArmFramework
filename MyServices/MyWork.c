//------------------------------------------------------------------------------
//  Work kezeles
//
//    File: MyWork.c
//------------------------------------------------------------------------------
#include "MyWork.h"
#include <string.h>
#include <stddef.h>
//------------------------------------------------------------------------------
// Work hozzaadasa a lancolt listahoz. A lista az idopont alapjan van sorba
// rendezve.
static void MyWork_insert(MyWorkQueue_t* workQueue, MyWork_t* work)
{
    MyWork_t* current;
    current = workQueue->head;
    while (current != NULL)
    {
        if (work->expiry < current->expiry)
        {
            break;
        }

        current = current->next;
    }

    if (current == NULL)
    {
        // Kesobbre szurja be
        work->prev = workQueue->tail;
        work->next = NULL;

        if (workQueue->tail != NULL)
        {
            workQueue->tail->next = work;
        }
        else
        {
            workQueue->head = work;
        }

        workQueue->tail = work;
    }
    else
    {
        // Eleje szurja be
        work->next = current;
        work->prev = current->prev;

        if (current->prev != NULL)
        {
            current->prev->next = work;
        }
        else
        {
            workQueue->head = work;
        }

        current->prev = work;
    }

    work->pending = true;
}
//------------------------------------------------------------------------------
// Work torlese a lancolt listabol.
static void MyWork_remove(MyWorkQueue_t* workQueue, MyWork_t* work)
{
    if (work->prev != NULL)
    {
        work->prev->next = work->next;
    }
    else
    {
        workQueue->head = work->next;
    }

    if (work->next != NULL)
    {
        work->next->prev = work->prev;
    }
    else
    {
        workQueue->tail = work->prev;
    }

    work->prev = NULL;
    work->next = NULL;
    work->pending = false;
}
//------------------------------------------------------------------------------
// Taszk ebresztese
static void MyWork_wakeTask(MyWorkQueue_t* workQueue)
{
    if (workQueue->taskHandle != NULL)
    {
        xTaskNotifyGive(workQueue->taskHandle);
    }
}
//------------------------------------------------------------------------------
// Taszk ebresztese megszakitas alol
static void MyWork_wakeTaskFromISR(MyWorkQueue_t* workQueue,
                                  BaseType_t* higherPriorityTaskWoken)
{
    if (workQueue->taskHandle != NULL)
    {
        vTaskNotifyGiveFromISR(workQueue->taskHandle, higherPriorityTaskWoken);
    }
}
//------------------------------------------------------------------------------
static void MyWork_addSyncWaiter(MyWork_t* work, MyWorkSync_t* sync)
{
    // Szinkronizacios obejktumok lancolt listajahoz adas
    sync->next = work->syncWaiters;
    work->syncWaiters = sync;
    sync->waiting = true;
}
//------------------------------------------------------------------------------
// Taszkok ebresztese, amik az adott work-re varnak
static void MyWork_wakeSyncWaiters(MyWork_t* work)
{
    MyWorkSync_t* sync;
    sync = work->syncWaiters;
    work->syncWaiters = NULL;

    // Vegig a lancolt listan, es mindegyiken meghivja a szinkronizaciot...
    while (sync != NULL)
    {
        MyWorkSync_t* next;
        next = sync->next;

        sync->next = NULL;
        sync->waiting = false;

        xSemaphoreGive(sync->semaphore);

        sync = next;
    }
}
//------------------------------------------------------------------------------
// A workoket es az idozitest is futtato taszk
static void MyWork_task(void* argument)
{
    MyWorkQueue_t* workQueue = (MyWorkQueue_t* )argument;

    // Taszk fociklus.
    for (;;)
    {
        // Varakozas ideje. Eddig var, vagy esemenyig.
        uint32_t waitTime;

        // Minden work futtatasa, aminek az ideje letelt.
        for (;;)
        {
            MyWork_t* work;
            uint64_t now;

            // Aktualis ido lekerdezese
            // now = xTaskGetTickCount();

            // Soron kovetkezo work kivetel a listabol
            taskENTER_CRITICAL();
            work = workQueue->head;

            if (work == NULL)
            {
                // Nincs mit futtatni.
                taskEXIT_CRITICAL();
                break;
            }

            // ?? Ido lekeres lehet, hogy ide kellene ??
            // De akkor a kritikus kapu sokaig van fogva.
            now = xTaskGetTickCount();

            if (now < work->expiry)
            {
                // A soron levo vorkot nem kell meg futtatni. Kilepes es varas.
                taskEXIT_CRITICAL();
                break;
            }

            // A work torlese a listabol
            MyWork_remove(workQueue, work);

            // Jelezes, hogy a work fut.
            work->running = true;

            taskEXIT_CRITICAL();

            // Work futtatasa...
            if (work->handler != NULL)
            {
                work->handler(work, work->callbackData);
            }

            // Futas jelezes torolheto
            taskENTER_CRITICAL();
            work->running = false;
            // Ha van beregisztralva a veget varo szinkronizacios objektumok,
            // akkor azokon meghivja a varakozast.
            // De ez is csak akkor futhat, ha kozben a futo work callbackben
            // nem lett az kozben ujrainditva.
            if ((!work->pending) && (work->syncWaiters != NULL))
            {
                MyWork_wakeSyncWaiters(work);
            }

            taskEXIT_CRITICAL();
        } // for

        // Annak kiszamitasa, hogy mennyi ideig kell a taszkot altatatni a
        // kovetkezo utemezett work-ig.
        taskENTER_CRITICAL();

        if (workQueue->head == NULL)
        {
            // Nins fuggoben levo work. Vegtelen ideig alszik.
            waitTime = portMAX_DELAY;
        }
        else
        {
            uint64_t now = MyRTOS_getTick();
            MyWork_t* work = workQueue->head;

            if (now <= work->expiry)
            {
                // Letelt az ido. Nem fog varni!
                waitTime = 0;
            }
            else
            {
                // Delta szamitas
                waitTime = work->expiry - now;
            }
        }

        taskEXIT_CRITICAL();

        // Varakozas esemenyre, vagy meghataroott ideig.
        if (waitTime != 0)
        {
            (void)ulTaskNotifyTake(true, waitTime);
        }

    } // for
}
//------------------------------------------------------------------------------
// Work queue inicializalasa
bool MyWork_queueInit(MyWorkQueue_t* workQueue,
                      StackType_t* taskStack,
                      uint32_t taskStackSize,
                      UBaseType_t taskPriority,
                      const char* taskName)
{
    if (workQueue == NULL)
    {
        return false;
    }

    if (taskStack == NULL)
    {
        return false;
    }

    if (taskStackSize == 0U)
    {
        return false;
    }

    workQueue->head = NULL;
    workQueue->tail = NULL;
    workQueue->taskHandle = NULL;

    // Workoket futtato taszk letrehozasa (statikus!)
    workQueue->taskHandle = xTaskCreateStatic(MyWork_task,
                                              taskName,
                                              taskStackSize,
                                              workQueue,
                                              taskPriority,
                                              taskStack,
                                              &workQueue->taskTcb);

    if (workQueue->taskHandle == NULL)
    {
        // Hiba a taszk letrehozasnal
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------
// Work letrehozasa
void MyWork_init(MyWork_t* work,
                 MyWorkQueue_t* workQueue,
                 MyWorkHandler* handler,
                 void* callbackData)
{
    ASSERT(work);

    work->prev = NULL;
    work->next = NULL;

    work->workQueue = workQueue;
    work->handler = handler;
    work->callbackData=callbackData;

    work->expiry = 0;

    work->pending = false;
    work->running = false;

    work->syncWaiters = NULL;
}
//------------------------------------------------------------------------------
// Work azonnali inditasa
bool MyWork_submit(MyWork_t* work)
{
    MyWorkQueue_t* workQueue;
    bool wasHead;
    ASSERT(work);
    workQueue = work->workQueue;

    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    taskENTER_CRITICAL();

    // Ha a work mar fut, vagy mar varakozik, akkor nem adja a listahoz
    if (work->pending || work->running)
    {
        taskEXIT_CRITICAL();
        return false;
    }


    // A mostani idopontot allitja be hozza, igy azonnal el fog indulni.
    work->expiry = MyRTOS_getTick();

    // Work hozzaadasa a lancolt listahoz.
    MyWork_insert(workQueue, work);

    // Ha ez a lista elso eleme, akkor a taszkot ebreszteni kell!
    wasHead = (workQueue->head == work);

    taskEXIT_CRITICAL();

    if (wasHead)
    {
        // taszk ebresztese
        MyWork_wakeTask(workQueue);
    }

    return true;
}
//------------------------------------------------------------------------------
// Kesleltetett work beutemezese
bool MyWork_schedule(MyWork_t* work, TickType_t delay)
{
    MyWorkQueue_t* workQueue;
    bool wasHead;
    uint64_t now = MyRTOS_getTick();
    ASSERT(work);
    workQueue = work->workQueue;

    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    taskENTER_CRITICAL();

    // Ha a work mar fut, vagy mar varakozik, akkor nem adja a listahoz
    if (work->pending || work->running)
    {
        taskEXIT_CRITICAL();
        return false;
    }

    // Futtatas idopontjanak kiszamitasa
    work->expiry = now + delay;
    // Hozzaadas a listahoz
    MyWork_insert(workQueue, work);

    // csak akkor kelti a taszkot, ha ez a legkorabbi elem
    wasHead = (workQueue->head == work);

    taskEXIT_CRITICAL();

    if (wasHead)
    {
        // Taszk ebresztese
        MyWork_wakeTask(workQueue);
    }

    return true;
}
//------------------------------------------------------------------------------
// Egy mar beutemezett work ujra utemezese
bool MyWork_reschedule(MyWork_t* work, TickType_t delay)
{
    MyWorkQueue_t* workQueue;
    uint64_t now = MyRTOS_getTick();
    ASSERT(work);

    workQueue = work->workQueue;

    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    taskENTER_CRITICAL();

    // Ha pending, akkor levesszuk a jelenlegi helyerol.
    // Ha running, akkor NEM szabad Remove()-ot hivni,
    // mert a running work nincs a listán.
    if (work->pending)
    {
        MyWork_remove(workQueue, work);
    }

    // Uj futasi idopont kiszamitasa
    work->expiry = now + delay;

    // Listahoz adas
    MyWork_insert(workQueue, work);

   //  Fontos:
   //
   //  Ha a work eppen running volt, akkor most egyszerre:
   //  - running = true
   //  - pending = true
   //  Ez szanekos!
   //
   //  A jelenlegi handler befejezodik, majd a work task
   //  a kovetkező iteracioban latni fogja a pending peldanyt.

    taskEXIT_CRITICAL();

    // Taszk ebresztese
    MyWork_wakeTask(workQueue);

    return true;
}
//------------------------------------------------------------------------------
// Work futtatasrol valo lemondas
bool MyWork_cancel(MyWork_t* work)
{
    MyWorkQueue_t* workQueue;
    ASSERT(work);
    workQueue = work->workQueue;

    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    taskENTER_CRITICAL();

    // Ha nincs beutemezve a work, akkor nincs mit torolni.
    if (!work->pending)
    {
        taskEXIT_CRITICAL();
        return false;
    }

    // Torles a listabol
    MyWork_remove(workQueue, work);
    taskEXIT_CRITICAL();

    // taszk ebresztese
    MyWork_wakeTask(workQueue);

    return true;
}
//------------------------------------------------------------------------------
// Work inditasa megszakitasbol
bool MyWork_submitFromISR(MyWork_t* work, BaseType_t* higherPriorityTaskWoken)
{
    MyWorkQueue_t* workQueue;
    uint64_t now = MyRTOS_getTickFromIsr();
    bool wasHead;
    ASSERT(work);
    ASSERT(higherPriorityTaskWoken);
    workQueue = work->workQueue;
    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    {
        UBaseType_t interruptStatus;

        interruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        if (work->pending || work->running)
        {
            portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
            return false;
        }


        work->expiry = now;

        MyWork_insert(workQueue, work);

        wasHead = (workQueue->head == work);

        portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
    }

    if (wasHead)
    {
        MyWork_wakeTaskFromISR(workQueue, higherPriorityTaskWoken);
    }

    return true;
}
//------------------------------------------------------------------------------
// Work utemezese megsazkitas alol
bool MyWork_scheduleFromISR(MyWork_t* work,
                            TickType_t delay,
                            BaseType_t* higherPriorityTaskWoken)
{
    MyWorkQueue_t* workQueue;
    uint64_t now = MyRTOS_getTickFromIsr();
    bool wasHead;
    ASSERT(work);
    ASSERT(higherPriorityTaskWoken);
    workQueue = work->workQueue;

    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    {
        UBaseType_t interruptStatus;

        interruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        if (work->pending || work->running)
        {
            portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
            return false;
        }


        work->expiry = now + delay;

        MyWork_insert(workQueue, work);

        wasHead = (workQueue->head == work);

        portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
    }

    if (wasHead)
    {
        MyWork_wakeTaskFromISR(workQueue, higherPriorityTaskWoken);
    }

    return true;
}
//------------------------------------------------------------------------------
// Work ujra utemezese megszakitas alol
bool MyWork_rescheduleFromISR(MyWork_t* work,
                              TickType_t delay,
                              BaseType_t* higherPriorityTaskWoken)
{
    MyWorkQueue_t* workQueue;
    uint64_t now = MyRTOS_getTickFromIsr();
    ASSERT(work);
    ASSERT(higherPriorityTaskWoken);
    workQueue = work->workQueue;

    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    {
        UBaseType_t interruptStatus;

        interruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        /// if (work->running)
        /// {
        ///     portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
        ///     return false;
        /// }

        if (work->pending)
        {
            MyWork_remove(workQueue, work);
        }


        work->expiry = now + delay;

        MyWork_insert(workQueue, work);

        portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
    }

    MyWork_wakeTaskFromISR(workQueue, higherPriorityTaskWoken);

    return true;
}
//------------------------------------------------------------------------------
// Work torlese megszakitas alol
bool MyWork_cancelFromISR(MyWork_t* work,
                          BaseType_t* higherPriorityTaskWoken)
{
    MyWorkQueue_t* workQueue;
    ASSERT(work);
    ASSERT(higherPriorityTaskWoken);
    workQueue = work->workQueue;
    // Vedelem, ha nem lenne a work hozzaadva vakai queue-hoz.
    if (workQueue == NULL)
    {
        return false;
    }

    {
        UBaseType_t interruptStatus;

        interruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

        if (!work->pending)
        {
            portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
            return false;
        }

        MyWork_remove(workQueue, work);

        portCLEAR_INTERRUPT_MASK_FROM_ISR(interruptStatus);
    }

    MyWork_wakeTaskFromISR(workQueue, higherPriorityTaskWoken);

    return true;
}
//------------------------------------------------------------------------------
// Szinkronizacios objektum letrehozasa
void MyWork_syncInit(MyWorkSync_t* sync)
{
    ASSERT(sync);

    sync->next = NULL;
    sync->waiting = false;

    // Szemaphor eletrehozasa
    sync->semaphore = xSemaphoreCreateBinaryStatic(&sync->semaphoreBuffer);
    ASSERT(sync->semaphore);

    // Ures semaphore-al indul.
    xSemaphoreTake(sync->semaphore,0);
}
//------------------------------------------------------------------------------
// Work leallitasa. A rutin addig nem ter vissza, amig a work be nem fejezte
// a mukodest (amenyniben az fut.)
bool MyWork_cancelSync(MyWork_t* work, MyWorkSync_t* sync)
{
    MyWorkQueue_t* workQueue;
    TaskHandle_t currentTask;

    if ((work == NULL) || (sync == NULL))
    {
        return false;
    }

    workQueue = work->workQueue;

    if (workQueue == NULL)
    {
        return false;
    }

    // A jelenlegi taszk handler lekerdezese. A notifikacio neki fog majd menni.
    currentTask = xTaskGetCurrentTaskHandle();

    // Vedelem Sajat magaban nem indit varakozast. Ez akkor lenne, ha peldaul
    // egy futo work meghivna ezt a fuggvenyt.
    if (currentTask == workQueue->taskHandle)
    {
        return false;
    }


    taskENTER_CRITICAL();

    // Ha mar korabban meg lett hiva a szinkronizalt leallitas erre az
    // objektumra, akkor itt nem csinal semmit.
    if (sync->waiting)
    {
        taskEXIT_CRITICAL();

        return false;
    }

    // Ha a work be van utemezve, de meg nem fut, akkor egyszeruen torli a
    // queue-bol
    if (work->pending)
    {
        MyWork_remove(workQueue,work);
    }


    // Ha a work nem fut, akkor ebred a taszk, amiben majd lefut a sync
    if (!work->running)
    {
        taskEXIT_CRITICAL();
        MyWork_wakeTask(workQueue);

        return true;
    }


    // A work meg fut. A varakozast kero sync objektumot beregisztralja ala
    MyWork_addSyncWaiter(work, sync);

    taskEXIT_CRITICAL();

    // Taszk ebresztese, mivela  varakozok listaja valtozott
    MyWork_wakeTask(workQueue);

    // Varakozas a sync objektumra
    xSemaphoreTake(sync->semaphore, portMAX_DELAY);


    return true;
}
//------------------------------------------------------------------------------
// A work varakozik (be van utemezve) allapot lekerdezese
bool MyWork_isPending(const MyWork_t* work)
{
    bool pending;

    ASSERT(work);

    taskENTER_CRITICAL();
    pending = work->pending;
    taskEXIT_CRITICAL();

    return pending;
}

//------------------------------------------------------------------------------
// A work fut allapot lekerdezese
bool MyWork_isRunning(const MyWork_t* work)
{
    bool running;

    ASSERT(work);

    taskENTER_CRITICAL();
    running = work->running;
    taskEXIT_CRITICAL();

    return running;
}
//------------------------------------------------------------------------------
// A work foglalt allapot lekerdezese
bool MyWork_isBusy(const MyWork_t* work)
{
    bool busy;

    ASSERT(work);

    taskENTER_CRITICAL();

    busy = (work->pending) || (work->running);

    taskEXIT_CRITICAL();

    return busy;
}
//------------------------------------------------------------------------------
// A work nyugalmi allapotban allapot lekerdezese
bool MyWork_isIdle(const MyWork_t* work)
{
    bool idle;

    ASSERT(work);

    taskENTER_CRITICAL();
    idle = (!work->pending) && (!work->running);
    taskEXIT_CRITICAL();

    return idle;
}
//------------------------------------------------------------------------------
