//------------------------------------------------------------------------------
//  Idozitett, szekvencialis vegrehajtast segito modul
//
//    File: MySeq.c
//------------------------------------------------------------------------------
#include "MySeq.h"

//------------------------------------------------------------------------------
//Szekvencia inditasa, a megadott timer utemezesevel.
void MySeq_start(MySeq_t* mySeq,
                 MySwTimer_t* timer,
                 MySeqFunc_t** steps,
                 void* privData)
{
    mySeq->nextStep=steps;
    mySeq->timer=timer;
    mySeq->privData=privData;
    mySeq->run=true;
    MySwTimer_start(timer, 0, 0);
}
//------------------------------------------------------------------------------
//Szekvencia futtatasa.
//true-val ter vissza, ha a szekvenciaval vegzett.
bool MySeq_run(MySeq_t* mySeq)
{
    //A szekvencia nincs elinditva. Kilepes.
    if (mySeq->run==false) return false;

    if (MySwTimer_expired(mySeq->timer)==false)
    {   //A timer meg nem jart le. Kilepes.
        return false;
    }

    //Kovetkezo vegrehajtando feladat kijelolese
    MySeqFunc_t* func=*mySeq->nextStep;

    //van meg mit vegrehajtani? (NULL-al kell lezarni a szekvenciat)
    if (func == NULL)
    {   //vege a szekvencianak

        mySeq->run=false;
        //Kesz jelzes a hivo oldalnak.)
        return true;
    }

    //A hivott callback visszateresi erteke mondja meg, hogy a kovetkezo
    //lepese elott mennyit kell varni.
    uint32_t waitTime=func(mySeq->privData);

    //Timer indul
    MySwTimer_start(mySeq->timer, waitTime, 0);

    //Tablazat uj elemere all.
    mySeq->nextStep++;

    //Meg nincs kesz-->false
    return false;
}
//------------------------------------------------------------------------------
//Szekvencia vegrehajtas felbeszakitasa
void MySeq_abort(MySeq_t* mySeq)
{
    mySeq->run=false;
    MySwTimer_stop(mySeq->timer);
}
//------------------------------------------------------------------------------
