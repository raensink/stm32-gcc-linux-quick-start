#pragma once

/*
================================================================================================#=
TRACE
core/swtrace/trc.h

Description:
    Trace is a software trace logging API similar to printf().

    It's purpose is to provide a developer with visibility into the run-time
    operation of their software to support troubleshooting and automated testing.
    Trace does this by providing a convenient "printf" style interface for generating
    custom log messages that are sent to a backend that can be customized based on
    the needs of the application and constraints of the platform.

    Trace is not intended to be a general purpose logging or serial output facility.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/


/*
------------------------------------------------------------------------------------------------+-
Software Trace Logging Levels
------------------------------------------------------------------------------------------------+-
These logging levels are used to classify the severity of each trace message.

ERROR
    The ERROR level is used to indicate that the code has detected something wrong
    that needs to be fixed.  Typically, these are software design, coding, or configuration errors.
    The developer's goal should be to drive the number of ERROR message occurrences to zero.

FATAL
    The FATAL level is identical to ERROR (as described above) in all respects
    with one additional system behavior: logging a FATAL will result in a system reset.
    Use FATAL rather than ERROR to "fail early and fail often" and when a reset is
    the only viable method to recover the system from the error.  The developer's goal should be
    to drive the number of FATAL message occurrences to zero.

INFO
    The INFO level is used to log significant events that are part of the normal operation
    of the software.  It may also be used to log the system's identity and current configuration.
    Examples include:  state changes, connection and disconnection, and messages passing
    through external interfaces.

DEBUG
    The DEBUG level is identical to INFO (as described above) in all respects.
    The only difference is that it provides an additional level of severity
    that can be enabled and disabled independently from the others.
------------------------------------------------------------------------------------------------+-
*/
typedef enum
{
    trcLvlDebug,
    trcLvlInfo,
    trcLvlError,
    trcLvlFatal,
    trcLvlNone,
}   trcLvl;


/*
------------------------------------------------------------------------------------------------+-
Software Trace Types
------------------------------------------------------------------------------------------------+-
*/
typedef enum
{
    trcTypeRaw = 0,
    trcTypeHex,
    trcTypeCom,
}   trcType;


// -----------------------------------------------------------------------------+-
// BUILD-TIME CONFIGURATION
//
// Levels that are enabled result in trace messages delivered to the backend.
// Levels that are not enabled will be optimized out at build time.
// These are the default configurations for each level.
// The caller can change the defaults at build time by providing
// their own definitions prior to the inclusion of this file.
// -----------------------------------------------------------------------------+-
#ifndef TRC_ENABLE_LVL_ERROR
#define TRC_ENABLE_LVL_ERROR 1
#endif

#ifndef TRC_ENABLE_LVL_INFO
#define TRC_ENABLE_LVL_INFO 1
#endif

#ifndef TRC_ENABLE_LVL_DEBUG
#define TRC_ENABLE_LVL_DEBUG 0
#endif


// -----------------------------------------------------------------------------+-
// This is the core function that maps the API trace log functions
// into a platform specific backend to handle the given trace message.
// A typical handler would write the information to a serial port.
// -----------------------------------------------------------------------------+-
extern void TRC_Core( trcType traceType,
        const char *fileName, const char *functionName, int lineNumber,
        trcLvl traceLevel, const char *formatStr, ...);



// -----------------------------------------------------------------------------+-
// trcASSERT - a conditional FATAL
// -----------------------------------------------------------------------------+-
#define trcAssert(_condition_, formatStr, ...) if(!(_condition_)) { TRC_Core( \
    trcTypeCom, __BASE_FILE__, __FUNCTION__, __LINE__, \
    trcLvlFatal, formatStr, ##__VA_ARGS__ ); }


// -----------------------------------------------------------------------------+-
// trcFATAL
// -----------------------------------------------------------------------------+-
#define trcFatal(formatStr, ...) TRC_Core( \
    trcTypeCom, __BASE_FILE__, __FUNCTION__, __LINE__, \
    trcLvlFatal, formatStr, ##__VA_ARGS__ )



// -----------------------------------------------------------------------------+-
// trcERROR
// -----------------------------------------------------------------------------+-
#if TRC_ENABLE_LVL_ERROR == 1

#define trcError(formatStr, ...) TRC_Core( \
    trcTypeCom, __BASE_FILE__, __FUNCTION__, __LINE__, \
    trcLvlError, formatStr, ##__VA_ARGS__ )
#else
#define trcError(formatStr, ...) do{} while(0)
#endif



// -----------------------------------------------------------------------------+-
// trcINFO
// -----------------------------------------------------------------------------+-
#if TRC_ENABLE_LVL_INFO == 1

#define trcInfo(formatStr, ...) TRC_Core( \
    trcTypeCom, __BASE_FILE__, __FUNCTION__, __LINE__, \
    trcLvlInfo, formatStr, ##__VA_ARGS__ )
#else
#define trcInfo(formatStr, ...) do{} while(0)
#endif



// -----------------------------------------------------------------------------+-
// trcDEBUG
// -----------------------------------------------------------------------------+-
#if TRC_ENABLE_LVL_DEBUG == 1

#define trcDebug(formatStr, ...)  TRC_Core( \
    trcTypeCom, __BASE_FILE__, __FUNCTION__, __LINE__, \
    trcLvlDebug, formatStr, ##__VA_ARGS__ )
#else
#define trcDebug(formatStr, ...) do{} while(0)
#endif


// -----------------------------------------------------------------------------+-
// trcRaw
//
// Trace function to bypass core formatting and
// instead log a raw or otherwise pre-formatted string.
// -----------------------------------------------------------------------------+-
#if TRC_ENABLE_LVL_DEBUG == 1

#define trcRaw(msgStr, ...)  TRC_Core( \
    trcTypeRaw, __BASE_FILE__, __FUNCTION__, __LINE__, \
    trcLvlDebug, msgStr, ##__VA_ARGS__ )
#else
#define trcRaw(formatStr, ...) do{} while(0)
#endif


// -----------------------------------------------------------------------------+-
// trcHex
//
// Trace function to format the given byte string as hex values.
// -----------------------------------------------------------------------------+-
#if TRC_ENABLE_LVL_DEBUG == 1

#define trcHex(byteStr, ...)  TRC_Core( \
    trcTypeHex, __BASE_FILE__, __FUNCTION__, __LINE__, \
    trcLvlDebug, byteStr, ##__VA_ARGS__ )
#else
#define trcHex(formatStr, ...) do{} while(0)
#endif


