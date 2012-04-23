LIB = libfritz++.a
OBJS = cc++/url.o cc++/mime.o cc++/urlstring.o cc++/soap.o CallList.o Config.o Fonbooks.o Fonbook.o FonbookManager.o FritzClient.o FritzFonbook.o HttpClient.o Listener.o LookupFonbook.o Tools.o LocalFonbook.o Nummerzoeker.o OertlichesFonbook.o SoapClient.o TcpClient.o TelLocalChFonbook.o XmlFonbook.o

CXXFLAGS ?= -g -O2 -Wall -fPIC

.PHONY: all test clean

### for cc++ dir
INCLUDES += -I.

all: $(LIB) test

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $(DEFINES) $(INCLUDES) $<

$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	@-echo Built $(LIB).

clean:
	@-make -C test clean
	@-rm $(LIB) $(OBJS) $(DEPFILE) $(TEST_OBJS) $(TEST_EXEC)

###
# Tests
test: 
	@-make -C test

###
# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)