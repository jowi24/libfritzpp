/*
 * libfritz++
 *
 * Copyright (C) 2007-2013 Joachim Wilke <libfritz@joachim-wilke.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "Log.h"

namespace fritz {

Log log;

std::string Log::getLocator(std::string file, int line) const {
	std::stringstream ss;
	ss << "[" << getPrefix() << "/" <<
			std::string(file, file.rfind('/') == std::string::npos ?
				 	          0 : file.rfind('/')+1, std::string::npos )
	   << ":" << line << "] ";
	return ss.str();
}

void Log::putLogMessage(const std::ostream &message, std::ostream *stream, std::string file, int line) {
	mutex.lock();
	*stream << getLocator(file, line) << message.rdbuf() << std::endl;
	mutex.unlock();
}

void Log::dlog(const std::ostream &message, std::string file, int line) {
	putLogMessage(message, dstream, file, line);
}

void Log::elog(const std::ostream &message, std::string file, int line) {
	putLogMessage(message, estream, file, line);
}

void Log::ilog(const std::ostream &message, std::string file, int line) {
	putLogMessage(message, istream, file, line);
}

void Log::setLogStreams(std::ostream *elog, std::ostream *ilog, std::ostream *dlog) {
	estream = elog;
	istream = ilog;
	dstream = dlog;
}

} /* namespace fritz */
