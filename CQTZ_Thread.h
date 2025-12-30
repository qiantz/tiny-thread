#ifndef CQTZ_THREAD_INCLUDE_
#define CQTZ_THREAD_INCLUDE_
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || defined(_WINDOWS) || defined(_WINDOWS_)
#include "CQTZ_Thread_Win.h"
#else
#include "CQTZ_Thread_Linux.h"
#endif
#endif // CQTZ_THREAD_INCLUDE_
