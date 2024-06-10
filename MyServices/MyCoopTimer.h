//------------------------------------------------------------------------------
//  Kooperativ eroforrasokban alkalmazhato szoftveres timer
//
//    File: MyCoopTimer.h
//------------------------------------------------------------------------------
#ifndef MYCOOPTIMER_H_
#define MYCOOPTIMER_H_

#include "MyRM.h"
#include "MySwTimer.h"

//Idozites letelte eseten, az eroforrascsoport timer managere altal hivott
//callback definicioja
typedef void MyCoopTimer_expiredFunc_t(void* callbackData);

struct coopResourceExtension_t;
//------------------------------------------------------------------------------
//MyCoopTimer valtozoi
typedef struct
{
    //A hasznalt szoftveres idozito valtozoi
    MySwTimer_t timer;

    //A timerhez tartozo kooperativ eroforras
    struct coopResourceExtension_t* owner;

    //Timer lejartakor hivodo, beregisztralhato callback funkcio
    MyCoopTimer_expiredFunc_t* expiredFunc;
    //A callback funkcioknal hasznalt tetszoleges user adat
    void* callbackData;
} MyCoopTimer_t;
//------------------------------------------------------------------------------
//Uj idozito hozzaadasa az eroforrashoz
void MyCoopTimer_createTimer(resource_t* resource, MyCoopTimer_t* timer);

//Idozito torlese az eroforrasbol
void MyCoopTimer_deleteTimer(resource_t* resource, MyCoopTimer_t* timer);

//Timer inditasa.
//interval: az az ido, amennyi ido mulva az elso lejarat kovetkezik
//periodTime: ha nem 0, akkor periodikus modban indulva ennyi idonkent hivodik
//            meg
void MyCoopTimer_start(MyCoopTimer_t* timer,
                     uint32_t interval,
                     uint32_t periodTime);

//Timer leallitasa
void MyCoopTimer_stop(MyCoopTimer_t* timer);

//Annak lekerdezese, hogy az idozites lejart-e
//true-t ad vissza, ha lejart. Egyben a jelzes torlese is.
bool MyCoopTimer_expired(MyCoopTimer_t* timer);

//Annak lekerdzese, hogy az idozito aktiv-e
bool MyCoopTimer_isActive(MyCoopTimer_t* timer);

//Idozites leteltekor meghivodo callback funkcio beregisztralasa
void MyCoopTimer_registerExpiredFunc(MyCoopTimer_t* timer,
                                     MyCoopTimer_expiredFunc_t* func,
                                     void* callbackData);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MYCOOPTIMER_H_
