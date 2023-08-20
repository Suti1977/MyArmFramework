//------------------------------------------------------------------------------
//  Bemenetek kezelese
//
//    File: MyInputs.c
//------------------------------------------------------------------------------
// Bement aktivitasok (Terv):
//   Rovid aktivalas:  __--______
//                         !
//   A rovidnel adott idovel hoszabb idopontban  egy jelzes es annak vegen
//                     __--------____
//                           !   !
//   Adott idonel hoszabb aktiv allapot eseten egy lista alapjan
//                     __-----------------------------_______
//                           !                !       !
//------------------------------------------------------------------------------
#include "MyInputs.h"
#include <string.h>

static void MyInputs_task(void* taskParam);
static void MyInputs_samplingAndAntibounce(MyInput_t* input, uint64_t time);
static void MyInputs_processInput(MyInput_t* input, uint64_t time);
//------------------------------------------------------------------------------
//Bement kezelo manager kezdeti inicializalasa
void MyInputs_initManager(MyInputs_manager_t* manager,
                          const MyInputs_managerConfig_t* config)
{
    //Modul valtozoinak kezdeti torlese.
    memset(manager, 0, sizeof(MyInputs_manager_t));

    //Valtozokat vedo mutex letrehozasa
    manager->mutex=xSemaphoreCreateMutexStatic(&manager->mutexBuffer);
    ASSERT(manager->mutex);


    //bemenetek mintavetelezeset, allapotok idoziteset megvalosito taszk
    //letrehozasa.
    manager->taskHandler=xTaskCreateStatic(MyInputs_task,
                                           config->taskName,
                                           config->stackSize,
                                           (void*)manager,
                                           config->taskPriority,
                                           config->taskStack,
                                           &manager->taskBuffer);
    ASSERT(manager->taskHandler);

    //Bemenetek kezeleset tamogato manager inicializalasa
    MySwTimer_initManager(&manager->timerManager);
}
//------------------------------------------------------------------------------
static void __attribute__((noreturn)) MyInputs_task(void* taskParam)
{
    MyInputs_manager_t* manager=(MyInputs_manager_t*) taskParam;

    //A taszk varakozasanak ideje. A legelso alkalomaml le fog futni a taszk
    TickType_t waitTime=0; //portMAX_DELAY;

    //Vezerles fociklusa
    while(1)
    {
        //varakozas esemenyre vagy idozitesre....
        uint32_t notifyEvents=0;
        xTaskNotifyWait(0, 0xffffffff, &notifyEvents, waitTime);

        //Aktualis ido lekerdezese
        uint64_t time=MyRTOS_getTick();

        //idorol idore utemesen meghivodo resz, melyben az egyes beregisztralt
        //bemenetek pollozasa tortenik.
        MyInput_t* input=manager->firstInput;
        while(input)
        {
            //Bemenet pergesmentesitett mintavetele
            MyInputs_samplingAndAntibounce(input, time);

            //Mintavett allapot magas szintu kiertekelese
            MyInputs_processInput(input, time);

            //Lancolt lista soron kovetkezo elemere ugras
            input=(MyInput_t*)input->next;
        }       

        //A bemenetek kezelesehez szukseges idozitesek lekerdezese
        waitTime=MySwTimer_getWaitTime32(&manager->timerManager);
    } //while(1)
}
//------------------------------------------------------------------------------
//bemenet pergesmentesitese
static void MyInputs_samplingAndAntibounce(MyInput_t* input, uint64_t time)
{
    //Bemenet mintavetelezese a beallitott callback funckio segitsegevel
    MyInput_sample_t sample=input->cfg->samplingFunc(input->cfg->privData);

    //Bemenet prellmentesitese...

    if (input->lastSample != sample)
    {   //Tortent allapot valtas az adott bemeneten az elozo mintavetel ota
        //prell idozites inditasa...
        input->antibounceStableTime = time + input->cfg->antibounceTime;
        //elmentjuk a valtozast
        input->lastSample = sample;
    } else
    {   //Az elozo mintavetelhez kepest a bemenet allapota nem valtozott.

        if (input->antibounceStableTime)
        {   //Idozites fut. Tehat ezen a bemeneten kezelni kell a pergest...

            //Ellenorizzuk, hogy eltelt e annyi ido, amennyi a stabil allapothoz
            //szukseges...
            if (time >= input->antibounceStableTime)
            {   //A bemenet adott idon tul stabil.

                //Megjegyezzuk a bemenet uj, stabil allapotat.
                input->state=sample;

                //Perges mentesitesi idozites leallhat.
                input->antibounceStableTime=0;
            }
        }
    }
}
//------------------------------------------------------------------------------
//Mar pergesmentesitett bemenet magas szintu kezelese
static void MyInputs_processInput(MyInput_t* input, uint64_t time)
{
    bool timingRequest=false;

    if (input->lastState != input->state)
    {   //Valtozott a bemenet

        //A kovetkezo koros valtozas figyeleshez elmenti az allapotot.
        input->lastState = input->state;


        //Most lett aktivalva a bemenet?
        if (input->state == input->cfg->activState)
        {   //ez az aktivalas pillanata.

            //Ha van beregisztralva callback, akkor meghivja
            if (input->cfg->activatedFunc)
            {
                input->cfg->activatedFunc(input->cfg->privData);
            }


            //Valtaskor az elso hosszu gombnyomas leirot alitja be...
            input->nextLongPress=input->cfg->longPress;

            if ((input->nextLongPress==NULL) ||
                (input->nextLongPress->time==0))
            {   //Az adott bemenethez nem tartozik hosszu aktivalasra kezelendo
                //esemeny.
                input->nextEventTime=0;
            } else
            {   //van beregisztralva esemeny hosszu aktivalasra. Az elso
                //idopont meghatarozasa.
                input->nextEventTime= time + input->nextLongPress->time;
            }

        } else
        {   //Ez az inaktivalas pillanata.

            //Ha van beregisztralva callback, akkor meghivja
            if (input->cfg->inactivatedFunc)
            {
                input->cfg->inactivatedFunc(input->cfg->privData);
            }

            //Az elozo (aktivalas ota eltelt ido es a mostani ido
            //kozotti kulonbseg alapjan, ha az egy adott idonel
            //rovidebb, akkor rovid aktivalas tortent. Nyomogomb eseten
            //peldaul rovid gombnyomas.

            //Itt bevezetunk egy minimum idokozt is, melynel viszont
            //hoszabbnak kell lennie, evvel is miniamlizalva a teves
            //jelzest.
            uint64_t delta = time - input->lastEdgeTimeStamp;
            if ((delta < input->cfg->shortTime) &&
                (delta >= input->cfg->minTime))
            {   //Rovid aktivalas a bemeneten.
                if (input->cfg->shortFunc)
                {
                    input->cfg->shortFunc(input->cfg->privData);
                }
            }

            //Az esetleges hosszu aktivalasi esemenyek idoziteset leallitja
            input->nextEventTime=0;
        } //if (input->state == input->cfg->activState) else

        //bemenet valtozas idopontjanak elmentese.
        input->lastEdgeTimeStamp=time;

    } //if (input->lastState != input->state)
    else
    if ((input->nextEventTime) && (time >= input->nextEventTime))
    {   //Stabil allapot van a bemeneten, es van beregisztralva a bemenethez
        //hosszu aktiv allapothoz tartozo esemeny, mely most be is kovetkezett.

        //A hozza tartozo funkcio meghivasa
        if (input->nextLongPress->func)
        {
            input->nextLongPress->func(input->cfg->privData);
        }

        //Kovetkezo hosszu gombnyomas leirora ugrik
        input->nextLongPress++;

        if ((input->nextLongPress==NULL) ||
            (input->nextLongPress->time==0))
        {   //Az adott bemenet minden hosszu idozitett jelzeset
            //kezeltuk. Nincs tovabbi esemenyre varakozas.
            input->nextEventTime=0;
        } else
        {   //A kovetkezo beregisztralt idopont beallitasa...
            input->nextEventTime = input->lastEdgeTimeStamp +
                                   input->nextLongPress->time;
        }
    }
}
//------------------------------------------------------------------------------
