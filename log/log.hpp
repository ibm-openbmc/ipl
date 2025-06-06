/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* Host Firmware for POWER Systems Project                                */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2025                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#pragma once

#include <syslog.h>

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#ifdef PHAL
#include <systemd/sd-journal.h>

#include <format> //c++20
#else
#include <iostream>
#endif

namespace logger
{
namespace detail
{
// Helper function to send logs.
template <typename... Args>
inline void log(int priority, const char* fmt, Args&&... args)
{
#ifdef PHAL
    std::string message = std::vformat(fmt, std::make_format_args(args...));
    sd_journal_send("PRIORITY=%i", priority, "MESSAGE=%s", message.c_str(),
                    NULL);
#else
    std::ostringstream oss;
    oss << fmt; // Start with the format string
    ((oss << " " << args), ...);
    std::cout << oss.str() << std::endl;
#endif
}
} // namespace detail

// Logging functions
template <typename... Args> inline void info(const char* fmt, Args&&... args)
{
    detail::log(LOG_INFO, fmt, std::forward<Args>(args)...);
}

template <typename... Args> inline void error(const char* fmt, Args&&... args)
{
    detail::log(LOG_ERR, fmt, std::forward<Args>(args)...);
}

template <typename... Args> inline void debug(const char* fmt, Args&&... args)
{
    detail::log(LOG_DEBUG, fmt, std::forward<Args>(args)...);
}

template <typename... Args> inline void imp(const char* fmt, Args&&... args)
{
    detail::log(LOG_NOTICE, fmt, std::forward<Args>(args)...);
}

} // namespace logger
