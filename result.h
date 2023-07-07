#pragma once
#include <cstddef>
#include <cstdio>
#include <exception>
#include <new>
#include <type_traits>
#include <utility>

#if defined(USE_EXCEPTIONS)
#include <stdexcept>
#endif

namespace detail {
template <typename From, typename To, typename = void>
struct is_narrowing_conversion_impl : std::true_type
{};

template <typename From, typename To>
struct is_narrowing_conversion_impl<From, To, std::void_t<decltype(To{std::declval<From>()})>> : std::false_type
{};
} // namespace detail

template <typename From, typename To>
struct is_narrowing_conversion : detail::is_narrowing_conversion_impl<From, To>
{};

#if defined(USE_EXCEPTIONS)
class bad_access : public std::logic_error
{
    using std::logic_error::logic_error;
};
#endif

namespace Result {
struct BadAccessNoThrow
{};
struct BadAccessTerminate
{};
#if defined(USE_EXCEPTIONS)
struct BadAccessThrow
{};
using DefaultBadAccess = BadAccessThrow;
#else
using DefaultBadAccess = BadAccessNoThrow;
#endif

struct EmptyValue
{
    bool operator==(const EmptyValue&) const noexcept(true)
    {
        return true;
    }
};
struct SimpleError
{
    bool operator==(const SimpleError&) const noexcept(true)
    {
        return true;
    }
};

template <typename, typename, typename>
struct ResultValue;

template <typename Value = EmptyValue>
struct Success
{
    using ok_t = Value;

    Success(const ok_t& value)
        : _value(value)
    {}

    Success(ok_t&& value)
        : _value(std::move(value))
    {}

    const Value& value() const noexcept(true)
    {
        return _value;
    }

    Value&& move() noexcept(std::is_move_assignable<Value>::value)
    {
        return std::move(_value);
    }

    template <typename T, typename std::enable_if<!is_narrowing_conversion<ok_t, T>::value, Value>::type* = nullptr>
    auto cast_to() const noexcept(std::is_copy_constructible<Success<T>>::value) -> Success<T>
    {
        return Success<T>(_value);
    }

    template <typename T, typename U = SimpleError, typename V = DefaultBadAccess>
    operator ResultValue<T, U, V>() const noexcept(noexcept(cast_to<T>()))
    {
        return ResultValue<T, U, V>(cast_to<T>());
    }

    explicit operator bool() const noexcept(true)
    {
        return true;
    }

private:
    ok_t _value;
};

template <typename ErrorType = SimpleError>
struct Failure
{
    using err_t = ErrorType;

    Failure(const err_t& error)
        : _error(error)
    {}

    Failure(err_t&& error)
        : _error(std::move(error))
    {}

    const ErrorType& error() const noexcept(true)
    {
        return _error;
    }

    ErrorType&& move() noexcept(std::is_move_constructible<err_t>::value)
    {
        return std::move(_error);
    }

    template <typename T, typename std::enable_if<std::is_same<typename std::common_type<T, err_t>::type, T>::value,
                                                  ErrorType>::type* = nullptr>
    auto cast_to() const noexcept(std::is_copy_constructible<Failure<T>>::value) -> Failure<T>
    {
        return Failure<T>(_error);
    }

    template <typename T, typename U, typename V,
              typename std::enable_if<std::is_same<typename std::common_type<T, err_t>::type, T>::value,
                                      ErrorType>::type* = nullptr>
    operator ResultValue<T, U, V>() const noexcept(noexcept(cast_to<U>()))
    {
        return ResultValue<T, U, V>(cast_to<U>());
    }

    operator Failure<SimpleError>() const noexcept(true)
    {
        return Failure<SimpleError>({});
    }

    explicit operator bool() const noexcept(true)
    {
        return false;
    }

private:
    err_t _error;
};

template <typename T, typename U>
constexpr size_t size_of()
{
    return sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U);
}

template <typename T, typename U>
constexpr size_t align_of()
{
    return sizeof(T) > sizeof(U) ? alignof(T) : alignof(U);
}

template <typename Value = EmptyValue, typename ErrorType = SimpleError, typename BadAccess = DefaultBadAccess>
struct ResultValue
{
public:
    using ok_t = Value;
    using err_t = ErrorType;
    using access_t = BadAccess;
    static_assert(!std::is_same<err_t, void>::value, "void error type is not allowed");
    static constexpr size_t _size = size_of<ok_t, err_t>();
    static constexpr size_t _align = align_of<ok_t, err_t>();

    template <typename T = ok_t, typename std::enable_if<std::is_copy_constructible<T>::value, T>::type* = nullptr>
    ResultValue(const Success<ok_t>& success)
        : _success(true)
    {
        new (&_storage) ok_t(success.value());
    }

    template <typename T = ok_t, typename std::enable_if<!std::is_copy_constructible<T>::value, T>::type* = nullptr>
    ResultValue(Success<ok_t>&& success)
        : _success(true)
    {
        new (&_storage) ok_t(success.move());
    }

    template <typename T = err_t, typename std::enable_if<std::is_copy_constructible<T>::value, T>::type* = nullptr>
    ResultValue(const Failure<err_t>& error)
        : _success(false)
    {
        new (&_storage) err_t(error.error());
    }

    template <typename T = err_t, typename std::enable_if<!std::is_copy_constructible<T>::value, T>::type* = nullptr>
    ResultValue(Failure<err_t>&& error)
        : _success(false)
    {
        new (&_storage) err_t(error.move());
    }

    void handle_error(const char* str = "") const noexcept(std::is_same<BadAccess, BadAccessNoThrow>::value)
    {
        // can be if constexpr with c++17 or templated with pre c++17
#if defined(USE_EXCEPTIONS)
        if (std::is_same<BadAccess, BadAccessThrow>::value)
            throw bad_access(str);
#endif
        fprintf(stderr, "%s\n", str);
        if (std::is_same<BadAccess, BadAccessNoThrow>::value)
            return;
        std::terminate();
    }

    // Add concept for that case, also handle primitive type
    template <typename Ret = ok_t, typename Access = access_t,
              typename std::enable_if<std::is_same<Access, BadAccessNoThrow>::value, bool>::type = true,
              typename std::enable_if<std::is_default_constructible<Ret>::value, bool>::type = true>
    auto value() const noexcept(noexcept(handle_error())) -> Ret
    {
        if (!_success || _moved)
        {
            handle_error("Attempting to get ResultValue::value()");
            return {};
        }
        return Ok();
    }

    template <typename Ret = const ok_t&, typename Access = access_t,
              typename std::enable_if<!std::is_same<Access, BadAccessNoThrow>::value, bool>::type = true>
    auto value() const noexcept(noexcept(handle_error())) -> Ret
    {
        if (!_success || _moved)
            handle_error("Attempting to get ResultValue::value()");
        return Ok();
    }

    template <typename Ret = ok_t, typename Access = access_t,
              typename std::enable_if<std::is_copy_constructible<Ret>::value, Ret>::type* = nullptr>
    auto value_or(const Ret& ret) const noexcept(true) -> ok_t
    {
        if (!_success || _moved)
            return ret;
        return Ok();
    }

    template <typename Ret = err_t, typename Access = access_t,
              typename std::enable_if<std::is_same<Access, BadAccessNoThrow>::value, bool>::type = true,
              typename std::enable_if<std::is_default_constructible<Ret>::value, bool>::type = true>
    auto error() noexcept(noexcept(handle_error())) -> Ret
    {
        if (_success || _moved)
        {
            handle_error("Attempting to get ResultValue::error()");
            return {};
        }
        return Err();
    }

    template <typename Ret = const err_t&, typename Access = access_t,
              typename std::enable_if<!std::is_same<Access, BadAccessNoThrow>::value, bool>::type = true>
    auto error() noexcept(noexcept(handle_error())) -> Ret
    {
        if (_success || _moved)
            handle_error("Attempting to get ResultValue::error()");
        return Err();
    }

    // What return type? ok_t or const ok_t&
    template <typename Ret = const err_t&, typename Access = access_t>
    const Ret& error_or(const Ret& ret) const noexcept(/*noexcept(Err())*/true) // -> Ret
    {
        if (_success || _moved)
            return ret;
        return Err();
    }

    auto move_ok() noexcept(noexcept(MoveOk())) -> ok_t&&
    {
        return MoveOk();
    }

    auto move_error() noexcept(noexcept(MoveErr())) -> err_t&&
    {
        return MoveErr();
    }

    void set_value(const ok_t& value) noexcept(std::is_nothrow_constructible<ok_t>::value)
    {
        _moved = false;
        _success = true;
        new (&_storage) ok_t(value);
    }

    void set_error(const err_t& error) noexcept(std::is_nothrow_constructible<err_t>::value)
    {
        _moved = false;
        _success = false;
        new (&_storage) err_t(error);
    }

    template <typename T = ok_t, typename std::enable_if<std::is_same<T, EmptyValue>::value, T>::type* = nullptr>
    void set_success() noexcept(noexcept(set_value({})))
    {
        set_value({});
    }

    template <typename T = err_t, typename std::enable_if<std::is_same<T, SimpleError>::value, T>::type* = nullptr>
    void set_failure() noexcept(noexcept(set_error({})))
    {
        set_error({});
    }

    explicit operator bool() const noexcept(true)
    {
        return _success;
    }

private:
    ok_t& Ok() noexcept(true)
    {
        return *reinterpret_cast<ok_t*>(&_storage);
    }
    const ok_t& Ok() const noexcept(true)
    {
        return *reinterpret_cast<const ok_t*>(&_storage);
    }
    ok_t&& MoveOk() noexcept(noexcept(handle_error()))
    {
        if (!_success || _moved)
            handle_error("Attempting to move in MoveOk");
        _moved = true;
        return std::move(*reinterpret_cast<ok_t*>(&_storage));
    }
    err_t& Err() noexcept(true)
    {
        return *reinterpret_cast<err_t*>(&_storage);
    }
    const err_t& Err() const noexcept(true)
    {
        return *reinterpret_cast<const err_t*>(&_storage);
    }
    err_t&& MoveErr() noexcept(noexcept(handle_error()))
    {
        if (_success || _moved)
            handle_error("Attempting to move in MoveErr");
        _moved = true;
        return std::move(*reinterpret_cast<err_t*>(&_storage));
    }

    typename std::aligned_storage<_size, _align>::type _storage;
    bool _moved = false;
    bool _success = false;
};

template <typename Value = EmptyValue,
          typename std::enable_if<std::is_copy_constructible<Value>::value, Value>::type* = nullptr>
[[nodiscard]] auto Ok(const Value& val = {}) -> Success<Value>
{
    return Success<Value>(val);
}

template <typename Value = EmptyValue,
          typename std::enable_if<!std::is_copy_constructible<Value>::value, Value>::type* = nullptr>
[[nodiscard]] auto Ok(Value&& val) -> Success<Value>
{
    return Success<Value>(std::move(val));
}

template <typename ErrorType = SimpleError,
          typename std::enable_if<std::is_copy_constructible<ErrorType>::value, ErrorType>::type* = nullptr>
[[nodiscard]] auto Error(const ErrorType& err = {}) -> Failure<ErrorType>
{
    return Failure<ErrorType>(err);
}

template <typename ErrorType = SimpleError,
          typename std::enable_if<!std::is_copy_constructible<ErrorType>::value, ErrorType>::type* = nullptr>
[[nodiscard]] auto Error(ErrorType&& err) -> Failure<ErrorType>
{
    return Failure<ErrorType>(std::move(err));
}
} // namespace Result