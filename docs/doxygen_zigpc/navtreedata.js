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
      [ "Getting the NCP WSTK going :", "md_applications_zigpc_Getting_Started.html#autotoc_md19", [
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
        [ "Device State Updates", "md_applications_zigpc_readme_user.html#autotoc_md64", null ],
        [ "Device Discovery", "md_applications_zigpc_readme_user.html#autotoc_md65", null ],
        [ "Device Command Relaying", "md_applications_zigpc_readme_user.html#autotoc_md66", null ],
        [ "Device GeneratedCommand Updates", "md_applications_zigpc_readme_user.html#autotoc_md67", null ],
        [ "Device Attribute Updates", "md_applications_zigpc_readme_user.html#autotoc_md68", null ],
        [ "Group Message Handling", "md_applications_zigpc_readme_user.html#autotoc_md69", null ],
        [ "Zigbee OTA Firmware Upgrade Support", "md_applications_zigpc_readme_user.html#autotoc_md70", null ],
        [ "Zigbee Sleepy End Device Support", "md_applications_zigpc_readme_user.html#autotoc_md71", null ],
        [ "Protocol Controller Diagnostics", "md_applications_zigpc_readme_user.html#autotoc_md72", null ],
        [ "Updating NCP Firmware", "md_applications_zigpc_readme_user.html#autotoc_md73", null ]
      ] ],
      [ "Getting Started with Including Zigbee 3.0 Device to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md74", [
        [ "Adding Zigbee Device to the ZigPC Network via Install-Code Method", "md_applications_zigpc_readme_user.html#autotoc_md75", null ],
        [ "Adding Zigbee Device to the ZigPC Network using Well-Known Key", "md_applications_zigpc_readme_user.html#autotoc_md76", null ],
        [ "Discovering UCL Cluster Command Support for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md77", null ],
        [ "Discovering UCL Cluster Attribute Updates for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md78", null ],
        [ "Sending a UCL Cluster Command to Z3Device to see Attribute Update", "md_applications_zigpc_readme_user.html#autotoc_md79", null ],
        [ "Removing a Device Already in the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md80", null ],
        [ "Updating Zigbee Device Firmware already on the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md81", [
          [ "Instructions on how to start an OTA update", "md_applications_zigpc_readme_user.html#autotoc_md82", null ],
          [ "Once the Unify Image Provider recognizes changes to images.json, the following actions will occur", "md_applications_zigpc_readme_user.html#autotoc_md83", null ]
        ] ],
        [ "Retrieving Diagnostic information from ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md84", [
          [ "Steps to enable this behaviour", "md_applications_zigpc_readme_user.html#autotoc_md85", null ]
        ] ]
      ] ],
      [ "Technical Specifications", "md_applications_zigpc_readme_user.html#autotoc_md86", [
        [ "Device UNID Format", "md_applications_zigpc_readme_user.html#autotoc_md87", null ],
        [ "Recognized SmartStart DSK Format", "md_applications_zigpc_readme_user.html#autotoc_md88", null ],
        [ "Zigbee Device Support", "md_applications_zigpc_readme_user.html#autotoc_md89", null ]
      ] ],
      [ "Appendix", "md_applications_zigpc_readme_user.html#autotoc_md90", [
        [ "Setting Up the Zigbee NCP Connected to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md91", [
          [ "Using Pre-built NCP FW Images", "md_applications_zigpc_readme_user.html#autotoc_md92", null ],
          [ "Building NCP FW Images Using Studio", "md_applications_zigpc_readme_user.html#autotoc_md93", null ]
        ] ],
        [ "Setting Up the Sample Zigbee Z3Light FW Image", "md_applications_zigpc_readme_user.html#autotoc_md94", null ],
        [ "Flashing Zigbee Devices", "md_applications_zigpc_readme_user.html#autotoc_md95", null ]
      ] ]
    ] ],
    [ "ZigPC Release Notes", "md_applications_zigpc_release_notes.html", [
      [ "[1.3.1] - March 2023", "md_applications_zigpc_release_notes.html#autotoc_md97", [
        [ "Changed (1.3.1)", "md_applications_zigpc_release_notes.html#autotoc_md98", null ]
      ] ],
      [ "[1.3.0] - February 2023", "md_applications_zigpc_release_notes.html#autotoc_md99", [
        [ "Added (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md100", null ],
        [ "Changed (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md101", null ]
      ] ],
      [ "[1.2.0] - August 2022", "md_applications_zigpc_release_notes.html#autotoc_md102", [
        [ "Added (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md103", null ],
        [ "Changed (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md104", null ]
      ] ],
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md105", [
        [ "Added (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md106", null ],
        [ "Changed (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md107", null ]
      ] ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md108", [
        [ "Added (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md109", null ],
        [ "Changed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md110", null ],
        [ "Removed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md111", null ],
        [ "Fixed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md112", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md113", [
        [ "Added (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md114", null ],
        [ "Changed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md115", null ],
        [ "Fixed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md116", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md117", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md118", [
        [ "Fixed (1.0.1)", "md_applications_zigpc_release_notes.html#autotoc_md119", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md120", [
        [ "Added (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md121", null ],
        [ "Changed (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md122", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md123", [
        [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md124", null ]
      ] ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md125", null ]
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
"attribute__transitions_8h_source.html",
"classast_1_1reducer.html#a25059aa6202f4512b050cc71cbd68f7f",
"classzigpc_1_1gateway_1_1ZCLFrameMulticastRequest.html#a5e688dabb453b74954ad3c45378fe245",
"dir_764a147c4b3d2a201d6fe3833354436e.html",
"gecko_2contiki-conf_8h.html#a10f9a3d7baef58ccc23228c3bd29c1fb",
"group__attribute__mapper__process.html#gga6e51263d3336cb0bfbdcaed166152794a9407fa220edcac0e636b0ebe503ee0ce",
"group__attribute__store__type__registration.html#gga1c827b34925d83f45258ba05eb94f412a2ca4781df91b80808b52b1627b7c9593",
"group__ctimer.html#gaddaf856390b1f3f572b62def662088aa",
"group__dotdot__attribute__id__definitions.html#ga20e6d08d3f6d46542bf7e8c707eea22e",
"group__dotdot__attribute__id__definitions.html#ga476230535af3098080deb2e8edae78ae",
"group__dotdot__attribute__id__definitions.html#ga6ef13aad2e55897d4b0353a7478f8d06",
"group__dotdot__attribute__id__definitions.html#gaa21cbbdd989711d84c2c1dfd2620587b",
"group__dotdot__attribute__id__definitions.html#gacbeaf23796de8c9a4f2ff1f1db3cc227",
"group__dotdot__attribute__id__definitions.html#gaf9398c6d2974320d8f33c17a00e758de",
"group__dotdot__cluster__command__id__definitions.html#ga679d104524b6686b17d5aad63de8faaf",
"group__dotdot__cluster__command__id__definitions.html#gafcf1951a29b1ef9a4313d430abbf5195",
"group__dotdot__mqtt.html#ga0503fedf3b3e2cfa8c53b8929e8cf170",
"group__dotdot__mqtt.html#ga0f315fc8855ca408347ce23a381d2540",
"group__dotdot__mqtt.html#ga1890502dbbb83fe62324aca3ca41ebb0",
"group__dotdot__mqtt.html#ga2132fd67f0b0d77e0425aa1360d120c2",
"group__dotdot__mqtt.html#ga2a3d1b82a0b6afb5db05285763309864",
"group__dotdot__mqtt.html#ga338c869fb15b9ea451d72d9f07a61bbd",
"group__dotdot__mqtt.html#ga3c65a3aca26d2d22aabc88415f4028d2",
"group__dotdot__mqtt.html#ga456f90d14e7acd10f9271b353d3cc0a7",
"group__dotdot__mqtt.html#ga4f8a0e6f6e7e6da9832b23f3408be8fe",
"group__dotdot__mqtt.html#ga590abe27d3a95547325a24203d604b30",
"group__dotdot__mqtt.html#ga63111ef80fab37af419a0b1691c02b75",
"group__dotdot__mqtt.html#ga6dce07cf18c5623ac7b06d66c5217b66",
"group__dotdot__mqtt.html#ga76b2792a200e5f682da4d6a942853e8c",
"group__dotdot__mqtt.html#ga80df3a95292e580dc162d9c22dec0f28",
"group__dotdot__mqtt.html#ga8945c91344ddedc38c3c257b3272d35c",
"group__dotdot__mqtt.html#ga91966f86ac8eabaeb2121a4e5b9b239f",
"group__dotdot__mqtt.html#ga9bcae1dbf7281cdd639fd44e351fe03e",
"group__dotdot__mqtt.html#gaa6640eda013f04cfa1a0a2fee2b86e45",
"group__dotdot__mqtt.html#gab0fb18cd548978bbc2b7c29da86ca17a",
"group__dotdot__mqtt.html#gab9d35ca765764e7c6cefcd1e068f8e10",
"group__dotdot__mqtt.html#gac3fda7df4d468c6c52f2f2a5799b261f",
"group__dotdot__mqtt.html#gacb5ad55951c86949c000ce1b00999c47",
"group__dotdot__mqtt.html#gad6731aee8e30f7ed6c453e93b16e6a69",
"group__dotdot__mqtt.html#gae181ae6575bfaf3413bd64e5768fa9ea",
"group__dotdot__mqtt.html#gae967ec09a258d9950aa4c318fc715169",
"group__dotdot__mqtt.html#gaf2daac47d7482430e12b227ad80efb2e",
"group__dotdot__mqtt.html#gafae4beb090b923bdf0c0fb3f914ea974",
"group__dotdot__mqtt__attributes.html#ga09531025a258a949f30a125310476203",
"group__dotdot__mqtt__attributes.html#ga1ea3a734ed9470dca4a3010840fdcfff",
"group__dotdot__mqtt__attributes.html#ga36b51a8602b1d951883347a9828746ec",
"group__dotdot__mqtt__attributes.html#ga4c88673c97448d5a3ce4d547d6813812",
"group__dotdot__mqtt__attributes.html#ga6030c8c3aa2a72e03e5a25e60191f62c",
"group__dotdot__mqtt__attributes.html#ga765f570fad4f7e57fed7a1514094405b",
"group__dotdot__mqtt__attributes.html#ga8be924d3d35d983c1ea7092857bddbcf",
"group__dotdot__mqtt__attributes.html#gaa155c378c1482e7e7ab8cdc54bd8b70d",
"group__dotdot__mqtt__attributes.html#gab377a5d429cb13d351bf8c8949dc1ed3",
"group__dotdot__mqtt__attributes.html#gac88bdcd1189e1f5f57d1da6ae0de6fc3",
"group__dotdot__mqtt__attributes.html#gade065f738ac0b81eaf448e0b34fe9ac0",
"group__dotdot__mqtt__attributes.html#gaf2a9c03ce963798db4141b7b40e95aa1",
"group__dotdot__mqtt__command__helpers.html#ga201ee755896e36770f34159dbb8da34e",
"group__dotdot__mqtt__command__helpers.html#ga78089e2bd28256ab8bc8455aa11a7298",
"group__dotdot__mqtt__command__helpers.html#gac0cc0d664f4b810ed21c483abd45242b",
"group__dotdot__mqtt__generated__commands.html#ga1ad6c331364b64fecc3a77652c04e12c",
"group__dotdot__mqtt__generated__commands.html#gab5eca08fbdc00d083bcd54e5d0dc5c77",
"group__dotdot__mqtt__group__commands.html#ga129e2049fcb3ea84248fd9759b1c7566",
"group__dotdot__mqtt__group__commands.html#ga57b2a1795ff9824b931d934fc55fbdf5",
"group__dotdot__mqtt__group__commands.html#gaa155acfc58af151c849b0596176f3fd4",
"group__dotdot__mqtt__group__commands.html#gae79d53e2dfe1b4b0859c25a182ccaefe",
"group__dotdot__mqtt__helpers.html#ga3746c471ce2a288618f4fe1b1958ebcf",
"group__dotdot__mqtt__helpers.html#ga9a4077f91fde6bf9b85cc7d02266bce6",
"group__dotdot__mqtt__helpers.html#gaf94477b2fd5247fe2ee3ff004494cdd4",
"group__dotdot__mqtt__helpers__ccp.html#ga4dee461f3c902a2acfc9528f78a44a91",
"group__dotdot__mqtt__helpers__ccp.html#gab6e8796c51d720fb4de9f7bcd125ec37",
"group__dotdot__mqtt__hpp.html#ga07c1533efd6f173c7a2b53d7afe9b95c",
"group__dotdot__mqtt__hpp.html#ga3beab3385b25be0ede07793630b95c4b",
"group__dotdot__mqtt__hpp.html#ga6c5240fe0dca57ae443b736fcb2491ba",
"group__dotdot__mqtt__hpp.html#ga9b567630351481b4f433e2588c0d3d58",
"group__dotdot__mqtt__hpp.html#gacb20379a68e09fad6c27defd22fceab5",
"group__dotdot__mqtt__hpp.html#gafddb516ed8cd2f47d0bddf770f22779a",
"group__dotdot__mqtt__send__commands.html#ga4891ac5b91b42a5f95a53cabaa6d7c55",
"group__dotdot__mqtt__send__commands.html#gaa08dd363dfbee23d95f9c7af22dc76b5",
"group__dotdot__mqtt__send__commands.html#gaf7907632b232c88669e185457c1596b5",
"group__etimer.html#ga4f73b01caad4528df4ddadc9fbd30eac",
"group__mqtt__client.html#ggaca6b39d8cd48e315ea885c3f715a7ff3abc12cb422945aa55456de7119c99a8be",
"group__ringbuf.html#ga2a370b54ed5d194e188a94e505b48eeb",
"group__uic__stdin.html#gadf8c2d3481fa99ef2b97979a8013b033",
"group__unify__dotdot__attribute__store__helpers.html#ga0187cdaca43ef02c3015b20d82838b47",
"group__unify__dotdot__attribute__store__helpers.html#ga0845ed3bbd56d9ac7e28a7d0040cff6f",
"group__unify__dotdot__attribute__store__helpers.html#ga0eb6d6d124afdd6b992be003574fe564",
"group__unify__dotdot__attribute__store__helpers.html#ga155fbf6f476440977c8a31e89aeb2e19",
"group__unify__dotdot__attribute__store__helpers.html#ga1d22947e156edff9e52aa687539b1f7d",
"group__unify__dotdot__attribute__store__helpers.html#ga24e409a055f7768c23ab0779e4a102b0",
"group__unify__dotdot__attribute__store__helpers.html#ga2c71ce5a47da9c39efd63b05cbe243df",
"group__unify__dotdot__attribute__store__helpers.html#ga334a443008b03b56e09479527c933055",
"group__unify__dotdot__attribute__store__helpers.html#ga3b82a284acb24e9cf2e50af148dbf36d",
"group__unify__dotdot__attribute__store__helpers.html#ga42a706027e1850046b11073713440507",
"group__unify__dotdot__attribute__store__helpers.html#ga49ef0039d29010b64afa9b668d26bcd4",
"group__unify__dotdot__attribute__store__helpers.html#ga507ff7b1be1aea73cdeb04d94047fe34",
"group__unify__dotdot__attribute__store__helpers.html#ga586bdcf3b3579ac0fd338629549846fb",
"group__unify__dotdot__attribute__store__helpers.html#ga5ecce20c45ea4d2ee0b2c6292c03e1db",
"group__unify__dotdot__attribute__store__helpers.html#ga66958f80a7d7e229b7da28934c0a8dcd",
"group__unify__dotdot__attribute__store__helpers.html#ga6d9f79d0b896e3e2b5c67bfc3b7debbc",
"group__unify__dotdot__attribute__store__helpers.html#ga7489da485d59ce5b0339f41778e7ac63",
"group__unify__dotdot__attribute__store__helpers.html#ga7c12b6c400a087bf1bfa5362c408b484",
"group__unify__dotdot__attribute__store__helpers.html#ga83d10b9665adaad0cf698547e1eb2b00",
"group__unify__dotdot__attribute__store__helpers.html#ga8a69e5c56596801d5681cb552630cacf",
"group__unify__dotdot__attribute__store__helpers.html#ga90d915c9defe199e5e1c123b8de9b45a",
"group__unify__dotdot__attribute__store__helpers.html#ga98223a583881449209fdb7f2cd154825",
"group__unify__dotdot__attribute__store__helpers.html#ga9f1aaec73eed5db84cd647d4a5147d6c",
"group__unify__dotdot__attribute__store__helpers.html#gaa6dca50618daad8bbc2a714075a46d5c",
"group__unify__dotdot__attribute__store__helpers.html#gaadc8db4c898c73c29fe894bd9b6d904c",
"group__unify__dotdot__attribute__store__helpers.html#gab53aefad370d3a3ae922195028d688f2",
"group__unify__dotdot__attribute__store__helpers.html#gabcda89426e7884dd05848f42d44a2a08",
"group__unify__dotdot__attribute__store__helpers.html#gac4ede0ec299a576f235f7ddf5586a411",
"group__unify__dotdot__attribute__store__helpers.html#gacb602e040885ffb70663ee0c83fb4392",
"group__unify__dotdot__attribute__store__helpers.html#gad239d979854b6c28a22e849e6ecdf4a6",
"group__unify__dotdot__attribute__store__helpers.html#gad9b77bc2a128843a7c7eacc790f35f5b",
"group__unify__dotdot__attribute__store__helpers.html#gae1f28e478a9b6e006c03d8bb4a5cbfa7",
"group__unify__dotdot__attribute__store__helpers.html#gae95ece8be6425273968b096fd34a3561",
"group__unify__dotdot__attribute__store__helpers.html#gaf03e675a783a0d50ebfb10730a5fdaf0",
"group__unify__dotdot__attribute__store__helpers.html#gaf705d8db995fbd045caa5fb7c555966b",
"group__unify__dotdot__attribute__store__helpers.html#gafea91e113b1798fa6278686370222387",
"group__zap.html#ga0357b870f948d1866780fb0b5768464a",
"group__zap.html#ga305268e3efd4ac97a515679a2f113212",
"group__zap.html#ga5fd7096b234f705f31007105c6503b1e",
"group__zap.html#ga8f95f5ddffc8eaaa974c3a7d7bd005f0",
"group__zap.html#gac4c7013a8402d3149f0998e43cb09279",
"group__zap.html#gaef3653bca0c389840bac2ea704286d47",
"group__zap.html#gga0dd0f45e1897e6c01cb33bd3cb2533cfad32175b1a0c8cb4a63a0e8d836c083c3",
"group__zap.html#gga40ac8fb1660228196bd93446589455a6a3f339ef7275eb84596ee82e060b001af",
"group__zap.html#gga4cc3bfb58de4cc9d514e4e94bb26fe44af9709ab3b87a2a69776ffee6385d5a69",
"group__zap.html#gga8ba3467dc028fe5438bc355d548b5699abe4eac7cd584cd8ad0cf30e2654d090d",
"group__zap.html#ggabd1e1243e24397faefcbb311d2538b27a1d8b4ba0bb67aaa08ca0f019a00d31b9",
"group__zap.html#ggaeadee2a62b1aee6e9d51e93567349116a451c2b1d31c0855e297c2845dbc36df0",
"group__zigpc__common__zigbee.html#ga409fa62928c89da1c543a2b59019b2c2",
"group__zigpc__datastore.html#ga391d5794187cae767f2ea09fb537d62c",
"group__zigpc__group__map.html",
"group__zigpc__zcl__definitions.html#gae059fe666d20ec59e91a80705095f50d",
"group__zigpc__zcl__definitions.html#gga22a23b1d6a8445410cc511764179b661ace043cb418f558633e91e2b87453ce76",
"group__zigpc__zcl__definitions.html#gga548dfa4f1afc477f2655bae3e31fba6fad6626b87ccccacfd618d380aebe4dce9",
"group__zigpc__zcl__definitions.html#ggaa03d837e0450405c4857b29e7b64d894abcbe69e6bda9f0386adccb2038170a55",
"md_applications_zigpc_readme_user.html#autotoc_md84",
"namespacezigpc__datastore_1_1binding.html#aa7bb491baf5819feef2b6b57fbf162ea",
"smartstart__management_8hpp.html",
"struct__uic__mqtt__dotdot__door__lock__supported__commands__.html#a755b9ad236e936be06dd498f26d05f5e",
"structast_1_1scope.html",
"structuic__mqtt__dotdot__aox__locator__updated__state__t.html#a1b662c85e10c98ecf005f586b24d5b2a",
"structuic__mqtt__dotdot__color__control__command__move__color__fields__t.html#a15a6c17c7ee2f74d5291125bc0ffd30b",
"structuic__mqtt__dotdot__diagnostics__state__t.html#a5871f4f21c0d40ee03fd9625aa557895",
"structuic__mqtt__dotdot__door__lock__state__t.html#a9e810e6de758cdfc68349610aee2d5ca",
"structuic__mqtt__dotdot__electrical__measurement__updated__state__t.html#a449ad019d5d431b960a91ddf189470af",
"structuic__mqtt__dotdot__level__state__t.html",
"structuic__mqtt__dotdot__pm25__updated__state__t.html#a5acfca30b22d134b7837913d082bdcdd",
"structuic__mqtt__dotdot__relativity__humidity__updated__state__t.html#a4b2528bade88b4b94954449079725fb6",
"structuic__mqtt__dotdot__thermostat__state__t.html#a5dc3928eecd522988a1506cea68df10c",
"structzigpc__attrmgmt__on__frame__receive__t.html",
"structzigpc__zclcmdparse__thermostat__get__relay__status__log__response__t.html#a8d68f9fb05ab687636640050a584a9dc",
"unify__dotdot__attribute__store__force__read__attributes__command__callbacks_8c.html#a7ce0df38eb467e59f209470c8f5ac4e6",
"zigpc__config_8c.html#a8fdca44881420da5d2da08787d0f4113",
"zigpc__ucl_8hpp.html#a0ae3f2434936eef3e9855e9437c03292aeb9d70b8abb8cc33ddb33743c528c28c"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';