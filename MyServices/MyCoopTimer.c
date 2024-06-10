//------------------------------------------------------------------------------
//  Kooperativ eroforrasokban alkalmazhato szoftveres timer
//
//    File: MyCoopTimer.c
//------------------------------------------------------------------------------
#include "MyCoopTimer.h"
#include "MyCoopResourceGroup.h"
#include <string.h>

static void MyCoopTimer_expired_cb(void* callbackData);
//------------------------------------------------------------------------------
//Uj idozito hozzaadasa az eroforrashoz
void MyCoopTimer_createTimer(resource_t* resource, MyCoopTimer_t* timer)
{
    memset(timer, 0, sizeof(MyCoopTimer_t));

    timer->owner = (coopResourceExtension_t*)resource->ext;
    coopResourceGroup_t* group=(coopResourceGroup_t*)timer->owner->group;

    //Elobb az eroforrast hozza kell adni egy eroforars csoporthoz, es csak
    //utana szabad a timert letrehozni, mivel a csoport timer managerere szukseg
    //van!
    ASSERT(group);

    //Timer hozzaadodik a csoporthoz tartozo kozos timer managerhez
    MySwTimer_addTimer(&group->timerManager, &timer->timer);

    //A timer lejartakor meghivand callback beregisztralasa
    MySwTimer_registerExpiredFunc(&timer->timer,
                                  MyCoopTimer_expired_cb,
                                  timer);
}
//------------------------------------------------------------------------------
//Idozito torlese az eroforrasbol
void MyCoopTimer_deleteTimer(resource_t* resource, MyCoopTimer_t* timer)
{
    timer->owner = (coopResourceExtension_t*)resource->ext;
    coopResourceGroup_t* group=(coopResourceGroup_t*)timer->owner->group;

    //Elobb az eroforrast hozza kell adni egy eroforars csoporthoz, es csak
    //utana szabad a timert letrehozni, mivel a csoport timer managerere szukseg
    //van!
    ASSERT(group);

    MySwTimer_deleteTimer(&group->timerManager, &timer->timer);
}
//------------------------------------------------------------------------------
//Timer inditasa.
//interval: az az ido, amennyi ido mulva az elso lejarat kovetkezik
//periodTime: ha nem 0, akkor periodikus modban indulva ennyi idonkent hivodik
//            meg
void MyCoopTimer_start(MyCoopTimer_t* timer,
                     uint32_t interval,
                     uint32_t periodTime)
{
    MySwTimer_start(&timer->timer, interval, periodTime);
}
//------------------------------------------------------------------------------
//Timer leallitasa
void MyCoopTimer_stop(MyCoopTimer_t* timer)
{
    MySwTimer_stop(&timer->timer);
}
//------------------------------------------------------------------------------
//Annak lekerdezese, hogy az idozites lejart-e
//true-t ad vissza, ha lejart. Egyben a jelzes torlese is.
bool MyCoopTimer_expired(MyCoopTimer_t* timer)
{
    return MySwTimer_expired(&timer->timer);
}
//------------------------------------------------------------------------------
//Annak lekerdzese, hogy az idozito aktiv-e
bool MyCoopTimer_isActive(MyCoopTimer_t* timer)
{
    return MySwTimer_isActive(&timer->timer);
}
//------------------------------------------------------------------------------
//Idozites leteltekor meghivodo callback funkcio beregisztralasa
void MyCoopTimer_registerExpiredFunc(MyCoopTimer_t* timer,
                                     MyCoopTimer_expiredFunc_t* func,
                                     void* callbackData)
{
    timer->expiredFunc=func;
    timer->callbackData=callbackData;
}
//------------------------------------------------------------------------------
//A timer lejartakor az eroforras csoport kozos timer managere altal hivott
//callback.
static void MyCoopTimer_expired_cb(void* callbackData)
{
    MyCoopTimer_t* timer=(MyCoopTimer_t*) callbackData;

    //Jelzes az eroforrasnak, hogy timer esemeny tortent, ezert azt futtatni
    //kell!
    MyCoopResource_setEvent(timer->owner->resource,
                            MY_COOP_RESOURCE_EVENT__RUN_REQUEST);

    //Ha van beregisztralva hozza callback, akkro azt i s meghivja
    if (timer->expiredFunc)
    {
        timer->expiredFunc(timer->callbackData);
    }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
