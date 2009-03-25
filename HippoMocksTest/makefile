WARNFLAGS = -Wall -Wextra -Werror -pedantic -pedantic-errors
CXXOPTS0X = -std=c++0x -I../HippoMocks0x/ $(WARNFLAGS)
CXXOPTS = -std=c++98 -I../HippoMocks/ $(WARNFLAGS) -ansi
CXX = g++ $(CXXOPTS)
CXX0X = g++ $(CXXOPTS0X)

all: test test0x

%_0x.o: %.cpp 
	$(CXX0X) -c -o $@ $^

test0x: is_virtual_0x.o test_0x.o test_args_0x.o test_class_args_0x.o test_class_mock_0x.o test_constref_params_0x.o test_cv_funcs_0x.o test_except_0x.o test_functors_0x.o test_optional_0x.o test_ref_args_0x.o test_retval_0x.o test_transaction_0x.o yaffut_0x.o
	$(CXX) -o $@ $^

test: is_virtual.o test.o test_args.o test_class_args.o test_class_mock.o test_constref_params.o test_cv_funcs.o test_except.o test_functors.o test_optional.o test_ref_args.o test_retval.o test_transaction.o yaffut.o
	$(CXX) -o $@ $^


