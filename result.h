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

template <typename From, typename To>
constexpr bool is_narrowing_conversion_v = is_narrowing_conversion<From, To>::value;

#if defined(USE_EXCEPTIONS)
class bad_access : public std::logic_error
{
    using std::logic_error::logic_error;
};
#endif

namespace Result {
struct EmptyValue
{
    bool operator==(const EmptyValue&) const
    {
        return true;
    }
};
struct SimpleError
{
    bool operator==(const SimpleError&) const
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
    Success(ok_t value)
        : _value(value)
    {}

    Value value() const
    {
        return _value;
    }

    template <typename T, typename std::enable_if_t<!is_narrowing_conversion_v<ok_t, T>, Value>* = nullptr>
    auto cast_to() const -> Success<T>
    {
        return Success<T>(_value);
    }

    template <typename T, typename U, typename V>
    operator ResultValue<T, U, V>() const
    {
        return ResultValue<T, U, V>(cast_to<T>());
    }

    explicit operator bool() const
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
    Failure(err_t error)
        : _error(error)
    {}

    ErrorType error() const
    {
        return _error;
    }

    template <typename T,
              typename std::enable_if_t<std::is_same<std::common_type_t<T, err_t>, T>::value, ErrorType>* = nullptr>
    auto cast_to() const -> Failure<T>
    {
        return Failure<T>(_error);
    }

    template <typename T, typename U, typename V,
              typename std::enable_if_t<std::is_same<std::common_type_t<T, err_t>, T>::value, ErrorType>* = nullptr>
    operator ResultValue<T, U, V>() const
    {
        return ResultValue<T, U, V>(cast_to<U>());
    }

    operator Failure<SimpleError>() const
    {
        return Failure<SimpleError>({});
    }

    explicit operator bool() const
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

    ResultValue(const Success<ok_t>& success)
        : _success(true)
    {
        new (&_storage) ok_t(success.value());
    }
    ResultValue(const Failure<err_t>& error)
        : _success(false)
    {
        new (&_storage) err_t(error.error());
    }
    // Add concept for that case, also handle primitive type
    template <typename Ret = ok_t, typename Access = access_t,
              std::enable_if_t<std::is_same<Access, BadAccessNoThrow>::value, bool> = true>
    auto value() -> Ret
    {
        if (!_success)
        {
            handle_error("Attempting to get ResultValue::value()");
            return {};
        }
        return Ok();
    }

    template <typename Ret = const ok_t&, typename Access = access_t,
              std::enable_if_t<!std::is_same<Access, BadAccessNoThrow>::value, bool> = true>
    auto value() -> Ret
    {
        if (!_success)
            handle_error("Attempting to get ResultValue::value()");
        return Ok();
    }
    // Add concept for that case, also handle primitive type
    template <typename Ret = err_t, typename Access = access_t,
              std::enable_if_t<std::is_same<Access, BadAccessNoThrow>::value, bool> = true>
    auto error() -> Ret
    {
        if (_success)
        {
            handle_error("Attempting to get ResultValue::error()");
            return {};
        }
        return Err();
    }

    template <typename Ret = const err_t&, typename Access = access_t,
              std::enable_if_t<!std::is_same<Access, BadAccessNoThrow>::value, bool> = true>
    auto error() -> Ret
    {
        if (_success)
            handle_error("Attempting to get ResultValue::error()");
        return Err();
    }

    template <typename T = ok_t, typename std::enable_if_t<std::is_same<T, EmptyValue>::value, T>* = nullptr>
    void set_success()
    {
        set_value({});
    }

    template <typename T = err_t, typename std::enable_if_t<std::is_same<T, SimpleError>::value, T>* = nullptr>
    void set_failure()
    {
        set_error({});
    }

    void set_value(const ok_t& value)
    {
        _success = true;
        new (&_storage) ok_t(value);
    }

    void set_error(const err_t& error)
    {
        _success = false;
        new (&_storage) err_t(error);
    }

    explicit operator bool() const
    {
        return _success;
    }

private:
    ok_t& Ok()
    {
        return *reinterpret_cast<ok_t*>(&_storage);
    }
    const ok_t& Ok() const
    {
        return *reinterpret_cast<const ok_t*>(&_storage);
    }
    ok_t&& MoveOk()
    {
        _moved = true;
        return std::move(*reinterpret_cast<ok_t*>(&_storage));
    }
    err_t& Err()
    {
        return *reinterpret_cast<err_t*>(&_storage);
    }
    const err_t& Err() const
    {
        return *reinterpret_cast<const err_t*>(&_storage);
    }

    void handle_error(const char* str) const
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

    std::aligned_storage_t<_size, _align> _storage;
    bool _moved = false;
    bool _success = false;
};

template <typename Value = EmptyValue>
auto Ok(Value val = {})
{
    return Success<Value>(val);
}

template <typename ErrorType = SimpleError>
auto Error(ErrorType err = {})
{
    return Failure<ErrorType>(err);
}
} // namespace Result