//------------------------------------------------------------------------------
//  CSV formatumu kimenet eloallitasat segito modul
//
//    File: MyCSV.h
//------------------------------------------------------------------------------
#ifndef MYCSV_H_
#define MYCSV_H_

#include "MyCommon.h"
//------------------------------------------------------------------------------
//CSV formatumu sorok epitesehez szukseges valtozok halmaza
typedef struct
{
    char* buff;
    char* buildPtr;
    uint32_t available;
} MyCSV_builder_t;
//------------------------------------------------------------------------------
//CSV epito struktura inicializalasa
void MyCSV_initBuild(MyCSV_builder_t* builder, char* buffer, uint32_t bufferSize);


void MyCSV_addStr(MyCSV_builder_t* builder, const char* str);
void MyCSV_addInt32(MyCSV_builder_t* builder, const int32_t* data);
void MyCSV_addUint32(MyCSV_builder_t* builder, const uint32_t* data);
void MyCSV_addInt64(MyCSV_builder_t* builder, const int64_t* data);
void MyCSV_addUint64(MyCSV_builder_t* builder, const uint64_t* data);
void MyCSV_addFloat(MyCSV_builder_t* builder, const float* data);
void MyCSV_addDouble(MyCSV_builder_t* builder, const double* data);
void MyCSV_addBool(MyCSV_builder_t* builder, const bool* data);

#endif //MYCSV_H_
