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
      [ "[1.6.0] - Aug 2024", "md_applications_zigpc_release_notes.html#autotoc_md98", [
        [ "Changed (1.6.0)", "md_applications_zigpc_release_notes.html#autotoc_md99", null ]
      ] ],
      [ "[1.5.0] - Feb 2024", "md_applications_zigpc_release_notes.html#autotoc_md100", [
        [ "Added (1.5.0)", "md_applications_zigpc_release_notes.html#autotoc_md101", null ],
        [ "Changed (1.5.0)", "md_applications_zigpc_release_notes.html#autotoc_md102", null ]
      ] ],
      [ "[1.4.0] - August 2023", "md_applications_zigpc_release_notes.html#autotoc_md103", [
        [ "Added (1.4.0)", "md_applications_zigpc_release_notes.html#autotoc_md104", null ],
        [ "Changed (1.4.0)", "md_applications_zigpc_release_notes.html#autotoc_md105", null ]
      ] ],
      [ "[1.3.1] - March 2023", "md_applications_zigpc_release_notes.html#autotoc_md106", [
        [ "Changed (1.3.1)", "md_applications_zigpc_release_notes.html#autotoc_md107", null ]
      ] ],
      [ "[1.3.0] - February 2023", "md_applications_zigpc_release_notes.html#autotoc_md108", [
        [ "Added (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md109", null ],
        [ "Changed (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md110", null ]
      ] ],
      [ "[1.2.0] - August 2022", "md_applications_zigpc_release_notes.html#autotoc_md111", [
        [ "Added (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md112", null ],
        [ "Changed (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md113", null ]
      ] ],
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md114", [
        [ "Added (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md115", null ],
        [ "Changed (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md116", null ]
      ] ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md117", [
        [ "Added (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md118", null ],
        [ "Changed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md119", null ],
        [ "Removed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md120", null ],
        [ "Fixed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md121", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md122", [
        [ "Added (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md123", null ],
        [ "Changed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md124", null ],
        [ "Fixed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md125", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md126", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md127", [
        [ "Fixed (1.0.1)", "md_applications_zigpc_release_notes.html#autotoc_md128", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md129", [
        [ "Added (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md130", null ],
        [ "Changed (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md131", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md132", [
        [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md133", null ]
      ] ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md134", null ]
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
"cc_8h_source.html",
"classast_1_1path__match__eval.html#a9d836adbb8b21cb6c26d9f9746ac0617",
"classunify_1_1node__state__monitor_1_1node.html#a57fe43d4e5a1442d3c56b3b27b5c060a",
"classzigpc_1_1gateway_1_1ZCLFrameUnicastRequest.html#a584c0e4e9a8142c88cdf81072d2f5e97",
"dir_5e26d867e8aedae8413a7ee8befe6072.html",
"functions_vars_j.html",
"group__attribute__poll.html#ga44d668009791e60abcb81ca62d1abace",
"group__dotdot__attribute__id__definitions.html#ga00ca111ca33739bc3b2c80d8a485e69c",
"group__dotdot__attribute__id__definitions.html#ga4fdde7a5f1708e9d086f700d092775bf",
"group__dotdot__attribute__id__definitions.html#gaa744dc2a6a2517257bbb14d62c2e04b7",
"group__dotdot__attribute__id__definitions.html#gaf715eb2af0fc83373864b00b79e5b4df",
"group__dotdot__cluster__command__id__definitions.html#gae9857eb293a34a428aed779274cd3a81",
"group__dotdot__mqtt.html#ga0d1cabc9e83b1af175ba1d4488062e2f",
"group__dotdot__mqtt.html#ga1ee4681f822be1cc2427ca9f11d9bc58",
"group__dotdot__mqtt.html#ga30326491659a3950c97e4ec5ed172839",
"group__dotdot__mqtt.html#ga41d18c7648c210924794aac582390d4d",
"group__dotdot__mqtt.html#ga5508401b53f1d1532461af8e34e47d06",
"group__dotdot__mqtt.html#ga69885117d4a77d67f6ef7ea96298d068",
"group__dotdot__mqtt.html#ga7bcf816cc6f6595f7fa3fe8d8113779a",
"group__dotdot__mqtt.html#ga8c4828bab09a04e5f3a4ef0c077ea9e9",
"group__dotdot__mqtt.html#ga9ed25fcfcd1cfab01f2ee6c3cecf617f",
"group__dotdot__mqtt.html#gab249c4d1e246cc883495f0b58d015120",
"group__dotdot__mqtt.html#gac4e813b18200e2fe52770002421e54dd",
"group__dotdot__mqtt.html#gad6f39e3011603d087007f35bfbd4c485",
"group__dotdot__mqtt.html#gae97695ef8e84aa57df88eb8ab5345fff",
"group__dotdot__mqtt.html#gafa52bb3181193ad414f0e61baeea7b47",
"group__dotdot__mqtt__attributes.html#ga1b0776b426441644f5b22d9c959c8c09",
"group__dotdot__mqtt__attributes.html#ga470869af70487cd2c0cb00d3669d45a0",
"group__dotdot__mqtt__attributes.html#ga6ed99bf80a9035ea75b5680060630e3b",
"group__dotdot__mqtt__attributes.html#ga9a2f91ddc4a0d2d9db7a5a81cedf99ff",
"group__dotdot__mqtt__attributes.html#gac1de074e595c4ec497002a4b7a407b9f",
"group__dotdot__mqtt__attributes.html#gae764843102b4a37c81591cf39712b595",
"group__dotdot__mqtt__command__helpers.html#ga422bc0cb4dc38f4fbda5f8d3431db129",
"group__dotdot__mqtt__command__helpers.html#gae36070b8ac9b5f81009bbda8327bb018",
"group__dotdot__mqtt__generated__commands.html#gada6a11cacc8fde2400c6931af843c8b8",
"group__dotdot__mqtt__group__commands.html#ga702449312901178b4c0f69ff966a5126",
"group__dotdot__mqtt__group__commands.html#gaf75c73305d75fe45afc32ca4424f133c",
"group__dotdot__mqtt__helpers.html#ga95d53399e2f3bce87b57dee18c75dbae",
"group__dotdot__mqtt__helpers__ccp.html#ga41ca181308e5f6973a04967a681af3c7",
"group__dotdot__mqtt__helpers__ccp.html#gaf5278e1b04ca359b50bc523aba31e6b3",
"group__dotdot__mqtt__hpp.html#ga5202db9bbc19cee6ef429e28336ea360",
"group__dotdot__mqtt__hpp.html#gaa5db69d16c875880cc26fcad31a45a73",
"group__dotdot__mqtt__send__commands.html#ga05f1655e33b87bd958e981d65ac70113",
"group__dotdot__mqtt__send__commands.html#gab0f1ad5b4f18c4df0cd11153d12189fa",
"group__identify__cluster__mapper.html",
"group__sl__log.html#ga35923ad2e4bf4111f7db919dc463794b",
"group__unify__dotdot__attribute__store__helpers.html#ga0199d6445582881cec48201c0f68f32b",
"group__unify__dotdot__attribute__store__helpers.html#ga0bb1c72a07e7dee303b8958d2e5d88b3",
"group__unify__dotdot__attribute__store__helpers.html#ga15298fb9f6596ce3b187f9ea58591e12",
"group__unify__dotdot__attribute__store__helpers.html#ga20f0f4868822f93302d3532ff65ee147",
"group__unify__dotdot__attribute__store__helpers.html#ga2c1626729c7fa1e1715e852af754a5ae",
"group__unify__dotdot__attribute__store__helpers.html#ga360311f814f15fb47488d52c5bb6da24",
"group__unify__dotdot__attribute__store__helpers.html#ga40b6748745495df0c272ab1078b3b801",
"group__unify__dotdot__attribute__store__helpers.html#ga4a992e8093bd1038a42bea584be743aa",
"group__unify__dotdot__attribute__store__helpers.html#ga55755d2f0f3bae7cc62f293415e75b40",
"group__unify__dotdot__attribute__store__helpers.html#ga603327610d58a58883314a2843631c64",
"group__unify__dotdot__attribute__store__helpers.html#ga6b9d1b67329f4db66d74d35e4ba26dc1",
"group__unify__dotdot__attribute__store__helpers.html#ga754e5660d875af83ffa4b0ba4210e4c1",
"group__unify__dotdot__attribute__store__helpers.html#ga81108377279c8215478498015ca5f149",
"group__unify__dotdot__attribute__store__helpers.html#ga8b2e5f1a4dff074fa542604bed018e20",
"group__unify__dotdot__attribute__store__helpers.html#ga95975f75257502cf7589cd806ad8f40e",
"group__unify__dotdot__attribute__store__helpers.html#ga9f73fb4a84647a372777801ba018f61e",
"group__unify__dotdot__attribute__store__helpers.html#gaaa7529aab20138ad1edc43c3d4ca5f09",
"group__unify__dotdot__attribute__store__helpers.html#gab4052613f7479b1ce8a88bcb5f94bef6",
"group__unify__dotdot__attribute__store__helpers.html#gabe87c102419e162ae3d9982d900f4220",
"group__unify__dotdot__attribute__store__helpers.html#gac9d3bd0c0aeb53ff5e68f3f38281aa70",
"group__unify__dotdot__attribute__store__helpers.html#gad468c6fefefa087e08bb7ae614b02aea",
"group__unify__dotdot__attribute__store__helpers.html#gadef087e84e1c70eac8b616c5a8c8b2d9",
"group__unify__dotdot__attribute__store__helpers.html#gae9fb76bec4dbeba60476f816db2a43d7",
"group__unify__dotdot__attribute__store__helpers.html#gaf44dd181667af394995aae0f6ef51f35",
"group__unify__dotdot__attribute__store__helpers.html#gaff6c9d0f5ef630a82e0373b61d6485b5",
"group__zap.html#ga28c0abe00d086c31c1d802875c8a68aa",
"group__zap.html#ga7b5625b83443bb2f7d9d07417c054e20",
"group__zap.html#gacb2e10e6a7f74f6e9be01c568d605517",
"group__zap.html#gga0dd0f45e1897e6c01cb33bd3cb2533cfab0ef57f025b132713adc217d7650d8fc",
"group__zap.html#gga42fe46fd2322a514ebed0de661b2a526a60f0d5f9f770810ed9c116b43a319d17",
"group__zap.html#ggaa3ff17a179862e659a861c5c8a23eaf7a625d8428511bc6ebd187a326838d0cec",
"group__zap.html#ggae20e61884df7b5cd1ce720d64e2033b9a19e7450004681317133812e69c2ef6e0",
"group__zigpc__datastore.html#ga22e26e804b10ac31345b002c8ff88315",
"group__zigpc__zcl__definitions.html#ga4bdb2c8d71cd8690c735f453b2c5b36c",
"group__zigpc__zcl__definitions.html#gga3e6f3ce5edbbb40bc788f8904dd65eafa6ced77d121ec1cd41621c8418b3419e8",
"group__zigpc__zcl__definitions.html#gga5f418afbcd769280272644a5e5c6ed57a74de4dda166aad01c0969bfbbe7edc8d",
"group__zigpc__zcl__util.html#gac5674da34a32f0a3793e3b130e20c791",
"namespaceast.html#ab6da2cc5568b6ef77459778f07cfd460",
"ota__cache_8hpp.html#aaccc301536abd5df84ddcd0b58ada430",
"struct____attribute____.html#aa1221ecb49d40c6f61bc3450574cbc76",
"struct__uic__mqtt__dotdot__pressure__measurement__supported__commands__.html#a49650d2b92628de37e5caecda0f0e29a",
"structgateway__frame__t.html#ab9e05e56e86023cb4556324100b1c31d",
"structuic__mqtt__dotdot__application__monitoring__updated__state__t.html#a6b2aaf6e26c332b2263f246da937529e",
"structuic__mqtt__dotdot__color__control__command__step__color__temperature__fields__t.html#a27d555f73b8b1082396ea5f180e268f0",
"structuic__mqtt__dotdot__door__lock__command__clear__all__pin__codes__response__fields__t.html#a7e1769e9d1bdce2cc1176bb300e23039",
"structuic__mqtt__dotdot__door__lock__updated__state__t.html#a281ba89b7a4d791149528cbb56384cd5",
"structuic__mqtt__dotdot__electrical__measurement__updated__state__t.html#a66087e3929dc846d44df7547df25387f",
"structuic__mqtt__dotdot__level__updated__state__t.html",
"structuic__mqtt__dotdot__power__configuration__updated__state__t.html#a5a12f7a1180cb36bcf356a0da451c3d4",
"structuic__mqtt__dotdot__shade__configuration__updated__state__t.html",
"structuic__mqtt__dotdot__time__state__t.html#afcc42efbed673ff0d4931d39ab91a45b",
"structzigpc__config__t.html#ae899e01b118538a571fcbdd09eb57b02",
"timer_8c.html",
"unify__dotdot__attribute__store__command__callbacks__zwave__fan__control_8c.html#a7ce0df38eb467e59f209470c8f5ac4e6",
"zigpc__binding__response_8h.html#a3772e10d2158395294ced41ca79088ca",
"zigpc__net__mgmt__internal_8h.html#ab8464654dbb0669bbf90ac23d0c608d2"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';