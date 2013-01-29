LIB = gtest.a
OBJS = gtest_main.o gtest-all.o

all: $(OBJS) $(LIB)

%.o: %.cc
	$(CXX) -g -ggdb -I.. -o $@ -c $<
	
$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	
clean:
	@-rm -f $(OBJS) $(LIB)