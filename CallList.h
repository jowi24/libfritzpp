/*
 * libfritz++
 *
 * Copyright (C) 2007-2012 Joachim Wilke <libfritz@joachim-wilke.de>
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

#ifndef CALLLIST_H
#define CALLLIST_H

#include <string>
#include <vector>
#include <thread>

namespace fritz{

class CallList;

class CallEntry {
public:
	enum eCallType {
		ALL      = 0,
		INCOMING = 1,
		MISSED   = 2,
		OUTGOING = 3
	};
	enum eElements {
		ELEM_TYPE,
		ELEM_DATE,
		ELEM_REMOTENAME,
		ELEM_REMOTENUMBER,
		ELEM_LOCALNAME,
		ELEM_LOCALNUMBER,
		ELEM_DURATION,
	};
	eCallType   type;
	std::string date;
	std::string time;
	std::string remoteName;
	std::string remoteNumber;
	std::string localName;
	std::string localNumber;
	std::string duration;
	time_t      timestamp;
	bool matchesFilter();
	bool matchesRemoteNumber(std::string number);
};

class CallList {
private:
	std::thread *thread;
	std::vector<CallEntry> callListIn;
	std::vector<CallEntry> callListMissed;
	std::vector<CallEntry> callListOut;
	std::vector<CallEntry> callListAll;
	time_t lastCall;
	time_t lastMissedCall;
	bool valid;
	static CallList *me;
    CallList();
public:
	static CallList *GetCallList(bool create = true);
	/**
	 * Activate call list support.
	 * This method fetches the call list from the fritz box. Following calls to
	 * CallList::getCallList() return a reference to this call list object.
	 * If CreateCallList is not called before a call to getCallList() this triggers fetching
	 * the call list in a separate thread (which is possibly not wanted).
	 */
	static void CreateCallList();
	static void DeleteCallList();
    virtual ~CallList();
	void run();
	void reload();
	bool isValid() { return valid; }
	CallEntry *retrieveEntry(CallEntry::eCallType type, size_t id);
	size_t getSize(CallEntry::eCallType type);
	size_t missedCalls(time_t since);
	time_t getLastCall() { return lastCall; }
	time_t getLastMissedCall() { return lastMissedCall; }
	/**
	 * Sorts the calllist's entries by the given element and in given order.
	 * @param the element used for sorting
	 * @param true if sort order is ascending, false otherwise
	 */
	void sort(CallEntry::eElements element = CallEntry::ELEM_DATE, bool ascending = true);

};

}

#endif /*CALLLIST_H_*/
