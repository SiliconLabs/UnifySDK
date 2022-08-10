# ZigPC Coding Standard #

## Purpose of this document ##

This document is intended for developers. It describes the
coding standard used in the Zigbee Protocol Controller, which
is an adaptation / subset of the
[Unify Coding Standard](../../doc/standards/coding-standard.md)

This is version **1.0** of the ZigPC coding standard.

## Rules ##
These rules generally apply to the ZigPC. They start at the most
general functional unit of code (a folder / component) and narrow
to the more specific variables. It is recommended to read the
generic coding standard to see where these rules are derived from.

### 1. Folders ###
Folders are used in the ZigPC to denote components and to describe
the project hierarchy.

Folder names SHALL follow the general rule of &lt;component_name&gt; without
any prefix of zigpc or uic etc...
This is to avoid repetition of information - ZigPC is already at the top
level of the folder hierarchy (uic/applications/zigpc/ etc...)

Folder names SHALL be in "snake_case" - only lower case with underscores
used as name seperators

Care should be taken to ensure each module name is less than 10 characters.
Abbreviations should be used where appropriate.

### 2. Files ###
Files are the logical division of components in the ZigPC and represent
a separation of responsibilities.

File names SHALL be in "snake_case" as discussed in [1](1 - Folders).

File names SHALL be of the format "zigpc_xxxx_yyyy" where &lt;xxxx&gt; refers to
a the module and &lt;yyyy&gt; refers to the responsibility covered by that file.
Please balance conciseness with descriptiveness, if possible.

Headers and .c/.cpp files SHOULD match, with the exception where
certain functionality is intended to be private and/or a header
describes an API with multiple different responsibilities.
In these cases, multiple .c/.cpp files will correspond to one header.

### 3. C++ Classes ###
C++ class names SHOULD be written in "CapitalCase" when defined.
C++ methods/fields SHOULD be written in "camelCase".

If a component is written entirely in C++, it SHALL define its
own namespace under a global zigpc namespace.

### 4. Functions ###
Functions in the general ZigPC hierarchy SHOULD be written in "snake_case".
The exception is when interfacing with stack-level functionality in the
zigbee_host library. In this case, "camelCase" is recommended.

The general format of each function name SHOULD take the form of "zigpc_xxxx_yyyy"
where &lt;xxxx&gt; is the name of the module and &lt;yyyy&gt; is the specific name of
the function.
&lt;yyyy&gt; SHOULD be of the form "verb_noun" or of "noun_verb_noun", as specified in
the general coding standard.
It is recommended to keep these function name short and concise to improve
readibility.

### 5. Variables
The rules of variable names depend on their scope. In general,
variables should be kept to as limited a scope as possible.
Global variables SHOULD only be used if absolutely necessary.

Constant variable SHOULD be exposed in the global scope behind
function calls (zigpc_xxxx_get_yyyy() etc...)

#### 5.1 Global Variables
Variables in the global scope SHOULD be capitalized.
'static const xxxx' is the recommended way to define global variables.
The '#define' macro SHOULD only be used if absolutely required or
if defining a variable intended to set the length of an array.

A prefix SHALL be used when defining a global variable in the form of
"ZIGPC_XXXX_YYYY" where <XXXX> is the module name and YYYY describes the specific
use case of that variable

#### 5.2 Function Parameters and Local Variables
Function parameters SHALL be in lower "snake_case". A prefix SHALL NOT be used
when defining a function parameter. The name of each variable should
clearly describe what the variable represents, with abbreviations as necessary.

Function parameters SHOULD match between the function definition and function
prototype (defined in an associated header).

Local variables SHOULD have the same rules applied (and as specified in the
general coding standard). However, conciseness should be favored for
function parameters and descriptiveness should be favored for local variables.
