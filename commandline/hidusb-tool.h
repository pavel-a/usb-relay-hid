#ifndef __USBHIDCHEAP_H_INCLUDED__
#define __USBHIDCHEAP_H_INCLUDED__

#include "targetver.h"
#include "hiddata.h"

#define USB_CFG_VENDOR_ID       0x16c0 /* 5824 = voti.nl */
#define USB_CFG_DEVICE_ID       0x05DF /* obdev's shared PID for HIDs */

#ifndef DEBUG_PRINT
#if 0 //ifdef DEBUG
#	define DEBUG_PRINT(arg)    printf arg
#else
#	define DEBUG_PRINT(arg)
#endif
#endif /*DEBUG_PRINT*/

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define snprintf   _snprintf
#endif

#endif /*__USBHIDCHEAP_H_INCLUDED__*/
