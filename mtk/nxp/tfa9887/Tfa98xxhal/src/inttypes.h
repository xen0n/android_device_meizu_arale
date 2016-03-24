#ifndef _INTTYPES_H_
#define _INTTYPES_H_

//Replace this with you own

#if !defined(uint8_t)
	#define uint8_t unsigned char
#endif

#if !defined (int8_t)
	#define int8_t signed char
#endif

#if !defined (uint16_t)
	#define uint16_t unsigned short
#endif

#if !defined (int16_t)
	#define int16_t signed short
#endif

#if !defined (uint32_t)
	#define uint32_t unsigned long
#endif

#if !defined (int32_t)
	#define int32_t signed long
#endif

#if !defined (bool)
	  typedef unsigned char bool;
	  #define true ((bool)(1==1))
	  #define false ((bool)(1==0))
	#define __BOOL_DEFINED
#endif


#endif //_INTTYPES_H_