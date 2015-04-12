/* =========================================================================
 * This file is part of logging-c++ 
 * =========================================================================
 * 
 * (C) Copyright 2004 - 2009, General Dynamics - Advanced Information Systems
 *
 * logging-c++ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program; If not, 
 * see <http://www.gnu.org/licenses/>.
 *
 */

///////////////////////////////////////////////////////////
//  Formatter.h
///////////////////////////////////////////////////////////

#ifndef __LOGGING_FORMATTER_H__
#define __LOGGING_FORMATTER_H__

#include <string>
#include "logging/LogRecord.h"

namespace logging
{

/*!
 *  \class Formatter
 *  \brief  This class provides default formatting capabilities.  The syntax
 *  for the format string maps to that which is used in log4j.
 *
 *  c = Log Name
 *  p = Log Level
 *  d = Date/Time
 *  F = File name
 *  L = Line number
 *  M = Function
 *  m = Log message
 *  t = Thread id
 *
 *  The default format looks like this:
 *  [%c] %p %d %F %L ==> %m
 */
class Formatter
{
public:
    static const char DEFAULT_FORMAT[];

    Formatter() : mFmt(DEFAULT_FORMAT) {}
    Formatter(const std::string& fmt) : mFmt(fmt) {}
    virtual ~Formatter();

    virtual std::string format(LogRecord* record) const;

private:
    static const char THREAD_ID[];
    static const char LOG_NAME[];
    static const char LOG_LEVEL[];
    static const char TIMESTAMP[];
    static const char FILE_NAME[];
    static const char LINE_NUM[];
    static const char MESSAGE[];
    static const char FUNCTION[];

    void replace(std::string& str, const std::string& search,
            const std::string& replace) const;

    std::string mFmt;
};

}
#endif
