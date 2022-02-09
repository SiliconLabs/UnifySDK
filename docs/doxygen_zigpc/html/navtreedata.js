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
        [ "Protocol Controller Diagnostics", "md_applications_zigpc_readme_user.html#autotoc_md58", null ]
      ] ],
      [ "Getting Started with Including Zigbee 3.0 Device to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md59", [
        [ "Adding Zigbee Device to the ZigPC Network via Install-Code Method", "md_applications_zigpc_readme_user.html#autotoc_md60", null ],
        [ "Adding Zigbee Device to the ZigPC Network using Well-Known Key", "md_applications_zigpc_readme_user.html#autotoc_md61", null ],
        [ "Discovering UCL Cluster Command Support for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md62", null ],
        [ "Discovering UCL Cluster Attribute Updates for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md63", null ],
        [ "Sending a UCL Cluster Command to Z3Device to see Attribute Update", "md_applications_zigpc_readme_user.html#autotoc_md64", null ],
        [ "Removing a Device Already in the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md65", null ],
        [ "Updating Zigbee Device Firmware already on the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md66", [
          [ "Instructions on how to start an OTA update:", "md_applications_zigpc_readme_user.html#autotoc_md67", null ],
          [ "Once the Unify Image Provider recognizes changes to images.json, the following actions will occur:", "md_applications_zigpc_readme_user.html#autotoc_md68", null ]
        ] ],
        [ "Retrieving Diagnostic information from ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md69", [
          [ "Steps to enable this behaviour:", "md_applications_zigpc_readme_user.html#autotoc_md70", null ]
        ] ]
      ] ],
      [ "Technical Specifications", "md_applications_zigpc_readme_user.html#autotoc_md71", [
        [ "Device UNID Format", "md_applications_zigpc_readme_user.html#autotoc_md72", null ],
        [ "Recognized SmartStart DSK Format", "md_applications_zigpc_readme_user.html#autotoc_md73", null ],
        [ "Zigbee Device Support", "md_applications_zigpc_readme_user.html#autotoc_md74", null ]
      ] ],
      [ "Appendix", "md_applications_zigpc_readme_user.html#autotoc_md75", [
        [ "Setting Up the Zigbee NCP Connected to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md76", [
          [ "Using Pre-built NCP FW Images", "md_applications_zigpc_readme_user.html#autotoc_md77", null ],
          [ "Building NCP FW Images Using Studio", "md_applications_zigpc_readme_user.html#autotoc_md78", null ]
        ] ],
        [ "Setting Up the Sample Zigbee Z3Light FW Image", "md_applications_zigpc_readme_user.html#autotoc_md79", null ],
        [ "Flashing Zigbee Devices", "md_applications_zigpc_readme_user.html#autotoc_md80", null ]
      ] ]
    ] ],
    [ "ZigPC Release Notes", "md_applications_zigpc_release_notes.html", [
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md82", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md83", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md84", null ],
        [ "Removed", "md_applications_zigpc_release_notes.html#autotoc_md85", null ],
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md86", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md87", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md88", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md89", null ],
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md90", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md91", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md92", [
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md93", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md94", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md95", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md96", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md97", null ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md99", null ]
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
"classGroupAccessor.html#a3ee4601c8a04b92d17b72aedbbe2c74c",
"classzigpc_1_1gateway_1_1BindingRequestRequest.html#a34e91c18a3eed679c89087b7ecb56aa9",
"dir_1258395076e871e9609ba7f43babf823.html",
"globals_defs_d.html",
"group__attribute__store__value__helpers.html#ga2179ceede1afad2f87c6dbe35809b6ab",
"group__lc.html#ga3983e0c026396d5c4506779d770007ba",
"group__process.html#ga4159c99c908ca521cf8b9fdda4b7f64c",
"group__sl__log.html#ggaf7788b8693561747f76339f6537ff8aba08914048403ca38994c4ca9e2eb5bfb5",
"group__unify__datastore.html#ga94ec0317094e42566008726743f2ebfe",
"group__zigpc__common__zigbee.html#ga28289aa16ec1ca2fea5ac65394369748",
"group__zigpc__datastore.html#ga35abd88c58baeb185ecd2f073b52138a",
"group__zigpc__group__map.html#ga6374b901901d04d0dffdbbda5e084285",
"md_applications_zigpc_readme_developer.html#autotoc_md27",
"posix_2contiki-conf_8h.html#a306501617396cc95a2d8a85e8a99dfb7",
"structmqtt__client.html#a8a17fa4fbd2cbf13e94eba9c10ecd740",
"structzigpc__zcl__frame__data__t.html",
"zigpc__discovery__events_8hpp.html#a74f62ebef56649faf5f875cce02ed65f"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';