# result_code 

## Motivation
It is header only implementation of `std::expected`-like solution. Allows for returning either value or error (in that case no value is contained in the returned object).

## Functionality

Define return type as a `Result::ResultValue<T,E>` and return either `Success<T>` or `Failure<E>` using `Ok(T)` or `Error(E)`. There are move ctors defined for non-trivial types to avoid unnecessary allocations.
After object is returned its state can be checked and retrieved.

## Why another implementation?
To learn. To create small useful implementation. 

## Further work
* [X] Allow to work with codebase where exceptions are not permitted
* [X] Move `T` or `E`, add designated functions
* [X] Allow casting error codes (add operator to make it easier)
* [X] Add possibility to set value/error after object is constructed
* [ ] Change name to be easier to use (instead of `Result::ResultValue` maybe `Result::Expected`)
* [ ] Add tutorial

## Tutorial
TBD

## License
Licensed under BSD license.
