WARNFLAGS = -Wall -Wextra -pedantic
CXXOPTS0X = -std=c++0x -I../HippoMocks0x/ $(WARNFLAGS)
CXXOPTS = -std=c++98 -I../HippoMocks/ $(WARNFLAGS) -ansi -g
CXX = g++ $(CXXOPTS)
CXX0X = g++ $(CXXOPTS0X)
TARGETS = test.exe test0x.exe

OBJECTS = is_virtual.o test.o test_args.o test_autoptr.o test_cfuncs.o test_class_args.o test_constref_params.o test_cv_funcs.o test_dontcare.o test_except.o test_exception_quality.o test_filter.o test_functors.o test_inparam.o test_membermock.o test_mi.o test_nevercall.o test_optional.o test_outparam.o test_overload.o test_ref_args.o test_regression_arg_count.o test_retval.o test_transaction.o test_zombie.o yaffut.o


OBJECTS_0x = $(OBJECTS:%.o=%_0x.o)

GARBAGE = HippoMocksTest.ncb HippoMocksTest.suo

all: $(TARGETS)

clean:
	$(RM) $(OBJECTS) $(OBJECTS_0x) $(TARGETS) $(GARBAGE) $(OBJECTS:%.o=%.d) $(OBJECTS_0x:%.o=%.d)

runtest:
	./test.exe
	./test0x.exe

%.o: %.cpp makefile
	$(CXX) -c -o $@ $< -MMD -MP

%_0x.o: %.cpp makefile
	$(CXX0X) -c -o $@ $< -MMD -MP

test0x.exe: $(OBJECTS_0x)
	$(CXX0X) -o $@ $^

test.exe: $(OBJECTS)
	$(CXX) -o $@ $^

-include $(OBJECTS:%.o=%.d)
-include $(OBJECTS_0x:%.o=%.d)


