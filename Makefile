LIB = libfritz++.a
OBJS = cc++/url.o cc++/mime.o cc++/urlstring.o CallList.o Config.o Fonbooks.o Fonbook.o FonbookManager.o FritzClient.o FritzFonbook.o HttpClient.o Listener.o Tools.o LocalFonbook.o Nummerzoeker.o OertlichesFonbook.o TelLocalChFonbook.o XmlFonbook.o

CXXFLAGS ?= -g -O2 -Wall -fPIC

### libtcpclient++
INCLUDES += -I../libtcpclient++
#LIBS += libtcpclient++/tcpclient++.a

### for cc++ dir
INCLUDES += -I.

all: $(LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $(DEFINES) $(INCLUDES) $<

$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	@-echo Built $(LIB).

clean:
	@-rm $(LIB) $(OBJS) $(DEPFILE)
	
# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)