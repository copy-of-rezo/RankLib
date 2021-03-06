
/**
 * lemur-platform.h
 *
 * The point of this header is to contain the kinds of things
 * that are stored in platform.h on Unix:
 *    * endian constants
 *    * type size constants
 *    * typedefs
 */

#ifndef LEMUR_PLATFORM_H
#define LEMUR_PLATFORM_H

/*** Begin C-only section ***/

#ifndef WIN32
  /* G++ support */
  #include <stddef.h>
  #include <limits.h>
  #include <float.h>
  #include <unistd.h>
  #include <sys/socket.h>
  #include <netdb.h>
  #include <arpa/inet.h>

  typedef long long INT64;
  typedef unsigned long long UINT64;
  typedef signed int INT32;
  typedef unsigned int UINT32;
  typedef signed short INT16;
  typedef unsigned short UINT16;
  typedef off_t FILE_OFFSET;
  typedef int socket_t;

  #ifndef __CYGWIN__
    #define HAS_READDIR_R
  #endif /* __CYGWIN__ */

  #ifndef INADDR_NONE
  /* solaris does not define INADDR_NONE */
  #define INADDR_NONE INADDR_BROADCAST
  #endif

  #define LEMUR_ALIGN       16
  #define LEMUR_PREALIGN
  #define LEMUR_POSTALIGN   __attribute__ ((aligned))
#else
  #define NOGDI
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <winsock.h>

  /* Windows Visual C++ support */
  typedef __int64 INT64;
  typedef unsigned __int64 UINT64;
  typedef signed int INT32;
  typedef signed short INT16;
  typedef signed char INT8;
  typedef unsigned int UINT32;
  typedef int FILE_OFFSET;
  typedef SOCKET socket_t;

  #if _MSC_VER == 1200
    /* Specific defines for VC++ 6 */
    #define LEMUR_BROKEN_MIN
    #define LEMUR_BROKEN_MAX
    #define LEMUR_BROKEN_AUTOPTR
    #define LEMUR_NO_REMOVE
  #endif
 
  #define LEMUR_ALIGN     16
  #define LEMUR_PREALIGN  __declspec(align(16))
  #define LEMUR_POSTALIGN

  #define LEMUR_USING_FINDFIRST
#endif

#define PTR_TO_SIZET(s)   ( (size_t) ( (void*) (s) ) )
#define PTR_TO_INT(s)     ( (int) ( (size_t) ( (void*) (s) ) ) )

#ifndef MAX_UINT64
#define MAX_UINT64  ( ~ ((UINT64) 0) )
#endif

#ifndef MAX_INT64
#define MAX_INT64   ( (INT64) ( MAX_UINT64 >> 1 ) )
#endif

#ifndef MIN_INT64
#define MIN_INT64   ( (INT64) ( MAX_UINT64 ^ ( (UINT64) MAX_INT64 ) ) )
#endif

#ifndef MAX_UINT32
#define MAX_UINT32  ( ~ ((UINT32) 0) )
#endif

#ifndef MAX_INT32
#define MAX_INT32   ( (INT32) (MAX_UINT32 >> 1) )
#endif

#ifndef MIN_INT32
#define MIN_INT32   ( (INT32) ( MAX_UINT32 ^ ( (UINT32) MAX_INT32 ) ) )
#endif

#endif /* LEMUR_PLATFORM_H */

