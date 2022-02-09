# Unify Developer Guide
This is a guide for developers who wish to get started on adding or modifying the
Unify SDK source code. 

For instruction on how to build please see
- [Build Instructions](readme_building.md)

Other Guides related to development is
- [Library Overview](overview.md)
- [Debugging Guide](readme_debug.md)
- [Rust Developer Guide](readme_rust.md)

API reference is found here:
- [Common Components API](../../doxygen_uic/html/index.html)
- [ZPC API](../../doxygen_zpc/html/index.html)
- [ZigPC API](../../doxygen_zigpc/html/index.html)
- [UCL MQTT API](../../reference_ucl_mqtt/html/index.html)

## Build system and components

### CMake

We created additional functions on top of the usual cmake functions which makes
it more easier to create new targets. This section explains how to use cmake to
create new uic-components, mocks and unit-tests. These functions should cover 
all possible use-cases. Their semantics is similar to the existing cmake
framework, making it easy for developers who are familiar with cmake to
understand what they do. A good tip is to also take a look at the current cmake
files to find similar constructions which you can use as an example when
defining your own.

#### Defining new uic-components

Uic-components are declared SHARED using cmake's `add_library`.

Newly created components need to comply with the following requirements:

1. Need to be declared as SHARED
2. Need to be prefixed with uic_

An example:

~~~ cmake
add_library(uic_my_component SHARED my_source.c)
~~~

There is scripting in place that will verify if these conditions are met. In
the unlikely event you want to suppress these errors,
`disable_target_check(<my_exception_target>)` can be used.

#### Mock uic-components

The cmock framework is used to generate mocks of uic components. More
specifically, cmock generates a mock implementation for provided header files.
This means no source files are needed to create mocks!
To ease the use of mocking in the uic ecosystem we have a cmake function to
create mock libraries:

~~~ cmake
target_add_mock(<target> <additional_headers>)
~~~

Target_add_mock creates a new cmake shared library named `<target>_mock` which
can be included into unit-test targets. It will *only* mock headers that are
declared *public* for that given target.
Given the following example:

~~~ cmake
target_include_directories(mytarget PUBLIC include PRIVATE my/private/include/dir)
target_add_mock(mytarget)
~~~

will create a library named `mytarget_mock` containing mock implementations of
headers that reside in the folder `${CMAKE_CURRENT_LIST_DIR}/include`. Headers
residing in folder `my/private/include/dir` are not mocked and therefore not
in the mock library.

##### Additional headers

For further customization `target_add_mock` has the possibility to add
additional headers. Its intended to mock headers that are needed by the test
but where declared private or part of an external library that needs to be mocked.

#### Unit-testing

`target_add_unittest` is used to create unit-tests for a given target. In it
simplest form all you need is

~~~ cmake
target_add_unittest(uic_my_component SOURCES test.c)
~~~

In test.c you define your unity test cases. No need to add additional include
and link_libraries, they are all deducted from the `uic_my_component`.
What happens internally is that the function relinks the target it creates a
test for, in this case `uic_my_component`. This enable us to link mocks in the
production code. the function has a `DEPENDS` argument that is purposed for that.

##### DEPENDS

the DEPENDS argument enables you to switch a dependant library with a mock
version. One that is generated with target_add_mock for instance. the `DEPENDS`
argument behaves similar to cmake's `target_link_libraries`. Given the
following situation:

~~~ cmake
add_library(uic_my_component SHARED mycomponent.c)
add_library(uic_my_dependant SHARED mysource.c)

# uic_my_component depends on uic_my_dependant
target_link_libraries(uic_my_component uic_my_dependant)

# create an mocking library of component uic_my_dependant
target_add_mock(uic_my_dependant)
~~~

We have a uic component uic_my_component that has a dependency on
uic_my_dependant. We want to make an unit-test for `uic_my_component` but
instead of using `uic_my_dependant` we want to use the `uic_my_dependant_mock`
mock version in our test. we can achieve this by adding the mock as a
dependency of the the unittest. Since all targets defined in `DEPENDS` have
priority over the actual dependencies of the `uic_my_component` target, they
get selected first. Thus, the mock version is used instead of the actual
dependency:

~~~ cmake
target_add_unittest(uic_my_component 
SOURCES source.c
DEPENDS uic_my_dependant_mock
)
~~~

**Note** `DEPENDS` accepts a list of items space separated.

##### NAME

With the `NAME` argument we can give the test a custom name. This way we can
build multiple tests of the same component. e.g.

~~~ cmake
target_add_unittest(uic_my_component 
NAME uic_custom_test1 
SOURCES source.c
)

target_add_unittest(uic_my_component 
NAME uic_custom_test2
SOURCES source.c
)
~~~

#### EXCLUDE

Mocking a header which is part of the same target will not build, as you will
get duplicate symbol errors. There is a symbol present in the actual production
code, and one in the mock. With `EXCLUDE` you can easily leave out actual
production sources. thereby making your test compile:

~~~ cmake
add_library(uic_my_component a.c b.c c.c)
target_add_unittest(uic_my_component
SOURCES test.c
DEPENDS a_mock
EXCLUDE a.c)
~~~

`EXCLUDE` expects an string that it can match against source files. Typically
you want to list the sources files you want to exclude. e.g. `a.c` or if its
nested `src/a.c` for example. In theory you could exclude a selection of files
by specifying, for example, 'network'. this will exclude all source files which
contains 'network' from the build. 

#### Stub

There are some exceptions where a simple mock not suffices. These components
typically expose hooks of existing component. These components are post-fixed
with _stub. they can be added in the DEPENDS argument.
at the moment of writing we have 2:

* uic_contiki_stub, functions to forward time or dequeue the events on the queue.
* uic_mqtt_stub, hooks to show mqtt state.

#### Do's and dont's

* The `NAME` argument is optional and doesn't have to be explicitly specified.
* Dont add production sources in the `SOURCES` section.
(e.g. ../src/my_internal_src.c). These objects are already loaded and will
probably cause linker errors.
* Do not add the target under test in the `DEPENDS` section, its already
implied because of the first argument. you can leave it out. e.g. dont do this:
``` target_add_unittest(mytarget DEPENDS mytarget) ```
* The `DEPENDS` section typically contains only dependencies to _mock targets.

#### CMake File Formatting

The ZPC repository includes config-files for formatting of CMake files using
the Python module cmake-format. cmake-format can be install using the pip
installation utility.

~~~ bash
python -m pip install cmake_format
~~~

To auto-format a CMake file run the command:

~~~ bash
python -m cmake_format -i ./<some dir>/CMakeLists.txt
~~~

#### Example

Define all test cases in a file, where each test case is a function
with the prefix test_,

~~~ c
#include <unity.h>
#include "foo.h"
void foo_case1_test() {
  TEST_ASSERT_TRUE( foo_true() )
}
~~~

#### References

* Unity <http://www.throwtheswitch.org/unity>
* [CMock_Summery](https://github.com/ThrowTheSwitch/CMock/blob/v2.5.2/docs/CMock_Summary.md)
* <http://www.throwtheswitch.org/cmock>

#### Code Coverage

When making unit tests, code coverage is helpful for determining which parts of the code are tested.
When building the Unify project, enable code coverage by adding setting the CMAKE flags: `CMAKE_GCOV=True` and `CMAKE_BUILD_TYPE=Debug` by running `cmake <other CMAKE flags> -DCMAKE_BUILD_TYPE=Debug -DCMAKE_GCOV=True ..` when building.
After building and running the unit tests, the tool `gcovr` can be used to generate coverage reports.

Within the build folder run:

~~~ bash
mkdir -p gcovr && gcovr -r . -e '.+mock.+' -e '.+test.+' -e '.+unity.+' --html-details --html --output gcovr/gcovr.html
~~~

Then, open `<build_folder>/gcovr/gcovr.html` in your favorite browser.

The code coverage report is also available on Jenkins by clicking the `Coverage Report` link.

##### Dependencies

Install the `gcovr` package using `apt-get` on Ubuntu/Debian or `brew` on Mac. `gcovr` is already installed in the Unify docker image.

## Debian Package for a UIC Component

The Unify SDK includes some macros for making a debian packages
for a component.

The following steps are needed to make a package:

1. Implement add_component_to_uic() function in cmake/include/package.cmake for your component
with the following arguments:
    - PKG_NAME      component package Name
    - PKG_FILE_NAME Debian package filename for the component
    - PKG_DEPNDS    Component debian package dependencies.
    - PKG_EXTRA     Component debian package Control extra info
  
    See the [CPack manual](https://cmake.org/cmake/help/v3.6/module/CPackDeb.html) for more information on these fields.

1. In the component-specific CMakeFile.txt (for application/zpc/CMakeFile.txt)
implement a call to cpack_add_component() for filling up the CPack component
information.

1. Pass "COMPONENT \<component name\> as an argument to all install() calls for the
component (see the application/zpc/CMakeFile.txt for example).

1. Add your component name to the following line, as shown in
cmake/include/package.cmake.
    ~~~cmake
    set(CPACK_COMPONENTS_ALL uic-zpc libuic uic-all \<your component\>)
    ~~~

1. Add your component name to uic-all component (append name to this list as
shown in cmake/include/package.cmake).
    ~~~cmake
    add_component_to_uic(
      uic-all
      "${CMAKE_PROJECT_NAME}-all"
      "uic-zpc,libuic, <your component>"
      ""
      )
   ~~~

## How To Make A New IoT Service {#How_to_make_a_new_IoT_Service}

To build an IoT Service from scratch, it is recommended to follow the steps
below. IoT Services come in many forms, and exact steps will depend on the
detailed requirements of the IoT Service.

1. Find API and SDK bindings for target service. An IoT service connects the
   Unify SDK with an external service. Service should be interpreted in the
   broadest sense; it can be a cloud service, or it can be a UI operated by end
   users. Or something in between. The first step in writing an IoT Service is
   determining which APIs and SDKs to use for the target service. If the IoT
   Service connects to a cloud service, then the cloud service SDK is a good
   place to start. Alternatively, choosing a framework with good bindings to the
   cloud service. If the IoT Service is a UI, then a UI framework should be
   chosen based on the product requirements.
2. Choose an MQTT client. Now choose an MQTT client based on the programming
   language and frameworks selected in the previous step. Most MQTT clients
   should be compatible with the Unify SDK. If encryption is desired, make sure
   to choose a client that supports certificate-based TLS.
3. Discover nodes using MQTT topics. Discovering nodes and resources that can be
   displayed or manipulated is fundamental to any IoT Service. This is achieved
   by subscribing to particular MQTT topics. See the Service Discovery chapter
   of the [Unify Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).
4. Implement Network Management. If the IoT Service needs to do network
   management, consider which operations are needed and implement them as
   described in the Network Management chapter of the
   [Unify Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).
   Here is [an example of adding a node to a network](#Including_a_Device).
5. Interact with nodes using MQTT topics. Interacting with the nodes by either
   reading out status or controlling their state is fundamental to most IoT
   services. Subscribe and publish to the relevant MQTT topics as described in
   the Application Level Control chapter of the
   [Unify Specification](https://siliconlabs.github.io/UnifySDK/unify-spec/html/index.html).
6. Consider re-using appropriate shared components from the Unify SDK codebase.
   A valuable feature of Unify is the common components shared by many modules
   of the SDK. Code reuse saves both development time and offers IoT Service
   users a uniform experience operating and configuring different IoT services.

The Unify SDK provides a number of components that are candidates for re-use in
IoT Services. They are listed below.

1. Unify MQTT Client. The MQTT client is built on top of Eclipse Mosquitto MQTT
   and handles MQTT connection, subscribing and publishing. It implements
   re-connections and caching of published messages, which have been published
   while the client has been disconnected from the broker. See the section Unify
   Library Framework/MQTT Client in Unify Documentation for details.
2. Logging System. The logging system features logging with severity level
   tagging and log filtering. See the section Unify Library Framework/Logging
   System in the Unify Documentation for details.
3. Config System. The config system implements parsing of config files and
   command line arguments. See the Unify Library Framework/Config System in
   Unify Documentation for details.

The UPVL and GMS applications use the shared components and are good examples to
peruse. They can be found in the Unify source code in the `applications/gms/`
and `applications/upvl/` directories.

## Directory Structure and File Naming Conventions

### Rules

* All filenames must be lower case except the following:
  * CMakeLists.txt
  * Dockerfile
  * Makefile
  * Jenkinsfile
  * Files provided by external libraries
* Use _ for spaces in file/folder names

### Structure of Components (Best Practice)

This is a best practice for component folder structure.
Note that exceptions exist for this, such as Contiki and libs2.

~~~
.
|-- uic_my_sample_component/
|   |-- platform/           // Platform specific
|   |   `-- posix/          // Posix specific
|   |-- include/            // Public interface of components
|   |-- resource/           // Various resources, e.g. xml files for code generation etc.
|   |-- src/                // Source code and private header files
|   |-- test/               // Unittest for component
|   |-- tools/              // Tools e.g. for building, python scripts etc.
|   `-- CMakeLists.txt      // CMakeLists.txt for the component
~~~

* Library names and folders of Unify components are required to be prefixed with uic_, 
i.e. "uic_my_sample_component".

### Structure

~~~
.
|-- applications/               // Unify application specific code goes here
|   |-- zpc/                    // ZPC application code
|   |   |-- components/         // ZPC components
|   |   `-- ...
|   |-- zigpc/                  // ZIGPC application code
|   |   |-- components/         // ZIGPC components
|   |   `-- ...
|   |-- gms/                    // GMS application code
|   |-- upvl/                   // UPVL application code
|   `-- ...
|-- cmake/                      // Cmake toolchains, modules and includes
|   |-- include/
|   |   `--                     // Various cmake includes e.g. compiler_options.cmake, doxygen.cmake, ...
|   |-- modules/                // cmake modules
|   |-- armhf_debian.cmake      // Toolchain files goes here in the format `<arch>(_<dist>).cmake`
|-- components/                 // Shared components (components shared between Unify applications)
|   |-- uic_config/             // config component
|   |   |-- platform/
|   |   |   |-- posix/          // Posix platform specific must be in platform/posix folder for all components (except external components, where this is handled differently)
|   |   |-- include/            // non platform specific public interfaces goes here
|   |   |-- src/                // non platform specific source code and private interfaces goes here
|   |   |-- test/               // Unittests for the component goes in here
|   |   `-- CMakeLists.txt
|   |-- uic_log/                // Logging framework
|   |-- uic_mqtt_client/        // MQTT client library
|   |-- ...
|   `-- CMakeLists.txt
|-- dist/                       // Files for the "target" platform goes here e.g. init scripts, config files, etc.
|   `-- CMakeLists.txt
|-- doc/                        // Documentation written in markdown format
|   `-- readme_developer.md     // This document
|-- docker/Dockerfile           // Dockerfile for compiling and cross-compiling ZPC
|-- CMakeLists.txt              // Root CMakeLists.txt
`-- Jenkinsfile                 // Jenkinsfile for CI on Jenkins
~~~

## Component Templates

A [cookiecutter](https://cookiecutter.readthedocs.io/) template helps
to quickly add the boilerplate code for ZPC-components.

Cookiecutter is written in Python and the simplest way to install it is from PyPI using
[pipx](https://pipxproject.github.io/pipx/).

~~~
pip install --user pipx
~~~

pipx is a utility for installing executable packages (i.e., packages with an entry-point)
from PyPI and should be kept in their own virtualenv.
When configured correctly, they are placed in your PATH.
The pipx installation will tell you what PATH-changes you need
to perform.

After installing and configuring pipx, you can install cookiecutter:

~~~ bash
pipx install cookiecutter
~~~

Then, from your `components` directory, execute cookiecutter and point it to
the template repository (assuming you have your SSH-keys all set up, but you can also
use HTTPS):

~~~ bash
cookiecutter ssh://git@stash.silabs.com/uic/zpc_cookiecutter.git
~~~

You will be asked some questions regarding your component, with default values in
square-brackets.

After this is done, manually add your new component subdirectory to the
`CMakeLists.txt` in the `components` directory.

### Demo

Here you can see a demo-session of adding a component with cookiecutter.

*Note! This Git-repository used in the demo is outdated and did only exist
to demonstrate this functionality for the team to consider.*


<a href=https://asciinema.org/a/WwG0ZZEHk5f2EzWVjlJeGENK1> 
  Heres a small video demonstrating how to use the cookiecutter<br/>
  <img src="https://asciinema.org/a/WwG0ZZEHk5f2EzWVjlJeGENK1.png" width="60%">
</a>

## Clang-Format (autoformat code)

Clang-format is a tool that can format code according to coding style configuration.
The `.clang-format` file in this repository depends on clang-format 10.

### Install on Mac

~~~ bash
brew install llvm
ln -s "$(brew --prefix llvm)/bin/clang-format" "/usr/local/bin/clang-format"
~~~

### Install on Ubuntu

~~~ bash
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main" > /etc/apt/sources.list.d/llvm.list
sudo apt-get update
sudo apt-get install clang-format-10
~~~

### Visual Studio Code

In Visual Studio Code, you can install clang-format plugin (xaver.clang-format),
which allows you to format files with clang-format (also supports format on save).
See plugin documentation for details.

### Command Line

You can run clang-format from the command line.

~~~ bash
clang-format -i <filename>  # -i causes to do in-file changes
~~~

## Pre-commit Hooks

The ZPC repository has a config-file for the pre-commit.org git hook
system. With the pre-commit hooks, utilities such as clang-format and
cmake-format will be run a all files before committing them to the repository.

The pre-commit hooks must be activated locally on your machine to
work.

First, the pre-commit Python module has to be installed:

~~~ bash
python -m pip install pre-commit
~~~

Second, install the git hook:

~~~ bash
python -m pre-commit install
~~~

From now on, all new files will be formatted before they are committed.


## Creating a New Application

1. Create a new folder in `applications` folder e.g., `applications/my_uic_application`.
2. Create a main function within the `applications/my_uic_application` folder.

    The following is an example of a the bare minimum main.c file to get started.
    A more comprehensive demo is located in `applications/demo_uic_lib`,
    where some of the shared Unify libraries are demonstrated as well, including MQTT client, logging, and the configuration system.

        #include "uic_main.h"
        #include "process.h"

        // Demo process definition
        PROCESS(my_uic_application_process, "My Unify Application Process");

        // Demo process implementation
        PROCESS_THREAD(my_uic_application_process, ev, data)
        {
          PROCESS_BEGIN()
          while (1) {
            switch (ev) {
              case PROCESS_EVENT_INIT:
                // Add Init code here
                break;
              case PROCESS_EVENT_EXIT:
                // Add Exit code here
                break;
            }
            PROCESS_WAIT_EVENT();
          }
          PROCESS_END()
        }

        // Setup fixture, called on start-up
        sl_status_t my_uic_application_setup(void)
        {
          // Start the demo process
          process_start(&my_uic_application_process, NULL);
          return SL_STATUS_OK;
        }

        int main(int argc, char **argv)
        {
          // Setup fixtures
          uic_fixt_setup_step_t uic_fixt_setup_steps_list[]
            = {{my_uic_application_setup, "My Unify Application"}, {NULL, "Terminator"}};

          // Shutdown fixtures
          uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
            = {{NULL, "Terminator"}};

          // Run uic_main and return result
          return uic_main(uic_fixt_setup_steps_list,
                          uic_fixt_shutdown_steps_list,
                          argc,
                          argv,
                          "1.0" /* My application version */);
        }

3. Hook `my_uic_application` into the Build System.

    After adding the main function to a `main.c` file under `applications/my_uic_application`, modify the cmake 
    configuration file to include `my_uic_application` in the build by adding `add_subdirectory(my_uic_application)` to `applications/CMakeLists.txt`

        add_executable(my_uic_application main.c)
        target_link_libraries(my_uic_application uic)
        # Include my_uic_application in binary package
        install(TARGETS my_uic_application RUNTIME DESTINATION bin)

    

4. Compile

    Now, everything should be set up to compile my_uic_application as part of the
    Unify build. To cross compile the application for Raspberry Pi, run
    following commands in the root of source folder:
    > NB: Prerequisite is to have the Unify docker image (see
    > [Docker](../docker/readme_developer.md))

        docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
        mkdir build
        cd build
        cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake ..
        ninja
        ninja deb

## Unify SDK Library (Unify Main) and IPC

The Unify SDK library is implemented as an event-driven system.
It runs in a single thread and uses the proto-threads from the Contiki OS as an event queue system.
The event system is centered around a single select call, which monitors all external file descriptors and is responsible for triggering a timer, which is implemented in the Unify Main component.
This design was chosen to avoid concurrency issues between threads.

The following sequence diagram shows how the Unify Main loop triggers events based on file descriptors in `process1` and timers in `process2`.

![Unify Main Events](assets/img/sequence_uic_ipc.png)

Additional information on this is available in the Unify Library doxygen.
