//------------------------------------------------------------------------------
//  Blockos FIFO kezelo modul, mely masolas mentes
//
//    File: MyBlockFifo.c
//------------------------------------------------------------------------------
#include "MyBlockFifo.h"
#include <string.h>


//------------------------------------------------------------------------------
//MyBlockFifo inicializalasa
void MyBlockFifo_init(MyBlockFifo_t* fifo,
                      void* buffer,
                      uint16_t bufferSize,
                      uint16_t itemSize)
{
    ASSERT(fifo);
    ASSERT(itemSize);

    MyBlockFifo_reset(fifo);

    fifo->buffer=buffer;
    fifo->itemSize=itemSize;

    //A maximalisan tarolhato elemek szamanak kisazmitasa
    fifo->maxItemCount=bufferSize / itemSize;
}
//------------------------------------------------------------------------------
//Fifo alaphelyzetbe hozasa
void MyBlockFifo_reset(MyBlockFifo_t* fifo)
{
    fifo->readIndex=0;
    fifo->writeIndex=0;
    fifo->count=0;
}
//------------------------------------------------------------------------------
//Szabad memoriaterulet lekerdezese irasra
void* MyBlockFifo_getWritePtrFromIsr(MyBlockFifo_t* fifo)
{
    if (fifo->count >= fifo->maxItemCount)
    {   //Nincs tobb hely a fifoban!
        return NULL;
    }

    return fifo->buffer + (fifo->writeIndex * fifo->itemSize);
}
//------------------------------------------------------------------------------
//Iro pointer leptetese
void MyBlockFifo_commitWriteFromIsr(MyBlockFifo_t* fifo)
{
    register uint16_t idx=fifo->writeIndex;
    idx++;
    if (idx >= fifo->maxItemCount) idx=0;

    fifo->writeIndex = idx;
    fifo->count++;
}
//------------------------------------------------------------------------------
//Olvashato adatra mutato pointer lekerdezese
void* MyBlockFifo_getReadPtrFromIsr(MyBlockFifo_t* fifo)
{
    if (fifo->count == 0)
    {   //A fifo ures
        return NULL;
    }

    return fifo->buffer + (fifo->readIndex * fifo->itemSize);
}
//------------------------------------------------------------------------------
//Olvaso pozicio leptetese
void MyBlockFifo_commitReadFromIsr(MyBlockFifo_t* fifo)
{
    register uint16_t idx=fifo->readIndex;
    idx++;
    if (idx >= fifo->maxItemCount) idx=0;
    fifo->readIndex = idx;

    fifo->count--;
}
//------------------------------------------------------------------------------
//Szabad helyek lekerdezese
uint16_t MyBlockFifo_getFreeFromIsr(MyBlockFifo_t* fifo)
{
    return fifo->maxItemCount - fifo->count;
}
//------------------------------------------------------------------------------
//Fifoban tarolt elemek szamanak lekerdezese
uint16_t MyBlockFifo_getAvailableFromIsr(MyBlockFifo_t* fifo)
{
    return fifo->count;
}
//------------------------------------------------------------------------------
//Szabad memoriaterulet lekerdezese irasra
void* MyBlockFifo_getWritePtr(MyBlockFifo_t* fifo)
{
    void* ret;
    MY_ENTER_CRITICAL();
    ret=MyBlockFifo_getWritePtrFromIsr(fifo);
    MY_LEAVE_CRITICAL();
    return ret;
}
//------------------------------------------------------------------------------
//Iro pointer leptetese
void MyBlockFifo_commitWrite(MyBlockFifo_t* fifo)
{
    MY_ENTER_CRITICAL();
    MyBlockFifo_commitWriteFromIsr(fifo);
    MY_LEAVE_CRITICAL();
}
//------------------------------------------------------------------------------
//Olvashato adatra mutato pointer lekerdezese
void* MyBlockFifo_getReadPtr(MyBlockFifo_t* fifo)
{
    void* ret;
    MY_ENTER_CRITICAL();
    ret=MyBlockFifo_getReadPtrFromIsr(fifo);
    MY_LEAVE_CRITICAL();
    return ret;
}
//------------------------------------------------------------------------------
//Olvaso pozicio leptetese
void MyBlockFifo_commitRead(MyBlockFifo_t* fifo)
{
    MY_ENTER_CRITICAL();
    MyBlockFifo_commitReadFromIsr(fifo);
    MY_LEAVE_CRITICAL();
}
//------------------------------------------------------------------------------
//Szabad helyek lekerdezese
uint16_t MyBlockFifo_getFree(MyBlockFifo_t* fifo)
{
    uint16_t ret;
    MY_ENTER_CRITICAL();
    ret=MyBlockFifo_getFreeFromIsr(fifo);
    MY_LEAVE_CRITICAL();
    return ret;
}
//------------------------------------------------------------------------------
//Fifoban tarolt elemek szamanak lekerdezese
uint16_t MyBlockFifo_getAvailable(MyBlockFifo_t* fifo)
{
    uint16_t ret;
    MY_ENTER_CRITICAL();
    ret=MyBlockFifo_getAvailableFromIsr(fifo);
    MY_LEAVE_CRITICAL();
    return ret;
}
//------------------------------------------------------------------------------

