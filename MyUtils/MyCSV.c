//------------------------------------------------------------------------------
//  CSV formatumu kimenet eloallitasat segito modul
//
//    File: MyCSV.c
//------------------------------------------------------------------------------
#include "MyCSV.h"
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

//------------------------------------------------------------------------------
//CSV epito struktura inicializalasa
void MyCSV_initBuild(MyCSV_builder_t* builder, char* buffer, uint32_t bufferSize)
{
    builder->buff=buffer;
    builder->buildPtr=buffer;
    builder->available=bufferSize;
}
//------------------------------------------------------------------------------
void MyCSV_addStr(MyCSV_builder_t* builder, const char* str)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%s;", str);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
void MyCSV_addInt32(MyCSV_builder_t* builder, const int32_t* data)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%" PRId32 ";", *data);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
void MyCSV_addUint32(MyCSV_builder_t* builder, const uint32_t* data)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%" PRIu32 ";", *data);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
void MyCSV_addInt64(MyCSV_builder_t* builder, const int64_t* data)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%" PRId64 ";", *data);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
void MyCSV_addUint64(MyCSV_builder_t* builder, const uint64_t* data)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%" PRIu64 ";", *data);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
void MyCSV_addFloat(MyCSV_builder_t* builder, const float* data)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%f;", (double) *data);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
void MyCSV_addDouble(MyCSV_builder_t* builder, const double* data)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%f;", *data);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
void MyCSV_addBool(MyCSV_builder_t* builder, const bool* data)
{
    int written;
    written=snprintf(builder->buildPtr, builder->available, "%d;", *data);
    builder->available -= written;
    builder->buildPtr +=written;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
