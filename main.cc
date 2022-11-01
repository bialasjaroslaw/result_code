#include "result.h"

#include <gtest/gtest.h>

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

#if __cplusplus > 201402L
#define CPP17
#endif

#if !defined(USE_EXCEPTIONS)
#define SKIP_IF_NO_EXCEPTIONS GTEST_SKIP() << "Skip due to disabled exceptions"
#define bad_access std::runtime_error
#else
#define SKIP_IF_NO_EXCEPTIONS
#endif

TEST(Construct, SimpleSuccess)
{
    auto val = Result::Ok(); // just success, Success<NoneValue>
    EXPECT_TRUE(val);
    EXPECT_EQ(val.value(), Result::EmptyValue{});
    static_assert(std::is_same<decltype(val), Result::Success<Result::EmptyValue>>::value);
}

TEST(Construct, ValueSuccess)
{
    auto val = Result::Ok(1); // success with value
    EXPECT_EQ(val.value(), 1);
    static_assert(std::is_same<decltype(val), Result::Success<int>>::value);
}

TEST(Construct, UserSuccessValueFromError)
{
    Result::ResultValue<ResultCode> customResult = Result::Error();
    EXPECT_FALSE(customResult);
}

TEST(Construct, UserSuccessValue)
{
    auto val = Result::Ok(ResultCode::Any); // success with value
    EXPECT_EQ(val.value(), ResultCode::Any);
    static_assert(std::is_same<decltype(val), Result::Success<ResultCode>>::value);
}

TEST(Construct, ComplexSuccessValueFromError)
{
    Result::ResultValue<std::string> val = Result::Error();
    EXPECT_FALSE(val);
}

TEST(Construct, SimpleError)
{
    auto val = Result::Error(); // error, but we dont care about code
    EXPECT_EQ(val.error(), Result::SimpleError{});
    static_assert(std::is_same<decltype(val), Result::Failure<Result::SimpleError>>::value);
}

TEST(Construct, ValueError)
{
    auto val = Result::Error(ErrorCode::Any); // error with code
    EXPECT_EQ(val.error(), ErrorCode::Any);
    static_assert(std::is_same<decltype(val), Result::Failure<ErrorCode>>::value);
}

TEST(Construct, UserSuccessAndErrorValueFromError)
{
    Result::ResultValue<ResultCode, ErrorCode> customResultErr = Result::Error(ErrorCode::Any);
    EXPECT_FALSE(customResultErr);
    EXPECT_EQ(customResultErr.error(), ErrorCode::Any);
}

TEST(Construct, UserSuccessAndErrorValueFromSuccess)
{
    Result::ResultValue<ResultEnumCode, ErrorCode> customResultEnum = Result::Ok(ResultEnumCode::Any);
    EXPECT_TRUE(customResultEnum);
    EXPECT_EQ(customResultEnum.value(), ResultEnumCode::Any);
}

TEST(Access, SuccessAccess)
{
    Result::ResultValue<Result::EmptyValue> val = Result::Ok();
    EXPECT_TRUE(val);
    EXPECT_EQ(val.value(), Result::EmptyValue{});
}
#if defined(CPP17)
TEST(Access, SuccessAccessCTAD)
{
    Result::ResultValue val = Result::Ok();
    EXPECT_TRUE(val);
    EXPECT_EQ(val.value(), Result::EmptyValue{});
}
#endif
TEST(Access, SuccessAccessErrorWithThrow)
{
    SKIP_IF_NO_EXCEPTIONS;
    Result::ResultValue<Result::EmptyValue> val = Result::Ok();
    EXPECT_THROW(val.error(), bad_access);
}

TEST(Access, SuccessWithExplicitTypeValueAccess)
{
    Result::ResultValue<int> val = Result::Ok(1);
    static_assert(std::is_same<decltype(val.value()), const int&>::value);
    EXPECT_TRUE(val);
    EXPECT_EQ(val.value(), 1);
}

TEST(Access, SuccessWithExplicitTypeValueAccessErrorWithThrow)
{
    SKIP_IF_NO_EXCEPTIONS;
    Result::ResultValue<int> val = Result::Ok(1);
    EXPECT_THROW(val.error(), bad_access);
}

TEST(Access, SuccessWithValueAccess)
{
    Result::ResultValue<int> val = Result::Ok(1);
    EXPECT_TRUE(val);
    EXPECT_EQ(val.value(), 1);
}
#if defined(CPP17)
TEST(Access, ErrorAccessCTAD)
{
    Result::ResultValue val = Result::Error();
    EXPECT_FALSE(val);
    EXPECT_EQ(val.error(), Result::SimpleError{});
}
#endif
TEST(Access, ErrorAccess)
{
    Result::ResultValue<Result::EmptyValue, Result::SimpleError> val = Result::Error();
    EXPECT_FALSE(val);
    EXPECT_EQ(val.error(), Result::SimpleError{});
}

TEST(Access, ErrorAccessSuccessWithThrow)
{
    SKIP_IF_NO_EXCEPTIONS;
    Result::ResultValue<Result::EmptyValue, Result::SimpleError> val = Result::Error();
    EXPECT_THROW(val.value(), bad_access);
}

TEST(Access, ErrorWithExplicitTypeValuesAccess)
{
    Result::ResultValue<Result::EmptyValue, int> val = Result::Error(1);
    EXPECT_EQ(val.error(), 1);
    EXPECT_FALSE(val);
}

TEST(Access, ErrorWithExplicitTypeValuesAccessSuccessWithThrow)
{
    SKIP_IF_NO_EXCEPTIONS;
    Result::ResultValue<int, int> val = Result::Error(1);
    EXPECT_THROW(val.value(), bad_access);
}

TEST(Access, NoThrowErrorAccess)
{
    Result::ResultValue<int, int, Result::BadAccessNoThrow> val = Result::Ok(1);
    EXPECT_EQ(val.value(), 1);
    static_assert(std::is_same<decltype(val.value()), int>::value);
    EXPECT_EQ(val.error(), 0); // default error is int{}
}

TEST(Access, NoThrowSuccessAccess)
{
    Result::ResultValue<int, int, Result::BadAccessNoThrow> val = Result::Error(1);
    EXPECT_EQ(val.error(), 1);
    EXPECT_EQ(val.value(), 0); // default value is int{}
}

TEST(Access, TerminateErrorAccess)
{
    Result::ResultValue<int, int, Result::BadAccessTerminate> val = Result::Ok(1);
    EXPECT_EQ(val.value(), 1);
    EXPECT_DEATH_IF_SUPPORTED(val.error(), "");
}

TEST(Access, TerminateSuccessAccess)
{
    Result::ResultValue<int, int, Result::BadAccessTerminate> val = Result::Error(1);
    EXPECT_EQ(val.error(), 1);
    EXPECT_DEATH_IF_SUPPORTED(val.value(), "");
}

TEST(Conversion, SuccessValueToFullyQualifiedResult)
{
    Result::ResultValue<int, Result::SimpleError> val = Result::Ok(1);
    EXPECT_EQ(val.value(), 1);
}

TEST(Conversion, SuccessValueToFullyQualifiedResultWithCustomTypes)
{
    Result::ResultValue<int, int> val = Result::Ok(1);
    EXPECT_EQ(val.value(), 1);
}

TEST(Conversion, ErrorValueToFullyQualifiedResult)
{
    Result::ResultValue<Result::EmptyValue, int> val = Result::Error(1);
    EXPECT_EQ(val.error(), 1);
}

TEST(Conversion, ErrorValueToFullyQualifiedResultWithCustomTypes)
{
    Result::ResultValue<int, int> val = Result::Error(1);
    EXPECT_EQ(val.error(), 1);
}

TEST(Conversion, SuccessValueNoNarrowingTypeConversionWithInt)
{
    Result::ResultValue<long, ErrorCode> val = Result::Ok(1); // precision is not lost
    EXPECT_EQ(val.value(), 1);
}

TEST(Conversion, SuccessValueNoNarrowingTypeConversionWithFP)
{
    Result::ResultValue<double, ErrorCode> val = Result::Ok(1.1f);
    EXPECT_FLOAT_EQ(val.value(), 1.1);
}
TEST(Conversion, SuccessValueNoNarrowingTypeConversionWithUnisgned)
{
    Result::ResultValue<uint8_t, ErrorCode> val = Result::Ok(static_cast<unsigned char>(69));
    EXPECT_EQ(val.value(), 69);
}
#if defined(CPP17)
TEST(Cast, SuccessCastToUnsignedSamePrecisionCTAD)
{
    Result::ResultValue val = Result::Ok(1UL).cast_to<unsigned long>();
    static_assert(std::is_same<decltype(val), Result::ResultValue<unsigned long, Result::SimpleError>>::value);
    EXPECT_EQ(val.value(), 1);
}
#endif
TEST(Cast, SuccessCastToUnsignedSamePrecision)
{
    Result::ResultValue<unsigned long> val = Result::Ok(1UL).cast_to<unsigned long>();
    static_assert(std::is_same<decltype(val), Result::ResultValue<unsigned long, Result::SimpleError>>::value);
    EXPECT_EQ(val.value(), 1);
}

TEST(Cast, SuccessCastToHigherPrecisionFP)
{
    Result::ResultValue<double, ErrorCode> val = Result::Ok(1.1f);
    EXPECT_FLOAT_EQ(val.value(), 1.1);
}

TEST(Cast, SuccessCastNoNarrowingTypeConversionWithUnisgned)
{
    Result::ResultValue<uint8_t, ErrorCode> val = Result::Ok(static_cast<unsigned char>(69));
    EXPECT_EQ(val.value(), 69);
}
#if defined(CPP17)
TEST(Deducing, DeducingResultType)
{
    Result::ResultValue val = Result::Ok(static_cast<unsigned char>(69));
    static_assert(std::is_same<decltype(val), Result::ResultValue<unsigned char, Result::SimpleError>>::value);
    EXPECT_EQ(val.value(), 69);
}

TEST(Deducing, DeducingSuccessType)
{
    Result::Success val = Result::Ok(69LL);
    static_assert(std::is_same<decltype(val), Result::Success<long long>>::value);
    EXPECT_EQ(val.value(), 69LL);
}

TEST(Deducing, DeducingErrorType)
{
    Result::Failure val = Result::Error(1LL);
    static_assert(std::is_same<decltype(val), Result::Failure<long long>>::value);
    EXPECT_EQ(val.error(), 1LL);
}
#endif
TEST(Deducing, AutoDeducingSuccessType)
{
    auto val = Result::Ok(69LL);
    static_assert(std::is_same<decltype(val), Result::Success<long long>>::value);
    EXPECT_EQ(val.value(), 69LL);
}

TEST(Deducing, AutoDeducingErrorType)
{
    auto val = Result::Error(1LL);
    static_assert(std::is_same<decltype(val), Result::Failure<long long>>::value);
    EXPECT_EQ(val.error(), 1LL);
}

TEST(Conditions, IfWithSuccess)
{
    auto val = Result::Ok(69LL);
    if (val)
        EXPECT_TRUE(true);
    else
        EXPECT_TRUE(false);
}

TEST(Conditions, IfWithFailure)
{
    auto val = Result::Error(69LL);
    if (!val)
        EXPECT_TRUE(true);
    else
        EXPECT_TRUE(false);
}

TEST(Conditions, IfWithLogicalConditions)
{
    auto val1 = Result::Ok(69LL);
    auto val2 = Result::Ok(69LL);
    if (val1 && val2)
        EXPECT_TRUE(true);
    else
        EXPECT_TRUE(false);
}

TEST(Conditions, IfWithResultSuccess)
{
    Result::ResultValue<long long> val = Result::Ok(69LL);
    if (val)
        EXPECT_TRUE(true);
    else
        EXPECT_TRUE(false);
}

TEST(Conditions, IfWithResultError)
{
    Result::ResultValue<Result::SimpleError, long long> val = Result::Error(69LL);
    if (!val)
        EXPECT_TRUE(true);
    else
        EXPECT_TRUE(false);
}

TEST(Conditions, ResultSuccessStaticCastToBool)
{
    Result::ResultValue<long long> val = Result::Ok(69LL);
    EXPECT_TRUE(static_cast<bool>(val));
}

TEST(Conditions, ResultErrorStaticCastToBool)
{
    Result::ResultValue<Result::SimpleError, long long> val = Result::Error(69LL);
    EXPECT_FALSE(static_cast<bool>(val));
}

TEST(Conditions, SuccessStaticCastToBool)
{
    auto val = Result::Ok(69LL);
    EXPECT_TRUE(static_cast<bool>(val));
}

TEST(Conditions, ErrorStaticCastToBool)
{
    auto val = Result::Error(69LL);
    EXPECT_FALSE(static_cast<bool>(val));
}

TEST(Setters, SetSuccessDifferentValue)
{
    Result::ResultValue<int> val = Result::Ok(69);
    val.set_value(13);
    EXPECT_EQ(val.value(), 13);
}

TEST(Setters, SetErrorDifferentValue)
{
    Result::ResultValue<int, int> val = Result::Error(69);
    val.set_error(42);
    EXPECT_EQ(val.error(), 42);
}

TEST(Setters, SetSuccessAfterError)
{
    Result::ResultValue<int, int> val = Result::Error(69);
    val.set_value(13);
    EXPECT_EQ(val.value(), 13);
}

TEST(Setters, SetErrorAfterSuccess)
{
    Result::ResultValue<int, int> val = Result::Ok(69);
    val.set_error(13);
    EXPECT_EQ(val.error(), 13);
}

TEST(Setters, SetSuccessWithEmptyValue)
{
    Result::ResultValue<Result::EmptyValue, Result::SimpleError> val = Result::Error();
    val.set_value({});
    EXPECT_TRUE(val);
    EXPECT_EQ(val.value(), Result::EmptyValue{});
}

TEST(Setters, SetSuccess)
{
    Result::ResultValue<Result::EmptyValue, Result::SimpleError> val = Result::Error();
    val.set_success();
    EXPECT_TRUE(val);
    EXPECT_EQ(val.value(), Result::EmptyValue{});
}

TEST(Setters, SetErrorWithEmptyValue)
{
    Result::ResultValue<Result::EmptyValue, Result::SimpleError> val = Result::Ok();
    val.set_error({});
    EXPECT_FALSE(val);
    EXPECT_EQ(val.error(), Result::SimpleError{});
}

TEST(Setters, SetError)
{
    Result::ResultValue<Result::EmptyValue, Result::SimpleError> val = Result::Ok();
    val.set_failure();
    EXPECT_FALSE(val);
    EXPECT_EQ(val.error(), Result::SimpleError{});
}

//int main(int argc, char** argv)
//{
//    ::testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
//}