GTEST_LIB = gtest/gtest.a
OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
EXEC = $(notdir $(subst /test,test,$(CURDIR)))

all: $(EXEC)

$(EXEC): $(OBJS) $(GTEST_LIB) $(STATIC_LIBS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(GTEST_LIB) $(STATIC_LIBS) $(LDFLAGS) -o $(EXEC)  

%.o: %.cpp
	$(CXX) -I.. $(CXXFLAGS) -o $@ -c $<
	
$(GTEST_LIB):
	$(MAKE) -C $(dir $(GTEST_LIB))

clean:
	@-make -C $(dir $(GTEST_LIB)) clean
	@-rm -f $(OBJS) $(DEPFILE) $(EXEC)
	
###
# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(CXXFLAGS) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)