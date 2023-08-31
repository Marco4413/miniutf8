## miniutf8

An [stb-style](https://github.com/nothings/stb) C++ library which allows easy UTF-8 Encoded String manipulation.

This library should work on C++17 since `std::u32string` and `std::string_view` were a thing.

### How to use

This library is straight forward to use, you just need to copy `miniutf8.hpp` into your project, and have a `.cpp` file
that includes it and defines `MINIUTF8_IMPLEMENTATION` (*just like stb libraries*).

Remember that this library is under the **MIT license**, the license is also **inside the source file** so you are not required to do anything.
**However, I would really appreciate to be credited.**

### Additional notes

If you do not want the `UTF8` namespace to be global you can put it under a custom one:

Header:
```cpp
// Do your usual header include guards

// Include files required by miniutf8
#include <cinttypes>
#include <cstring>
#include <string>
#include <string_view>

// MINIUTF8_EXT_INCLUDE disables includes from miniutf8
//  so that it does not include files inside a namespace.
#define MINIUTF8_EXT_INCLUDE
// MINIUTF8_NO_GUARDS tells miniutf8 to ignore include guards, so if the library
//  was already included in another file, it includes declarations anyway.
#define MINIUTF8_NO_GUARDS
// NOTE: MINIUTF8_NO_GUARDS is undefined after including miniutf8.

// Include miniutf8 under another namespace
namespace MyNamespace
{
    #include "miniutf8.hpp"
}
```

Source:
```cpp
#define MINIUTF8_IMPLEMENTATION
#include "path-to-your-header"
```

### Projects that use this

- [Marco4413/json](https://github.com/Marco4413/json)
  - This project can also be used as an example on how to include `miniutf8` under a different namespace.
