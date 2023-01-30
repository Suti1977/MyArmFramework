#ifndef MY_VERSION_H_
#define MY_VERSION_H_

//verzioszam osszeallitasat segito makro
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))

//Verzioszam egyes elemeinek lekerdezeset segito makrok
#define VERSION_MAJOR(v)    (v>>16)
#define VERSION_MINOR(v)    ((v>>8)&0xff)
#define VERSION_BUGFIX(v)   (v & 0xff)


#endif	//MY_VERSION_H_