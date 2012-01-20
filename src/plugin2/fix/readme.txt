Microsoft Visual C++ 2003 compiler is famous for its broken support for standards.
Unfortunately, Gecko SDK already uses some features provided by C99.
So here we use stdint.h provided by msinttypes project:
http://code.google.com/p/msinttypes/
We copied this file to VC++ include directory.

Then, we did some fix in nptypes.h of Gecko plugin sdk.
Replace typedef for C99 types with #include <stdint.h>.

/*
 * Header file for ensuring that C99 types ([u]int32_t and bool) and
 * true/false macros are available.
 */
#if defined(WIN32) || defined(OS2)
  /*
   * Win32 and OS/2 don't know C99, so define [u]int_16/32 here. The bool
   * is predefined tho, both in C and C++.
   */

  /*
  typedef short int16_t;
  typedef unsigned short uint16_t;
  typedef int int32_t;
  typedef unsigned int uint32_t;
  */
  // Use stdint.h provided here: http://code.google.com/p/msinttypes/
  #include <stdint.h>
#elif defined(_AIX) || defined(__sun) || defined(__osf__) || defined(IRIX) || defined(HPUX)
...

