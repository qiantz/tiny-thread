#ifndef CQTZ_THREAD_INCLUDE_
#define CQTZ_THREAD_INCLUDE_
#include "config/config.h"
#if BUILD_WITH_WINDOWS
#include "CQTZ_Thread_Win.h"
#else
#include "CQTZ_Thread_Linux.h"
#endif
#endif // CQTZ_THREAD_INCLUDE_
