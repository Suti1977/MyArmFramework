//------------------------------------------------------------------------------
//  Idozitett, szekvencialis vegrehajtast segito modul
//
//    File: MySeq.h
//------------------------------------------------------------------------------
#ifndef MYSEQ_H_
#define MYSEQ_H_

#include "MySwTimer.h"
//------------------------------------------------------------------------------
//Szekvencia lepeseit vegrehajto callback fuggvenyek prototipusa.
//Visszateresi ertekben a varakozasi idot adja vissza.
typedef uint32_t MySeqFunc_t(void* priv);

//------------------------------------------------------------------------------
//Szekvencia vezerles valtozoi
typedef struct
{
    //true, ha fut a szekvencia
    bool run;

    //A soron kovetkezo lepesre mutat
    MySeqFunc_t** nextStep;

    //A hivott callback fuggvenyeknek atadott tetszoleges adat.
    void* privData;

    //A szekvenciat vezerlo szoftveres timer.
    MySwTimer_t* timer;
} MySeq_t;
//------------------------------------------------------------------------------
//Szekvencia inditasa, a megadott timer utemezesevel.
void MySeq_start(MySeq_t* mySeq,
                 MySwTimer_t* timer,
                 MySeqFunc_t** steps,
                 void* privData);

//Szekvencia futtatasa.
//true-val ter vissza, ha a szekvenciaval vegzett.
bool MySeq_run(MySeq_t* mySeq);

//Szekvencia vegrehajtas felbeszakitasa
void MySeq_abort(MySeq_t* mySeq);
//------------------------------------------------------------------------------
#endif //MYSEQ_H_
