FRITZ_LIB = ../libfritz++.a
GTEST_LIB = gtest/gtest.a
OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
EXEC = libfritztest

all: $(EXEC)

$(EXEC): $(OBJS) $(FRITZ_LIB) $(GTEST_LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(EXEC) $(OBJS) $(GTEST_LIB) $(FRITZ_LIB) 

%.o: %.cpp
	$(CXX) -I.. $(CXXFLAGS) -o $@ -c $(DEFINES) $(INCLUDES) $<
	
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
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(CXXFLAGS) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)