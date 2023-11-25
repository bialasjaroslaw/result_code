#include "result.h"

enum class ErrorCode { Any };

enum class ResultCode { Any };

enum ResultEnumCode { Any };

struct NonDefaultConstructible {
    NonDefaultConstructible(int val) : _val(val) {}
    int get() const { return _val; }
    int _val;
};
static_assert(!std::is_default_constructible<NonDefaultConstructible>::value);

struct MoveOnly {
    MoveOnly() = delete;
    MoveOnly(int val) : _val(val) {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& other) : _val(other._val) {}
    MoveOnly& operator=(MoveOnly&& other)
    {
        _val = other._val;
        return *this;
    }
    int get() const { return _val; }
    int _val;
};
static_assert(std::is_move_constructible<MoveOnly>::value);
static_assert(std::is_move_assignable<MoveOnly>::value);
static_assert(!std::is_default_constructible<MoveOnly>::value);
static_assert(!std::is_copy_constructible<MoveOnly>::value);
static_assert(!std::is_copy_assignable<MoveOnly>::value);

int main()
{
    // Invalid conversions
#if defined(FAIL_WRONG_ERROR_TYPE)
    Result::Expected<int, Result::SimpleError> val0 = Result::Error(1);
#endif
#if defined(FAIL_WRONG_SUCCESS_TYPE)
    Result::Expected<Result::EmptyValue, int> val1 = Result::Ok(1);
#endif
#if defined(FAIL_WRONG_ERROR_TYPE_ENUM)
    Result::Expected<ResultCode, ErrorCode> val2 = Result::Error(1);
#endif
#if defined(FAIL_INT_TO_ENUM_CONVERSION)
    Result::Expected<ResultEnumCode, ErrorCode> val3 = Result::Ok(1);
#endif
#if defined(FAIL_NARROWING_ASSIGNEMENT)
    Result::Expected<unsigned long, ErrorCode> val4 = Result::Ok(-1);
#endif
    // No default ctor
#if defined(FAIL_NO_CTOR1)
    Result::Expected<int, int> val5;
#endif
#if defined(FAIL_NO_CTOR2)
    Result::Expected<int, int> val6{};
#endif
#if defined(FAIL_NO_IMPLICIT_BOOL_CONVERSION)
    Result::Expected<int> val7 = Result::Ok(1);
    bool b1 = val7;
#endif
#if defined(FAIL_NO_IMPLICIT_VALUE_GET)
    Result::Expected<int> val8 = Result::Ok(1);
    int i1 = val8;
#endif
#if defined(FAIL_NO_STATIC_CAST_VALUE)
    Result::Expected<int> val9 = Result::Ok(1);
    static_cast<int>(val9);
#endif
#if defined(FAIL_NO_THROW_FOR_NON_TRIVIAL_VALUE)
    static_assert(!std::is_default_constructible<NonDefaultConstructible>::value);
    Result::Expected<NonDefaultConstructible, Result::SimpleError, Result::BadAccessNoThrow> val = Result::Error();
    val.value();
#endif
#if defined(FAIL_NO_THROW_FOR_NON_TRIVIAL_ERROR)
    static_assert(!std::is_default_constructible<NonDefaultConstructible>::value);
    Result::Expected<Result::EmptyValue, NonDefaultConstructible, Result::BadAccessNoThrow> val = Result::Error();
    val.error();
#endif
#if defined(FAIL_CAN_NOT_SET_SUCCESS)
    Result::Expected<int, int> val = Result::Error(1);
    val.set_success();
#endif
#if defined(FAIL_CAN_NOT_SET_FAILURE)
    Result::Expected<int, int> val = Result::Error(1);
    val.set_failure();
#endif
#if defined(FAIL_VALUE_OR_MOVABLE)
    Result::Expected<MoveOnly> val = Result::Ok(MoveOnly(1));
    EXPECT_EQ(val.value_or(MoveOnly(12)).get(), 12);
#endif
#if defined(FAIL_DISCARD_OK)
    Result::Ok();
#endif
#if defined(FAIL_DISCARD_FAIL)
    Result::Error();
#endif
}
