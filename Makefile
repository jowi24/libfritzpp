BOOST_INCLUDES ?= -I/usr/include/boost
BOOST_LIBRARIES ?= -lboost_system -lboost_thread -lpthread

LIB = libfritz++.a
OBJS = CallList.o Config.o Fonbooks.o Fonbook.o FonbookManager.o FritzClient.o FritzFonbook.o HttpClient.o Listener.o Log.o LookupFonbook.o Tools.o LocalFonbook.o Nummerzoeker.o OertlichesFonbook.o SoapClient.o TcpClient.o TelLocalChFonbook.o XmlFonbook.o

CXXFLAGS ?= -g -O2 -Wall -fPIC
CXXFLAGS += -std=c++11
LDFLAGS  += $(BOOST_LIBRARIES) -lgcrypt

.PHONY: all test clean

CXXFLAGS += $(BOOST_INCLUDES)

all: $(LIB) test

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $(DEFINES) $<

$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	@-echo Built $(LIB).

clean:
	@-make -C test clean
	@-rm $(LIB) $(OBJS) $(DEPFILE) $(TEST_OBJS) $(TEST_EXEC)

###
# Tests
test: 
	@-CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" make -C test

###
# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(CXXFLAGS) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)