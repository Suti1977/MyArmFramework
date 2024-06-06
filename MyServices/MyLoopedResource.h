//------------------------------------------------------------------------------
//  Loop csoportba helyezheto eroforras
//
//    File: MyLoopedResource.h
//------------------------------------------------------------------------------
#ifndef MYLOOPEDRESOURCE_H_
#define MYLOOPEDRESOURCE_H_

#include "MyRM.h"
#include "MySM.h"
#include "MySwTimer.h"

//Eroforras inditasat eloiro esemeny flag
#define GROUPED_RESOURCE_EVENT__START_REQUEST   BIT(30)
//Eroforras leallitasat kero esemeny flag
#define GROUPED_RESOURCE_EVENT__STOP_REQUEST    BIT(31)
//------------------------------------------------------------------------------
//Az eroforrast vezerlo valtozok halmaza
typedef struct
{
    //A taszknak kuldott eventek
    EventBits_t events;

    //true, ha a loop azert fut, mert az applikacio altal megadott idozites
    //letelt.
    bool timed;

    //A ciklusnak erre az esemenyre kell varnia. Ezt a loop-ban, vagy a Starting
    //funkciokban is modosithatja az applikacio
    EventBits_t waitedEvents;

    //A varakozasi ido tick-ben, amennyi ido mulva ujra futnia kell a loopnak.
    uint32_t waitTime;

    //leallitasi kerelem aktiv jelzes az eroforrasra. A loopban futo folyamat
    //ez alapjan tudhatja, hogy leallitasi kerelmet kapott a managertol.
    bool stopRequest;


    //Feladat kesz jelzes. Azoknal az eroforrasoknal, melyek elinditasa utan
    //azok mukodese egy ido utan befejezodik, ott ezen a flagen keresztul tudja
    //jelezni azt a start vagy loop callbackban futtatott kod.
    //Hatasara a manager fele RESOURCE_DONE allapot kerul jelzesre, es az
    //eroforras ujra a start feltetelre fog varni.
    bool done;

    //Az applikacio jelezheti, hogy nem szabad az eroforrast leallitani.
    //Ez akkor fontos, ha olyan folyamatot futtatunk, melynek mindenkepen vegig
    //kell tudni futnia.
    bool prohibitStop;

    //Az eroforras mukodik jelzes. Alapertelmezesben ez ben van allitva, de van
    //lehetoseg a startFunc() callbackben a flaget torolni. Igy a loop funkcio
    //ugy indul el, hogy az eroforras manager fele nem jelez "elindult"
    //allapotot.
    //A loop-ban igy van lehetoseg az eroforrast inditani, majd ha kesz, akkor
    //a flaget true-ba allitani, mely hatasara a manager fele "elindult" jelzest
    //kuld.
    //Az applikacio a loop-bol kilepve jelezheti, ha elindult az eroforras.
    bool run;
} loopedRsource_control_t;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Az eroforras inicializalasakor meghivodo callback definicioja
typedef status_t loopedResourceInitFunc_t(void* callbackData);
//Az eroforras inditasi kerelmekor hivott callback rutin definicioja
typedef void loopedResourceStartReqFunc_t(void* callbackData);
//Az eroforras leallitasi kerelmenel hivott callback rutin definicioja
typedef void loopedResourceStopReqFunc_t(void* callbackData);
//Az eroforras elinditasara, a taszkban meghivodo rutin
typedef status_t loopedResourceStartFunc_t(void* callbackData,
                                           loopedRsource_control_t* control);
//Az eroforras leallitasi kerelme utan a taszkban meghivodo rutin
typedef status_t loopedResourceStopFunc_t(void* callbackData);
//Hiba eseten, a taszkbol hivott callback definicioja
typedef void loopedResourceErrorFunc_t(void* callbackData, status_t errorCode);
//Eroforrast futtato callback definicioja
typedef status_t loopedResourceLoopFunc_t(void* callbackData,
                                          loopedRsource_control_t* control);
//------------------------------------------------------------------------------
//Taszkal rendelkezo eroforras inicializalasanal hasznalt struktura.
typedef struct
{
    //Az eroforras neve.
    const char* name;

    //Az eroforras callbckjai szamara atadott tetszoleges parameter
    void* callbackData;

    //Az eroforras inicializalasakor meghivodo callback definicioja
    //Eroforras manager taszkja alol hivva.
    loopedResourceInitFunc_t* initFunc;
    //Az eroforras inditasi kerelmekor hivott callback rutin definicioja
    //Eroforras manager taszkja alol hivva.
    loopedResourceStartReqFunc_t* startRequestFunc;
    //Az eroforras leallitasi kerelmenel hivott callback rutin definicioja
    //Eroforras manager taszkja alol hivva.
    loopedResourceStopReqFunc_t* stopRequestFunc;

    //Az eroforras elinditasara, a taszkban meghivodo rutin
    loopedResourceStartFunc_t* startFunc;
    //Az eroforras leallitasi kerelme utan a taszkban meghivodo rutin
    loopedResourceStopFunc_t* stopFunc;
    //Hiba eseten, a taszkbol hivott callback definicioja
    loopedResourceErrorFunc_t* errorFunc;
    //Eroforrast futtato callback definicioja
    loopedResourceLoopFunc_t* loopFunc;
} loopedResource_config_t;
//------------------------------------------------------------------------------
//Eroforras bovitmeny valtozoi.
typedef struct
{
    //letrehozaskor kapott konfiguracio
    const loopedResource_config_t* cfg;

    //A hozza tartozo eroforrasra mutat (a tulajdonosra)
    resource_t* resource;

    //Csoport lancolt listajaban a kovetkezo elemet cimzo pointer
    resource_t* next;

    //Az eroforrashoz tartozo vezerlo esemeny flagek
    //Ez kerul masoloasra a control strukturaba futtataskor.
    //Nem szabad hasznalni az eroforrason belul!
    uint32_t inputEvents;

    //Az eroforarst futtato csoportra mutat
    struct resourceGroup_t* group;

    //Vezerlo valtozok, melyeken keresztul adjuk at a fuggevyneknek a
    //kapott esemeny flageket, de ezen keresztul modosithatjak a callbackek
    //a varakozasi idot, vagy irhatjak elo a varakozasra az eventeket.
    loopedRsource_control_t control;

    //Eroforras futtatas allapotgepe
    MySM_t sm;

    //Loopot idozito timer. (A csoporthoz valo hozzaadaskor kerul beallitasra,
    //es a csoport managerehez regisztralasra. )
    MySwTimer_t loopTimer;

    //Utolso hibakod
    status_t errorCode;
} loopedResourceExtension_t;
//------------------------------------------------------------------------------
//Taszkal tamogatott eroforras letrehozasa
void MyLoopedResource_create(resource_t* resource,
                             loopedResourceExtension_t* ext,
                             const loopedResource_config_t* cfg);

//Eroforras kiertekelese/futtatasa
//[Csoport taszkjabol hiva]
void MyLoopedResource_runResource(resource_t* resource);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MYLOOPEDRESOURCE_H_
