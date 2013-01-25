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
#ifndef LOG_H_
#define LOG_H_

#include <sstream>
#include <iostream>
#include <mutex>

namespace fritz {

class Log {
private:
	std::mutex    mutex;
	std::string   prefix = "libfritz++";
	std::ostream *dstream = &std::cout;
	std::ostream *estream = &std::cerr;
	std::ostream *istream = &std::cout;

	std::string getLocator(std::string file, int line) const;
	void        putLogMessage(const std::ostream &message, std::ostream *stream, std::string file, int line);

protected:
	std::string getPrefix() const             { return prefix;         }
	void        setPrefix(std::string prefix) { this->prefix = prefix; }

public:
	void dlog(const std::ostream &message, std::string file, int line);
	void elog(const std::ostream &message, std::string file, int line);
	void ilog(const std::ostream &message, std::string file, int line);

	void setLogStreams(std::ostream *elog, std::ostream *ilog, std::ostream *dlog);
};

extern Log log;

#define DBG(x) {::fritz::log.dlog(std::stringstream().flush() << x, std::string(__FILE__), __LINE__);}
#define INF(x) {::fritz::log.ilog(std::stringstream().flush() << x, std::string(__FILE__), __LINE__);}
#define ERR(x) {::fritz::log.elog(std::stringstream().flush() << x, std::string(__FILE__), __LINE__);}


} /* namespace fritz */
#endif /* LOG_H_ */
