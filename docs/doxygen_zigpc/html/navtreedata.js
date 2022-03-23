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
      [ "Understanding ZigPC Functions", "md_applications_zigpc_readme_user.html#autotoc_md45", [
        [ "Device Addressing", "md_applications_zigpc_readme_user.html#autotoc_md46", null ],
        [ "Zigbee Node Commissioning", "md_applications_zigpc_readme_user.html#autotoc_md47", [
          [ "Z3 Install Code-Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md48", null ],
          [ "Well-Known-Key Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md49", null ]
        ] ],
        [ "Device State Updates", "md_applications_zigpc_readme_user.html#autotoc_md50", null ],
        [ "Device Discovery", "md_applications_zigpc_readme_user.html#autotoc_md51", null ],
        [ "Device Command Relaying", "md_applications_zigpc_readme_user.html#autotoc_md52", null ],
        [ "Device GeneratedCommand Updates", "md_applications_zigpc_readme_user.html#autotoc_md53", null ],
        [ "Device Attribute Updates", "md_applications_zigpc_readme_user.html#autotoc_md54", null ],
        [ "Group Message Handling", "md_applications_zigpc_readme_user.html#autotoc_md55", null ],
        [ "Zigbee OTA Firmware Upgrade Support", "md_applications_zigpc_readme_user.html#autotoc_md56", null ],
        [ "Zigbee Sleepy End Device Support", "md_applications_zigpc_readme_user.html#autotoc_md57", null ],
        [ "Protocol Controller Diagnostics", "md_applications_zigpc_readme_user.html#autotoc_md58", null ],
        [ "Updating NCP Firmware", "md_applications_zigpc_readme_user.html#autotoc_md59", null ]
      ] ],
      [ "Getting Started with Including Zigbee 3.0 Device to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md60", [
        [ "Adding Zigbee Device to the ZigPC Network via Install-Code Method", "md_applications_zigpc_readme_user.html#autotoc_md61", null ],
        [ "Adding Zigbee Device to the ZigPC Network using Well-Known Key", "md_applications_zigpc_readme_user.html#autotoc_md62", null ],
        [ "Discovering UCL Cluster Command Support for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md63", null ],
        [ "Discovering UCL Cluster Attribute Updates for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md64", null ],
        [ "Sending a UCL Cluster Command to Z3Device to see Attribute Update", "md_applications_zigpc_readme_user.html#autotoc_md65", null ],
        [ "Removing a Device Already in the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md66", null ],
        [ "Updating Zigbee Device Firmware already on the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md67", [
          [ "Instructions on how to start an OTA update:", "md_applications_zigpc_readme_user.html#autotoc_md68", null ],
          [ "Once the Unify Image Provider recognizes changes to images.json, the following actions will occur:", "md_applications_zigpc_readme_user.html#autotoc_md69", null ]
        ] ],
        [ "Retrieving Diagnostic information from ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md70", [
          [ "Steps to enable this behaviour:", "md_applications_zigpc_readme_user.html#autotoc_md71", null ]
        ] ]
      ] ],
      [ "Technical Specifications", "md_applications_zigpc_readme_user.html#autotoc_md72", [
        [ "Device UNID Format", "md_applications_zigpc_readme_user.html#autotoc_md73", null ],
        [ "Recognized SmartStart DSK Format", "md_applications_zigpc_readme_user.html#autotoc_md74", null ],
        [ "Zigbee Device Support", "md_applications_zigpc_readme_user.html#autotoc_md75", null ]
      ] ],
      [ "Appendix", "md_applications_zigpc_readme_user.html#autotoc_md76", [
        [ "Setting Up the Zigbee NCP Connected to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md77", [
          [ "Using Pre-built NCP FW Images", "md_applications_zigpc_readme_user.html#autotoc_md78", null ],
          [ "Building NCP FW Images Using Studio", "md_applications_zigpc_readme_user.html#autotoc_md79", null ]
        ] ],
        [ "Setting Up the Sample Zigbee Z3Light FW Image", "md_applications_zigpc_readme_user.html#autotoc_md80", null ],
        [ "Flashing Zigbee Devices", "md_applications_zigpc_readme_user.html#autotoc_md81", null ]
      ] ]
    ] ],
    [ "ZigPC Release Notes", "md_applications_zigpc_release_notes.html", [
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md83", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md84", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md85", null ]
      ] ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md86", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md87", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md88", null ],
        [ "Removed", "md_applications_zigpc_release_notes.html#autotoc_md89", null ],
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md90", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md91", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md92", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md93", null ],
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md94", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md95", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md96", [
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md97", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md98", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md99", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md100", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md101", null ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md103", null ]
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
"classGroupAccessor.html#a32ba84eaf8fd2182efe4a6a7b49cf87c",
"classzigpc_1_1gateway_1_1BindingRequestRequest.html#a31ed2d90e86a0cc28f67b738b28b6c5a",
"dir_1233302668cfc38a9302dcaeddaa7aed.html",
"globals_a.html",
"group__attribute__store__fixt.html#ga97a838c2d31c83650be7846cec9f85ba",
"group__etimer.html#ga94628979920ab92a3cc0e69e537bef7d",
"group__process.html#ga203287c010773e1223347682148a3e0c",
"group__sl__log.html#ga3f62e87ad64f94f78f10cbbce654d039",
"group__unify__config.html#gab6dc511b499e865e3ba3147d058ae622",
"group__zigpc__common__zigbee.html#ga11ce1a659aa35b00e3b487d752ae2903",
"group__zigpc__config.html#ga1c8e61bf283a2413bc33b15dc057444c",
"group__zigpc__gateway__notify.html#gga68b826da48d6db275d1cd028d9ec0cfca947ad9d6d46363cecebe5c7b6e27d4fd",
"index.html#ZigPC",
"ota__cache_8hpp.html#a178f8f66256ed0b2c9fadffe047e39d0",
"structgateway__frame__t.html#a061e16682489bfd2deee42a67bc914e9",
"structzigpc__gateway__on__read__attribute__response__t.html#a6932828fba973937d8fdb0aef698b123",
"zigpc__controller__int_8hpp.html#ac6998c2c672b1ae3e5911d9a73e13163",
"zigpc__stdin__fixt_8h_source.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';