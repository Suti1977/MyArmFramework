#ifndef MY_STATUS_H_
#define MY_STATUS_H_

//Fuggvenek visszateresi ertekenek tipusdefinicioja
typedef int mystatus_t;

#ifndef status_t
typedef mystatus_t status_t;
#endif


//statusz kodok eloallitasara szolgalo makro
#define MAKE_STATUS(group, code) ((((group)*100) + (code)))


#ifndef kStatusGroup_Generic
#define kStatusGroup_Generic	0
#endif 

//Altalanos status visszateresi kodok
//Ezek reszben az ASF-hez igazodnak, igy az ASF-ben kapott fuggvenyekkel
//kompatibilis visszateresi erteket kapunk
enum _generic_status
{
    kStatus_Success 			= MAKE_STATUS(kStatusGroup_Generic, 0),
    kStatus_Fail 				= MAKE_STATUS(kStatusGroup_Generic, 1),
    kStatus_ReadOnly 			= MAKE_STATUS(kStatusGroup_Generic, 2),
    kStatus_OutOfRange 			= MAKE_STATUS(kStatusGroup_Generic, 3),
    kStatus_InvalidArgument 	= MAKE_STATUS(kStatusGroup_Generic, 4),
    kStatus_Timeout 			= MAKE_STATUS(kStatusGroup_Generic, 5),
    kStatus_NoTransferInProgress= MAKE_STATUS(kStatusGroup_Generic, 6),    
    kStatus_Busy                = MAKE_STATUS(kStatusGroup_Generic, 7),
};


#endif 	//MY_STATUS_H_