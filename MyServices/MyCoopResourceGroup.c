//------------------------------------------------------------------------------
//  Kooperative mukodesu eroforrasok csoportjat futtato modul
//
//    File: MyCoopResourceGroup.c
//------------------------------------------------------------------------------
#include "MyCoopResourceGroup.h"
#include "MyCoopResource.h"
//#include "MyCoopTimer.h"
#include <string.h>

#define MY_COOP_RESOURCE_GROUP_TRACE    0

static void __attribute__((noreturn)) MyCoopResourceGroup_task(void* taskParam);
static status_t MyCoopResourceGroup_resource_init(void* param);
static status_t MyCoopResourceGroup_resource_start(void* param);
static status_t MyCoopResourceGroup_resource_stop(void* param);

#define COOP_RESOURCE_GROUP_EVENET__RUN_REQUEST  BIT(1)
//------------------------------------------------------------------------------
//Eroforrasok csoportjat futtato modul letrehozasa es inicializalasa
void MyCoopResourceGroup_create(coopResourceGroup_t* group,
                                const coopResourceGroupConfig_t* cfg)
{
    memset(group, 0, sizeof(coopResourceGroup_t));
    group->cfg=cfg;

    //Eroforrasok csoportjat futtato taszk letrehozasa...
    //Ha a konfiguracioban van megadva stack buffer, akkor statikus
    //task jon letre. Ha nincs, akkor dinamikusan allokalja a taszkot.
    if (cfg->taskStackBuffer)
    {   //Van allokalva taszk. Statilus taszk letrehozas
        #if configSUPPORT_STATIC_ALLOCATION
        group->taskHandle=xTaskCreateStatic( MyCoopResourceGroup_task,
                                             cfg->name,
                                             cfg->taskStackSize,
                                             (void*)group,
                                             cfg->taskPriority,
                                             cfg->taskStackBuffer,
                                             &group->taskBuffer);
        ASSERT(group->taskHandle);

        #else
        //A FreeRTOS-ben engedelyezni kell a statikus allokaciot, ha ebben a
        //modban hasznaljuk a lib-et!
        ASSERT(0);
        #endif
    } else
    {
        //Eroforrast futtato taszk letrehozasa
        if (xTaskCreate(MyCoopResourceGroup_task,
                        cfg->name,
                        (const configSTACK_DEPTH_TYPE) cfg->taskStackSize,
                        (void*)group,
                        cfg->taskPriority,
                        &group->taskHandle)!=pdPASS)
        {
            ASSERT(0);
        }
    }


    //Modul valtozoit vedo mutex letrehozasa...
    #if configSUPPORT_STATIC_ALLOCATION
    group->mutex=xSemaphoreCreateRecursiveMutexStatic(&group->mutexBuffer);
    #else
    group->mutex=xSemaphoreCreateRecursiveMutex();
    #endif

    //Idozitok managerenek letrehozasa
    MySwTimer_initManager(&group->timerManager);
}
//------------------------------------------------------------------------------
//Eroforras hozaadasa a csoporthoz.
void MyCoopResourceGroup_add(coopResourceGroup_t* group,
                         resource_t* resource)
{
    xSemaphoreTake(group->mutex, portMAX_DELAY);

    //Az eroforras vezerlo callbackjeit a modulhoz iranyitjuk...
    resource->funcs.init =MyCoopResourceGroup_resource_init;
    resource->funcs.start=MyCoopResourceGroup_resource_start;
    resource->funcs.stop =MyCoopResourceGroup_resource_stop;
    resource->funcsParam =resource;

    coopResourceExtension_t* ext=(coopResourceExtension_t*) resource->ext;

    //Az eroforrast az altala kezelt elemek lancolt listajahoz adjuk...
    if (group->resources==NULL)
    {   //Ez az elso elem
        group->resources=resource;
    } else
    {   //Mar van eleme a listanak. Az utan fuzzuk a most hozzaadottat...
        ((coopResourceExtension_t*)group->lastResource->ext)->next = resource;
    }
    //Ez lesz a lista utolso eleme.
    ext->next=NULL;
    group->lastResource=resource;

    //Az eroforrashoz megjegyzi a hozza tartozo csoportot is.
    ext->group=(struct coopResourceGroup_t*) group;

    //Az loop idozito hozzaadasa az idozites managerhez...
    MyCoopTimer_createTimer(resource, &ext->loopTimer);

    xSemaphoreGive(group->mutex);
}
//------------------------------------------------------------------------------
//Egy eroforras init-kor hivodo callback
//[MyRM taszkbol hivva]
static status_t MyCoopResourceGroup_resource_init(void* param)
{
    status_t status=kStatus_Success;
    resource_t* resource=(resource_t*) param;
    coopResourceExtension_t* ext=(coopResourceExtension_t*) resource->ext;

    #if MY_COOP_RESOURCE_GROUP_TRACE
    printf("MyCoopResourceGroup_resource_init()\n");
    #endif

    if (ext->cfg->initFunc)
    {   //init funkcio meghivasa, mivel van ilyen beallitva
        status=ext->cfg->initFunc(ext->cfg->callbackData);
    }

    //Megj: Ha a callback nem kStatus_Success-el terne vissza, akkor a modul
    //      hiba allapotot fog felvenni. Ezt a MyRM Eroforras manager oldja meg
    return status;
}
//------------------------------------------------------------------------------
//Egy eroforras inditasi kerelmekor hivodo callback
//[MyRM taszkbol hivva]
static status_t MyCoopResourceGroup_resource_start(void* param)
{
    status_t status=kStatus_Success;
    resource_t* resource=(resource_t*) param;
    coopResourceExtension_t* ext=(coopResourceExtension_t*) resource->ext;

    #if MY_COOP_RESOURCE_GROUP_TRACE
    printf("MyCoopResourceGroup_resource_start()\n");
    #endif

    if (ext->cfg->startRequestFunc)
    {   //Inditasi kerelem funkcio meghivasa, mivel van ilyen beallitva
        ext->cfg->startRequestFunc(ext->cfg->callbackData);
    }

    MyCoopResourceGroup_setResourceControlEvent(resource,
                                         MY_COOP_RESOURCE_EVENT__START_REQUEST);

    return status;
}
//------------------------------------------------------------------------------
//Egy eroforras leallitasi kerelmekor hivodo callback
//[MyRM taszkbol hivva]
static status_t MyCoopResourceGroup_resource_stop(void* param)
{
    status_t status=kStatus_Success;
    resource_t* resource=(resource_t*) param;
    coopResourceExtension_t* ext=(coopResourceExtension_t*) resource->ext;

    #if MY_COOP_RESOURCE_GROUP_TRACE
    printf("MyCoopResourceGroup_resource_stop()\n");
    #endif


    if (ext->cfg->stopRequestFunc)
    {   //Lealitasi kerelem funkcio meghivasa, mivel van ilyen beallitva
        ext->cfg->stopRequestFunc(ext->cfg->callbackData);
    }

    MyCoopResourceGroup_setResourceControlEvent(resource,
                                         MY_COOP_RESOURCE_EVENT__STOP_REQUEST);

    return status;
}
//------------------------------------------------------------------------------
//Eroforrasokat futtato taszk
static void __attribute__((noreturn)) MyCoopResourceGroup_task(void* taskParam)
{
    coopResourceGroup_t* this=(coopResourceGroup_t*) taskParam;

    uint32_t waitTime=portMAX_DELAY;

    //Generalt esemeny, melyre indulaskor az allapotgepek le tudnak futni, es
    //fel tudjak venni az alapallapotot.
    xTaskNotify(this->taskHandle,
                COOP_RESOURCE_GROUP_EVENET__RUN_REQUEST,
                eSetBits);

    //Csoport fociklus...
    while(1)
    {
        //Varakozas esemenyre, vagy idozitesre...
        uint32_t events;        

        xTaskNotifyWait(0, 0xff, &events, waitTime);

        //TimerManager futtatasa...
        //A futtatas kozben a lejart eroforras idozitokhoz tartozo expired
        //callback meghivodik, es az egyes eroforrasokhoz beallitasra kerul
        //a kiszolgalast kerelmezo esemeny.
        MySwTimer_runManager(&this->timerManager, MyRTOS_getTick());

        //eroforrasok futtatasa
        resource_t* resource=this->resources;
        while(resource)
        {
            coopResourceExtension_t* ext;
            ext=(coopResourceExtension_t*) resource->ext;

            MyCoopResource_runResource(resource);

            resource=ext->next;
        }

        //Idozites managerben az ido frissitese
        MySwTimer_setTime(&this->timerManager, MyRTOS_getTick());
        //Taszk varakozasi ido lekerdezese
        waitTime=MySwTimer_getWaitTime32(&this->timerManager);
    }
}
//------------------------------------------------------------------------------
//Kooperativ eroforrashoz tartozo esemeny beallitasa
void MyCoopResourceGroup_setResourceEvent(resource_t* resources,
                                          uint32_t events)
{
    coopResourceExtension_t* ext=(coopResourceExtension_t*)resources->ext;
    coopResourceGroup_t* group=(coopResourceGroup_t*)ext->group;

    //Esemeny hozzaadasa
    MY_ENTER_CRITICAL();
    ext->inputEvents_async |= events;
    ext->controlEvents_async |= MY_COOP_RESOURCE_EVENT__INPUT_EVENT;
    MY_LEAVE_CRITICAL();

    //Eroforrasokat futtato csoportos szal ebresztese
    xTaskNotify(group->taskHandle,
                COOP_RESOURCE_GROUP_EVENET__RUN_REQUEST,
                eSetBits);
}
//------------------------------------------------------------------------------
//Kooperativ eroforrashoz tartozo esemeny beallitasa megszakitsbol
void MyCoopResourceGroup_setResourceEventFromIsr(resource_t* resources,
                                                 uint32_t events)
{
    coopResourceExtension_t* ext=(coopResourceExtension_t*)resources->ext;
    coopResourceGroup_t* group=(coopResourceGroup_t*)ext->group;

    //Esemeny hozzaadasa
    ext->inputEvents_async |= events;
    ext->controlEvents_async |= MY_COOP_RESOURCE_EVENT__INPUT_EVENT;

    //Eroforrasokat futtato csoportos szal ebresztese...

    BaseType_t higherPriorityTaskWoken=pdFALSE;
    xTaskNotifyFromISR(group->taskHandle,
                       events,
                       eSetBits,
                       &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
//------------------------------------------------------------------------------
//Kooperativ eroforrashoz tartozo vezerlo esemeny beallitasa
void MyCoopResourceGroup_setResourceControlEvent(resource_t* resources,
                                                 uint32_t events)
{
    coopResourceExtension_t* ext=(coopResourceExtension_t*)resources->ext;
    coopResourceGroup_t* group=(coopResourceGroup_t*)ext->group;

    //Esemeny hozzaadasa
    MY_ENTER_CRITICAL();
    ext->controlEvents_async |= events;
    MY_LEAVE_CRITICAL();

    //Eroforrasokat futtato csoportos szal ebresztese
    xTaskNotify(group->taskHandle,
                COOP_RESOURCE_GROUP_EVENET__RUN_REQUEST,
                eSetBits);
}
//------------------------------------------------------------------------------
//Kooperativ eroforrashoz tartozo idozites letelt esemeny beallitasa. A timer
//manager alol hivodik.
void MyCoopResourceGroup_setResourceTimerExpired(resource_t* resources)
{
    coopResourceExtension_t* ext=(coopResourceExtension_t*)resources->ext;
    coopResourceGroup_t* group=(coopResourceGroup_t*)ext->group;

    //Esemeny hozzaadasa
    ext->controlEvents_async |= MY_COOP_RESOURCE_EVENT__TIMER_EXPIRED;
}
//------------------------------------------------------------------------------
