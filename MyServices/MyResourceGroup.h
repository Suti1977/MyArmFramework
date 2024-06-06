//------------------------------------------------------------------------------
//  Eroforrasok csoportjat futtato modul
//
//    File: MyResourceGroup.h
//------------------------------------------------------------------------------
#ifndef MYRESOURCEGROUP_H_
#define MYRESOURCEGROUP_H_

#include "MyRM.h"
#include "MySwTimer.h"

//Eroforrasok csoportjat futtato modul konfiguracios adatai
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
}resourceGroupConfig_t;
//------------------------------------------------------------------------------
//MyResourceGroup valtozoi
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
    const resourceGroupConfig_t* cfg;

    //A csoporthoz adott eroforrasok lancolt listaja...
    resource_t* resources;
    resource_t* lastResource;

    //Eroforrasok kozos idozito managgere
    MySwTimerManager_t  timerManager;

} resourceGroup_t;
//------------------------------------------------------------------------------
//Eroforrasok csoportjat futtato modul letrehozasa es inicializalasa
void MyResourceGroup_create(resourceGroup_t* group,
                            const resourceGroupConfig_t* cfg);

//Eroforras hozaadasa a csoporthoz.
void MyResourceGroup_add(resourceGroup_t* group,
                         resource_t* resource);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MYRESOURCEGROUP_H_
