FRITZ_LIB = ../libfritz++.a
GTEST_LIB = gtest/gtest.a
OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
EXEC = libfritztest

INCLUDES += -I..
LDFLAGS += -lpthread -lgcrypt -lccgnu2

all: $(EXEC)

$(EXEC): $(OBJS) $(FRITZ_LIB) $(GTEST_LIB)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(GTEST_LIB) $(FRITZ_LIB) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $(DEFINES) $(INCLUDES) $<
	
$(GTEST_LIB):
	make -C gtest
	
$(FRITZ_LIB):
	make -C ..

	
clean:
	@-make -C gtest clean
	@-rm $(OBJS) $(DEPFILE) $(EXEC)
	
###
# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)