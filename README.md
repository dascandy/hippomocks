[![GCC C++11](https://github.com/jeremy-ir/hippomocks/actions/workflows/gcc-cpp11.yml/badge.svg)](https://github.com/jeremy-ir/hippomocks/actions/workflows/gcc-cpp11.yml)
[![GCC C++20](https://github.com/jeremy-ir/hippomocks/actions/workflows/gcc-cpp20.yml/badge.svg)](https://github.com/jeremy-ir/hippomocks/actions/workflows/gcc-cpp20.yml)
[![Clang](https://github.com/jeremy-ir/hippomocks/actions/workflows/clang.yml/badge.svg)](https://github.com/jeremy-ir/hippomocks/actions/workflows/clang.yml)
[![MSVC](https://github.com/jeremy-ir/hippomocks/actions/workflows/msvc.yml/badge.svg)](https://github.com/jeremy-ir/hippomocks/actions/workflows/msvc.yml)


- [Introduction](#introduction)
- [To-Do](#to-do)
- [MockRespository class](#mockrespository-class)
  - [OnCall() Method](#oncall-method)
    - [OnCallFunc(funcName)](#oncallfuncfuncname)
  - [ExpectCall() Method](#expectcall-method)
    - [ExpectCallFunc(funcName)](#expectcallfuncfuncname)
    - [ExpectCallsFunc(funcName, numCalls)](#expectcallsfuncfuncname-numcalls)
    - [ExpectMinCallsFunc(funcName, minCalls)](#expectmincallsfuncfuncname-mincalls)
    - [Disable ExpectCall Order Requirement](#disable-expectcall-order-requirement)
  - [NeverCall() Method](#nevercall-method)
    - [NeverCallFunc(funcName)](#nevercallfuncfuncname)
- [TCall Class](#tcall-class)
  - [Return() Method](#return-method)
  - [With() Method](#with-method)
  - [After() Method](#after-method)
  - [Do() Method](#do-method)
  - [Match() Method](#match-method)
  - [Throw() Method](#throw-method)
- [Other Methods](#other-methods)
  - [In() Method](#in-method)
  - [Out() Method](#out-method)
  - [Deref() Method](#deref-method)
- [Gotchas](#gotchas)
- [Further Examples](#further-examples)
- [Special Thanks](#special-thanks)


# Introduction
HippoMocks is a C-compatible, C++-based, single-header mocking framework originally developed by Peter Bindels ([Github](https://github.com/dascandy/)), and modified with additional functionality. Unlike most mocking frameworks, which use either object seams (which can only be used against object member functions), or linker/preprocessor seams (which force a mock replacement for the entire build), HippoMocks patches JMP OpCodes into the source in order to replace function calls with mocks. It is able to reverse this process as well, providing a very clean methodology for mocking functions without changing production code, maintaining access to the original functions, and enabling compatibility across multiple compilers and operating systems.

# To-Do
- [ ] Update documentation with C++ usage model
- [ ] Fix MacOS integration (changes to [permission protections in ld64](https://stackoverflow.com/a/61924409) mean that mprotect no longer works as expected in OSX) 
- [x] Test and (if not working) enable With() filtering for NeverCallFunc()
- [x] Enable OnCallsFunc() (C-style mock setting a minimum number of expectations)
- [x] Migrate test framework to Google Test
- [ ] Increase the number of unit tests
- [x] Enable CI to gate checkins on the unit tests
- [ ] Add general comments to hippomocks.h to make it more understandable
- [ ] Add doxygen comments for the functions and classes

# MockRespository class
MockRespository is the object that is responsible for the storage of function mocks, expectations, and return values as well as the patching and un-patching of the JMP OpCodes to facilitate temporary mocking of functions. Declaration of the MockRepository is fairly simple.

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  ...
}
```

After declaring a mock, the mock will persist for the lifespan of the MockRepository object. Therefore, if you declare your MockRepository as a global, mocks created will exists for the duration of the program's life; if you create it in a test, mocks created will exist until you leave the local scope of the test.

## OnCall() Method
OnCall() is used to mock a function. The name of the function name behaves like a function pointer. In the event that it is called, you may make assertions or take actions on it.

### OnCallFunc(funcName)
``` C++
// test.cpp
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  mocks.OnCallFunc(bar)...
 
  foo();
}
```

## ExpectCall() Method
ExpectCall() is used to mock a function. It functions almost identically to OnCall(); however, if the function is not called, upon destruction of the MockRepository object, an exception will be thrown. Additionally, it will expect the function to be called once and only once per instance of ExpectCall(). If you have multiple calls to the function you want to mock, you will have to have multiple mocks, one for each call. The order of the ExpectCall() methods matters; the framework will expect the mocks to be called in the listed order.

### ExpectCallFunc(funcName)
``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;

  // bar1() must be called, then bar2()
  mocks.ExpectCallFunc(bar1)...
  mocks.ExpectCallFunc(bar2)...
  foo();
}
```

### ExpectCallsFunc(funcName, numCalls)
Set a number of times a function must be called.

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
 
  // bar() must be called 3 times in a row
  mocks.ExpectCallsFunc(bar, 3)...
 
  foo();
}
```

### ExpectMinCallsFunc(funcName, minCalls)
Sets a minumum number of times a mock must be called.

``` C++
#include "hippomocks.h"

TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;

  // bar() must be called at least 3 times in a row
  mocks.ExpectMinCallsFunc(bar, 3)...

  foo();
}
```

### Disable ExpectCall Order Requirement
In order to disable the order expectation functionality, you can denote that by setting the "autoExpect" flag to "false".

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  mocks.autoExpect = false;
 
  // bar1() and bar2() must be called, but in any order
  mocks.ExpectCallFunc(bar1)...
  mocks.ExpectCallFunc(bar2)...
 
  FooBar();
}
```

## NeverCall() Method
Unlike the former two functions, NeverCall() throws an exception if the function you are mocking is ever called.

### NeverCallFunc(funcName)
``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  
  // If bar() is called, throw an exception into the test
  mocks.NeverCallFunc(bar)...
 
  Foo();
}
```
You can further filter the NeverCall() trigger by using the [After() method](#after-method).

# TCall Class
The TCall class cannot be instantiated directly. Nevertheless, it useful in order to create conditionals or sequences between mocks that have been instantiated.

## Return() Method
Return() must be used to terminate a mock that returns a value (any non-void function).

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  mocks.OnCallFunc(returnByValueFunc).Return(55);
 
  foo();
}
```

Now, whenever ReturnByValue() is called, it will return a result of 55. If you are returning a value through a function argument (pass-by-reference, pass-by-pointer), refer to the [Out() Method](#out-method).

## With() Method
With() lets you assert that a function is called with specific arguments. If you do not care about all of the arguments that are being passed in, you may use "_" for each arg you want to not match against. If any of the arguments do not match those in the With() statement, an exception will be thrown.

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  mocks.ExpectCallFunc(multiArgFunc).With(12, "hello", _)...;
 
  foo();
}
```

When MultiArgFunc() is called, it will expect a first numerical argument of 12 and a second string argument of "hello". It does not care about what the third argument is. If you want to do something more complicated than a simple value comparison, refer to the [In() method](#in-method) for general purpose complex operations, the [Deref() method](#deref-method) if you specifically want to get the value behind a pointer, or the [Do() method](#do-method) where none of these options suffice.

## After() Method
After() allows you to mandate a specific sequence that function calls must take. If a mocked function is called before the Call object it is "After'ed", an exception will be thrown. This is useful when you only care about the order of a subset of the functions you are mocking.

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  mocks.autoExpect = false
  ...
  Call& alpha = mocks.ExpectCallFunc(bar1).Return(33);
  mocks.OnCallFunc(bar2).After(alpha);
  ...
  FooBar();
}
```

You can chain After() Methods if you want a call to be after several calls.

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository mocks;
  mocks.autoExpect = false
  ...
  Call& alpha = ...
  Call& beta = ...
  Call& gamma = ...
  mocks.OnCallFunc(bar4).After(alpha).After(beta).After(gamma);
  ...
  foo();
}
```

## Do() Method
Do() lets you specify a custom function to execute upon call of a mock. This can be used to create a custom fake function, but can also be used to track other information, such as the number of times a mock is called.

``` C++
#include "hippomocks.h"
 
void bar_fake(int* value, int param) {
  if(param < 0) {
    *value = 13;
  } else {
    *value = 22;
  }
}
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(bar).Do(bar_fake);
 
  foo();
}
```

``` C++
#include "hippomocks.h"
 
int counter = 0;
void barCounter_fake(int* value, int param) {
    counter++;
}
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(bar).Do(barCounter_fake).Return(12);
  counter = 0; // Reset the global value for the test
 
  foo();
  EXPECT_EQ(1, counter);
}
```

Note that you can also use a lambda to help reduce clutter in your test files.

``` C++
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(bar)
       .Do([](auto* value, auto param) { (param < 0) ? *value = 13 : *value = 22; });
 
  foo();
}
```

You can even inline unit test checkers, though you cannot use any checkers that return a value due to how labmdas function. (For simple dereferencing and matching, use the [Deref() method](#deref-method), as it is cleaner)

``` C++
#include "hippomocks.h"

TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(bar)
       .Do([](auto* value, auto param) {
                EXPECT_NE(value, nullptr)
                EXPECT_EQ(0xFA, *value)
            }
          );
 
  foo();
}
```

## Match() Method
Match() lets you set filters to generate more complex return values without having to resort to overly complex custom functions. Match() takes a Boolean function as an argument. It compares the arguments that are passed in to a mocked function

``` C++
#include "hippomocks.h"
 
bool belowThree(int* value, int param) { return (param < 3); }
bool aboveTwelve(int* value, int param) { return (param > 12); }
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(bar).Return(12);
  mocks.OnCallFunc(bar).Match(belowThree).Return(-45);
  mocks.OnCallFunc(bar).Match(aboveTwelve).Return(77);
 
  foo();
}
```

If the value of our param argument is below 3, we will return -45. If it is above 12, we will return 77. Otherwise, we default and return 12.

## Throw() Method
Throw() lets you throw a custom exception when a function is called. It can be used in conjunction with [Match()](#match-method) to set up complex conditional assertions.

``` C++
#include "hippomocks.h"
 
bool belowThree(int* value, int param) { return (param < 3); }
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(bar).Return(12);
  mocks.OnCallFunc(bar).Match(belowThree).Throw(std::exception());
  
  EXPECT_THROW(foo(), std::exception);
}
```

This can be particularly useful if you are working in C where you do not have exceptions. It is common that, if you fatal in a C program, you exit. However, a unit test framework will typically have to spawn a new thread to run [Death Tests](https://www.youtube.com/watch?v=HNu-Rsno_wU), so you can use the HippoMocks framework to turn it into a much faster Exception Test.

``` C++
#include "hippomocks.h"
  
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(exit).Throw(std::exception);
 
  EXPECT_THROW(foo(), std::exception);
}
```

# Other Methods
## In() Method
The In() Method allows you to capture arguments that are being passed into a mocked function for further testing. This is very useful when you don't care about the exact value of the argument, merely if it adheres to a set of properties (i.e. the value passed is a prime number, the address a pointer is set to somewhere in an address range). 
WARNING: DEREFERENCING A POINTER WILL ONLY WORK IF THE DATA IT POINTS TO EXISTS AFTER THE FUNCTION HAS BEEN CALLED. DATA THAT IS SUBJECT TO CLOBBERING WILL NOT BE GUARANTEED TO EXIST AFTER THE FUNCTION HAS FINISHED EXECUTING. If you need to verify the value behind a pointer, either use the [Deref() method](#deref-method) or use a complex mock with the [Do() method](#do-method).

``` C++
// source.cpp
void bar(int& input); //or
void bar(int input);

// test.cpp
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  int paramVal;
  mocks.OnCallFunc(bar).With(In(paramVal));
 
  foo();
  EXPECT_TRUE(paramVal % 2 == 0); // Check if the parameter being passed in is even
}
```

``` C++
// source.cpp
void bar(int* input);

// test.cpp
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  int* paramValPtr = NULL;
   
  mocks.OnCallFunc(bar).With(In(paramValPtr));
 
  Foo();
  EXPECT_TRUE(*paramValPtr % 2 == 0); // Check if the parameter is even
                                      // Be aware of the risk of clobbering
}
```
Note: for the purposes of [With() method](#with-method) matching, In() is treated like "_"


## Out() Method
The Out() Method allows you to specify a return value for an argument that is passed in conjunction with the [With() method](#with-method).

``` C++
// source.cpp
void returnByPointerFunc(int* retVal);

// test.cpp
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(returnByPointerFunc).With(Out(72));
 
  foo();
}
```

When returnByPointer() is called, the mock will intercept the call and set the argument to 72. The source code will continue to execute as normal afterwards.

Note: for the purposes of [With() method](#with-method) matching, Out() is treated like "_"

## Deref() Method
The Deref() Method allows you to check the value stored at the address a pointer is pointing to.

``` C++
// source.cpp
void bar(int* input);

// test.cpp
#include "hippomocks.h"
 
TEST(MyTestSuite, MyTestName) {
  MockRepository Mocks;
  mocks.OnCallFunc(bar).With(Deref(0xFA));
 
  foo();
}
```
When bar() is called, the first argument (which is a pointer) will be dereferenced after an NPE check and compared to the value of 0xFA. If the dereferenced pointer matches, the test will pass, otherwise the mocking framework with throw an exception.

Note: this only works with pass-by-pointer parameters.
Note: for the purposes of [With() method](#with-method) matching, Deref() is treated like "_"

# Gotchas
* If you mock an empty function, the opcode patching that Hippomocks performs will clobber code that comes lower in the compiled object file. You may see issues like Segmentation Faults, trace/breakpoint trap, floating point exception, illegal instruction error, etc.
``` C++
void foo() {
  return;
}

// If function foo is mocked, function bar is clobbered
int bar() {
  //...
}
```

The approach to work around this is to pad out the empty function with nop commands such that the padding is greater than the jmp opcode that is patched. The amount of nop padding required to prevent failures is compiler and architecture specific.
``` C++
void foo() {
  asm(
    "nop\n"
    "nop\n"
    "nop\n"
    ...
  )
  return;
}

// If function foo is mocked, function bar is no longer clobbered
int bar() {
  //...
}
```

# Further Examples
For more examples of how to use all of these methods, refer to the [tests folder](HippoMocksTest).

For testing Variadic Functions, please see [this Stack Overflow comment](https://stackoverflow.com/questions/36749410/hippomocks-mocking-function-with-variable-count-of-args/38769627#38769627).

# Special Thanks
Thanks to [Steven Taylor](https://github.com/SttIntc), [Jacob Kirby](https://github.com/jacobkir), and [Louis Duvoisin](https://github.com/lduv1) for battle testing this wiki. 
