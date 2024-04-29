#pragma once

#ifdef TT_DIAGNOSTICS_TRACER_DEBUG

#include "TTDiagnosticsHelper.hpp"
#include <minitrace.h>
#define DT_INIT(...) mtr_init(__VA_ARGS__)
#define DT_FLUSH(...) mtr_flush(__VA_ARGS__)
#define DT_SHUTDOWN(...) mtr_shutdown(__VA_ARGS__)
#define DT_META_PROCESS_NAME(...) MTR_META_PROCESS_NAME(__VA_ARGS__)
#define DT_META_THREAD_NAME(...) MTR_META_THREAD_NAME(__VA_ARGS__)
#define DT_START(...) MTR_START(__VA_ARGS__)
#define DT_BEGIN(...) MTR_BEGIN(__VA_ARGS__)
#define DT_END(...) MTR_END(__VA_ARGS__)
#define DT_STEP(...) MTR_STEP(__VA_ARGS__)
#define DT_INSTANT(...) MTR_INSTANT(__VA_ARGS__)
#define DT_FINISH(...) MTR_FINISH(__VA_ARGS__)
#define DT_UNIQUE_PATH(filename) TTDiagnosticsHelper::generateUniquePath(filename, ".trace.json")

#else

#define DT_INIT(...)
#define DT_FLUSH(...)
#define DT_SHUTDOWN(...)
#define DT_META_PROCESS_NAME(...)
#define DT_META_THREAD_NAME(...)
#define DT_START(...)
#define DT_BEGIN(...)
#define DT_END(...)
#define DT_STEP(...)
#define DT_INSTANT(...)
#define DT_FINISH(...)
#define DT_UNIQUE_PATH(filename)

#endif