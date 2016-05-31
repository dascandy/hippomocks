cmake_minimum_required(VERSION 3.0)

project(HippoMocksTests CXX)

add_executable(${PROJECT_NAME}
	Framework.cpp
	is_virtual.cpp
	main.cpp
	test_args.cpp
	test_array.cpp
	test_autoptr.cpp
	test_cfuncs.cpp
	test_class_args.cpp
	test_com_support_stdcall.cpp
	test_constref_params.cpp
	test.cpp
	test_cv_funcs.cpp
	test_do.cpp
	test_dontcare.cpp
	test_except.cpp
	test_exception_quality.cpp
	test_filter.cpp
	test_inparam.cpp
	test_membermock.cpp
	test_mi.cpp
	test_nevercall.cpp
	test_objectreturn.cpp
	test_optional.cpp
	test_outparam.cpp
	test_overload.cpp
	test_ref_args.cpp
	test_regression_arg_count.cpp
	test_retval.cpp
	test_transaction.cpp
	test_zombie.cpp
)
target_link_libraries(${PROJECT_NAME} 
	PRIVATE 
		HippoMocks
)
add_test(test ${PROJECT_NAME})
