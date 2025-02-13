# CMake Integration

```{toctree}
---
maxdepth: 2
titlesonly:
---
standards/cmake-standard.md
```

The Unify project uses CMake to generate build files.

We created additional functions on top of the usual CMake functions which makes
it easier to create new targets. This section explains how to use CMake to
create new Unify components, mocks and unit tests. These functions should cover
all possible use-cases. Their semantics is similar to the existing CMake
framework, making it easy for developers who are familiar with CMake to
understand what they do. A good tip is to also take a look at the current CMake
files to find similar constructions which you can use as an example when
defining your own.

## Defining new Unify components

Unify components are declared SHARED using CMake's `add_library`.

Newly created components need to comply with the following requirements:

1. Need to be declared as SHARED
2. Need to be prefixed with unify_

An example:

``` cmake
add_library(unify_my_component SHARED my_source.c)
```

There is scripting in place that will verify if these conditions are met. In
the unlikely event you want to suppress these errors,
`disable_target_check(<my_exception_target>)` can be used.

## Mock Unify components

The [CMock framework](http://www.throwtheswitch.org/cmock) is used to generate
mocks of Unify components. More specifically, CMock generates a mock
implementation for provided header files. This means no source files are needed
to create mocks! To ease the use of mocking in the Unify ecosystem we have a
CMake function to create mock libraries:

``` cmake
target_add_mock(<target> <additional_headers>)
```

Target_add_mock creates a new CMake shared library named `<target>_mock` which
can be included in unit-test targets. It will *only* mock headers that are
declared *public* for that given target.
Given the following example:

``` cmake
target_include_directories(mytarget PUBLIC include PRIVATE my/private/include/dir)
target_add_mock(mytarget)
```

will create a library named `mytarget_mock` containing mock implementations of
headers that reside in the folder `${CMAKE_CURRENT_LIST_DIR}/include`. Headers
residing in folder `my/private/include/dir` are not mocked and therefore not
in the mock library.

### Additional headers

For further customization, `target_add_mock` has the possibility to add
additional headers. It's intended to mock headers that are needed by the
test but were declared private or part of an external library that needs to be mocked.

## Unit Testing

`target_add_unittest` is used to create unit tests for a given target. In the
simplest form all you need is

``` cmake
target_add_unittest(unify_my_component SOURCES test.c)
```

In test.c you define your unity test cases. No need to add additional include
and link_libraries, they are all deducted from the `unify_my_component`.
What happens internally is that the function relinks the target it creates a
test for, in this case, `unify_my_component`. This enables us to link mocks in the
production code. the function has a `DEPENDS` argument that is purposed for that.

### DEPENDS

the DEPENDS argument enables you to switch a dependent library with a mock
version. One that is generated with target_add_mock for instance. the `DEPENDS`
argument behaves similar to CMake's `target_link_libraries`. Given the
following situation:

``` cmake
add_library(unify_my_component SHARED mycomponent.c)
add_library(unify_my_dependant SHARED mysource.c)

# unify_my_component depends on unify_my_dependant
target_link_libraries(unify_my_component unify_my_dependant)

# create an mocking library of component unify_my_dependant
target_add_mock(unify_my_dependant)
```

We have a Unify component unify_my_component that has a dependency on
unify_my_dependant. We want to make a unit test for `unify_my_component`` but
instead of using`unify_my_dependant` we want to use the `unify_my_dependant_mock`
mock version in our test. we can achieve this by adding the mock as a
dependency of the unit test. Since all targets defined in `DEPENDS`have
priority over the actual dependencies of the`unify_my_component` target, they
get selected first. Thus, the mock version is used instead of the actual
dependency:

``` cmake
target_add_unittest(unify_my_component
SOURCES source.c
DEPENDS unify_my_dependant_mock
)
```

**Note** `DEPENDS` accepts a list of items space-separated.

### NAME

With the `NAME` argument, we can give the test a custom name. This way we can
build multiple tests of the same component. e.g.

``` cmake
target_add_unittest(unify_my_component
NAME unify_custom_test1
SOURCES source.c
)

target_add_unittest(unify_my_component
NAME unify_custom_test2
SOURCES source.c
)
```

### EXCLUDE

Mocking a header that is part of the same target will not build, as you will
get duplicate symbol errors. There is a symbol present in the actual production
code, and one in the mock. With `EXCLUDE` you can easily leave out actual
production sources. thereby making your test compile:

``` cmake
add_library(unify_my_component a.c b.c c.c)
target_add_unittest(unify_my_component
SOURCES test.c
DEPENDS a_mock
EXCLUDE a.c)
```

`EXCLUDE` expects a string that it can match against source files. Typically
you want to list the source files you want to exclude. e.g. `a.c` or if its
nested `src/a.c` for example. In theory, you could exclude a selection of files
by specifying, for example, 'network'. this will exclude all source files which contain 'network' from the build.

### Stub

There are some exceptions where a simple mock does not suffice. These components
typically expose hooks of an existing component. These components are post-fixed
with _stub. they can be added to the DEPENDS argument.
at the moment of writing we have 2:

- uic_contiki_stub, functions to forward time or dequeue the events on the queue.
- uic_mqtt_stub, hooks to show MQTT state.

### Do's and dont's

- The `NAME` argument is optional and doesn't have to be explicitly specified.
- Don't add production sources in the `SOURCES` section.
(e.g. ../src/my_internal_src.c). These objects are already loaded and will
probably cause linker errors.
- Do not add the target under test in the `DEPENDS` section, it's already
implied because of the first argument. you can leave it out. e.g. don't do this:
``` target_add_unittest(mytarget DEPENDS mytarget) ```
- The `DEPENDS` section typically contains only dependencies to _mock targets.

### Unit Test Example

Define all test cases in a file, where each test case is a function
with the prefix test_,

``` c
#include <unity.h>
#include "foo.h"
void foo_case1_test() {
  TEST_ASSERT_TRUE( foo_true() )
}
```

### References

- <http://www.throwtheswitch.org/unity>
- <http://www.throwtheswitch.org/cmock>

## Code Coverage

When making unit tests, code coverage is helpful for determining which parts of the code are tested.
When building the Unify project, enable code coverage by adding setting the CMAKE flags: `CMAKE_GCOV=True` and `CMAKE_BUILD_TYPE=Debug` by running `cmake <other CMAKE flags> -DCMAKE_BUILD_TYPE=Debug -DCMAKE_GCOV=True ..` when building.
After building and running the unit tests, the tool `gcovr` can be used to generate coverage reports.

Within the build folder run:

``` bash
mkdir -p gcovr && gcovr -r . -e '.+mock.+' -e '.+test.+' -e '.+unity.+' --html-details --html --output gcovr/gcovr.html
```

Then, open `<build_folder>/gcovr/gcovr.html` in your favorite browser.

The code coverage report is also available on Jenkins by clicking the `Coverage Report` link.

### Dependencies

Install the `gcovr` package using `apt-get` on Ubuntu/Debian or `brew` on Mac. `gcovr` is already installed in the Unify docker image.

## Debian Package for a Unify Component

The Unify SDK includes a macro for making a Debian package
for a component.

To make a new package, just make CMake component and decorate the
install artifacts of the package with the component name:

Ie:

```cmake
install(TARGETS eed RUNTIME DESTINATION "bin" COMPONENT uic-eed)
```

Make *debconf* scripts for the package ie `config`, `templates`, `postinst` etc if needed. See `man debconf` for details.

Finally, define a Debian package for the CMake component

```cmake
add_component_to_uic(
  uic-eed # Package Name
  "Universal IOT controller - Emulated End Device"
  "${CMAKE_PROJECT_NAME}-eed" # Package Debian filename
  "libfoo" # Package Depends on
  "" # Package replaces
  "debconf/config;\
   debconf/templates;\
   debconf/conffiles;\
   debconf/postinst;\
   debconf/prerm")
```

## CMake File Formatting

The Unify repository includes config-files for the formatting of CMake files using
the Python module cmake-format. cmake-format can be installed using the pip
installation utility.

``` bash
python -m pip install cmake_format
```

To auto-format a CMake file run the command:

``` bash
python -m cmake_format -i ./<some dir>/CMakeLists.txt
```
