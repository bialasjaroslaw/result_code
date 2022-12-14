cmake_minimum_required(VERSION 3.14)

project(result_code VERSION 0.1 LANGUAGES CXX)

add_library(result_code INTERFACE result.h)
option(RESULT_CODE_USE_EXCEPTIONS "Use exceptions for bad access errors" ON)
option(RESULT_CODE_ENABLE_TESTS "Build tests using gtest" OFF)
option(RESULT_CODE_USE_CXX_17 "Use C++17 to build" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(RESULT_CODE_USE_CXX_17)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 14)
endif()

if(RESULT_CODE_USE_EXCEPTIONS)
    target_compile_definitions(result_code INTERFACE USE_EXCEPTIONS)
endif()

if(RESULT_CODE_ENABLE_TESTS)
    # Custom macro to simplify check for fails in compilation
    macro(ADD_FAILING_TEST)
	set(oneValueArgs TARGET SOURCE DEFINE MATCH_STR)
	set(options "")
	set(multiValueArgs "")
	cmake_parse_arguments(FAILING_ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	add_executable(${FAILING_ARGS_TARGET} ${FAILING_ARGS_SOURCE})
	set_target_properties(${FAILING_ARGS_TARGET} PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
	target_compile_definitions(${FAILING_ARGS_TARGET} PRIVATE ${FAILING_ARGS_DEFINE})
	add_test(NAME ${FAILING_ARGS_TARGET}
	     COMMAND ${CMAKE_COMMAND} --build . --target ${FAILING_ARGS_TARGET} --config $<CONFIGURATION>
	     WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
	set_tests_properties(${FAILING_ARGS_TARGET} PROPERTIES
	    PASS_REGULAR_EXPRESSION ${FAILING_ARGS_MATCH_STR})
    endmacro()

    include(FetchContent)
    include(CTest)
    include(GoogleTest)

    enable_testing()
    # Fetch gtest
    FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG        58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    # gtest tests in one module
    add_executable(main main.cc)
    target_link_libraries(main PUBLIC result_code gtest_main)
    gtest_discover_tests(main)
    # Add all failing tests as separate cases to check fail reasons
    ADD_FAILING_TEST(TARGET FAIL_WRONG_ERROR_TYPE SOURCE will_fail.cpp DEFINE FAIL_WRONG_ERROR_TYPE 
	MATCH_STR "no matching function for call to.*Result::Failure")
    ADD_FAILING_TEST(TARGET FAIL_WRONG_SUCCESS_TYPE SOURCE will_fail.cpp DEFINE FAIL_WRONG_SUCCESS_TYPE 
	MATCH_STR "no matching function for call to.*Result::Success")
    ADD_FAILING_TEST(TARGET FAIL_WRONG_ERROR_TYPE_ENUM SOURCE will_fail.cpp DEFINE FAIL_WRONG_ERROR_TYPE_ENUM 
	MATCH_STR "conversion from.*Result::Failure<int>.*to non-scalar type")
    ADD_FAILING_TEST(TARGET FAIL_INT_TO_ENUM_CONVERSION SOURCE will_fail.cpp DEFINE FAIL_INT_TO_ENUM_CONVERSION 
	MATCH_STR "no matching function for call to.*Result::Success")
    ADD_FAILING_TEST(TARGET FAIL_NARROWING_ASSIGNEMENT SOURCE will_fail.cpp DEFINE FAIL_NARROWING_ASSIGNEMENT 
	MATCH_STR "no matching function for call to.*Result::Success")
    ADD_FAILING_TEST(TARGET FAIL_NO_CTOR1 SOURCE will_fail.cpp DEFINE FAIL_NO_CTOR1 
	MATCH_STR "no matching function for call to.*Result::ResultValue<int, int>::ResultValue")
    ADD_FAILING_TEST(TARGET FAIL_NO_CTOR2 SOURCE will_fail.cpp DEFINE FAIL_NO_CTOR2 
	MATCH_STR "no matching function for call to.*Result::ResultValue<int, int>::ResultValue")
    ADD_FAILING_TEST(TARGET FAIL_NO_IMPLICIT_BOOL_CONVERSION SOURCE will_fail.cpp DEFINE FAIL_NO_IMPLICIT_BOOL_CONVERSION 
	MATCH_STR "cannot convert.*Result::ResultValue<int.*>.*to.*bool")
    ADD_FAILING_TEST(TARGET FAIL_NO_IMPLICIT_VALUE_GET SOURCE will_fail.cpp DEFINE FAIL_NO_IMPLICIT_VALUE_GET 
	MATCH_STR "cannot convert.*Result::ResultValue<int.*>.*to.*int")
    ADD_FAILING_TEST(TARGET FAIL_NO_STATIC_CAST_VALUE SOURCE will_fail.cpp DEFINE FAIL_NO_STATIC_CAST_VALUE 
	MATCH_STR "invalid.*static_cast.*Result::ResultValue<int.*>.*to type.*int")
endif()
