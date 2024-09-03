//------------------------------------------------------------------------------
//  LED vezerlo motor (eroforraskent implementalva)
//
//    File: MyLedEngine.c
//------------------------------------------------------------------------------
//gondolatok:
//  Kulonbozo programokat lehessen implementaln, melyeket parancsra el
//  lehet inditani, vagy le lehet allitani.
//  Lehessen olyan program is, melyet csak el kell inditani, es ha lefutott,
//  automatikusan leall. (done jelzes)
//  Az egyes programokhoz lehessen beregisztralni egy callbacket, mely akkor
//  hivodik, amikor  a program lefutott. Ezt a callbacket a start funkciokor
//  lehet megadni.
//  A programoknak legyen egy init fuggvenye, melyet indulaskor/ujrainditaskor
//  meghiv a modul.
//  A programoknak legyen egy loop fuggvenye, melyet utemesen hivogat a modul,
//  amikor a program fut.
//  Az egyes programoknak prioritast lehessn beallitani. A magasabb prioritasut
//  elinditva, az alacsonyabb prioritasu programot nem engedi futni.
//  Egy magasabb prioritasu program vegeztevel az alacsonyabb prioritasu kezd
//  el ujra mukodni, elolrol (ujra init).
//  Az egyes programokat barmikor el lehet inditani/meg lehet allitani, akkor is
//  ha nem azok vannak eloterben.
//  Legyen tobb led vezerlest futtatni kepes engine, mely ala az egyes programok
//  beregisztralhatok.
//  Az enginek-nek legyen init es deinit callbackje, melyekben a hasznalt
//  eroforrasokat vehetik igenybe/mondhatjak le.
//------------------------------------------------------------------------------

#include "MyLedEngine.h"
#include <string.h>

#define MY_LED_ENGINE_TRACE    0

static status_t MyLedEngine_resourceStart(void* callbackData,
                                taskedResource_control_t* control);
static status_t MyLedEngine_resourceStop(void* callbackData);
static void     MyLedEngine_resourceError(void* callbackData, status_t errorCode);
static status_t MyLedEngine_resourceLoop(void* callbackData,
                               taskedResource_control_t* control);
static void MyLedEngine_resourceStatusCB( resource_t* resource,
                                        resourceStatus_t resourceStatus,
                                        resourceErrorInfo_t* errorInfo,
                                        void* callbackData);
//Valamelyik LED programra keres erkezett, ezert azt ellenorizni kell...
#define MY_LED_ENGINE_EVENT__CHECK_PRG_LIST    BIT(0)
//Loop funkcio futtatasanak kerelme.
#define MY_LED_ENGINE_EVENT__RUN_LOOP          BIT(1)
//------------------------------------------------------------------------------
//Engine kezdeti inicializalasa
void MyLedEngine_init(MyLedEngine_t* engine, const MyLedEngine_config_t* cfg)
{
    //Modul valtozoinak kezdeti torlese.
    memset(engine, 0, sizeof(MyLedEngine_t));

    //Valtozokat vedo mutex letrehozasa
    engine->mutex=xSemaphoreCreateMutex();
    ASSERT(engine->mutex);

    engine->initFunc=cfg->initFunc;
    engine->deinitFunc=cfg->deinitFunc;
    engine->callbackData=cfg->callbackData;

    //TASZK-al tamogatott eroforras letrehozasa...
    taskedResource_config_t resourceCfg;
    memset(&resourceCfg, 0, sizeof(resourceCfg));

    //Az eroforras neve. Ugyan ezt hasznalja a letrehozott taszk nevenek is.
    resourceCfg.name=cfg->name;
    //Az eroforrast futtato taszk prioritasa
    resourceCfg.taskPriority=cfg->taskPriority;
    //Az eroforrast futtato taszk szamara allokalt taszk merete
    resourceCfg.taskStackSize=cfg->taskStackSize;
    //Az eroforras callbckjai szamara atadott tetszoleges parameter
    resourceCfg.callbackData=engine;
    //Az eroforras elinditasara, a taszkban meghivodo rutin
    resourceCfg.startFunc=MyLedEngine_resourceStart;
    //Az eroforras leallitasi kerelme utan a taszkban meghivodo rutin
    resourceCfg.stopFunc=MyLedEngine_resourceStop;
    //Hiba eseten, a taszkbol hivott callback definicioja
    resourceCfg.errorFunc=MyLedEngine_resourceError;
    //Eroforrast futtato callback definicioja
    resourceCfg.loopFunc=MyLedEngine_resourceLoop;

    MyTaskedResource_create(&engine->resource,
                            &engine->resourceExtension,
                            &resourceCfg);

    //eroforras taszkjat vezerlo esemenymezo handlerenek lekerdezese
    engine->taskHandler=MyTaskedResource_getTaskHandler(&engine->resource);

    //User bekotese az eroforrashoz. Ezen az useren keresztul tudja az engine
    //a MyLedEngine_startPrg()-ban elinditani az eroforrast.
    resourceUser_config_t engineUser_cfg=
    {
        .name="MyLedEngineInternal",
        .resource=&engine->resource,
        .statusFunc=MyLedEngine_resourceStatusCB,
        .callbackData=engine,
    };
    MyRM_createUser(&engine->engineUser, &engineUser_cfg);
}
//------------------------------------------------------------------------------
//Az eroforras elinditasara, a taszkban meghivodo rutin
static status_t MyLedEngine_resourceStart(void* callbackData,
                                        taskedResource_control_t* control)
{
    (void) control;
    #if MY_LED_ENGINE_TRACE
    printf("MyLedEngine_resourceStart()\n");
    #endif

    MyLedEngine_t* engine=(MyLedEngine_t*)callbackData;
    status_t status=kStatus_Success;

    //Ha van beregisztralva init funkcio, akkor azt meghivja
    if (engine->initFunc)
    {
        status=engine->initFunc(engine->callbackData);
    }

    //Taszkot ebreszteni kepes sajat notofy eventek beallitsa
    control->waitedEvents=0xff;

    return status;
}
//------------------------------------------------------------------------------
//Az eroforras leallitasi kerelme utan a taszkban meghivodo rutin
static status_t MyLedEngine_resourceStop(void* callbackData)
{
    #if MY_LED_ENGINE_TRACE
    printf("MyLedEngine_resourceStop()\n");
    #endif

    MyLedEngine_t* engine=(MyLedEngine_t*)callbackData;
    status_t status=kStatus_Success;

    //Ha van beregisztralva deinit funkcio, akkor azt meghivja
    if (engine->deinitFunc)
    {
        status=engine->deinitFunc(engine->callbackData);
    }

    return status;
}
//------------------------------------------------------------------------------
//A hasznalt eroforras allapotvaltozasakor, peldaul hibajakor hivodo
//callback.
static void MyLedEngine_resourceStatusCB( resource_t* resource,
                                        resourceStatus_t resourceStatus,
                                        resourceErrorInfo_t* errorInfo,
                                        void* callbackData)
{
    MyLedEngine_t* engine=(MyLedEngine_t*)callbackData;
    (void) resource;
    (void) callbackData;
    (void) resourceStatus;
    (void) errorInfo;

    #if MY_LED_ENGINE_TRACE
    printf("MyLedEngine_resourceStatusCB() resourceStatus:%d\n", resourceStatus);
    #endif

    if (resourceStatus==RESOURCE_ERROR)
    {   //Az engine-hez rendelt valamelyik eroforras hibara futott.
        #if MY_LED_ENGINE_TRACE
        printf("MyLedEngine_resourceStatusCB() resource: %s   errorCode:%d\n",
               ((resource_t*)errorInfo->resource)->resourceName,
               errorInfo->errorCode);
        #endif

        //A hasznalt eroforrasokat leallja, igy azok felszabadulnak.
        //MyRM_unuseResource(&engine->engineUser);

        //A hasznalt eroforras ujrainditasa.
        vTaskDelay(5);
        MyRM_restartResource(&engine->engineUser);
    }
}
//------------------------------------------------------------------------------
//Hiba eseten, a taszkbol hivott callback
static void MyLedEngine_resourceError(void* callbackData, status_t errorCode)
{
    MyLedEngine_t* engine=(MyLedEngine_t*)callbackData;
    (void) errorCode;
    #if MY_LED_ENGINE_TRACE
    printf("MyLedEngine error. Code:%d\n", errorCode);
    #endif

    //A hasznalt eroforrasokat leallja, igy azok felszabadulnak.
    MyRM_unuseResource(&engine->engineUser);
}
//------------------------------------------------------------------------------
//Eroforrast futtato callback
static status_t MyLedEngine_resourceLoop(void* callbackData,
                                       taskedResource_control_t* control)
{
    status_t status=kStatus_Success;
    MyLedEngine_t* engine=(MyLedEngine_t*)callbackData;

    uint64_t time=control->time;    //MyRTOS_getTick();

    //Beallitjuk, hogy a vezerlo szal mely esemenyekre hivja ujra ezt a loop
    //funkciot.
    control->waitedEvents=0xff;

    MyLedPrg_t* activePrg=engine->activePrg;

    xSemaphoreTake(engine->mutex, portMAX_DELAY);

    if ((control->events & MY_LED_ENGINE_EVENT__CHECK_PRG_LIST) ||
       (engine->prgListCheckRequest))
    {   //ki kell ertekelni a programok listajat, mert valamelyik programon
        //kerelem van.

        engine->prgListCheckRequest=false;

        //xSemaphoreTake(engine->mutex, portMAX_DELAY);
        //vegig halad a programok lancolt listajan, es az elso magasabb
        //prioritasu elemnel ki fog lepni.
        MyLedPrg_t* prg=engine->prgList.first;
        for(; prg; prg=(MyLedPrg_t*)prg->prgList.next)
        {
            if (prg->run)
            {   //A soron levo program fut.
                break;
            }
        } //for

        if (prg==NULL)
        {   //Nincs mar futo program
            activePrg=prg;
            engine->activePrg=activePrg;
        } else
        {
            if (activePrg==NULL)
            {   //Ez elott nem futott meg program.
                //Ez lesz elinditva.
                activePrg=prg;

                engine->activePrg=activePrg;
                //Az elso loop futast azonnal elkezdjuk. Ehhez az idozitot
                //ugy kell bealitani, hogy azonnal fusson.
                engine->nextExecutionTime=time;
            } else
            if (prg != activePrg)
            {   //Ez egy masik program, mint amit eddig futtattunk!
                if (activePrg->restartable)
                {   //a megszakitott program automatikusan ujraindithato
                    //nem piszkaljuk a "run" flagjet, viszont toroljuk annak
                    //inited jelzeset, igy az amikor visszakerul ra a vezerles
                    //ujra lefuttatja az init() callback funkciojat.
                    activePrg->inited=false;
                } else
                {   //Egy olyan programot szakitott felbe, melyet nem lehet
                    //automatikusan ujrainditani. A run flagjet toroljuk, igy az
                    //a magasabb prioritasu program lefutasa utan mar nem fog
                    //ujraindulni.
                    activePrg->run=false;
                    activePrg->inited=false;

                    //Ha ennek a programnak van done callbackje, akkor azt
                    //meg kell hivni, hogy az arra varo folyamatok tovabb
                    //lephessenek.
                    if (activePrg->doneFunc)
                    {   //TODO: lehetne jelezni, hogy ez egy megszakitott prg. !
                        activePrg->doneFunc(activePrg->doneCallbackData);
                        activePrg->doneFunc=NULL;
                    }
                }

                //Atallunk az uj programra.
                activePrg=prg;
                engine->activePrg=activePrg;
                //Az elso loop futast azonnal elkezdjuk. Ehhez az idozitot
                //ugy kell bealitani, hogy azonnal fusson.
                engine->nextExecutionTime=time;
            } //if (prg != activePrg)
        }

        //xSemaphoreGive(engine->mutex);
    } //if (control->events & LED_ENGINE_EVENT__CHECK_PRG_LIST)

    //Az aktualisan futtatott program vegrehajtasa...

    if (activePrg==NULL)
    {   //Az engine nem futtat programot.
        control->waitTime=portMAX_DELAY;

        #if MY_LED_ENGINE_TRACE
        printf(">>>Led engine done.\n");
        #endif

        //A hasznalt eroforrasok lealljanak!
        MyRM_unuseResource(&engine->engineUser);

        goto exit;
    }

    //xSemaphoreTake(engine->mutex, portMAX_DELAY);
    if (activePrg->inited==false)
    {   //A program meg nincs inicializalva. Azt meg kell tenni.
        //Ugyan ez a szituacio, ha egy programra rainditanak, mivel a startPrg()
        //funkcioban az inited flag is torlodik.

        //Jelzes,hogy inicializalva van a program
        activePrg->inited=true;
        //xSemaphoreGive(engine->mutex);

        //Az idozitesek ujraindulnak ettol az idoponttol.
        engine->nextExecutionTime=time;

        //Ha van beregisztralva init funkcio, akkor azt meghivja
        if (activePrg->initFunc)
        {
            xSemaphoreGive(engine->mutex);
            status=activePrg->initFunc(activePrg->callbackData);
            xSemaphoreTake(engine->mutex, portMAX_DELAY);
            if (status) goto error;
        }
    } else
    {
        //xSemaphoreGive(engine->mutex);
    }

    if (control->events & MY_LED_ENGINE_EVENT__RUN_LOOP)
    {   //Kenyszeritett loop futtatas keresre

    } else
    {
        if (time<engine->nextExecutionTime)
        {   //A loop funkciot meg nem kell futtatni. Nem telt el az elozo futas
            //ota annyi ido, mint amit a funcio eloirt.
            //Kiszamitjuk, hogy mennyi ido van meg addig hatra. A taszk annyit
            //fog varakozni.
            control->waitTime=(uint32_t) (engine->nextExecutionTime - time);
            goto exit;
        }
    }

    MyLedPrg_control_t prgControl;
    prgControl.done=false;
    prgControl.waitTime=0;
    prgControl.resourceControl=control;

    //Led program futtatasa...
    if (activePrg->loopFunc)
    {
        xSemaphoreGive(engine->mutex);
        status=activePrg->loopFunc(&prgControl, activePrg->callbackData);
        xSemaphoreTake(engine->mutex, portMAX_DELAY);
        if (status) goto error;
    }

    if (prgControl.done)
    {   //A led program vegzett.
        //xSemaphoreTake(engine->mutex, portMAX_DELAY);

        //toroljuk, hogy fut a program
        activePrg->run=false;
        engine->activePrg=NULL;

        //Ha van beregisztralt done callback, akkor az meghivasra kerul.
        if (activePrg->doneFunc)
        {
            activePrg->doneFunc(activePrg->doneCallbackData);
            activePrg->doneFunc=NULL;
        }
        //xSemaphoreGive(engine->mutex);

        //0 varakozasi idot allitunk be, igy a fuggvenybol visszaterve
        //az eroforras vezerlo eventek kiertekelese utan azonnal ujra le fog
        //futni ez a loop funkcio, melynek az elejen ujra ellenorzi a listat.
        prgControl.waitTime=0;
        engine->prgListCheckRequest=true;
    } else
    {   //Tovabb fut a program, idozitessel (ami akar lehet 0 is)

        //Meghatarozzuk a kovetkezo idozitesi pillanatot, amikor a loop-ot
        //futtatni kell.
        engine->nextExecutionTime=time + prgControl.waitTime;
        //Kiszamitjuk, hogy mennyi ido van meg addig hatra. A taszk annyit
        //fog varakozni.
        control->waitTime=(uint32_t) (engine->nextExecutionTime - time);
    }

error:
exit:
    xSemaphoreGive(engine->mutex);
    return status;
}
//------------------------------------------------------------------------------
//Program konfiguralasa a rendszerbe. A programok prioritasat a hozzaadas
//sorrendje hatarozza meg.
void MyLedEngine_configurePrg(MyLedPrg_t* prg, const MyLedPrg_config_t* cfg)
{
    //Program vezerlo struktura valtozoinak nullazasa
    memset(prg, 0, sizeof(MyLedPrg_t));

    //Konfiguracio atvetele
    prg->initFunc=cfg->initFunc;
    prg->loopFunc=cfg->loopFunc;
    prg->callbackData=cfg->callbackData;
    prg->restartable=cfg->restartable;

    //megjegyezzuk, hogy a programot melyik engine hajtja. Kesobb ez alapjan
    //lesz a megfelelo engine triggerelve
    MyLedEngine_t* engine=cfg->engine;
    prg->engine=(struct MyLedEngine_t*) engine;

    //Program hozzaadasa a megfelelo engine lancolt listajahoz...
    xSemaphoreTake(engine->mutex, portMAX_DELAY);

    if (engine->prgList.first==NULL)
    {   //Meg nincs beregisztralva hasznalo. Ez lesz az elso.
        engine->prgList.first=prg;
        prg->prgList.prev=NULL;
    } else
    {   //Mar van a listanak eleme. Az utolso utan fuzzuk.
        prg->prgList.prev=(struct ledPrg_t*) engine->prgList.last;

        ((MyLedPrg_t*)engine->prgList.last)->prgList.next=(struct ledPrg_t*) prg;
    }
    //A sort lezarjuk. Ez lesz az utolso.
    prg->prgList.next=NULL;
    engine->prgList.last=prg;

    xSemaphoreGive(engine->mutex);
}
//------------------------------------------------------------------------------
//Az engine-hez tartozo osszes program leallitasa
void MyLedEngine_stopAll(MyLedEngine_t* engine)
{
    MyLedPrg_t* prg=engine->prgList.first;

    xSemaphoreTake(engine->mutex, portMAX_DELAY);
    while(prg)
    {
        prg->run=false;
        prg=(MyLedPrg_t*)prg->prgList.next;
    }
    xSemaphoreGive(engine->mutex);

    //Jelzes a taszknak, hogy fusson le a lista kiertekelese
    MyTaskedResource_setEvent(&engine->resource,
                              MY_LED_ENGINE_EVENT__CHECK_PRG_LIST);
}
//------------------------------------------------------------------------------
//Led program elinditasa
void MyLedEngine_startPrg(MyLedPrg_t* prg,
                        MyLedPrg_doneFunc_t* doneFunc,
                        void* callbackData)
{
    #if MY_LED_ENGINE_TRACE
    printf("MyLedEngine_startPrg()\n");
    #endif
    MyLedEngine_t* engine=(MyLedEngine_t*)prg->engine;
    xSemaphoreTake(engine->mutex, portMAX_DELAY);
    prg->doneFunc=doneFunc;
    prg->doneCallbackData=callbackData;
    prg->inited=false;
    prg->run=true;

    //Eroforras hasznalatba vetele, ha meg nem lenne elinditva.
    //Ha mar el van inditva, akkor nem tortenik muvelet.
    MyRM_useResource(&engine->engineUser);

    //(A mutexet csak az eroforras engedelyezese utan szabad visszaadni!)
    xSemaphoreGive(engine->mutex);

    //Jelzes a taszknak, hogy fusson le a lista kiertekelese
    MyTaskedResource_setEvent(&engine->resource,
                              MY_LED_ENGINE_EVENT__CHECK_PRG_LIST);
}
//------------------------------------------------------------------------------
//Led program leallitasa
void MyLedEngine_stopPrg(MyLedPrg_t* prg)
{
    #if MY_LED_ENGINE_TRACE
    printf("MyLedEngine_stopPrg()\n");
    #endif

    MyLedEngine_t* engine=(MyLedEngine_t*)prg->engine;

    xSemaphoreTake(engine->mutex, portMAX_DELAY);
    prg->run=false;
    xSemaphoreGive(engine->mutex);

    //Jelzes a taszknak, hogy fusson le a lista kiertekelese
    MyTaskedResource_setEvent(&engine->resource,
                              MY_LED_ENGINE_EVENT__CHECK_PRG_LIST);
}
//------------------------------------------------------------------------------
//Program loop funkciojanak futtatasanak kerlme.
void MyLedEngine_runLoop(MyLedEngine_t* engine)
{
    //Jelzes a taszknak, hogy fusson le a lista kiertekelese
    MyTaskedResource_setEvent(&engine->resource,
                              MY_LED_ENGINE_EVENT__RUN_LOOP);
}
//------------------------------------------------------------------------------
