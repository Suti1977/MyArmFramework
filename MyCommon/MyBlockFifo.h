//------------------------------------------------------------------------------
//  Blockos FIFO kezelo modul, mely masolas mentes
//
//    File: MyBlockFifo.h
//------------------------------------------------------------------------------
#ifndef MYBLOCKFIFO_H_
#define MYBLOCKFIFO_H_

#include "MyCommon.h"
//------------------------------------------------------------------------------
//MyBlockFifo sajat valtozoi
typedef struct
{
    //Az adatok szamara az applikacioban allokalt buffer
    uint8_t*    buffer;
    //A tarolt elemek merete
    uint16_t    itemSize;
    //A buffer meret es az itemSize alapjan szamolt maximalis elemszam
    uint16_t    maxItemCount;

    //Iro poinbter
    uint16_t     writeIndex;
    //Olvaso pointer
    uint16_t     readIndex;
    //Fifoban talalhato elemek szama
    uint16_t     count;
} MyBlockFifo_t;
//------------------------------------------------------------------------------
//MyBlockFifo inicializalasa
void MyBlockFifo_init(MyBlockFifo_t* fifo,
                      void* buffer,
                      uint16_t bufferSize,
                      uint16_t itemSize);

//Fifo alaphelyzetbe hozasa
void MyBlockFifo_reset(MyBlockFifo_t* fifo);

//Szabad memoriaterulet lekerdezese irasra megszakitas alol
void* MyBlockFifo_getWritePtrFromIsr(MyBlockFifo_t* fifo);

//Szabad memoriaterulet lekerdezese irasra
void* MyBlockFifo_getWritePtr(MyBlockFifo_t* fifo);

//Iro pointer leptetese megszakitas alol
void MyBlockFifo_commitWriteFromIsr(MyBlockFifo_t* fifo);

//Iro pointer leptetese
void MyBlockFifo_commitWrite(MyBlockFifo_t* fifo);

//Olvashato adatra mutato pointer lekerdezese megszakitas alol
void* MyBlockFifo_getReadPtrFromIsr(MyBlockFifo_t* fifo);

//Olvashato adatra mutato pointer lekerdezese
void* MyBlockFifo_getReadPtr(MyBlockFifo_t* fifo);

//Olvaso pozicio leptetese megszakitas alol
void MyBlockFifo_commitReadFromIsr(MyBlockFifo_t* fifo);

//Olvaso pozicio leptetese
void MyBlockFifo_commitRead(MyBlockFifo_t* fifo);

//Szabad helyek lekerdezese megszakitas alol
uint16_t MyBlockFifo_getFreeFromIsr(MyBlockFifo_t* fifo);

//Szabad helyek lekerdezese
uint16_t MyBlockFifo_getFree(MyBlockFifo_t* fifo);

//Fifoban tarolt elemek szamanak lekerdezese megszakitas alol
uint16_t MyBlockFifo_getAvailableFromIsr(MyBlockFifo_t* fifo);

//Fifoban tarolt elemek szamanak lekerdezese
uint16_t MyBlockFifo_getAvailable(MyBlockFifo_t* fifo);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif //MYBLOCKFIFO_H_
