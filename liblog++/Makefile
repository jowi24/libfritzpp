AFILE = $(notdir $(subst /.a,.a,$(addsuffix .a,$(CURDIR))))
OBJS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

.PHONY: all clean

all: $(AFILE) 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
$(AFILE): $(OBJS)
	@ar ru $(AFILE) $(OBJS)
	
clean:
	@-rm -f $(AFILE) $(OBJS) $(DEPFILE)

###
# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(CXXFLAGS) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)