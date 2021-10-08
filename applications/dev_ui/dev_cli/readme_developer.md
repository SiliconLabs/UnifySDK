# Dev CLI Developer's Guide

This guide describes how to cross compile the Dev CLI for Raspberry Pi and
install the _Developer Command Line Interface_ (Dev CLI) on the Raspberry Pi.

The Dev CLI tool provides a quick and easy way to interact
with the special commands and increase the developing speed of protocol controllers
and other components. In addition, it is also a way to share custom tools.

## Building

The Dev CLI can be cross compiled using the Docker image described in
[readme.md](../../../docker/readme_developer.md). This guide assumes you have
already built the docker image.

From the root of the UIC source directory, run the following:

``` bash
docker run -it --rm  -v`pwd`:`pwd` -w `pwd` uic_armhf
mkdir build
cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake ..
ninja
ninja deb
```

After running this, the dev_cli binary is located in
`./applications/dev_ui/dev_cli/dev_cli`. The Debian installer package is located
in `./uic-dev-cli_<version>_armhf.deb`.

## Adding New Commands to Dev CLI

The folder structure is shown below. The relevant folder when
making a new command is the _commands_ folder inside _components_.

```
dev_ui/applications/dev_cli
├── CMakeLists.txt
├── components
│   ├── CMakeLists.txt
│   ├── dev_cli_config
│   ├── dev_commands
│   │   ├── CMakeLists.txt
│   │   ├── lib  <--- This is command specific libraries
│   │   ├── src  <--- Input a new src file here to make a command
│   │   └── test <--- Testing specific command functionalities
│   └── dev_run_cli
├── doc
├── main.c
├── readme_developer.md
└── readme_user.md
```

The _src_ folder contains the following files (**8-17-2020**):

```

uic/applications/dev_ui/dev_cli/components/dev_commands/src/
├── a_dev_cli_ui_help.cpp               # Display commands and usages
├── add_smartstart_entry.cpp            # Manipulating SmartStart list
├── get_smart_start_list.cpp            # Manipulating SmartStart list
├── remove_smartstart_entry.cpp         # Manipulating SmartStart list
└── template.cpp.EXAMPLE                # Template example

```

Each file represents a command. As a result, making a new command can be done by copy
pasting _template.cpp.EXAMPLE_ and renaming it to a desired new command. The
command name is not generated from file name, but inside the .cpp file.

The template file looks as follows:

```cpp
/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// Standard library
#include <cstring>
#include <unistd.h>

// Import command handler class and project libraries
#include "dev_cli_config.h"
#include "dev_cli_command_runner.h"
#include "dev_cli_command_handler.hpp"

// Process events for a command
#include "dev_cli_run.h"

// UIC component
#include "sys/process.h"
#include "sl_log.h"
#include "uic_mqtt.h"

// Pre deceleration of the main function for command
int template_main();

/*
Initiating the command command is done by doing as follows:
The first argument is the command you need to input as argument for CLI => Template.command
The second argument is a small text describing the functionality and intent of command => Template.arguments_help
The third argument is if you need additional input arguments then you explain these there => Template.arguments_help
The fourth and last argument is your function you want to be main. You pass it to the class and it will setup a pointer to it => Template.command_ptr
*/

CommandTemplate Template("command_name", "command help", "usage: [ip host]", template_main);

int template_main()
{
    // Execute anything you want in this main loop

    // Arguments are saved in config and options can be seen with help or in dev_cli_config
    // It is possible to add arguments in dev_cli_config

    // End the command by broadcasting process command done
    // If you have callback methods the broadcast event should be in the last callback function
    process_post(PROCESS_BROADCAST, PROCESS_COMMAND_DONE, 0);
    return 0;
}
```

Starting with the template, refactoring names to your command name, and inputting the
desired functionalities, is sufficient when creating a new command. To make additional command arguments, such as prompting for a DSK, change the dev_cli_config. This will be generic in the future.

After creating a new command source file, add it to the source list in:

```

uic/applications/dev_ui/dev_cli/CMakeLists.txt

set(COMMAND_CLASSES
  components/dev_commands/src/dev_cli_ui_help.cpp
  components/dev_commands/src/add_smartstart_entry.cpp
  components/dev_commands/src/get_smart_start_list.cpp
  components/dev_commands/src/remove_smartstart_entry.cpp
  => New command class here
)

```
