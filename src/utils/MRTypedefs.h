#pragma once
//-----------------------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------------------
#ifdef WIN32
using ISCriticalSection = CRITICAL_SECTION;
#else
using ISCriticalSection = pthread_mutex_t;
#endif
//-----------------------------------------------------------------------------
#ifdef WIN32
using ISErrorNumber = DWORD;
#else
using ISErrorNumber = int;
#endif
//-----------------------------------------------------------------------------
using ISVectorString = std::vector<std::string>;
using ISVectorChar = std::vector<char>;
using ISTimePoint = std::chrono::time_point<std::chrono::steady_clock>;
//-----------------------------------------------------------------------------
