cmake_minimum_required(VERSION 2.6)
project (libfritz++)
set(SRCS cc++/url.cpp cc++/urlstring.cpp cc++/mime.cpp cc++/soap.cpp
         CallList.cpp Config.cpp 
         Fonbooks.cpp Fonbook.cpp FonbookManager.cpp FritzClient.cpp FritzFonbook.cpp 
         HttpClient.cpp Listener.cpp LocalFonbook.cpp LookupFonbook.cpp Nummerzoeker.cpp OertlichesFonbook.cpp 
         SoapClient.cpp TcpClient.cpp TelLocalChFonbook.cpp Tools.cpp XmlFonbook.cpp)
add_library(fritz++ STATIC ${SRCS})
