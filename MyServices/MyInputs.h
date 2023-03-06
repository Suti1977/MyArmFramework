//------------------------------------------------------------------------------
//  Bemenetek kezelese
//
//    File: MyInputs.h
//------------------------------------------------------------------------------
#ifndef MY_INPUTS_H_
#define MY_INPUTS_H_

#include "MyCommon.h"
#include "MySwTimer.h"

#ifndef MyInput_sample_t
typedef uint32_t MyInput_sample_t;
#endif
//-----------------------------------------------------------------------------
//A bement kezelo altal hivhato callback funkciok definicioja
typedef void MyInput_func_t(void* privData);
typedef MyInput_sample_t MyInput_samplingFunc(void* privData);
//------------------------------------------------------------------------------
//Hosszan tarto bemeneti allapot eseten az egyes vizsgalando idont, es annak
//eleresekor meghivando callback funkcio leiroja.
typedef struct
{
    //Az aktiv allapot kezdete ota mert idopont, mejnel jelezni kell
    uint32_t time;
    //A jelzeskor meghivando callback funkcio
    MyInput_func_t* func;
} MyInput_longState_t;
//------------------------------------------------------------------------------
//Bemenet kezeles konfiguralo struktura
typedef struct
{
    //Tetszoleges bemenet azonosito
    uint32_t id;

    //A bemenet aktiv allapota
    MyInput_sample_t activState;

    //bemenetet mintavevo fuggveny
    MyInput_samplingFunc* samplingFunc;
    //Bemenet aktivalasanak pillanataban meghivando callback
    MyInput_func_t* activatedFunc;
    //Bement inaktivalasakor meghivando callback
    MyInput_func_t* inactivatedFunc;

    //A bementhez tartozo minimalis aktivalasi idejo [ms]
    uint32_t minTime;

    //A rovid aktivalas maximalis ideje [ms]
    uint32_t shortTime;
    //Rovid aktivalas eseten meghivando callback
    MyInput_func_t* shortFunc;

    //Hosszabb aktiv allapotok eseten meghatarozhato idopontok, es hivando
    //a hozzajuk rendelt callbackek listaja.
    //A lista lehet NULL, vagy a listat a time=NULL zarja.
    const MyInput_longState_t* longPress;

    //A hivott callbackeknek atadott tetszoleges adattartalom.
    void* privData;

    //A bementhez tartozo prell mentesitesi ido
    uint32_t antibounceTime;
} MyInput_config_t;
//------------------------------------------------------------------------------
//Egyetlen bemenet valtozoit leiro struktura.
typedef struct
{
    //A bemenet kezeles konfiguracioja
    const MyInput_config_t* cfg;

    //Az elozo mintaveteli allapot. Ez lehet akar tobb bit is.
    MyInput_sample_t lastSample;

    //A bemenet stabil, prellmentesitett allapota. Ez lehet akar tobb bit is.
    MyInput_sample_t state;
    //A bemenet elozo stabil allapota. (Ez lehet akar tobb bit is.)
    MyInput_sample_t lastState;


    //Prell mentesitesi idozites vege.
    uint64_t antibounceStableTime;

    //Az utolso fel/le futo el/allapot valtozas idopillanata
    uint64_t lastEdgeTimeStamp;

    //Adott idonel hosazbb aktiv allapotokhoz kiadott jelzeseket tartja nyilvan
    const MyInput_longState_t* nextLongPress;

    //A kovetkezo olyan idopillanat, amikor a bemeneten esemeny kovetkezhet be.
    uint64_t nextEventTime;

    //Nyomogombhoz tartozo idozites
    MySwTimer_t timer;

    //lancolt listaban a kovetkezo bemenetre mutat
    struct MyInput_t* next;
} MyInput_t;
//------------------------------------------------------------------------------
//Bement kezelo valtozoi
typedef struct
{
    //A mintavetelezeseket es pergesmentesitest/idomerest vegrzo taszk handlere
    TaskHandle_t    taskHandler;
    //Statikusan letrehozott taszk valtozoi
    StaticTask_t taskBuffer;

    //Modul valtozoit vedo mutex
    SemaphoreHandle_t mutex;
    StaticSemaphore_t mutexBuffer;

    //A manager altal kezelt bemenetek lancolt listajanak elso ele
    MyInput_t* firstInput;

    MySwTimerManager_t  timerManager;
} MyInputs_manager_t;
//------------------------------------------------------------------------------
//Bemenet kezelest megvalosito manager konfiguracioja
typedef struct
{
    //A bemenet kezelest futtato taszk neve
    const char* taskName;
    //Management taszkhoz allokalando stack merete
    uint32_t stackSize;
    //Management taszk prioritasa
    uint32_t taskPriority;
    //A manager taszk szamara elore allokalt stack memoriara mutat
    StackType_t* taskStack;

    //A bemenetek mintavetelezese ennyi idonkent tortenik
    uint32_t pollTime;
} MyInputs_managerConfig_t;
//------------------------------------------------------------------------------
//Bement kezelo manager kezdeti inicializalasa
void MyInputs_initManager(MyInputs_manager_t* manager,
                          const MyInputs_managerConfig_t* config);

//True-t ad vissza, ha a kezelesben meg futnak folymatok.
bool MyInputs_isActive(void);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MY_INPUTS_H_

