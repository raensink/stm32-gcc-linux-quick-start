
/*
================================================================================================#=
TRACE CORE
core/swtrace/trc-core.c

Description:
    This file contains the core software trace implementation.

SPDX-License-Identifier: MIT-0
================================================================================================#=
*/

#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#include "trc-core.h"
#include "trc-adaptation.h"



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Private Internal Data
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
static bool ModuleInitialized = false;

static trcLvl MinLevelToDispatch = trcLvlDebug;

#ifndef CONTENT_BUFFER_SIZE
#define CONTENT_BUFFER_SIZE (140U)
#endif
static char Content_Buffer[CONTENT_BUFFER_SIZE];


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Public API Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~


// ---------------------------------------------------------------------------------------------+-
// ---------------------------------------------------------------------------------------------+-
extern void TRC_Core( trcType trace_type,
        const char *file_name, const char *function_name, int line_number,
        trcLvl trace_level, const char *format_string, ...)
{
    int     num_chars;
    int     content_len;
    size_t  size = CONTENT_BUFFER_SIZE;
    va_list argptr;

    if (!ModuleInitialized) return;

    if (!(trace_level >= MinLevelToDispatch)) return;

    // Format the caller's message into the content buffer.
    // The vsnprintf() function does not write more than size bytes
    // including the terminating null byte.
    va_start(argptr, format_string);
    num_chars = vsnprintf(
        Content_Buffer, size, format_string, argptr
    );
    va_end(argptr);

    // As per the standard: a return value of size or more
    // means that the output was truncated.
    if(num_chars >= size)
    {
        // ensure null termination on string.
        // @@@ this should not be necessary??? @@@
        Content_Buffer[ size-1 ] = '\0';

        content_len = size-1;
    }
    else {
        content_len = num_chars;
    }

    TRC_Dispatch_Message((uint8_t *)Content_Buffer, content_len);
#if 0
#endif

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

#include "tlog.h"
#include "tlog-adaptation.h"
#include "platform/usart/usart-it-cli.h"


#ifndef LOG_LINE_RETURN_STRING
#define LOG_LINE_RETURN_STRING ("\r\n" ANSI_COLOR_RESET)
#endif

// =============================================================================================#=
// TODO: move these to a C Language Extensions platform module
// =============================================================================================#=
#define CLEX_UNUSED_VARIABLE(X)      ((void)(X))
#define CLEX_UNUSED_PARAMETER(X)     CLEX_UNUSED_VARIABLE(X)
#define CLEX_UNUSED_RETURN_VALUE(X)  CLEX_UNUSED_VARIABLE(X)


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
{
    int numChars;

    CLEX_UNUSED_PARAMETER(function_name);


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
        Content_Buffer,
        LOG_LINE_RETURN_STRING
    );

    if( numChars >= sizeof(Content_Buffer))
    {
        // ensure null termination on string.
        Content_Buffer[ sizeof(Content_Buffer)-1 ] = '\0';
    }

    bool boolReturn = tlog_output_formatted_trace_message( (uint8_t *)FullTraceMessageBuff, strlen(FullTraceMessageBuff) );
    if(!boolReturn) setLostMessageIndicator();

    //tlog_exit_critical_section();

    if (given_level == tLogLvlFatal) tlog_handle_assertion_failure();
}

void tLog_SetLogLevel(tLogLevel level)
{
    minLogLevelToPrint = level;
}

#endif
