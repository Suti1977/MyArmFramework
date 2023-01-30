#ifndef MY_HELPERS_H_
#define MY_HELPERS_H_

//Tomb elemszamait adja vissza
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) 	(sizeof(array) / sizeof(array[0]))
#endif

//Struktura egy elemenek meretet adja vissza
#define SIZEOF_MEMBER(type, member) sizeof(((type *)0)->member)

//BIT-re maszkot eloallito seged makro
#ifndef BIT
#define BIT(a)  (1ULL << a)
#endif

//Deffiniciok osszefuzesehez haszanlhato makrok
#define MY_PASTER( a, b )        a ## b
#define MY_EVALUATOR( a, b )     MY_PASTER(a, b)

//Definicio stringe alakitasa
#define MY_STRINGIZE2(s) #s
#define MY_STRINGIZE(s) MY_STRINGIZE2(s)




#endif //MY_HELPERS_H_

