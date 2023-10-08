/*
    42_Webserver
    Versions 2.0
    Copyright 2023
    Distributed under the MIT License (http://opensource.org/licenses/MIT)
*/

/*
    Usage description
*/

#pragma once

/* ---------------------------------------------------------------------------------------------- */
/*                                            Includes                                            */
/* ---------------------------------------------------------------------------------------------- */

#include <ctime>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

/* ---------------------------------------------------------------------------------------------- */
/*                                         Static Defines                                         */
/* ---------------------------------------------------------------------------------------------- */

// ANSI escape color codes.
#define BLACK               "\033[30m"
#define RED                 "\033[31m"
#define GREEN               "\033[32m"
#define YELLOW              "\033[33m"
#define BLUE                "\033[34m"
#define MAGENTA             "\033[35m"
#define CYAN                "\033[36m"
#define WHITE               "\033[37m"
#define DEFAULT             "\033[38m"

// ANSI escape text modifier codes.
#define BOLD                "\033[1m"
#define DIM                 "\033[2m"
#define ITALIC              "\033[3m"
#define UNDERLINE           "\033[4m"
#define BLINKING            "\033[5m"
#define RAPID_BLINKING      "\033[6m"
#define INVERSE             "\033[7m"
#define HIDDEN              "\033[8m"
#define STRIKETHROUGH       "\033[9m"

// ANSI escape reset code.
#define RESET               "\033[0m"

/* ---------------------------------------------------------------------------------------------- */
/*                                       Log Configuration                                        */
/* ---------------------------------------------------------------------------------------------- */

/* Edit here to change how the logger works. 1 for enabling a feature, 0 for disabling a feature. */

#define LOGGING_ENABLED						// Undefine this to disable all logging outputs
// #undef LOGGING_ENABLED

#define MAX_SEVERITY_LEVEL  ZfLog::DEBUG		// Severity level to which display logs

#define DISPLAY_TIMESTAMP   true			// Displays a timestamp yyyy-mm-dd hh:nn:ss
#define DISPLAY_SEVERITY    true			// Displays the log severity
#define DISPLAY_LOCATION    false			// Displays the location of the log call (file@line)

#define AUTO_NEWLINE        true			// Appends an automatic newline after each log call
#define NONE_SIMPLE_LOG     true			// NONE severity log ONLY shows the message

// Sets the minimum field width for certain log status elements
#define TIMESTAMP_WIDTH     22
#define SEVERITY_WIDTH      8
#define LOCATION_WIDTH      1

// Configuration to change the color of certain log elements.
#define TIMESTAMP_COLOR     CYAN

#define NONE_COLOR          DEFAULT
#define FATAL_COLOR         RED << BOLD
#define ERROR_COLOR         RED
#define WARNING_COLOR       YELLOW
#define INFO_COLOR          CYAN
#define DEBUG_COLOR         YELLOW
#define VERBOSE_COLOR       DEFAULT

#define LOCATION_COLOR      DEFAULT

#define MSG_TEXT_COLOR      DEFAULT

/* ---------------------------------------------------------------------------------------------- */
/*                                             Macros                                             */
/* ---------------------------------------------------------------------------------------------- */

#ifdef LOGGING_ENABLED
#   define ZFLOG(severity)  ZfLog::Logger(severity, __FILE__, __LINE__)
#else
#   define ZFLOG(severity)  if (false) ZfLog::Logger(severity, __FILE__, __LINE__)
#endif

#define LOG_NONE            ZFLOG(ZfLog::NONE)
#define LOG_FATAL           ZFLOG(ZfLog::FATAL)
#define LOG_ERROR           ZFLOG(ZfLog::ERROR)
#define LOG_WARNING         ZFLOG(ZfLog::WARNING)
#define LOG_INFO            ZFLOG(ZfLog::INFO)
#define LOG_DEBUG           ZFLOG(ZfLog::DEBUG)
#define LOG_VERBOSE         ZFLOG(ZfLog::VERBOSE)

#define LOG                 LOG_NONE
#define LOGN                LOG_NONE
#define LOGF                LOG_FATAL
#define LOGE                LOG_ERROR
#define LOGW                LOG_WARNING
#define LOGI                LOG_INFO
#define LOGD                LOG_DEBUG
#define LOGV                LOG_VERBOSE

/* ---------------------------------------------------------------------------------------------- */
/*                                     Logger Implementation                                      */
/* ---------------------------------------------------------------------------------------------- */

namespace ZfLog
{

enum Severity
{
    NONE    = 0,
    FATAL   = 1,
    ERROR   = 2,
    WARNING = 3,
    INFO    = 4,
    DEBUG   = 5,
    VERBOSE = 6,
};

inline const char* severityToString(Severity severity)
{
    switch (severity)
    {
    case NONE:      return "NONE";
    case FATAL:     return "FATAL";
    case ERROR:     return "ERROR";
    case WARNING:   return "WARN";
    case INFO:      return "INFO";
    case DEBUG:     return "DEBUG";
    case VERBOSE:   return "VERB";
    default:        return "NONE";
    }
}

class Logger
{
public:
    Logger(const Severity& severity, const char* fileName, const int lineNum)
        : m_severity(severity)
        , m_fileName(fileName)
        , m_lineNum(lineNum)
    {
        printStatusElements();
    }

    ~Logger()
    {
        if (m_severity > MAX_SEVERITY_LEVEL) return;
        if (AUTO_NEWLINE) m_osstream << "\n";
        if (m_severity == INFO || m_severity == NONE) {
            std::cout << m_osstream.str() << std::flush;
        }
        else {
            std::cerr << m_osstream.str() << std::flush;
        }
    }

    template <typename T>
    Logger& operator<<(const T& value)
    {
        m_osstream << value;
        return *this;
    }

private:
    void printStatusElements()
    {
        if (NONE_SIMPLE_LOG && m_severity == NONE) return;
        if (DISPLAY_TIMESTAMP) displayTimestamp();
        if (DISPLAY_SEVERITY) displaySeverity();
        if (DISPLAY_LOCATION) displayLocation();
        m_osstream << MSG_TEXT_COLOR;
    }

    std::string getFormatTimeString()
    {
        std::time_t currentTime = std::time(NULL);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));
        return "[" + std::string(buffer) + "]";
    }

    void displayTimestamp()
    {
        m_osstream << TIMESTAMP_COLOR << std::setw(TIMESTAMP_WIDTH) << std::left;
        m_osstream << getFormatTimeString();
        m_osstream << RESET;
    }

    std::string getFormatSeverityString()
    {
        return "[" + std::string(severityToString(m_severity)) + "]";
    }

    void displaySeverity()
    {
        switch(m_severity)
        {
        case NONE:      m_osstream << NONE_COLOR; break;
        case FATAL:     m_osstream << FATAL_COLOR; break;
        case ERROR:     m_osstream << ERROR_COLOR; break;
        case WARNING:   m_osstream << WARNING_COLOR; break;
        case INFO:      m_osstream << INFO_COLOR; break;
        case DEBUG:     m_osstream << DEBUG_COLOR; break;
        case VERBOSE:   m_osstream << VERBOSE_COLOR; break;
        default:        m_osstream << NONE_COLOR; break;
        }
        m_osstream << std::setw(SEVERITY_WIDTH) << std::left;
        m_osstream << getFormatSeverityString();
        m_osstream << RESET;
    }

    std::string getFormatLocationString()
    {
        char buffer[20];
        sprintf(buffer, "%d", m_lineNum);
        return "(" + std::string(m_fileName) + "@" + buffer + ")";
    }

    void displayLocation()
    {
        m_osstream << LOCATION_COLOR << std::setw(LOCATION_WIDTH) << std::left;
        m_osstream << getFormatLocationString() << " ";
        m_osstream << RESET;
    }

private:
    Severity m_severity;
    const char* m_fileName;
    const int m_lineNum;
    std::ostringstream m_osstream;
}; // Class Logger

} // namespace ZfLog
