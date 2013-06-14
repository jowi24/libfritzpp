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

#ifndef CONFIG_H
#define CONFIG_H

#include <mutex>
#include <iostream>
#include <string>
#include <vector>

#include "FritzClient.h"

namespace fritz {

const std::string HIDDEN = "<hidden>";
constexpr size_t RETRY_DELAY = 60;

/**
 * Global config class for libfritz++.
 * This class maintains all configuration information needed by classes part of libfritz++.
 * It is instantiated once automatically, a pointer gConfig is globally available.
 */
class Config {
public:
	enum eLoginType {
		UNKNOWN,
		PASSWORD,
		SID,
		LUA
	};
private:
	struct sConfig {
		std::string configDir;              			// path to libraries' config files (e.g., local phone book)
		std::string lang;                   			// webinterface language
		std::string url;                    			// fritz!box url
		int uiPort;						                // the port of the fritz box web interface
		int upnpPort;									// the port of the UPNP server of the fritz box
		int listenerPort;					            // the port of the fritz box call monitor
        std::string username;                           // fritz!box web interface username, if applicable
		std::string password;               			// fritz!box web interface password
		time_t lastRequestTime;                         // with eLoginType::SID: time of last request sent to fritz box
		eLoginType loginType;                           // type of login procedure
		std::string sid;                                // SID to access boxes with firmware >= xx.04.74
		std::string countryCode;            			// fritz!box country-code
		std::string regionCode;             			// fritz!box region-code
		std::vector <std::string> sipNames;				// the SIP provider names
		std::vector <std::string> sipMsns;              // the SIP provider msn numbers
		std::vector <std::string> msn;      			// msn's we are interesed in
		std::vector <std::string> selectedFonbookIDs; 	// active phone books
		std::string activeFonbook;						// currently selected Fonbook
		bool logPersonalInfo;							// log sensitive information like passwords, phone numbers, ...
	} mConfig;

    Config( std::string url, std::string username, std::string password );

public:
	/**
	 * Sets up the libfritz++ library.
	 * This has to be the first call to libfritz++.
	 * @param the hostname of the Fritz!Box device, defaults to fritz.box
	 * @param the password of the Fritz!Box device, defaults to an empty one
	 * @param allows personal information to be logged
	 */
    void static Setup(std::string hostname="fritz.box", std::string username="", std::string password="", bool logPersonalInfo = false );
	/**
	 * Sets arbitrary ports for connections to the Fritz!Box's listener and webinterface.
	 * @param the port to connect to the listener
	 * @param the port to connect to the webinterface
	 * @param the port to connect to the UPNP server
	 */
	void static SetupPorts( int listener, int ui, int upnp );
	/**
	 * Establishes MSN filtering.
	 * An MsnFilter enables the library to only notify the application on
	 * events which occur on one of the MSNs specified. A call to this method is only
	 * needed if filtering is wanted. Default is no filtering.
	 * @param the list of MSNs to filter on
	 */
	void static SetupMsnFilter( std::vector <std::string> vMsn );
	/**
	 * Sets up a directory for arbitrary data storage.
	 * This is currently used by local fonbook to persist the fonbook entries to a file.
	 * @param full path to the writable directory
	 */
	void static SetupConfigDir( std::string dir);

	/**
	 * Initiates the libfritz++ library.
	 * @param indicates, whether auto-detection of location settings was successful
	 * @param Sets the default value for countryCode. If locationSettingsDetected == true, this returns the detected countryCode.
	 * @param Sets the default value for regionCode. If locationSettingsDetected == true, this returns the detected regionCode.
	 */
	bool static Init( bool *locationSettingsDetected = nullptr, std::string *countryCode = nullptr, std::string *regionCode = nullptr );

	/**
	 * Closes all pending connections and objects held by libfritz++.
	 * Stores unsaved data.
	 */
	bool static Shutdown();

	std::string &getConfigDir( )                      { return mConfig.configDir; }
	std::string &getLang( )                           { return mConfig.lang; }
	void setLang( std::string l )                     { mConfig.lang = l; }
	std::string &getUrl( )                            { return mConfig.url; }
	int getUiPort( )				                  { return mConfig.uiPort; }
	int getListenerPort( )				              { return mConfig.listenerPort; }
	int getUpnpPort( )                                { return mConfig.upnpPort; }
	std::string &getPassword( )                       { return mConfig.password; }
    std::string &getUsername( )                       { return mConfig.username; }
	eLoginType getLoginType( )                        { return mConfig.loginType; }
	void setLoginType(eLoginType type)                { mConfig.loginType = type; }
	time_t getLastRequestTime()                       { return mConfig.lastRequestTime; }
	void updateLastRequestTime()                      { mConfig.lastRequestTime = time(nullptr); }
	std::string &getSid( )                            { return mConfig.sid; }
	void setSid(std::string sid)                      { mConfig.sid = sid; }
	std::string &getCountryCode( )        	          { return mConfig.countryCode; }
	void setCountryCode( std::string cc )             { mConfig.countryCode = cc; }
	std::string &getRegionCode( )                     { return mConfig.regionCode; }
	void setRegionCode( std::string rc )              { mConfig.regionCode = rc; }
	std::vector <std::string> &getSipNames( )         { return mConfig.sipNames; }
	void setSipNames( std::vector<std::string> names) { mConfig.sipNames = names; }
	std::vector <std::string> &getSipMsns( )          { return mConfig.sipMsns; }
	void setSipMsns( std::vector<std::string> msns)   { mConfig.sipMsns = msns; }
	std::vector <std::string> getMsnFilter( )         { return mConfig.msn; }
	std::vector <std::string> getFonbookIDs( )        { return mConfig.selectedFonbookIDs; }
	void setFonbookIDs(std::vector<std::string> v)    { mConfig.selectedFonbookIDs = v; }
	std::string &getActiveFonbook( )                  { return mConfig.activeFonbook; }
	void setActiveFonbook( std::string f )            { mConfig.activeFonbook = f; }
	bool logPersonalInfo( )							  { return mConfig.logPersonalInfo; };
	virtual ~Config();

	FritzClientFactory *fritzClientFactory;
};

extern Config* gConfig;

}

#endif /* CONFIG_H_ */
