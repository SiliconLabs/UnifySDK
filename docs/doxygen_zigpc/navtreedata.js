/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Zigbee Protocol Controller", "index.html", [
    [ "Introduction", "index.html#intro", null ],
    [ "References", "index.html#References", null ],
    [ "Components", "index.html#ZigPC", null ],
    [ "ZigPC Coding Standard", "md_applications_zigpc_coding_standard_8zigpc.html", [
      [ "Purpose of this document", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md1", null ],
      [ "Rules", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md2", [
        [ "1. Folders", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md3", null ],
        [ "2. Files", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md4", null ],
        [ "3. C++ Classes", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md5", null ],
        [ "4. Functions", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md6", null ],
        [ "5. Variables", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md7", [
          [ "5.1 Global Variables", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md8", null ],
          [ "5.2 Function Parameters and Local Variables", "md_applications_zigpc_coding_standard_8zigpc.html#autotoc_md9", null ]
        ] ]
      ] ]
    ] ],
    [ "ZigPC Developer Guide", "md_applications_zigpc_readme_developer.html", [
      [ "Build Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md13", [
        [ "Set up Gecko SDK", "md_applications_zigpc_readme_developer.html#autotoc_md14", null ]
      ] ],
      [ "Modifying Underlying SLC-based Zigbee Host Library", "md_applications_zigpc_readme_developer.html#autotoc_md15", [
        [ "Cross Compiling for Raspberry Pi Using Docker", "md_applications_zigpc_readme_developer.html#autotoc_md16", null ],
        [ "Compiling", "md_applications_zigpc_readme_developer.html#autotoc_md17", null ],
        [ "Advanced Feature - Run Raspberry Pi Unit Tests in Docker on the Host", "md_applications_zigpc_readme_developer.html#autotoc_md18", null ]
      ] ],
      [ "Install Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md19", null ],
      [ "Reading Console Logs", "md_applications_zigpc_readme_developer.html#autotoc_md20", [
        [ "Changing Log Levels", "md_applications_zigpc_readme_developer.html#autotoc_md21", null ]
      ] ],
      [ "Using Zigbee Network Analyzer to Monitor PAN", "md_applications_zigpc_readme_developer.html#autotoc_md22", [
        [ "Installing Simplicity Studio v5", "md_applications_zigpc_readme_developer.html#autotoc_md23", null ],
        [ "Detecting Devices in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md24", [
          [ "Connection via Ethernet Interface", "md_applications_zigpc_readme_developer.html#autotoc_md25", null ],
          [ "Connection via Serial USB Interface", "md_applications_zigpc_readme_developer.html#autotoc_md26", null ]
        ] ],
        [ "Capturing Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md27", null ],
        [ "Decrypting Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md28", null ]
      ] ]
    ] ],
    [ "ZigPC User's Guide", "md_applications_zigpc_readme_user.html", [
      [ "Overview", "md_applications_zigpc_readme_user.html#autotoc_md30", null ],
      [ "Hardware Prerequisites", "md_applications_zigpc_readme_user.html#autotoc_md31", [
        [ "Gateway Host Device: Raspberry Pi", "md_applications_zigpc_readme_user.html#autotoc_md32", null ],
        [ "Gateway NCP Device", "md_applications_zigpc_readme_user.html#autotoc_md33", null ],
        [ "PAN Zigbee Device", "md_applications_zigpc_readme_user.html#autotoc_md34", null ]
      ] ],
      [ "Installing ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md35", [
        [ "Description of Installed Components", "md_applications_zigpc_readme_user.html#autotoc_md36", [
          [ "Mosquitto MQTT System Service", "md_applications_zigpc_readme_user.html#autotoc_md37", null ],
          [ "Mosquitto MQTT Clients: mosquitto_pub, mosquitto_sub", "md_applications_zigpc_readme_user.html#autotoc_md38", null ],
          [ "libuic.so", "md_applications_zigpc_readme_user.html#autotoc_md39", null ],
          [ "uic-upvl", "md_applications_zigpc_readme_user.html#autotoc_md40", null ],
          [ "uic-gms", "md_applications_zigpc_readme_user.html#autotoc_md41", null ],
          [ "uic-image-provider", "md_applications_zigpc_readme_user.html#autotoc_md42", null ],
          [ "zigpc", "md_applications_zigpc_readme_user.html#autotoc_md43", null ]
        ] ]
      ] ],
      [ "Starting ZigPC on the Command Line", "md_applications_zigpc_readme_user.html#autotoc_md44", null ],
      [ "Using ZigPC in a multiprotocol configuration", "md_applications_zigpc_readme_user.html#autotoc_md45", [
        [ "NOTE", "md_applications_zigpc_readme_user.html#autotoc_md46", null ]
      ] ],
      [ "Understanding ZigPC Functions", "md_applications_zigpc_readme_user.html#autotoc_md47", [
        [ "Device Addressing", "md_applications_zigpc_readme_user.html#autotoc_md48", null ],
        [ "Zigbee Node Commissioning", "md_applications_zigpc_readme_user.html#autotoc_md49", [
          [ "Z3 Install Code-Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md50", null ],
          [ "Well-Known-Key Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md51", null ]
        ] ],
        [ "Device State Updates", "md_applications_zigpc_readme_user.html#autotoc_md52", null ],
        [ "Device Discovery", "md_applications_zigpc_readme_user.html#autotoc_md53", null ],
        [ "Device Command Relaying", "md_applications_zigpc_readme_user.html#autotoc_md54", null ],
        [ "Device GeneratedCommand Updates", "md_applications_zigpc_readme_user.html#autotoc_md55", null ],
        [ "Device Attribute Updates", "md_applications_zigpc_readme_user.html#autotoc_md56", null ],
        [ "Group Message Handling", "md_applications_zigpc_readme_user.html#autotoc_md57", null ],
        [ "Zigbee OTA Firmware Upgrade Support", "md_applications_zigpc_readme_user.html#autotoc_md58", null ],
        [ "Zigbee Sleepy End Device Support", "md_applications_zigpc_readme_user.html#autotoc_md59", null ],
        [ "Protocol Controller Diagnostics", "md_applications_zigpc_readme_user.html#autotoc_md60", null ],
        [ "Updating NCP Firmware", "md_applications_zigpc_readme_user.html#autotoc_md61", null ]
      ] ],
      [ "Getting Started with Including Zigbee 3.0 Device to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md62", [
        [ "Adding Zigbee Device to the ZigPC Network via Install-Code Method", "md_applications_zigpc_readme_user.html#autotoc_md63", null ],
        [ "Adding Zigbee Device to the ZigPC Network using Well-Known Key", "md_applications_zigpc_readme_user.html#autotoc_md64", null ],
        [ "Discovering UCL Cluster Command Support for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md65", null ],
        [ "Discovering UCL Cluster Attribute Updates for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md66", null ],
        [ "Sending a UCL Cluster Command to Z3Device to see Attribute Update", "md_applications_zigpc_readme_user.html#autotoc_md67", null ],
        [ "Removing a Device Already in the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md68", null ],
        [ "Updating Zigbee Device Firmware already on the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md69", [
          [ "Instructions on how to start an OTA update", "md_applications_zigpc_readme_user.html#autotoc_md70", null ],
          [ "Once the Unify Image Provider recognizes changes to images.json, the following actions will occur", "md_applications_zigpc_readme_user.html#autotoc_md71", null ]
        ] ],
        [ "Retrieving Diagnostic information from ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md72", [
          [ "Steps to enable this behaviour", "md_applications_zigpc_readme_user.html#autotoc_md73", null ]
        ] ]
      ] ],
      [ "Technical Specifications", "md_applications_zigpc_readme_user.html#autotoc_md74", [
        [ "Device UNID Format", "md_applications_zigpc_readme_user.html#autotoc_md75", null ],
        [ "Recognized SmartStart DSK Format", "md_applications_zigpc_readme_user.html#autotoc_md76", null ],
        [ "Zigbee Device Support", "md_applications_zigpc_readme_user.html#autotoc_md77", null ]
      ] ],
      [ "Appendix", "md_applications_zigpc_readme_user.html#autotoc_md78", [
        [ "Setting Up the Zigbee NCP Connected to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md79", [
          [ "Using Pre-built NCP FW Images", "md_applications_zigpc_readme_user.html#autotoc_md80", null ],
          [ "Building NCP FW Images Using Studio", "md_applications_zigpc_readme_user.html#autotoc_md81", null ]
        ] ],
        [ "Setting Up the Sample Zigbee Z3Light FW Image", "md_applications_zigpc_readme_user.html#autotoc_md82", null ],
        [ "Flashing Zigbee Devices", "md_applications_zigpc_readme_user.html#autotoc_md83", null ]
      ] ]
    ] ],
    [ "ZigPC Release Notes", "md_applications_zigpc_release_notes.html", [
      [ "[1.2.0] - August 2022", "md_applications_zigpc_release_notes.html#autotoc_md85", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md86", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md87", null ]
      ] ],
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md88", null ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md89", null ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md90", null ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md91", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md92", null ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md93", null ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md94", [
        [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md95", null ]
      ] ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md96", null ]
    ] ],
    [ "Modules", "modules.html", "modules" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classClusterAccessor.html#a75a3708fbe5fa706d3ad8e79a1b07e70",
"classsmartstart_1_1Management.html",
"classzigpc__example__metric.html#a0b505898bda622a5d559ac912930ea2f",
"files.html",
"group__attribute__resolver.html#ga0746c7462cffee017f64cd573fa352fa",
"group__attribute__store__value__helpers.html#ga4f8587c8649ac94509c63e91970bce82",
"group__lc.html#ga2c1bb4fa6d7a6ff951a41c73fc721109",
"group__process.html#ga4159c99c908ca521cf8b9fdda4b7f64c",
"group__sl__log.html#gaf7788b8693561747f76339f6537ff8ab",
"group__unify__config.html#gga3b41bf466c6ddabc6c1ce4aca94ab582a8b107faae792ea087f9b72cce3fddc87",
"group__zigpc__common__zigbee.html#ga180be56b44785909176823925f0f93c2",
"group__zigpc__controller.html#ga1a4557490c324fad08ab85b79cf843e7",
"group__zigpc__gateway__notify.html#gga68b826da48d6db275d1cd028d9ec0cfcac6925f0219a5f0728a53fa28f0995851",
"lc-switch_8h_source.html",
"ota__cache_8hpp.html#a311c1d29a0750b1d1581db2d1d7476e4",
"structgateway__frame__t.html#a061e16682489bfd2deee42a67bc914e9",
"structzigpc__gateway__on__ota__started__t.html#a88bcea815fba09547c2e1ef7d8433da6",
"zigpc__config_8c.html#ac2250237c57b7e771ab762bac13d4153",
"zigpc__net__mgmt__notify__int_8h_source.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';