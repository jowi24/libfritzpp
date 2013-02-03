/*
 * FakeClient_74_04_86.cpp
 *
 *  Created on: Apr 2, 2012
 *      Author: jo
 */

#include "FakeNoopClient.h"

#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>


namespace test {

class FakeBoxClient : public FakeNoopClient {
private:
	std::string version;
	std::string getFile(std::string filename) {
		std::stringstream ss;
#ifdef SOURCE_DIR
		ss << SOURCE_DIR << "/";
#endif
		ss << "test/" << version << "/" << filename;
		std::cout << "Reading " << ss.str() << std::endl;
		std::ifstream t(ss.str().c_str());
		std::string str((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
		return str;
	}

public:
	FakeBoxClient(std::string version)
	: version(version) {}

	virtual bool initCall(std::string &number) {
		return false;
	}

	virtual std::string requestLocationSettings() {
		return getFile("sipoptionen");
	}

	virtual std::string requestSipSettings() {
		return getFile("siplist");
	}

	virtual std::string requestCallList() {
		return getFile("foncalls_csv");
	}

	virtual std::string requestFonbook() {
		return getFile("fonbuch_xml");
	}

	virtual void writeFonbook(std::string xmlData) {
	}

	virtual bool reconnectISP() {
		return false;
	}

	virtual std::string getCurrentIP() {
		return "";
	}

};

class FakeBoxClientFactory : public FakeNoopClientFactory {
	virtual ~FakeBoxClientFactory() {}

	virtual fritz::FritzClient *create() {
		return new FakeBoxClient("74.04.86");
	}
};

}

