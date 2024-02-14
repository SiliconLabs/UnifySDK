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
    [ "ZigPC Coding Standard", "md_applications_zigpc_coding_standard_zigpc.html", [
      [ "Purpose of this document", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md1", null ],
      [ "Rules", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md2", [
        [ "1. Folders", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md3", null ],
        [ "2. Files", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md4", null ],
        [ "3. C++ Classes", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md5", null ],
        [ "4. Functions", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md6", null ],
        [ "5. Variables", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md7", [
          [ "5.1 Global Variables", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md8", null ],
          [ "5.2 Function Parameters and Local Variables", "md_applications_zigpc_coding_standard_zigpc.html#autotoc_md9", null ]
        ] ]
      ] ]
    ] ],
    [ "Table Of Contents", "md_applications_zigpc_Getting_Started.html", [
      [ "Preparing Hardware", "md_applications_zigpc_Getting_Started.html#autotoc_md13", null ],
      [ "Preparing the OS and installing required tools", "md_applications_zigpc_Getting_Started.html#autotoc_md14", null ],
      [ "Getting the source code on the devices", "md_applications_zigpc_Getting_Started.html#autotoc_md15", null ],
      [ "Docker Setup", "md_applications_zigpc_Getting_Started.html#autotoc_md16", null ],
      [ "Raspberry Pi Setup", "md_applications_zigpc_Getting_Started.html#autotoc_md17", null ],
      [ "Compiling code for the Raspberry Pi", "md_applications_zigpc_Getting_Started.html#autotoc_md18", null ],
      [ "Getting the NCP WSTK going", "md_applications_zigpc_Getting_Started.html#autotoc_md19", [
        [ "Troubleshooting NCP issues and versioning", "md_applications_zigpc_Getting_Started.html#autotoc_md20", [
          [ "Issue #1: NCP and Gateway mismatch", "md_applications_zigpc_Getting_Started.html#autotoc_md21", null ],
          [ "Issue #2: addressTable mismatch", "md_applications_zigpc_Getting_Started.html#autotoc_md22", null ]
        ] ]
      ] ],
      [ "Debugging with visual studio code, Cleaning the code", "md_applications_zigpc_Getting_Started.html#autotoc_md23", null ]
    ] ],
    [ "ZigPC Developer Guide", "md_applications_zigpc_readme_developer.html", [
      [ "Build Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md25", [
        [ "Set up Gecko SDK", "md_applications_zigpc_readme_developer.html#autotoc_md26", null ]
      ] ],
      [ "Modifying Underlying SLC-based Zigbee Host Library", "md_applications_zigpc_readme_developer.html#autotoc_md27", [
        [ "Cross Compiling for Raspberry Pi Using Docker", "md_applications_zigpc_readme_developer.html#autotoc_md28", null ],
        [ "Compiling", "md_applications_zigpc_readme_developer.html#autotoc_md29", null ],
        [ "Advanced Feature - Run Raspberry Pi Unit Tests in Docker on the Host", "md_applications_zigpc_readme_developer.html#autotoc_md30", null ]
      ] ],
      [ "Install Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md31", null ],
      [ "Reading Console Logs", "md_applications_zigpc_readme_developer.html#autotoc_md32", [
        [ "Changing Log Levels", "md_applications_zigpc_readme_developer.html#autotoc_md33", null ]
      ] ],
      [ "Using Zigbee Network Analyzer to Monitor PAN", "md_applications_zigpc_readme_developer.html#autotoc_md34", [
        [ "Installing Simplicity Studio v5", "md_applications_zigpc_readme_developer.html#autotoc_md35", null ],
        [ "Detecting Devices in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md36", [
          [ "Connection via Ethernet Interface", "md_applications_zigpc_readme_developer.html#autotoc_md37", null ],
          [ "Connection via Serial USB Interface", "md_applications_zigpc_readme_developer.html#autotoc_md38", null ]
        ] ],
        [ "Capturing Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md39", null ],
        [ "Decrypting Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md40", null ],
        [ "Adding custom clusters", "md_applications_zigpc_readme_developer.html#autotoc_md41", null ]
      ] ]
    ] ],
    [ "ZigPC User's Guide", "md_applications_zigpc_readme_user.html", [
      [ "Overview", "md_applications_zigpc_readme_user.html#autotoc_md43", null ],
      [ "Hardware Prerequisites", "md_applications_zigpc_readme_user.html#autotoc_md44", [
        [ "Gateway Host Device: Raspberry Pi", "md_applications_zigpc_readme_user.html#autotoc_md45", null ],
        [ "Gateway NCP Device", "md_applications_zigpc_readme_user.html#autotoc_md46", null ],
        [ "PAN Zigbee Device", "md_applications_zigpc_readme_user.html#autotoc_md47", null ]
      ] ],
      [ "Installing ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md48", [
        [ "Description of Installed Components", "md_applications_zigpc_readme_user.html#autotoc_md49", [
          [ "Mosquitto MQTT System Service", "md_applications_zigpc_readme_user.html#autotoc_md50", null ],
          [ "Mosquitto MQTT Clients: mosquitto_pub, mosquitto_sub", "md_applications_zigpc_readme_user.html#autotoc_md51", null ],
          [ "uic-upvl", "md_applications_zigpc_readme_user.html#autotoc_md52", null ],
          [ "uic-gms", "md_applications_zigpc_readme_user.html#autotoc_md53", null ],
          [ "uic-image-provider", "md_applications_zigpc_readme_user.html#autotoc_md54", null ],
          [ "zigpc", "md_applications_zigpc_readme_user.html#autotoc_md55", null ]
        ] ]
      ] ],
      [ "Starting ZigPC on the Command Line", "md_applications_zigpc_readme_user.html#autotoc_md56", null ],
      [ "Using ZigPC in a multiprotocol configuration", "md_applications_zigpc_readme_user.html#autotoc_md57", [
        [ "NOTE", "md_applications_zigpc_readme_user.html#autotoc_md58", null ]
      ] ],
      [ "Understanding ZigPC Functions", "md_applications_zigpc_readme_user.html#autotoc_md59", [
        [ "Device Addressing", "md_applications_zigpc_readme_user.html#autotoc_md60", null ],
        [ "Zigbee Node Commissioning", "md_applications_zigpc_readme_user.html#autotoc_md61", [
          [ "Z3 Install Code-Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md62", null ],
          [ "Well-Known-Key Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md63", null ]
        ] ],
        [ "Zigbee Network Parameters", "md_applications_zigpc_readme_user.html#autotoc_md64", null ],
        [ "Device State Updates", "md_applications_zigpc_readme_user.html#autotoc_md65", null ],
        [ "Device Discovery", "md_applications_zigpc_readme_user.html#autotoc_md66", null ],
        [ "Device Command Relaying", "md_applications_zigpc_readme_user.html#autotoc_md67", null ],
        [ "Device GeneratedCommand Updates", "md_applications_zigpc_readme_user.html#autotoc_md68", null ],
        [ "Device Attribute Updates", "md_applications_zigpc_readme_user.html#autotoc_md69", null ],
        [ "Group Message Handling", "md_applications_zigpc_readme_user.html#autotoc_md70", null ],
        [ "Zigbee OTA Firmware Upgrade Support", "md_applications_zigpc_readme_user.html#autotoc_md71", null ],
        [ "Zigbee Sleepy End Device Support", "md_applications_zigpc_readme_user.html#autotoc_md72", null ],
        [ "Protocol Controller Diagnostics", "md_applications_zigpc_readme_user.html#autotoc_md73", null ],
        [ "Updating NCP Firmware", "md_applications_zigpc_readme_user.html#autotoc_md74", null ]
      ] ],
      [ "Getting Started with Including Zigbee 3.0 Device to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md75", [
        [ "Adding Zigbee Device to the ZigPC Network via Install-Code Method", "md_applications_zigpc_readme_user.html#autotoc_md76", null ],
        [ "Adding Zigbee Device to the ZigPC Network using Well-Known Key", "md_applications_zigpc_readme_user.html#autotoc_md77", null ],
        [ "Discovering UCL Cluster Command Support for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md78", null ],
        [ "Discovering UCL Cluster Attribute Updates for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md79", null ],
        [ "Sending a UCL Cluster Command to Z3Device to see Attribute Update", "md_applications_zigpc_readme_user.html#autotoc_md80", null ],
        [ "Removing a Device Already in the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md81", null ],
        [ "Updating Zigbee Device Firmware already on the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md82", [
          [ "Instructions on how to start an OTA update", "md_applications_zigpc_readme_user.html#autotoc_md83", null ],
          [ "Once the Unify Image Provider recognizes changes to images.json, the following actions will occur", "md_applications_zigpc_readme_user.html#autotoc_md84", null ]
        ] ],
        [ "Retrieving Diagnostic information from ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md85", [
          [ "Steps to enable this behaviour", "md_applications_zigpc_readme_user.html#autotoc_md86", null ]
        ] ]
      ] ],
      [ "Technical Specifications", "md_applications_zigpc_readme_user.html#autotoc_md87", [
        [ "Device UNID Format", "md_applications_zigpc_readme_user.html#autotoc_md88", null ],
        [ "Recognized SmartStart DSK Format", "md_applications_zigpc_readme_user.html#autotoc_md89", null ],
        [ "Zigbee Device Support", "md_applications_zigpc_readme_user.html#autotoc_md90", null ]
      ] ],
      [ "Appendix", "md_applications_zigpc_readme_user.html#autotoc_md91", [
        [ "Setting Up the Zigbee NCP Connected to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md92", [
          [ "Using Pre-built NCP FW Images", "md_applications_zigpc_readme_user.html#autotoc_md93", null ],
          [ "Building NCP FW Images Using Studio", "md_applications_zigpc_readme_user.html#autotoc_md94", null ]
        ] ],
        [ "Setting Up the Sample Zigbee Z3Light FW Image", "md_applications_zigpc_readme_user.html#autotoc_md95", null ],
        [ "Flashing Zigbee Devices", "md_applications_zigpc_readme_user.html#autotoc_md96", null ]
      ] ]
    ] ],
    [ "ZigPC Release Notes", "md_applications_zigpc_release_notes.html", [
      [ "[1.5.0] - Feb 2024", "md_applications_zigpc_release_notes.html#autotoc_md98", [
        [ "Added (1.5.0)", "md_applications_zigpc_release_notes.html#autotoc_md99", null ],
        [ "Changed (1.5.0)", "md_applications_zigpc_release_notes.html#autotoc_md100", null ]
      ] ],
      [ "[1.4.0] - August 2023", "md_applications_zigpc_release_notes.html#autotoc_md101", [
        [ "Added (1.4.0)", "md_applications_zigpc_release_notes.html#autotoc_md102", null ],
        [ "Changed (1.4.0)", "md_applications_zigpc_release_notes.html#autotoc_md103", null ]
      ] ],
      [ "[1.3.1] - March 2023", "md_applications_zigpc_release_notes.html#autotoc_md104", [
        [ "Changed (1.3.1)", "md_applications_zigpc_release_notes.html#autotoc_md105", null ]
      ] ],
      [ "[1.3.0] - February 2023", "md_applications_zigpc_release_notes.html#autotoc_md106", [
        [ "Added (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md107", null ],
        [ "Changed (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md108", null ]
      ] ],
      [ "[1.2.0] - August 2022", "md_applications_zigpc_release_notes.html#autotoc_md109", [
        [ "Added (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md110", null ],
        [ "Changed (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md111", null ]
      ] ],
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md112", [
        [ "Added (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md113", null ],
        [ "Changed (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md114", null ]
      ] ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md115", [
        [ "Added (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md116", null ],
        [ "Changed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md117", null ],
        [ "Removed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md118", null ],
        [ "Fixed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md119", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md120", [
        [ "Added (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md121", null ],
        [ "Changed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md122", null ],
        [ "Fixed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md123", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md124", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md125", [
        [ "Fixed (1.0.1)", "md_applications_zigpc_release_notes.html#autotoc_md126", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md127", [
        [ "Added (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md128", null ],
        [ "Changed (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md129", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md130", [
        [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md131", null ]
      ] ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md132", null ]
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
        [ "Variables", "globals_vars.html", "globals_vars" ],
        [ "Typedefs", "globals_type.html", "globals_type" ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", "globals_eval" ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"attribute__store__validation_8h.html",
"classast_1_1reducer.html",
"classzigpc_1_1gateway_1_1ZCLFrameUnicastRequest.html#a23e8193be4e560c3aec97b20b8f6da3c",
"dir_7c87d75141af09ca270650942fb5c70f.html",
"gecko_2contiki-conf_8h.html#a306501617396cc95a2d8a85e8a99dfb7",
"group__attribute__mapper__process.html#ga9ddfac9cbb40bae7dc03d03f9edcad7c",
"group__attribute__store__type__registration.html#gga1c827b34925d83f45258ba05eb94f412a1c5cd3ab4a50f011eb33dc44f8080b35",
"group__ctimer.html#ga1f464e950a4fa11e8821b5c725921a15",
"group__dotdot__attribute__id__definitions.html#ga1c45a66f001210b76210847e616dc8a8",
"group__dotdot__attribute__id__definitions.html#ga43fce60d28c30342371f3f5c7799f0b1",
"group__dotdot__attribute__id__definitions.html#ga6c520b8890c4979b8534ec00d285b203",
"group__dotdot__attribute__id__definitions.html#ga9d5f4fc13c0224b43b02512bba1c709e",
"group__dotdot__attribute__id__definitions.html#gac76075ca4a932f35556c9fdcf64ea328",
"group__dotdot__attribute__id__definitions.html#gaef50d5bdb7d87e2f91993759ac2eeb82",
"group__dotdot__cluster__command__id__definitions.html#ga4b087e3326483a25396109188a002df4",
"group__dotdot__cluster__command__id__definitions.html#gad87fc0bf01ad3634f7512fcf45485766",
"group__dotdot__mqtt.html#ga023d5ecb7b79edbac02f289492989205",
"group__dotdot__mqtt.html#ga0ca6b3b3d585e6989af9a41e10ddd173",
"group__dotdot__mqtt.html#ga1674903d1dba1a03cfe55348724f3333",
"group__dotdot__mqtt.html#ga1f132a70f66adb8aca297f3490bdc9d4",
"group__dotdot__mqtt.html#ga277bd00b22666ce6ac662949ca86aefa",
"group__dotdot__mqtt.html#ga30f6814ba87a114095305a9846243f96",
"group__dotdot__mqtt.html#ga39d09482aa7baedf2cac39f99604fe94",
"group__dotdot__mqtt.html#ga4344e1c75693409e14fd6f94d2c44b85",
"group__dotdot__mqtt.html#ga4dec7eab4070c7905a349eb3c04026ee",
"group__dotdot__mqtt.html#ga5745a5f3061a7ab2ee9f95bd1d77e4bc",
"group__dotdot__mqtt.html#ga61b95156bf059490507d9d78da6d507b",
"group__dotdot__mqtt.html#ga6c0cdf1ad1623b17efd0b8e5d00624e9",
"group__dotdot__mqtt.html#ga75627829462fedeb96114f0ec08b14c5",
"group__dotdot__mqtt.html#ga7eb6c93244f76f5e135dd4a6695a9650",
"group__dotdot__mqtt.html#ga87cccbbbf04866d4293d871246a92003",
"group__dotdot__mqtt.html#ga8fec2886c29c2671a5748878e8625e7e",
"group__dotdot__mqtt.html#ga99aaa42c5f71def7b553de45e068cc48",
"group__dotdot__mqtt.html#gaa3fd495102e01c15c467e906aebfe463",
"group__dotdot__mqtt.html#gaae2f0b355513c85a6f6f6116344fa05e",
"group__dotdot__mqtt.html#gab7a626789ab06e5e383a1806d31b77ad",
"group__dotdot__mqtt.html#gac0e7e4d722750982e3d8cab09d01a959",
"group__dotdot__mqtt.html#gac8e9f8397119d59e96b9cd2a986a8f31",
"group__dotdot__mqtt.html#gad32c9ecf871d96dce53218bb0742f193",
"group__dotdot__mqtt.html#gadda9574664d12f0c3e559e9a48780df2",
"group__dotdot__mqtt.html#gae73d2707a4c834ddb662695a39a6e33b",
"group__dotdot__mqtt.html#gaf0212be3983bd0c181a2fb29de2339d8",
"group__dotdot__mqtt.html#gaf89a0a2732c75eb9f71037b378001b09",
"group__dotdot__mqtt__attributes.html#ga01282ec55e8b14c8c85a790b4752460e",
"group__dotdot__mqtt__attributes.html#ga181a5a707a0fce217861815cf5910185",
"group__dotdot__mqtt__attributes.html#ga31818672593c78e00729cf55f4f5fa86",
"group__dotdot__mqtt__attributes.html#ga4591768b19b488b46347d64bd23e7674",
"group__dotdot__mqtt__attributes.html#ga5b9c8b22ef140763bd10d7b723058bd0",
"group__dotdot__mqtt__attributes.html#ga6f3ba11483b81e4d933dbcd18bd0e0fd",
"group__dotdot__mqtt__attributes.html#ga8795bdf3879ca13e43e81ea86cb88c61",
"group__dotdot__mqtt__attributes.html#ga9b7f0f36bd2632c1722de63d6b752d5e",
"group__dotdot__mqtt__attributes.html#gaae6338f37bb9774b07c9b52079c75ccd",
"group__dotdot__mqtt__attributes.html#gac3fde288f9af07d18163f8993c08e93e",
"group__dotdot__mqtt__attributes.html#gad85eebaba514c6bd46d17b25b24d8504",
"group__dotdot__mqtt__attributes.html#gaec98534e2281a861c366d8f2ba57c559",
"group__dotdot__mqtt__command__helpers.html#ga0caed9c360a5fcb0bd9c78f47984911d",
"group__dotdot__mqtt__command__helpers.html#ga60232c153afbd4b1b509a63cc9513ac1",
"group__dotdot__mqtt__command__helpers.html#gaad1f40f99aa2f4005ff1fd691c9165f0",
"group__dotdot__mqtt__command__helpers.html#gafed91caf5f9552af38df61dea1ab017b",
"group__dotdot__mqtt__generated__commands.html#ga8be1f057858a182ac88745d09d01f030",
"group__dotdot__mqtt__group__commands.html#ga03389e8269fd5fdcee747f3528aa70a2",
"group__dotdot__mqtt__group__commands.html#ga490781c15e5e0451ddb4ac70983a9808",
"group__dotdot__mqtt__group__commands.html#ga93af6c2692ea4c9f6d58f6b0e6621b2c",
"group__dotdot__mqtt__group__commands.html#gad8d1bbb6078c35e0c65c221c6aaead19",
"group__dotdot__mqtt__helpers.html#ga1cd94fd0666a3344ea74d2aac84eb0a7",
"group__dotdot__mqtt__helpers.html#ga801bc5ac26b5bf1b75fe20c319b1db75",
"group__dotdot__mqtt__helpers.html#gadd937f26b16a68bd61ff61451863182f",
"group__dotdot__mqtt__helpers__ccp.html#ga2ee5dc4154f577d48451a8393a92f419",
"group__dotdot__mqtt__helpers__ccp.html#ga97461246ecd4836675840b0a313f9d3a",
"group__dotdot__mqtt__helpers__ccp.html#gaf19035357fc513835375996fbf5f487a",
"group__dotdot__mqtt__hpp.html#ga2622ebfc671a93c7caad4e618fbafe98",
"group__dotdot__mqtt__hpp.html#ga52e6cb4e4a1d1a0c84be8a0713699be1",
"group__dotdot__mqtt__hpp.html#ga830e151a73339e76316826d528bb216a",
"group__dotdot__mqtt__hpp.html#gaa844f005b6d8dcb78969f2fd7ac582fe",
"group__dotdot__mqtt__hpp.html#gae0ca52a6ab9e84f5aa6e27f95547c538",
"group__dotdot__mqtt__send__commands.html#ga0f430e65fcd2a8618612bbeb7f2a03e3",
"group__dotdot__mqtt__send__commands.html#ga6b9e815646f887afdc9cf7d4ea1bd8ad",
"group__dotdot__mqtt__send__commands.html#gac4ddf7f4e98ad4a3ca48bb6e35f93212",
"group__dotdot__mqtt__supported__generated__commands.html#ga53caac0adcce9b221c352419968f00d7",
"group__list.html#ga846aef3d9db78d0755e6bb39e7da485e",
"group__process.html#ga4d81d85c09c89d0e97cd0d239728a942",
"group__timer.html#ga6d71dececfce707c668e6257aad5906e",
"group__unify__config.html#ga72ab0cbc27a76ab2f1c7eb0de175b74c",
"group__unify__dotdot__attribute__store__helpers.html#ga02557143c332b96fbc21427d08988a0c",
"group__unify__dotdot__attribute__store__helpers.html#ga07529d886bcf24608caf175da4edec47",
"group__unify__dotdot__attribute__store__helpers.html#ga0cb21a07ebe4510a9d64b5f3ab5ffad4",
"group__unify__dotdot__attribute__store__helpers.html#ga10ebc12c08b7f6842c8bb492a72d96ef",
"group__unify__dotdot__attribute__store__helpers.html#ga16885e6bd25d9942719887d3c60f2388",
"group__unify__dotdot__attribute__store__helpers.html#ga1ccd4532da32f39dfdf1ac65db8687af",
"group__unify__dotdot__attribute__store__helpers.html#ga22da9cbc46b96f60cc58ebc26b23a244",
"group__unify__dotdot__attribute__store__helpers.html#ga285b64d82828bb50ccbdec1ff6a11916",
"group__unify__dotdot__attribute__store__helpers.html#ga2ded2e0b7dc1eaca67e453c80ebe9533",
"group__unify__dotdot__attribute__store__helpers.html#ga3300c89192b7eb1f3b4dcaca5d631559",
"group__unify__dotdot__attribute__store__helpers.html#ga38c888ddd1f38bb2af0727ea6946f648",
"group__unify__dotdot__attribute__store__helpers.html#ga3e359c4e57513a56cecc46fcac02ed84",
"group__unify__dotdot__attribute__store__helpers.html#ga43fc511897cb2b7c53237aa1b8d15f56",
"group__unify__dotdot__attribute__store__helpers.html#ga4924035b645ee36ce264676b0a2c2943",
"group__unify__dotdot__attribute__store__helpers.html#ga4e363091d9084832ab1cbaa186095043",
"group__unify__dotdot__attribute__store__helpers.html#ga53e95444fc9fd699ef7276c89749e258",
"group__unify__dotdot__attribute__store__helpers.html#ga595a9f3bf87b582e4e710909ceb3b48a",
"group__unify__dotdot__attribute__store__helpers.html#ga5ede7b724996147785d6590e9f9e8175",
"group__unify__dotdot__attribute__store__helpers.html#ga647ff2c7dcb7a8901dc626fd78200121",
"group__unify__dotdot__attribute__store__helpers.html#ga6acbf051115552afb2d85417f0ec5639",
"group__unify__dotdot__attribute__store__helpers.html#ga6f8fab73e5e958bfbc6e22ae3ae9d9bb",
"group__unify__dotdot__attribute__store__helpers.html#ga74f33793af335019878e541042e1636c",
"group__unify__dotdot__attribute__store__helpers.html#ga7b4c9d10a243153b9f17c0f54db1041a",
"group__unify__dotdot__attribute__store__helpers.html#ga810c25e70e22ff824a9b7972a8e3e643",
"group__unify__dotdot__attribute__store__helpers.html#ga858675c27d7d1b7f183e8438899d7cde",
"group__unify__dotdot__attribute__store__helpers.html#ga8b2e5f1a4dff074fa542604bed018e20",
"group__unify__dotdot__attribute__store__helpers.html#ga90144ff9384009133de1c3f04c883837",
"group__unify__dotdot__attribute__store__helpers.html#ga96003f7f2380054a450d584d7d9e9e66",
"group__unify__dotdot__attribute__store__helpers.html#ga9b3518f51edb4aa7bd233e97d13890f8",
"group__unify__dotdot__attribute__store__helpers.html#gaa0e1e55ba9aa314bcc8924ad5755641a",
"group__unify__dotdot__attribute__store__helpers.html#gaa682447103d1dfab052b39089bdcb849",
"group__unify__dotdot__attribute__store__helpers.html#gaabc9590392c0a0bc83c5e54031a02b03",
"group__unify__dotdot__attribute__store__helpers.html#gab0b40a10d13ad20ce8a1805fbe8429f2",
"group__unify__dotdot__attribute__store__helpers.html#gab6365249a89298af1aab230764d68fc0",
"group__unify__dotdot__attribute__store__helpers.html#gabb99219a6c552bedb8f2f89546b3acac",
"group__unify__dotdot__attribute__store__helpers.html#gac0f8e317364440f4cd92cd294453eb27",
"group__unify__dotdot__attribute__store__helpers.html#gac7653fdc81cdc0f96585e0f95b423122",
"group__unify__dotdot__attribute__store__helpers.html#gacc1b3be8ccdd07ea2ce5c603d1d38186",
"group__unify__dotdot__attribute__store__helpers.html#gad19d317f06d2684ef5c2dec48f36cdf3",
"group__unify__dotdot__attribute__store__helpers.html#gad7dbefde84f01c2f69ee8624c25f6436",
"group__unify__dotdot__attribute__store__helpers.html#gadd310bd7e0bcd7542bb0b418dd486382",
"group__unify__dotdot__attribute__store__helpers.html#gae277abd9eb125ca44b400ba1c7c0b4f9",
"group__unify__dotdot__attribute__store__helpers.html#gae84dc172dd484bd75b23aea431b5d473",
"group__unify__dotdot__attribute__store__helpers.html#gaed9d4d77da6f0b23a1c084fa944e0edd",
"group__unify__dotdot__attribute__store__helpers.html#gaf2f713f34efd2c4c27b4f7c9893b4b40",
"group__unify__dotdot__attribute__store__helpers.html#gaf80815566a5c46cf54add8996b1561fa",
"group__unify__dotdot__attribute__store__helpers.html#gafe4089bad3e29f91579866a83847599d",
"group__unify__sl__status__codes.html#gaec926f254dd9a6a7e9f7ccb59a120719",
"group__zap.html#ga26aaf6c757b73c3bf89addfffa287f37",
"group__zap.html#ga4e9b73c77f068834760711196178e947",
"group__zap.html#ga806c04fac8ccf54048e4ec5495ededef",
"group__zap.html#gab00b3dfce7e0d05dd865448da849da15",
"group__zap.html#gada1499853de2b480f0eedfe1cfd378de",
"group__zap.html#gga05405bfd09ed0499c21815e090a9388dadd89bb1cee72719b13559c72fdefc2fa",
"group__zap.html#gga32c99f0b7f7db154677df0839db0563bab0833636df85b819b9d0f0cabfefafe4",
"group__zap.html#gga40ac8fb1660228196bd93446589455a6a709a39dd36dfc57e6c33a1f7f5c23842",
"group__zap.html#gga6451d5bbf6ecebfdcb8b39d34437a1aea9fe3456650809cbf2f45d7c2811fec87",
"group__zap.html#gga9541648034da2e0751016c9ca658d1cca84605daf6010da52df51fb837bd0520c",
"group__zap.html#ggabfcf1ac97c9f2f9d389516af520039d1a53e51f1ddb16c37726ff5fac28e094c0",
"group__zap.html#ggaeb88bc686a9d13cb606fce1e1fe5b3d5a186bdc2ef3b9388f95c28716d5e9ed38",
"group__zigpc__common__zigbee.html#ga6258d1d49049c645af7e91d6122b1d51",
"group__zigpc__datastore.html#ga5b541bf46f367dd227aa5e70f697fa72",
"group__zigpc__group__map.html#ga6b647ce165ca755357fde69d91cc8949",
"group__zigpc__zcl__definitions.html#gac2ae3fe21680b2394b0a041bf56f0a86",
"group__zigpc__zcl__definitions.html#gga22a23b1d6a8445410cc511764179b661a7927edbd5c51bbdb092ca531da3c1bd5",
"group__zigpc__zcl__definitions.html#gga3e6f3ce5edbbb40bc788f8904dd65eafaca5f3ebaee10d24d09fa2fa637f51686",
"group__zigpc__zcl__definitions.html#gga4f93e22f89c79df0208d03e917b32844aa70ef52a9c16e1bb22b43ff96a1c19b2",
"group__zigpc__zcl__definitions.html#gga5f418afbcd769280272644a5e5c6ed57ad6823676dc7d9d0bdf8f3fab15afd729",
"group__zigpc__zcl__definitions.html#ggaa03d837e0450405c4857b29e7b64d894a5d779ecbfa902fb12fd6e45b2f9c2754",
"json__helpers_8hpp.html#a2134b28f052bf4da0489c9749e5ef69a",
"namespacemembers_eval.html",
"ota__cache_8hpp.html#aea44fd9a52c63b4412e6bf2710a88583",
"struct____attribute____.html#a9a4a3971d46b0a958c75f432bf14f698",
"struct__uic__mqtt__dotdot__pressure__measurement__supported__commands__.html#a49650d2b92628de37e5caecda0f0e29a",
"structmqtt__client.html#af18ed55735aa0dad24a63f8faeb07f76",
"structuic__mqtt__dotdot__barrier__control__updated__state__t.html#ac432fcd6949bface52824bcbfa224a0f",
"structuic__mqtt__dotdot__color__control__state__t.html#afa66ac01c6d85bdc25e888bc6ebf781d",
"structuic__mqtt__dotdot__door__lock__command__get__user__type__response__fields__t.html",
"structuic__mqtt__dotdot__electrical__measurement__state__t.html#a098d5cc96b6f71e3c238d352548fb510",
"structuic__mqtt__dotdot__electrical__measurement__updated__state__t.html#afb4b69728af0ccad05de9d6b93fc7b05",
"structuic__mqtt__dotdot__name__and__location__updated__state__t.html",
"structuic__mqtt__dotdot__protocol__controller__rf__telemetry__command__tx__report__fields__t.html#abdc358b8f3bd72915fd711c7b94adc20",
"structuic__mqtt__dotdot__system__metrics__updated__state__t.html#a0c477356555c8a3d72247a920dbc92bb",
"structuic__ota_1_1meta__t.html",
"structzigpc__zclcmdparse__door__lock__get__year__day__schedule__response__t.html#a30c93a7bb7a0117f8d6a5708b6f4bab4",
"unify__dotdot__attribute__store__command__callbacks__color__control_8c.html#ab4f8e5594b86d100d3502ba94c581663",
"unionzigpc__zclcmdparse__callback__data__t.html#a4796e7b6050a47b445581f261a4f366e",
"zigpc__gateway__int_8h.html#a4c7228075aec89265a2def0bd757913c"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';