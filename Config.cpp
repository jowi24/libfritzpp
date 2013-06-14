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

#include "Config.h"

#include "CallList.h"
#include "FonbookManager.h"
#include "Listener.h"
#include <liblog++/Log.h>
#include "Tools.h"

namespace fritz {

Config* gConfig = nullptr;

void Config::Setup(std::string hostname, std::string username, std::string password, bool logPersonalInfo) {

	if (gConfig)
		delete gConfig;
    gConfig = new Config( hostname, username, password);
	gConfig->mConfig.logPersonalInfo = logPersonalInfo;

}

bool Config::Init(bool *locationSettingsDetected, std::string *countryCode, std::string *regionCode){

	// preload phone settings from Fritz!Box
	bool validPassword = Tools::GetLocationSettings();
	if (gConfig->getCountryCode().empty() || gConfig->getRegionCode().empty()) {
		if (locationSettingsDetected)
			*locationSettingsDetected = false;
		if (countryCode)
			gConfig->setCountryCode(*countryCode);
		if (regionCode)
			gConfig->setRegionCode(*regionCode);
	} else {
		if (locationSettingsDetected)
			*locationSettingsDetected = true;
		if (countryCode)
			*countryCode = gConfig->getCountryCode();
		if (regionCode)
			*regionCode  = gConfig->getRegionCode();
	}

	// fetch SIP provider names
	Tools::GetSipSettings();

	return validPassword;
}

bool Config::Shutdown() {
	fritz::Listener::DeleteListener();
	fritz::FonbookManager::DeleteFonbookManager();
	fritz::CallList::DeleteCallList();
	if (gConfig) {
		delete gConfig;
		gConfig = nullptr;
	}
	INF("Shutdown of libfritz++ completed.");
	return true;
}

void Config::SetupPorts ( int listener, int ui, int upnp ) {
	if (gConfig) {
		gConfig->mConfig.listenerPort = listener;
		gConfig->mConfig.uiPort = ui;
		gConfig->mConfig.upnpPort = upnp;
	}
}

void Config::SetupMsnFilter( std::vector <std::string> vMsn) {
	if (gConfig)
		gConfig->mConfig.msn = vMsn;
}

void Config::SetupConfigDir(std::string dir)
{
	if (gConfig)
	gConfig->mConfig.configDir = dir;
}

Config::Config( std::string url, std::string username, std::string password) {
	mConfig.url          	= url;
    mConfig.username        = username;
	mConfig.password     	= password;
	mConfig.uiPort       	= 80;
	mConfig.listenerPort    = 1012;
	mConfig.upnpPort        = 49000;
	mConfig.loginType       = UNKNOWN;
	mConfig.lastRequestTime = 0;
	mConfig.logPersonalInfo = false;
	fritzClientFactory = new FritzClientFactory();
}

Config::~Config() {
}

}
