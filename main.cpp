#include "result.h"

enum class ErrorCode{
	Any
}
enum class ResultCode{
	Any
}
enum ResultEnumCode{
	Any
}

int main()
{
	// Success<T> and Error<E>
	auto val1 = Result::Ok(); // just success, Success<NoneValue>
	val1.value(); // ill formed compile error, function deleted
	val1.is_success(); // ok
	val1.status(); // true
	auto val2 = Result::Ok(1); // success with value
	val2.value(); // returns int
	auto val3 = Result::Error(); // error, but we dont care about code
	val3.error(); // ill formed, compile error, function deleted
	val3.is_error(); // ok
	val3.status(); // false
	auto val4 = Result::Error(ErrorCode::Any); // error with code
	val3.is_error(); // ErrorCode::Any
	// struct NoneValue{}; struct NoneError{}
	// ResultValue<success_t = NoneValue,error_t = NoneError>
	ResultValue<int> valSuccess = Result::Ok(1); // ok success_t = int, error_t = NoneError
	// ResultValue valSuccess = Result::Ok(1); // deduction hints ??
	valSuccess.error(); // function deleted for std::is_same<error_t, NoneError>
	valSuccess.value(); // 1
	ResultValue<NoneValue, int> valSuccess = Result::Err(1); // ok success_t = NoneValue, error_t = int
	// ResultValue valSuccess = Result::Err(1); // deduction hints ??
	valSuccess.error(); // 1
	valSuccess.value(); // function deleted for std::is_same<success_t, NoneValue>

	// Possible conversions
	ResultValue<int, NoneError> val5 = Result::Ok(1);
	ResultValue<int, int> val6 = Result::Ok(1);
	ResultValue<NoneValue, int> val7 = Result::Err(1);
	ResultValue<int, int> val8 = Result::Err(1);
	// Setters
	val8.set_value(1);
	val8.set_error(1);
	// this might be not so obvious
	ResultValue<long, ErrorCode> val13 = Result::Ok(1); // precision is not lost
	ResultValue<double, ErrorCode> val15 = Result::Ok(1.1f); // precision is not lost, but I can not tell if this can be accepted
	ResultValue<uint8_t, ErrorCode> val16 = Result::Ok(char('a')); // precision is not lost, but I can not tell if this can be accepted
	ResultValue<unsigned long, ErrorCode> val14 = Result::Ok(1).cast_to<unsigned int>(); // might cause issues but this is explicit, unsigned int is convertible to unsigned long without issues
	// this is even worse
	ResultValue<int, NoneError> val17 = Result::Err(1); // left side does not care about error value, only about error itself
	ResultValue<int, NoneError> val18 = val7; // same here but instead Error<E> -> ResultValue<T,NoneError>, it is ResultValue<T,E> -> ResultValue<T,NoneError>
	ResultValue<NoneValue, int> val19 = Result::Ok(1); // left side does not care about success value, only about success itself
	ResultValue<NoneValue, int> val20 = val6; // same here but instead Success<T> -> ResultValue<NoneValue,E>, it is ResultValue<T,E> -> ResultValue<NoneValue,E>

	// Invalid conversions
	ResultValue<int, NoneError> val9 = Result::Err(1); // success_t does not match
	ResultValue<NoneValue, int> val10 = Result::Ok(1); // error_t does not match
	ResultValue<ResultCode, ErrorCode> val11 = Result::Err(1); // error_t does not match neither can be converted
	ResultValue<ResultEnumCode, ErrorCode> val12 = Result::Ok(1); // success_t does not match neither can be converted
	ResultValue<unsigned long, ErrorCode> val14 = Result::Ok(1); // precision is not lost, but conversion might change value
	ResultValue<int, int> val21;
	// Getters on uninitialized object
	val21.value(); // throw??
	val21.error(); // throw??
	ResultValue<int, int> val22{};
	// Object is initialized, but no clear error or success
	val22.value(); // throw??
	val22.error(); // throw??
	// ill formed, disable explicit bool operator for Success<T>, Error<E> and ResultValue<T,E>
	if(val1) // Success<NoneValue>
		;
	if(val3) // Error<NoneError>
		;
	if(valSuccess) // ResultValue<NoneValue, int>
		;
	return 0;
}

ResultValue<int, int> is_even(int value)
{
	return value & 0x0001 ? Result::Error(value) : Result::Ok(value); // Error<int> and Success<int> are convertible to ResultValue<int,int>
}

ResultValue<int> is_odd(int value)
{
	return value & 0x0001 ? Result::Ok(value) : Result::Error(); // Error<NoneError> and Success<int> are convertible to ResultValue<int> - ResultValue<int,NoneError>
}

ResultValue<>/*ResultValue ??*/ is_divisible_by_3(int value)
{
	return value % 3 == 0 ? Result::Ok() : Result::Error(); // Error<NoneError> and Success<NoneValue> are convertible to ResultValue<> - ResultValue<NoneValue,NoneError>
}
