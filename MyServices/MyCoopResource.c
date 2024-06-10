//------------------------------------------------------------------------------
//  Azonos taszkon osztozo kooperative mukodesu eroforras
//
//    File: MyCoopResource.c
//------------------------------------------------------------------------------
#include "MyCoopResource.h"
#include <string.h>

#define COOP_RESOURCE_TRACING 0

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
    //ext->control.waitedEvents=MY_COOP_RESOURCE_EVENT__START_REQUEST;
}
//------------------------------------------------------------------------------
//Eroforras kiertekelese/futtatasa
//[Csoport taszkjabol hiva]
void MyCoopResource_runResource(resource_t* resource)
{
    coopResourceExtension_t* ext=(coopResourceExtension_t*) resource->ext;

    //Eroforrasnak szolo esemenyek atvetele. Kesobb ezeket hasznalja az
    //allapotgepben...
    MY_ENTER_CRITICAL();
    ext->controlEvents = ext->controlEvents_async;
    ext->controlEvents_async=0;

    if (ext->controlEvents==0)
    {   //Nincs olyan esemeny, ami miatt futtatni kell az eroforrast.
        //(A controlEvents mind a vezerlo, mind pedig a bemeneti esemenyek
        //eseten is valamelyik bitjen jelez.)
        MY_LEAVE_CRITICAL();
        return;
    }

    ext->control.events = ext->inputEvents_async;
    ext->inputEvents_async=0;
    MY_LEAVE_CRITICAL();

    //Ellenorzes, hogy a loop idozites letelte miatt kell e futtatni a taszkot.
    bool loopTimerExpired=MyCoopTimer_expired(&ext->loopTimer);
    if (loopTimerExpired)
    {
        //Jelzes az eroforrasnak, hogy az eloirt idozites letelte miatt (is)
        //fut.
        ext->control.timed=true;

        #if COOP_RESOURCE_TRACING
        printf("MyCoopResource loop timed! (%s)\n", ext->cfg->name);
        #endif
    }


    if ((ext->control.events & ext->control.waitedEvents)  ||
        (ext->control.timed) || (ext->controlEvents))
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
        {   //Van eloirva idozites a loopra!
            MyCoopTimer_start(&ext->loopTimer, ext->control.waitTime, 0);
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

        this->control.waitedEvents=0;
        this->control.waitTime=portMAX_DELAY;
        this->errorCode=kStatus_Success;

        //Ha futna a loop idozites, akkor azt le kell allitani!
        MyCoopTimer_stop(&this->loopTimer);
    }

    //Start kerelemre varakozik az eroforras
    if (this->controlEvents & MY_COOP_RESOURCE_EVENT__START_REQUEST)
    {   //Inditasi kerelem erkezett.
        this->controlEvents &= ~MY_COOP_RESOURCE_EVENT__START_REQUEST;

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

        //A start elott 0 idozites kerul beallitasra, igy ha azt a start
        //callbackben nem biraljak felul, akkor a loop fuggveny azonnal le
        //fog futni.
        this->control.waitTime=0;

        this->control.waitedEvents=0;
        this->control.done=0;
        this->control.prohibitStop=0;
        this->control.stopRequest=0;
        this->control.timed=0;
        this->errorCode=kStatus_Success;

        //Alapertelmezesben azt mondjuk, hogy a startFunc() callback meghivasa
        //utan elindult az eroforras. Ezt a jelzest felul lehet biralni a
        //startFunc() callbackben, de akkor a loop-ban futo applikacionak
        //kell tudnia jeleznie az eroforras manager fele, hogy elkeszult.
        this->control.run=1;

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

        if (this->control.waitTime)
        {   //Van eloirva varakozas a start fuggvenyben. Kilepes az allapot-
            //gepbol, hogy az esemenyek/idozitesek ervenyre juthassanak a
            //loop funkcio meghivasa elott.
            return status;
        }

        //Itt mivel az idozites 0, es emiatt azonnal futatjuk a loop funkciot,
        //jelezni kell az idozitettseget!
        this->control.timed=true;
    }

    //<--ide akkor jut, ha a start fuggvenyben nem mondtak azt, hogy elindult
    //az eroforras, tehat a control.run flag-je torolva lett.

    //Applikacios loop futtatasa, melyben az eroforras indulast vegrehajtja...
    if (this->cfg->loopFunc)
    {
        status=this->cfg->loopFunc(this->cfg->callbackData,
                                   &this->control);
        if (status) goto error;
    }

    if (this->control.run)
    {   //Az eroforras elindult. (A loop-ban lett beallitva a jelzes)
        //A run allapotot veszi fel.

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

        //Ezen a ponton ki kell lepni az allapotgepbol, hogy az esemeny
        //illetve idozitesek lefuthassanak a loop funkcio elott, a start
        //callbackben esetlegesen feluldefinialt feltetelek szerint, de csak ha
        //van eloirva idozites.
        if (this->control.waitTime)
        {
            //A tovabbiakban figyelni fogja a leallitasi kerelmet is.
            //this->control.waitedEvents = MY_COOP_RESOURCE_EVENT__STOP_REQUEST;

            return status;
        }

        //Itt mivel az idozites 0, es emiatt azonnal futatjuk a loop funkciot,
        //jelezni kell az idozitettseget!
        this->control.timed=true;
    }


    if (this->control.done)
    {   //Az eroforras befejezte a mukodeset. Jelzes a manager fele.
        MyRM_resourceStatus(this->resource, RESOURCE_DONE, status);

        //A tovabbiakban ujra a start feltetelre fog varni
        MYSM_CHANGE_STATE(MyCoopResource_sm_waitingForStart);
    }


    if (this->controlEvents & MY_COOP_RESOURCE_EVENT__STOP_REQUEST)
    {   //Leallitasi kerelem erkezett a manager felol.
        this->controlEvents &= ~MY_COOP_RESOURCE_EVENT__STOP_REQUEST;

        MYSM_CHANGE_STATE(MyCoopResource_sm_stopping);
    }


    //Applikacios loop futtatasa...
    if (this->cfg->loopFunc)
    {
        status=this->cfg->loopFunc(this->cfg->callbackData, &this->control);
        if (status) goto error;
    }

    //A leallitasi esemenyt barmikor fogadhatjuk ezek utan.
    //this->control.waitedEvents |= MY_COOP_RESOURCE_EVENT__STOP_REQUEST;
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

        #if COOP_RESOURCE_TRACING
        printf("MyCoopResource STOP. (%s)\n", this->cfg->name);
        #endif

        if (this->cfg->stopFunc)
        {   //Van megadva leallitasi funkcio. Meghivjuk...
            status=this->cfg->stopFunc(this->cfg->callbackData);
            if (status)
            {   //hiba volt a leallitas alatt. Hibakezeles...
                goto error;
            }
        }

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
        MyCoopTimer_stop(&this->loopTimer);
    }


    //Varakozas arra, hogy az eroforras leallitasi kerest kapjon a managertol.
    //(Ez szukseges, hogy torlodjon benne a hiba.)
    if (this->controlEvents & MY_COOP_RESOURCE_EVENT__STOP_REQUEST)
    {   //Leallitasi kerelem erkezett a manager felol.
        this->controlEvents &= ~MY_COOP_RESOURCE_EVENT__STOP_REQUEST;

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
//Kooperativ eroforrasnak esemeny kuldese
void MyCoopResource_setEvent(resource_t* resource, uint32_t event)
{
    MyCoopResourceGroup_setResourceEvent(resource, event);
}
//------------------------------------------------------------------------------
//Kooperativ eroforrasnak esemeny kuldese megszakitasbol
void MyCoopResource_setEventFromIsr(resource_t* resource, uint32_t event)
{
    MyCoopResourceGroup_setResourceEventFromIsr(resource, event);
}
//------------------------------------------------------------------------------


