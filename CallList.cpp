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

#include "CallList.h"

#include <algorithm>
#include <cstdlib>
#include <time.h>

#include "Tools.h"
#include "Config.h"
#include <liblog++/Log.h>
#include "FritzClient.h"

namespace fritz{

class CallEntrySort {
private:
	bool ascending;
	CallEntry::eElements element;
public:
	CallEntrySort(CallEntry::eElements element = CallEntry::ELEM_DATE, bool ascending = true) {
		this->element   = element;
		this->ascending = ascending;
	}
	bool operator() (CallEntry ce1, CallEntry ce2){
		switch(element) {
		case CallEntry::ELEM_DATE:
			return (ascending ? (ce1.timestamp < ce2.timestamp) : (ce1.timestamp > ce2.timestamp));
			break;
		case CallEntry::ELEM_DURATION:
			if (ce1.duration.size() < ce2.duration.size())
				return (ascending ? true : false);
			if (ce1.duration.size() > ce2.duration.size())
				return (ascending ? false : true);
			return (ascending ? (ce1.duration < ce2.duration) : (ce1.duration > ce2.duration));
			break;
		case CallEntry::ELEM_LOCALNAME:
			return (ascending ? (ce1.localName < ce2.localName) : (ce1.localName > ce2.localName));
			break;
		case CallEntry::ELEM_LOCALNUMBER:
			return (ascending ? (ce1.localNumber < ce2.localNumber) : (ce1.localNumber > ce2.localNumber));
			break;
		case CallEntry::ELEM_REMOTENAME:
			if (ce1.remoteName == "unknown" && ce2.remoteName == "unknown")
				return false;
			if (ce1.remoteName == "unknown")
				return (ascending ? true : false);
			if (ce2.remoteName == "unknown")
				return (ascending ? false : true);
			return (ascending ? (ce1.remoteName < ce2.remoteName) : (ce1.remoteName > ce2.remoteName));
			break;
		case CallEntry::ELEM_REMOTENUMBER:
			return (ascending ? (ce1.remoteNumber < ce2.remoteNumber) : (ce1.remoteNumber > ce2.remoteNumber));
			break;
		case CallEntry::ELEM_TYPE:
			return (ascending ? (ce1.type < ce2.type) : (ce1.type > ce2.type));
			break;
		default:
			ERR("invalid element given for sorting.");
			return false;
		}
	}
};

CallList *CallList::me = nullptr;

CallList::CallList()
: thread{nullptr}, lastCall{0}, lastMissedCall{0}, valid{false} {
	reload();
}

CallList *CallList::GetCallList(bool create){
	if(!me && create){
		me = new CallList();
	}
	return me;
}

void CallList::CreateCallList() {
	DeleteCallList();
	me = new CallList();
}

void CallList::DeleteCallList() {
	if (me) {
		DBG("deleting call list");
		delete me;
		me = nullptr;
	}
}

CallList::~CallList()
{
	thread->join(); //TODO cancellation?
	delete thread;
	DBG("deleted call list");
}

void CallList::run() {
	DBG("CallList thread started");

	FritzClient *fc = gConfig->fritzClientFactory->create();
	std::string msg = fc->requestCallList();
	delete fc;

	std::vector<CallEntry> callList;
	// parse answer
	size_t pos = 2;
	// parse body
	int count = 0;
	while ((pos = msg.find("\n", pos)) != std::string::npos /*&& msg[pos+1] != '\n'*/) {
		pos++;
		int type          = pos;
		if (msg[type] < '0' || msg[type] > '9') { // ignore lines not starting with a number (headers, comments, etc.) {
			DBG("parser skipped line in calllist");
			continue;
		}
		int dateStart     = msg.find(';', type)          +1;
		int timeStart	  = msg.find(' ', dateStart)     +1;
		int nameStart     = msg.find(';', timeStart)     +1;
		int numberStart   = msg.find(';', nameStart)     +1;
		int lNameStart    = msg.find(';', numberStart)   +1;
		int lNumberStart  = msg.find(';', lNameStart)    +1;
		int durationStart = msg.find(';', lNumberStart)  +1;
		int durationStop  = msg.find("\n", durationStart)-1;
		if (msg[durationStop] == '\r') // fix for new Fritz!Box Firmwares that use "\r\n" on linebreak
			durationStop--;

		CallEntry ce;
		// FB developers introduce new numbering in call type column: '4' is the new '3'
		type = atoi(&msg[type]);
		ce.type           = (CallEntry::eCallType) (type == 4 ? 3: type);
		ce.date           = msg.substr(dateStart,     timeStart     - dateStart     -1);
		ce.time           = msg.substr(timeStart,     nameStart     - timeStart     -1);
		ce.remoteName     = msg.substr(nameStart,     numberStart   - nameStart     -1);
		ce.remoteNumber   = msg.substr(numberStart,   lNameStart    - numberStart   -1);
		ce.localName      = msg.substr(lNameStart,    lNumberStart  - lNameStart    -1);
		ce.localNumber    = msg.substr(lNumberStart,  durationStart - lNumberStart  -1);
		ce.duration       = msg.substr(durationStart, durationStop -  durationStart +1);

		// put the number into the name field if name is not available
		if (ce.remoteName.size() == 0)
			ce.remoteName = ce.remoteNumber;

		//       01234567        01234
		// date: dd.mm.yy, time: hh:mm
		tm tmCallTime;
		tmCallTime.tm_mday = atoi(ce.date.substr(0, 2).c_str());
		tmCallTime.tm_mon  = atoi(ce.date.substr(3, 2).c_str()) - 1;
		tmCallTime.tm_year = atoi(ce.date.substr(6, 2).c_str()) + 100;
		tmCallTime.tm_hour = atoi(ce.time.substr(0, 2).c_str());
		tmCallTime.tm_min  = atoi(ce.time.substr(3, 2).c_str());
		tmCallTime.tm_sec  = 0;
		tmCallTime.tm_isdst = 0;

		ce.timestamp = mktime(&tmCallTime);

		// workaround for AVM debugging entries in CVS list
		if (ce.remoteNumber.compare("1234567") == 0 && ce.date.compare("12.03.2005") == 0)
			continue;

		callList.push_back(ce);

		count++;
	}
	INF("CallList -> read " << count << " entries.");

	valid = false;
	callListAll = callList;
	callListIn.clear();
	callListOut.clear();
	callListMissed.clear();
	lastCall = 0;
	lastMissedCall = 0;

	for(auto ce : callListAll) {
		if (lastCall < ce.timestamp)
			lastCall = ce.timestamp;

		switch (ce.type) {
		case CallEntry::INCOMING:
			callListIn.push_back(ce);
			break;
		case CallEntry::OUTGOING:
			callListOut.push_back(ce);
			break;
		case CallEntry::MISSED:
			if (lastMissedCall < ce.timestamp)
				lastMissedCall = ce.timestamp;
			callListMissed.push_back(ce);
			break;
		default:
			DBG("parser skipped unknown call type");
			continue;
		}
	}
	valid = true;
	DBG("CallList thread ended");
}

void CallList::reload() {
	if (thread) {
		thread->join();
		delete thread;
	}
	// runs operator() in threaded context
	thread = new std::thread(&CallList::run, this);
}

CallEntry *CallList::retrieveEntry(CallEntry::eCallType type, size_t id) {
	switch (type) {
	case CallEntry::ALL:
		return &callListAll[id];
	case CallEntry::INCOMING:
		return &callListIn[id];
	case CallEntry::OUTGOING:
		return &callListOut[id];
	case CallEntry::MISSED:
		return &callListMissed[id];
	default:
		return nullptr;
	}
}

size_t CallList::getSize(CallEntry::eCallType type) {
	switch (type) {
	case CallEntry::ALL:
		return callListAll.size();
	case CallEntry::INCOMING:
		return callListIn.size();
	case CallEntry::OUTGOING:
		return callListOut.size();
	case CallEntry::MISSED:
		return callListMissed.size();
	default:
		return 0;
	}
}

size_t CallList::missedCalls(time_t since) {
	size_t missedCalls = 0;
	for (auto ce : callListMissed) {
		// track number of new missed calls
		if (ce.timestamp > since) {
			if (ce.matchesFilter())
				missedCalls++;
		} else {
			break; // no older calls will match the missed-calls condition
		}
	}
	return missedCalls;
}

void CallList::sort(CallEntry::eElements element, bool ascending) {
	CallEntrySort ces(element, ascending);
	std::sort(begin(callListAll), end(callListAll), ces); //TODO: other lists?
}

bool CallEntry::matchesFilter() {
	// entries are filtered according to the MSN filter)
	if ( Tools::MatchesMsnFilter(localNumber))
		return true;
	else{
		// if local number does not contain any of the MSNs in MSN filter, we test
		// if it does contain any number (if POTS is used fritzbox reports "Festnetz"
		// instead of the local number)
		for (auto ch : localNumber) {
			if (ch >= '0' && ch <= '9')
				return false;
		}
		return true;
	}
}

bool CallEntry::matchesRemoteNumber(std::string number) {
	return (Tools::NormalizeNumber(number).compare(Tools::NormalizeNumber(remoteNumber)) == 0);
}

}

