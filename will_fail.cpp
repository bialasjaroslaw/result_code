#include "result.h"

enum class ErrorCode
{
    Any
};

enum class ResultCode
{
    Any
};

enum ResultEnumCode
{
    Any
};

int main()
{
    // Invalid conversions
#if defined(FAIL_WRONG_ERROR_TYPE)
    Result::ResultValue<int, Result::SimpleError> val0 = Result::Error(1);
#endif
#if defined(FAIL_WRONG_SUCCESS_TYPE)
    Result::ResultValue<Result::EmptyValue, int> val1 = Result::Ok(1);
#endif
#if defined(FAIL_WRONG_ERROR_TYPE_ENUM)
    Result::ResultValue<ResultCode, ErrorCode> val2 = Result::Error(1);
#endif
#if defined(FAIL_INT_TO_ENUM_CONVERSION)
    Result::ResultValue<ResultEnumCode, ErrorCode> val3 = Result::Ok(1);
#endif
#if defined(FAIL_NARROWING_ASSIGNEMENT)
    Result::ResultValue<unsigned long, ErrorCode> val4 = Result::Ok(-1);
#endif
    // No default ctor
#if defined(FAIL_NO_CTOR1)
    Result::ResultValue<int, int> val5;
#endif
#if defined(FAIL_NO_CTOR2)
    Result::ResultValue<int, int> val6{};
#endif
#if defined(FAIL_NO_IMPLICIT_BOOL_CONVERSION)
    Result::ResultValue<int> val7 = Result::Ok(1);
    bool b1 = val7;
#endif
#if defined(FAIL_NO_IMPLICIT_VALUE_GET)
    Result::ResultValue<int> val8 = Result::Ok(1);
    int i1 = val8;
#endif
#if defined(FAIL_NO_STATIC_CAST_VALUE)
    Result::ResultValue<int> val9 = Result::Ok(1);
    static_cast<int>(val9);
#endif
}