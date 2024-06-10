//------------------------------------------------------------------------------
//  Kooperative mukodesu eroforrasok csoportjat futtato modul
//
//    File: MyCoopResourceGroup.h
//------------------------------------------------------------------------------
#ifndef MYCOOPRESOURCEGROUP_H_
#define MYCOOPRESOURCEGROUP_H_

#include "MyRM.h"
#include "MySwTimer.h"

//------------------------------------------------------------------------------
//Cooperative eroforrasok csoportjat futtato modul konfiguracios parameterei
typedef struct
{
    //csoprot neve. (Nyomkoveteshez) Ez lesz a letrehozott taszk neve is.
    const char* name;

    //Az eroforrasokat futtato taszk prioritasa
    uint32_t taskPriority;
    //Az eroforrasokat futtato taszk szamara allokalt taszk merete
    uint32_t taskStackSize;
    //Az eroforrasokat futtato taszk buffere. Ha ez NULL, akkor dinamikusan
    //hozza letre a tasztkot.
    void* taskStackBuffer;
}coopResourceGroupConfig_t;
//------------------------------------------------------------------------------
//MyCoopResourceGroup valtozoi
typedef struct
{
    //A futtato taszkl handlere
    TaskHandle_t    taskHandle;
  #if configSUPPORT_STATIC_ALLOCATION
    StaticTask_t    taskBuffer;
  #endif

    //A manager valtozoit vedo mutex
    SemaphoreHandle_t   mutex;
  #if configSUPPORT_STATIC_ALLOCATION
    StaticSemaphore_t   mutexBuffer;
  #endif

    //letrehozaskor megadott konfiguracios adatok
    const coopResourceGroupConfig_t* cfg;

    //A csoporthoz adott eroforrasok lancolt listaja...
    resource_t* resources;
    resource_t* lastResource;

    //Eroforrasok kozos idozito managgere
    MySwTimerManager_t  timerManager;

} coopResourceGroup_t;
//------------------------------------------------------------------------------
//Eroforrasok csoportjat futtato modul letrehozasa es inicializalasa
void MyCoopResourceGroup_create(coopResourceGroup_t* group,
                                const coopResourceGroupConfig_t* cfg);

//Eroforras hozaadasa a csoporthoz.
void MyCoopResourceGroup_add(coopResourceGroup_t* group,
                             resource_t* resource);

//Kooperativ eroforrashoz tartozo esemeny beallitasa
void MyCoopResourceGroup_setResourceEvent(resource_t* resources,
                                          uint32_t events);

//Kooperativ eroforrashoz tartozo esemeny beallitasa megszakitsbol
void MyCoopResourceGroup_setResourceEventFromIsr(resource_t* resources,
                                                 uint32_t events);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MYCOOPRESOURCEGROUP_H_
