//------------------------------------------------------------------------------
//  LED vezerlo motor (eroforraskent implementalva)
//
//    File: MyLedEngine.h
//------------------------------------------------------------------------------
#ifndef MYLEDENGINE_H_
#define MYLEDENGINE_H_

#include "MyTaskedResource.h"
//------------------------------------------------------------------------------
//MyLedEngine hasznalatbavetelekor hivodo callback definicioja. Ebben kell a
//hasznalt eroforrasok igenyleset elkezdeni.
typedef status_t MyLedEngine_initFunc_t(void* callbackData);
//LedEngin mukodesenek vegen hivodo callback definicioja. Ebben kell a
//hasznalt eroforrasokrol valo lemondast megvalositani.
typedef status_t MyLedEngine_deinitFunc_t(void* callbackData);
//------------------------------------------------------------------------------
//MyLedEngine konfiguracios struktura, melyet initkor kell megadni
typedef struct
{
    //Az eroforras neve. Ugyan ezt hasznalja a letrehozott taszk nevenek is.
    const char* name;
    //Az eroforrast futtato taszk prioritasa
    uint32_t taskPriority;
    //Az eroforrast futtato taszk szamara allokalt taszk merete
    uint32_t taskStackSize;

    //MyLedEngine hasznalatbavetelekor hivodo callback. Ebben kell a
    //hasznalt eroforrasok igenyleset elkezdeni.
    MyLedEngine_initFunc_t* initFunc;

    //LedEngin mukodesenek vegen hivodo callback. Ebben kell a
    //hasznalt eroforrasokrol valo lemondast megvalositani.
    MyLedEngine_deinitFunc_t* deinitFunc;

    //callbackek szamara atadando teszoleges adat
    void* callbackData;
} MyLedEngine_config_t;
//------------------------------------------------------------------------------
//A led vezerlo programoknak atadott vezerlo struktura, mellyel az azokat
//futtatoto taszk mukodese befolyasolhato.
typedef struct
{
    //A varakozasi ido tick-ben
    uint32_t waitTime;
    //Program kesz jelzes. Azoknal a programoknal, melyek elinditasa utan
    //azok mukodese egy ido utan befejezodik, ott ezen a flagen keresztul tudja
    //jelezni azt a loop callbackban futtatott kod.
    bool done;

    //A led programot hajto engint futtato eroforras vezerlo blokkjara mutat.
    taskedResource_control_t* resourceControl;
} MyLedPrg_control_t;
//------------------------------------------------------------------------------
//LED vezerlo program indulasakor/ujraindulasakor hivodo callback definicioja.
typedef status_t MyLedPrg_initFunc_t(void* callbackData);
//LED vezerlo programot futtato loop funkcio definicioja.
typedef status_t MyLedPrg_loopFunc_t(MyLedPrg_control_t* control,
                                     void* callbackData);
//Program vegen hivodo callback funkcio definicioja
typedef void MyLedPrg_doneFunc_t(void* callbackData);

//Az egyes LED vezerlo programok handlerei. Minden programhoz letre kell egy
//ilyent hozni, és a megfelelo enginhez adni.
typedef struct
{
    //True-val jelzi, hogy a program el van inditva
    bool run;
    //true-val jelzi, hogy a programnak inditaskor/ujrainditaskor meg lett mar
    //hivva az init funkcioja.
    bool inited;
    //true eseten ha egy magasabb prioritasu program elveszi a futas jogat a
    //programtol, akkor ha ez a flag true, es ujra szabad az engine, akkor a
    //programot automatikusan ujrainditja
    bool restartable;

    //A programhot futtato Engin leirojara mutat
    struct MyLedEngine_t* engine;

    //LED vezerlo program indulasakor/ujraindulasakor hivodo callback
    MyLedPrg_initFunc_t* initFunc;
    //LED vezerlo programot futtato loop funkcio.
    MyLedPrg_loopFunc_t* loopFunc;
    //A callbackek szamara atadott tetszoleges adattartalom
    void* callbackData;

    //Program vegen hivodo callback funkcio, melyet a program inditasakor adtak
    //meg.
    MyLedPrg_doneFunc_t* doneFunc;
    //A doneFunc callbacknek atadott tetszoleges adattartalom
    void* doneCallbackData;

    //programok lancolt listajah tartozo valtozok.
    struct
    {
        struct ledPrg_t* next;
        struct ledPrg_t* prev;
    } prgList;

} MyLedPrg_t;
//------------------------------------------------------------------------------
//MyLedEngine valtozoi
typedef struct
{    
    //A modult, mint eroforrast kezelni kepes valtozok halmaza
    resource_t resource;
    taskedResourceExtension_t resourceExtension;

    //Az eroforrast hasznalni kepes user. Az engineknek csak egy usere van, ez.
    resourceUser_t engineUser;

    //Modul taszk handlere. Initkor lekerdezve.
    TaskHandle_t taskHandler;

    //A modul valtozoit vedo mutex
    SemaphoreHandle_t mutex;

    //MyLedEngine hasznalatbavetelekor hivodo callback. Ebben kell a
    //hasznalt eroforrasok igenyleset elkezdeni.
    MyLedEngine_initFunc_t* initFunc;

    //LedEngin mukodesenek vegen hivodo callback. Ebben kell a
    //hasznalt eroforrasokrol valo lemondast megvalositani.
    MyLedEngine_deinitFunc_t* deinitFunc;

    //callbackek szamara atadando teszoleges adat
    void* callbackData;

    //Az engin altal kezelt programok lancolt listajanak elso es utolso tagja.
    //A lista sorrendje hatarozza meg a kezelt programok prioritasat.
    struct
    {
        MyLedPrg_t* first;
        MyLedPrg_t* last;
    } prgList;

    //Az engin altal futtatott program
    MyLedPrg_t* activePrg;

    //program lista ujra ellenorzesenek kerelme. Akkor allitjuk be, amikor
    //nem egy kulso esemeny, hanem a futtatas kozben keletkezik valami olyan
    //feltetel, mely hatasara a programok listajat ujra kell ertekelni.
    bool prgListCheckRequest;

    //A loop funcio altal eloirt kovetkezo futtatasi idopont
    uint64_t nextExecutionTime;

} MyLedEngine_t;
//------------------------------------------------------------------------------
typedef struct
{
    //A programot futtato motor (tobb is lehet a rendszerben, meyek
    //egymastol fuggetlenul mukodnek. ) Egy programot csak egy engine-hez
    //szabad hozzaadni.
    MyLedEngine_t* engine;

    bool restartable;
    //A program indulasakor/ujrainditasakor mehivodo callbacket lehet itt megadni
    MyLedPrg_initFunc_t* initFunc;
    //A programot futtato loop funkcio
    MyLedPrg_loopFunc_t* loopFunc;
    //Tetszoleges adat, melyet a callbackek megkapnak
    void* callbackData;
} MyLedPrg_config_t;
//------------------------------------------------------------------------------
//Engine kezdeti inicializalasa
void MyLedEngine_init(MyLedEngine_t* engine, const MyLedEngine_config_t* cfg);

//Program konfiguralasa a rendszerbe. A programok prioritasat a hozzaadas
//sorrendje hatarozza meg.
void MyLedEngine_configurePrg(MyLedPrg_t* prg, const MyLedPrg_config_t* cfg);

//Az engine-hez tartozo osszes program leallitasa
void MyLedEngine_stopAll(MyLedEngine_t* engine);

//Led program elinditasa.
//prg: az inditando led program handlere
//doneFunc: megadhato egy callback, melyet akkor hiv, amikor a program lefutott.
//callbackData: a meagadott callbacknak atadott tetszoleges parameter
void MyLedEngine_startPrg(MyLedPrg_t* prg,
                        MyLedPrg_doneFunc_t* doneFunc,
                        void* callbackData);

//Led program leallitasa
//prg: a leallitando led program handlere
void MyLedEngine_stopPrg(MyLedPrg_t* prg);

//Program loop funkciojanak futtatasanak kerlme.
void MyLedEngine_runLoop(MyLedEngine_t* engine);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MYLEDENGINE_H_
