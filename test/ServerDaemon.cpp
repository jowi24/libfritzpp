/*
 * Server.cpp
 *
 *  Created on: Dec 20, 2010
 *      Author: jo
 */

#include <cc++/thread.h>
#include <FBServer.h>

int main() {
	new FBWebServer(80, "74.04.86");
	while (true) {
		ost::Thread::sleep(1000);
	}

}
