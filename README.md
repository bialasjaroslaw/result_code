# result_code 

## Motivation
It is header only implementation of `std::expected`-like solution. Allows for returning either value or error (in that case no value is contained in the returned object).

## Functionality

Define return type as a `Result::Expected<T,E>` and return either `Success<T>` or `Failure<E>` using `Ok(T)` or `Error(E)`. There are move ctors defined for non-trivial types to avoid unnecessary allocations.
After object is returned its state can be checked and retrieved.

## Why another implementation?
To learn. To create small useful implementation. 

## Further work
* [X] Allow to work with codebase where exceptions are not permitted
* [X] Move `T` or `E`, add designated functions
* [X] Allow casting error codes (add operator to make it easier)
* [X] Add possibility to set value/error after object is constructed
* [X] Change name to be easier to use (instead of `Result::ResultValue` maybe `Result::Expected`)
* [X] Add tutorial

## Tutorial
### Simple function
Define a function that could return a value, or error if unable to do so. Error is not important - this might be seen as equivalent of `std::optional<T>`.
```c++
Result::Expected<int> user_id(const std::string& name)
{
    Database db(...);
    auto user = db->user_by_name(name);
    if(!user->valid())
        return Result::Error{};
    return user->id;
}
// usage
{
    auto id = user_id("user");
    if(id) // or id.is_ok()
        // this alone could throw if result is Result::Error - without check
        std::cout << "User id is:" << id.value() << '\n';
    // without check but with default
    std::cout << "User id is:" << id.value_or(-1) << '\n';
}
```
Define a function with enum error code to distinguish error type
```c++
enum class ErrorType{
    Internal,
    Database,
    NoUser,
};
Result::Expected<int, ErrorType> user_id(const std::string& name){
    Database db(...);
    if (!db->connected())
        return Result::Error(ErrorType::Database);
    try{
        auto user = db->user_by_name(name);
    }
    catch(const Database::InternalError& ex){
        return Result::Error(ErrorType::Internal);
    }
    if(!user->valid())
        return Result::Error(ErrorType::NoUser);
    return user->id;
}
// usage
{
    auto id = user_id("user");
    if(id)
        std::cout << "User id is:" << id.value() << '\n';
    else{
        auto err = id.error();
        // handle various errors
        ...
    }
}
```
Work with heavier types
```c++
Result::Expected<std::string> read_line(file_ptr* ptr){
    if(ptr == nullptr || ptr->eof())
        return Result::Error{};
    return Result::Ok(ptr->read_line());
}
// usage
{
    auto f_ptr = file_ptr::open("file.txt", "r");
    while(true){
        auto maybe_line = read_line(f_ptr);
        if(!maybe_line)
            break;
        // this will throw if not checked
        auto line = maybe_line.move_ok();
        ... // process text line by line
    }
}
```
Almost like a cascade
```c++
Result::Expected<char> read_char(file_ptr* ptr){
     if(ptr == nullptr || ptr->eof())
         return Result::Error{};
     return Result::Ok(ptr->read_char());
}
Result::Expected<int> read_int(file_ptr* ptr){
     auto ch = read_char(ptr);
     // Can not make this work with ternary operator
     if(!ch)
         return ch.error();
     // Explicit to avoid issues with Result::Expected<T,T>
     return Result::Ok(ch.value - '0');
}
```
## Issues
### I can not use exceptions/want to use exceptions/want to terminate on bad access
By default, code is compiled with no exceptions, so in case of double move or ok with error set, it will either std::terminate or return default value. This can be changed by third template parameter specification
```c++
Result::Expected<int> fun();
// above is equivalent to
Result::Expected<int, Result::SimpleError, Result::BadAccessNoThrow> fun();
// or this in case USE_EXCEPTIONS is defined by passing -DRESULT_CODE_USE_EXCEPTIONS=ON to cmake
Result::Expected<int, Result::SimpleError, Result::BadAccessThrow> fun();
// last possibility is to set BadAccessTerminate, this will terminate application if something goes wrong
Result::Expected<int, Result::SimpleError, Result::BadAccessTerminate> fun();
```
### Something different
There are more examples what can be done or what is considered as an error in `main.cc` and `will_fail.cpp`. Please check them, usually test/fail cases are well named and are self-explanatory.
## License
Licensed under BSL license.
