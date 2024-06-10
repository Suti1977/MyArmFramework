//------------------------------------------------------------------------------
//  Azonos taszkon osztozo kooperative mukodesu eroforras
//
//    File: MyCoopResource.h
//------------------------------------------------------------------------------
#ifndef MYCOOPRESOURCE_H_
#define MYCOOPRESOURCE_H_

#include "MyRM.h"
#include "MySM.h"
#include "MyCoopTimer.h"
#include "MyCoopResourceGroup.h"

//Eroforras esemenyt kapott, mely miatt azt futtatni kell
#define MY_COOP_RESOURCE_EVENT__INPUT_EVENT     BIT(28)
//Eroforrashoz rendelt valamelyik timer lejart, futtatni kell az eroforrast
#define MY_COOP_RESOURCE_EVENT__TIMER_EXPIRED   BIT(29)
//Eroforras inditasat eloiro esemeny flag
#define MY_COOP_RESOURCE_EVENT__START_REQUEST   BIT(30)
//Eroforras leallitasat kero esemeny flag
#define MY_COOP_RESOURCE_EVENT__STOP_REQUEST    BIT(31)
//------------------------------------------------------------------------------
//Az eroforrast vezerlo valtozok halmaza
typedef struct
{
    //Az eroforrasnak kuldott eventek
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
} coopResource_control_t;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//Az eroforras inicializalasakor meghivodo callback definicioja
typedef status_t coopResourceInitFunc_t(void* callbackData);
//Az eroforras inditasi kerelmekor hivott callback rutin definicioja
typedef void coopResourceStartReqFunc_t(void* callbackData);
//Az eroforras leallitasi kerelmenel hivott callback rutin definicioja
typedef void coopResourceStopReqFunc_t(void* callbackData);
//Az eroforras elinditasara, a taszkban meghivodo rutin
typedef status_t coopResourceStartFunc_t(void* callbackData,
                                         coopResource_control_t* control);
//Az eroforras leallitasi kerelme utan a taszkban meghivodo rutin
typedef status_t coopResourceStopFunc_t(void* callbackData);
//Hiba eseten, a taszkbol hivott callback definicioja
typedef void coopResourceErrorFunc_t(void* callbackData, status_t errorCode);
//Eroforrast futtato callback definicioja
typedef status_t coopResourceLoopFunc_t(void* callbackData,
                                          coopResource_control_t* control);
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
    coopResourceInitFunc_t* initFunc;
    //Az eroforras inditasi kerelmekor hivott callback rutin definicioja
    //Eroforras manager taszkja alol hivva.
    coopResourceStartReqFunc_t* startRequestFunc;
    //Az eroforras leallitasi kerelmenel hivott callback rutin definicioja
    //Eroforras manager taszkja alol hivva.
    coopResourceStopReqFunc_t* stopRequestFunc;

    //Az eroforras elinditasara, a taszkban meghivodo rutin
    coopResourceStartFunc_t* startFunc;
    //Az eroforras leallitasi kerelme utan a taszkban meghivodo rutin
    coopResourceStopFunc_t* stopFunc;
    //Hiba eseten, a taszkbol hivott callback definicioja
    coopResourceErrorFunc_t* errorFunc;
    //Eroforrast futtato callback definicioja
    coopResourceLoopFunc_t* loopFunc;
} coopResource_config_t;
//------------------------------------------------------------------------------
//Eroforras bovitmeny valtozoi.
typedef struct
{
    //letrehozaskor kapott konfiguracio
    const coopResource_config_t* cfg;

    //A hozza tartozo eroforrasra mutat (a tulajdonosra)
    resource_t* resource;

    //Csoport lancolt listajaban a kovetkezo elemet cimzo pointer
    resource_t* next;

    //Az eroforrashoz tartozo kulso esemeny flagek
    //Ez kerul masoloasra a control strukturaba futtataskor.
    //Nem szabad hasznalni az eroforrason belul!
    uint32_t inputEvents;

    //Az eroforarst futtato csoportra mutat
    struct coopResourceGroup_t* group;

    //Vezerlo valtozok, melyeken keresztul adjuk at a fuggevyneknek a
    //kapott esemeny flageket, de ezen keresztul modosithatjak a callbackek
    //a varakozasi idot, vagy irhatjak elo a varakozasra az eventeket.
    coopResource_control_t control;

    //Az eroforrashoz beallitott vezerlo esemenyek. Ezek kerulnek atmasolasra
    //a controlEvents mezokbe, az eroforras futtatasakor.
    //Nem szabad hasznalni az eroforrason belul!
    uint32_t controlEvnts_async;

    //Az eroforrasnak kuldott vezerlo esemenyek
    EventBits_t controlEvents;

    //Eroforras futtatas allapotgepe
    MySM_t sm;

    //Loopot idozito timer. (Az eroforras  csoporthoz valo hozzaadaskor kerul
    //beallitasra, es a kooperativ eroforras csoport managerehez
    //beregisztralasra.)
    MyCoopTimer_t loopTimer;

    //Utolso hibakod
    status_t errorCode;
} coopResourceExtension_t;
//------------------------------------------------------------------------------
//Taszkal tamogatott eroforras letrehozasa
void MyCoopResource_create(resource_t* resource,
                           coopResourceExtension_t* ext,
                           const coopResource_config_t* cfg);

//Eroforras kiertekelese/futtatasa
//[Csoport taszkjabol hiva]
void MyCoopResource_runResource(resource_t* resource);

//A kooperativ eroforrashoz tartozo taszk handlerenek lekerdezese
static inline
TaskHandle_t MyCoopResource_getTaskHandler(resource_t* resource)
{
    return ((coopResourceGroup_t*)((coopResourceExtension_t*)resource->ext)->group)->taskHandle;
}

//Kooperativ eroforrasnak esemeny kuldese
void MyCoopResource_setEvent(resource_t* resource, uint32_t event);

//Kooperativ eroforrasnak esemeny kuldese megszakitasbol
void MyCoopResource_setEventFromIsr(resource_t* resource, uint32_t event);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MYCOOPRESOURCE_H_

