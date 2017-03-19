#ifdef _MSC_VER

#pragma once

// Target Windows XP or later.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#endif /*MSC_VER*/
