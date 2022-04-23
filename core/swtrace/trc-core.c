
/*
================================================================================================#=
TRACE CORE
core\swtrace\trc-core.c

Description:
    This file contains the core software trace implementation.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdbool.h>
#include <string.h>


#include "trc-core.h"
#include "trc-adaptation.h"



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Private Internal Data
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static bool ModuleInitialized = false;

static trcLvl MinLevelToPrint = trcLvlDebug;

// @@@@ TEMP
uint8_t  *temp_message      = (uint8_t *)"Hello, World! \n";
uint32_t  temp_message_len;
// @@@@ TEMP



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Public API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~


// ---------------------------------------------------------------------------------------------+-
// ---------------------------------------------------------------------------------------------+-
extern void TRC_Core( trcType traceType,
        const char *fileName, const char *functionName, int lineNumber,
        trcLvl traceLevel, const char *formatStr, ...)
{
    if (!ModuleInitialized) return;

    // Bail out unless the given level is above or equal to the current min level.
    if (!(traceLevel >= MinLevelToPrint)) return;

    temp_message_len = strlen((const char *)temp_message);
    TRC_Dispatch_Message(temp_message, temp_message_len);

    return;
}


// ---------------------------------------------------------------------------------------------+-
// ---------------------------------------------------------------------------------------------+-
void TRC_Initialize(void)
{
    TRC_Adapt_Init();
    ModuleInitialized = true;
    return;
}


#if 0
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@|@
================================================================================================#=
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
------------------------------------------------------------------------------------------------+-
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "tlog.h"
#include "tlog-adaptation.h"
#include "platform/usart/usart-it-cli.h"


#ifndef CONTENT_BUFFER_SIZE
#define CONTENT_BUFFER_SIZE (160U)
#endif

#ifndef LOG_LINE_RETURN_STRING
#define LOG_LINE_RETURN_STRING ("\r\n" ANSI_COLOR_RESET)
#endif

// =============================================================================================#=
// TODO: move these to a C Language Extensions platform module
// =============================================================================================#=
#define CLEX_UNUSED_VARIABLE(X)      ((void)(X))
#define CLEX_UNUSED_PARAMETER(X)     CLEX_UNUSED_VARIABLE(X)
#define CLEX_UNUSED_RETURN_VALUE(X)  CLEX_UNUSED_VARIABLE(X)


static char TraceMessageContentBuff[CONTENT_BUFFER_SIZE];
static char FullTraceMessageBuff[2*CONTENT_BUFFER_SIZE];


static bool LostMessageIndicator = false;

static const char* logLevelStrings[tLogLvlNumOf] = {
    [tLogLvlDebug] = "debg",
    [tLogLvlInfo]  = "info",
    [tLogLvlError] = "erro",
    [tLogLvlFatal] = "fatl",
};

static const char* map_log_level_to_string(tLogLevel given_level)
{
    if (given_level < tLogLvlNumOf) {
        return logLevelStrings[given_level];
    }
    else {
        return "nalv";
    }
};

#define MAX_CHARS_IN_FILE_PATH 46
static const char* abridged_file_path(const char *filePath)
{
    const char* returnString;

    uint32_t origPathlen = strlen(filePath);
    uint32_t skipCnt;

    for(skipCnt=0; skipCnt<origPathlen; skipCnt++)
    {
        // Skip any leading dots
        if(filePath[skipCnt] == '.') continue;

        // Skip any leading slashes
        if(filePath[skipCnt] == '/') continue;

        break;
    }
    // At this point, skipCnt equals
    // the number of characters to be skipped from left.

    // How many additional chars must be truncated from left
    // to fit within the max field width.
    int32_t truncCnt = (origPathlen-skipCnt) - MAX_CHARS_IN_FILE_PATH;
    if(truncCnt < 0) truncCnt = 0;

    returnString = &filePath[skipCnt+truncCnt];

    return returnString;
};


static void setLostMessageIndicator(void)
{
    LostMessageIndicator = true;
};

static char checkLostMessageIndicator(void)
{
    if( LostMessageIndicator )
    {
        LostMessageIndicator = false;
        return '@';
    }
    return '-';
};


// =============================================================================#=
// Public API Functions
// =============================================================================#=


void tLog_Core(const char *file_path, const char *function_name, int line_number,
        tLogLevel given_level, const char *format_string, ...)
{
    int numChars;

    CLEX_UNUSED_PARAMETER(function_name);


    //if ( false == tlog_enter_critical_section() )
    //{
        //setLostMessageIndicator();
        //return;
    //}

    // Format the caller's trace message into the content buffer.
    va_list argptr;
    va_start(argptr, format_string);
    numChars = vsnprintf(
        TraceMessageContentBuff,
        sizeof(TraceMessageContentBuff),
        format_string, argptr
    );
    va_end(argptr);
    if( numChars >= sizeof(TraceMessageContentBuff))
    {
        // ensure null termination on string.
        TraceMessageContentBuff[ sizeof(TraceMessageContentBuff)-1 ] = '\0';
    }

    // @@@ TODO: convert this to days, hours, mins, seconds, msecs;
    // or some better date time string.
    int currentTicks = tlog_get_current_ticks();

    // Format the calling function and line number and other info into the full log message.
    numChars = snprintf(
        FullTraceMessageBuff,
        sizeof(FullTraceMessageBuff),
        "%d %s: %*s:%03d %c %s%s",
        currentTicks,
        map_log_level_to_string(given_level),
        MAX_CHARS_IN_FILE_PATH,
        abridged_file_path(file_path),
        line_number,
        checkLostMessageIndicator(),
        TraceMessageContentBuff,
        LOG_LINE_RETURN_STRING
    );

    if( numChars >= sizeof(TraceMessageContentBuff))
    {
        // ensure null termination on string.
        TraceMessageContentBuff[ sizeof(TraceMessageContentBuff)-1 ] = '\0';
    }

    bool boolReturn = tlog_output_formatted_trace_message( (uint8_t *)FullTraceMessageBuff, strlen(FullTraceMessageBuff) );
    if(!boolReturn) setLostMessageIndicator();

    //tlog_exit_critical_section();

    if (given_level == tLogLvlFatal) tlog_handle_assertion_failure();
}

void tLog_Raw(const char *givenBuffer, uint32_t givenLen)
{
    if (!loggerInitialized)
    {
        return;
    }

    if (!enableLogging)
    {
        return;
    }

    if ( false == tlog_enter_critical_section() )
    {
        return;
    }

    bool boolReturn = tlog_output_formatted_trace_message( (uint8_t *)givenBuffer, givenLen );
    if(!boolReturn) setLostMessageIndicator();

    tlog_exit_critical_section();
}


void tLog_SetLogLevel(tLogLevel level)
{
    minLogLevelToPrint = level;
}

#endif
