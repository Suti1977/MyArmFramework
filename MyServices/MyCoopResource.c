//------------------------------------------------------------------------------
//  Azonos taszkon osztozo kooperative mukodesu eroforras
//
//    File: MyCoopResource.c
//------------------------------------------------------------------------------
#include "MyCoopResource.h"
#include <string.h>

#define COOP_RESOURCE_TRACING 1

MYSM_STATE(MyCoopResource_sm_waitingForStart);
MYSM_STATE(MyCoopResource_sm_starting);
MYSM_STATE(MyCoopResource_sm_run);
MYSM_STATE(MyCoopResource_sm_stopping);
MYSM_STATE(MyCoopResource_sm_error);
//------------------------------------------------------------------------------
//Taszkal tamogatott eroforras letrehozasa
void MyCoopResource_create(resource_t* resource,
                           coopResourceExtension_t* ext,
                           const coopResource_config_t* cfg)
{
    memset(ext, 0, sizeof(coopResourceExtension_t));

    //Konfiguracios objektum megjegyzese
    ext->cfg=cfg;

    //Eroforras letrehozasa...
    resource_config_t resourceCfg={0};
    resourceCfg.name=cfg->name,
    resourceCfg.callbackData=resource,
    resourceCfg.ext=ext,
    ext->resource=resource;
    MyRM_createResource(resource, &resourceCfg);


    //Eroforras allapotgep letrehozasa. A Startra varas allapottal fog indulni.
    MySM_init(&ext->sm, MyCoopResource_sm_waitingForStart, ext);
    //mivel az allapotgep futasa esemenyhez van kotve, ezert kezdetben be kell
    //allitani a start esemenyre valo varast.
    ext->control.waitedEvents=MY_COOP_RESOURCE_EVENT__START_REQUEST;
}
//------------------------------------------------------------------------------
//Eroforras kiertekelese/futtatasa
//[Csoport taszkjabol hiva]
void MyCoopResource_runResource(resource_t* resource)
{
    coopResourceExtension_t* ext=(coopResourceExtension_t*) resource->ext;

    //Eroforrasnak szolo esemenyek atvetele. Kesobb ezeket hasznalja az
    //allapotgepben.
    MY_ENTER_CRITICAL();
    ext->control.events = ext->inputEvents;
    ext->inputEvents=0;
    MY_LEAVE_CRITICAL();


    //Ellenorzes, hogy az idozites letelte miatt kell e futtatni a taszkot.
    bool loopTimerExpired=MySwTimer_expired(&ext->loopTimer);
    if (loopTimerExpired)
    {
        //Jelzes az eroforrasnak, hogy az eloirt idozites letelte miatt (is)
        //fut.
        ext->control.timed=true;

        #if COOP_RESOURCE_TRACING
        printf("MyCoopResource Timed! (%s)\n", ext->cfg->name);
        #endif
    }

    if ((ext->control.events & ext->control.waitedEvents)  ||
        (ext->control.timed))
    {   //Vannak olyan esemenyek, melyekre az eroforrast aktivalni kell.

        //A varakozos esemenyeket toroljuk. Azokat majd az allapotgep ujra
        //be fogja allitani.
        ext->control.waitedEvents=0;
        //Alapertelmezesben esemenre fog varni.
        ext->control.waitTime=portMAX_DELAY;

        //Allapotgep futtatasa. Ebben fut az eroforras...
        MySM_run(&ext->sm);

        //Az idozitettettseg jelzeset torolni kell.
        ext->control.timed=false;

        if (ext->control.waitTime != portMAX_DELAY)
        {   //Van eloirva idozites!
            MySwTimer_start(&ext->loopTimer, ext->control.waitTime, 0);
        }
    }
}
//------------------------------------------------------------------------------
//Inditasi kerelemra varakozas...
MYSM_STATE(MyCoopResource_sm_waitingForStart)
{
    coopResourceExtension_t* this=MYSM_USER_DATA(coopResourceExtension_t*);

    status_t status=kStatus_Success;

    if (MYSM_STATE_INIT())
    {
        #if COOP_RESOURCE_TRACING
        printf("MyCoopResource WAITING FOR START... (%s)\n", this->cfg->name);
        #endif

        //Start kerelemre varakozik a taszk
        this->control.waitedEvents=MY_COOP_RESOURCE_EVENT__START_REQUEST;
        //Vegtelen ideig
        this->control.waitTime=portMAX_DELAY;
        this->control.done=0;
        this->control.prohibitStop=0;
        this->control.stopRequest=0;
        this->errorCode=kStatus_Success;

        //Alapertelmezesben azt mondjuk, hogy a startFunc() callback meghivasa
        //utan elindult az eroforras. Ezt a jelzest felul lehet biralni a
        //startFunc() callbackben, de akkor a loop-ban futo applikacionak
        //kell tudnia jeleznie az eroforras manager fele, hogy elkeszult.
        this->control.run=1;

        //Ha futna a loop idozites, akkor azt le kell allitani!
        MySwTimer_stop(&this->loopTimer);
    }

    if (this->control.events & MY_COOP_RESOURCE_EVENT__START_REQUEST)
    {   //Inditasi kerelem erkezett.
        this->control.events &= ~MY_COOP_RESOURCE_EVENT__START_REQUEST;

        MYSM_CHANGE_STATE(MyCoopResource_sm_starting);
    }

    return status;
}
//------------------------------------------------------------------------------
//Eroforras indulo allapota
MYSM_STATE(MyCoopResource_sm_starting)
{
    coopResourceExtension_t* this=MYSM_USER_DATA(coopResourceExtension_t*);
    status_t status=kStatus_Success;

    if (MYSM_STATE_INIT())
    {
        #if COOP_RESOURCE_TRACING
        printf("MyCoopResource STARTING... (%s)\n", this->cfg->name);
        #endif

        //Eroforras indul...
        if (this->cfg->startFunc)
        {   //eroforrast indito funkcio meghivasa, mivel van ilyen beallitva
            status=this->cfg->startFunc(this->cfg->callbackData,
                                        &this->control);
            if (status)
            {
                goto error;
            }
        }

        if (this->control.run)
        {   //Az eroforras elindult.
            MYSM_CHANGE_STATE(MyCoopResource_sm_run);
        }

        return status;
    }


    //Applikacios loop futtatasa, melyben az eroforras indul.
    if (this->cfg->loopFunc)
    {
        status=this->cfg->loopFunc(this->cfg->callbackData,
                                   &this->control);
        if (status) goto error;
    }

    if (this->control.run)
    {   //Az eroforras elindult. (A loop-ban lett beallitva a jelzes)
        MYSM_CHANGE_STATE(MyCoopResource_sm_run);
    }

    return status;

error:
    this->errorCode=status;
    MYSM_CHANGE_STATE(MyCoopResource_sm_error);
}
//------------------------------------------------------------------------------
//Eroforras fut allapot
MYSM_STATE(MyCoopResource_sm_run)
{
    coopResourceExtension_t* this=MYSM_USER_DATA(coopResourceExtension_t*);
    status_t status=kStatus_Success;

    if (MYSM_STATE_INIT())
    {
        #if COOP_RESOURCE_TRACING
        printf("MyCoopResource RUN. (%s)\n", this->cfg->name);
        #endif

        //Jelzes a manager fele, hogy fut az eroforras
        MyRM_resourceStatus(this->resource, RESOURCE_RUN, status);
    }


    if (this->control.done)
    {   //Az eroforras befejezte a mukodeset. Jelzes a manager fele.
        MyRM_resourceStatus(this->resource, RESOURCE_DONE, status);

        //A tovabbiakban ujra a start feltetelre fog varni
        MYSM_CHANGE_STATE(MyCoopResource_sm_waitingForStart);
    }


    if (this->control.events & MY_COOP_RESOURCE_EVENT__STOP_REQUEST)
    {   //Leallitasi kerelem erkezett a manager felol.
        this->control.events &= ~MY_COOP_RESOURCE_EVENT__STOP_REQUEST;

        MYSM_CHANGE_STATE(MyCoopResource_sm_stopping);
    }


    //Applikacios loop futtatasa...
    if (this->cfg->loopFunc)
    {
        status=this->cfg->loopFunc(this->cfg->callbackData, &this->control);
        if (status) goto error;
    }

    //A leallitasi esemenyt barmikor fogadhatjuk ezek utan.
    this->control.waitedEvents |= MY_COOP_RESOURCE_EVENT__STOP_REQUEST;
    return status;

error:
    this->errorCode=status;
    MYSM_CHANGE_STATE(MyCoopResource_sm_error);
}
//------------------------------------------------------------------------------
//Leallitasi kerelem esete...
MYSM_STATE(MyCoopResource_sm_stopping)
{
    coopResourceExtension_t* this=MYSM_USER_DATA(coopResourceExtension_t*);
    status_t status=kStatus_Success;

    if (MYSM_STATE_INIT())
    {
        #if COOP_RESOURCE_TRACING
                printf("MyCoopResource STOPPING... (%s)\n", this->cfg->name);
        #endif

        //Jelzes a loopban futo folyamatnak, hogy leallitasi kerelmet kapott.
        this->control.stopRequest=true;
    }

    if (this->control.prohibitStop==0)
    {   //leallitasi kerelem van, es mar az applikacio is engedi, azt a prohibit
        //flag (mar) nem tiltja.

        if (this->cfg->stopFunc)
        {   //Van megadva leallitasi funkcio. Meghivjuk...
            status=this->cfg->stopFunc(this->cfg->callbackData);
            if (status)
            {   //hiba volt a leallitas alatt. Hibakezeles...
                goto error;
            }
        }

        #if COOP_RESOURCE_TRACING
                printf("MyCoopResource STOP. (%s)\n", this->cfg->name);
        #endif


        //Az eroforras leallt. Reportoljuk az eroforras manager fele...
        MyRM_resourceStatus(this->resource, RESOURCE_STOP, status);

        MYSM_CHANGE_STATE(MyCoopResource_sm_waitingForStart);
    }

    //Applikacios loop futtatasa, melyben folyik a leallitasi procedura...
    if (this->cfg->loopFunc)
    {
        status=this->cfg->loopFunc(this->cfg->callbackData, &this->control);
        if (status) goto error;
    }

    return status;

error:
    this->errorCode=status;
    MYSM_CHANGE_STATE(MyCoopResource_sm_error);
}
//------------------------------------------------------------------------------
MYSM_STATE(MyCoopResource_sm_error)
{
    coopResourceExtension_t* this=MYSM_USER_DATA(coopResourceExtension_t*);
    status_t status=kStatus_Success;

    if (MYSM_STATE_INIT())
    {
        #if COOP_RESOURCE_TRACING
            printf("MyCoopResource ERROR! (%s)\n", this->cfg->name);
        #endif

        if (this->cfg->errorFunc)
        {
            this->cfg->errorFunc(this->cfg->callbackData, this->errorCode);
        }

        //Jelzes a manager fele, hogy az eroforras hibara futott...
        MyRM_resourceStatus(this->resource, RESOURCE_ERROR, this->errorCode);

        //A hiba eseten csak a leallitasi kerelmet fogadjuk a manager felol.
        this->control.waitedEvents=MY_COOP_RESOURCE_EVENT__STOP_REQUEST;
        //Ha futna a loop timer, akkor azt le kell allitani
        MySwTimer_stop(&this->loopTimer);
    }


    //Varakozas arra, hogy az eroforras leallitasi kerest kapjon a managertol.
    //(Ez szukseges, hogy torlodjon benne a hiba.)
    if (this->control.events & MY_COOP_RESOURCE_EVENT__STOP_REQUEST)
    {   //Leallitasi kerelem erkezett a manager felol.
        this->control.events &= ~MY_COOP_RESOURCE_EVENT__STOP_REQUEST;

        #if COOP_RESOURCE_TRACING
            printf("MyCoopResource Error cleared! (%s)\n", this->cfg->name);
        #endif

        //Jelzes a manager fele, hogy az eroforras leallt.
        //Torlodni fog a hiba.
        MyRM_resourceStatus(this->resource, RESOURCE_STOP, kStatus_Success);

        MYSM_CHANGE_STATE(MyCoopResource_sm_waitingForStart);
    }

    return status;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
