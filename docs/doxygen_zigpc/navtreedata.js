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
      [ "[1.4.0] - August 2023", "md_applications_zigpc_release_notes.html#autotoc_md98", [
        [ "Added (1.4.0)", "md_applications_zigpc_release_notes.html#autotoc_md99", null ],
        [ "Changed (1.4.0)", "md_applications_zigpc_release_notes.html#autotoc_md100", null ]
      ] ],
      [ "[1.3.1] - March 2023", "md_applications_zigpc_release_notes.html#autotoc_md101", [
        [ "Changed (1.3.1)", "md_applications_zigpc_release_notes.html#autotoc_md102", null ]
      ] ],
      [ "[1.3.0] - February 2023", "md_applications_zigpc_release_notes.html#autotoc_md103", [
        [ "Added (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md104", null ],
        [ "Changed (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md105", null ]
      ] ],
      [ "[1.2.0] - August 2022", "md_applications_zigpc_release_notes.html#autotoc_md106", [
        [ "Added (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md107", null ],
        [ "Changed (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md108", null ]
      ] ],
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md109", [
        [ "Added (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md110", null ],
        [ "Changed (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md111", null ]
      ] ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md112", [
        [ "Added (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md113", null ],
        [ "Changed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md114", null ],
        [ "Removed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md115", null ],
        [ "Fixed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md116", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md117", [
        [ "Added (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md118", null ],
        [ "Changed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md119", null ],
        [ "Fixed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md120", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md121", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md122", [
        [ "Fixed (1.0.1)", "md_applications_zigpc_release_notes.html#autotoc_md123", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md124", [
        [ "Added (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md125", null ],
        [ "Changed (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md126", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md127", [
        [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md128", null ]
      ] ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md129", null ]
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
"gecko_2contiki-conf_8h.html#a646b9304f6310b769e2e869b5173158b",
"group__attribute__mapper__scope__settings.html#ga4e8aece2c6e15950d8d92d1261a07b3f",
"group__attribute__store__type__registration.html#gga1c827b34925d83f45258ba05eb94f412a2ca4781df91b80808b52b1627b7c9593",
"group__ctimer.html#ga2f9b6e3e3b86848124f344c8d020ea35",
"group__dotdot__attribute__id__definitions.html#ga1eeac803c79133add8fdbbeb0e120796",
"group__dotdot__attribute__id__definitions.html#ga4405ef6ec0fa4cd5b6e49147cfddbe16",
"group__dotdot__attribute__id__definitions.html#ga6c520b8890c4979b8534ec00d285b203",
"group__dotdot__attribute__id__definitions.html#ga9ffacaac457958bcf2622bd14c1c45d5",
"group__dotdot__attribute__id__definitions.html#gac940b79d6c107ebc9135fa0985fae34d",
"group__dotdot__attribute__id__definitions.html#gaf318af74b680575c8a2e8bebb69032a5",
"group__dotdot__cluster__command__id__definitions.html#ga55ec6dbbe827e4a0d15c0ba8c06982ad",
"group__dotdot__cluster__command__id__definitions.html#gaee8f9d83ae3447d0ffe07c24561101ec",
"group__dotdot__mqtt.html#ga04367b6edf69d12e914bc66fc654dd78",
"group__dotdot__mqtt.html#ga0e43d89ad255f3940e93819dce29b43d",
"group__dotdot__mqtt.html#ga180468bd864dca18d882df08ae40cd3b",
"group__dotdot__mqtt.html#ga21081489ab88f2d48b54e414b0535c5c",
"group__dotdot__mqtt.html#ga2a18201c82d27b7b5bc33928a6c92928",
"group__dotdot__mqtt.html#ga33b88aea143f868e49e45bd15dfbea97",
"group__dotdot__mqtt.html#ga3cb40760f324eae80bd675ec0830b486",
"group__dotdot__mqtt.html#ga465584e2dd55edaac1e121ff23f97b13",
"group__dotdot__mqtt.html#ga50219fbfb2d3890982d4316742638c18",
"group__dotdot__mqtt.html#ga5a3a55424e97b76ead54a68fb6c57a51",
"group__dotdot__mqtt.html#ga65280865c4c26e8a40243bee5947109c",
"group__dotdot__mqtt.html#ga706ef998923474917cc72d7aaac4ff32",
"group__dotdot__mqtt.html#ga79b360568dd6ab278dd06c5bf2d649b2",
"group__dotdot__mqtt.html#ga83f0a82324bf2074e9bf9f9e9bac8722",
"group__dotdot__mqtt.html#ga8b9e5909c828b48d25ccf677e0d1f7dc",
"group__dotdot__mqtt.html#ga95b0a084b9fe2e4180de2581103c1bf6",
"group__dotdot__mqtt.html#ga9fc6bd9a5ccbe74de6dcc905bc95ca96",
"group__dotdot__mqtt.html#gaa9ee26abb2d479c939913a624c28d44a",
"group__dotdot__mqtt.html#gab429db0b02ec53e5e401d601ccaaf9bd",
"group__dotdot__mqtt.html#gabdf54cb54e758318d025767b697b9864",
"group__dotdot__mqtt.html#gac6c3e3b8c3632b6f5e196691ef54a2f7",
"group__dotdot__mqtt.html#gad07650708d2b91d833038c877279cde7",
"group__dotdot__mqtt.html#gadc2df6a1e2a48ceb70a89e7d819b45f1",
"group__dotdot__mqtt.html#gae5d1e34dd341dfba3dffc36ffdd784b1",
"group__dotdot__mqtt.html#gaee98c42725f362fcd4b8a94ee85b6b0c",
"group__dotdot__mqtt.html#gaf78b1ed184d07bd4a1073681b8b7f036",
"group__dotdot__mqtt.html#ggae480a1995ae14a588b13f99209b07997ac9bfa3b503f9c3ba4b40d918ca237ba2",
"group__dotdot__mqtt__attributes.html#ga162c19d0cf9378b2f7b1497db19c35d7",
"group__dotdot__mqtt__attributes.html#ga2eb485d37eb422da8671fa946259291d",
"group__dotdot__mqtt__attributes.html#ga4512f724168c3c21888770d4f4d05e12",
"group__dotdot__mqtt__attributes.html#ga5b9686f7929ceff316607e24dce7696b",
"group__dotdot__mqtt__attributes.html#ga6ed99bf80a9035ea75b5680060630e3b",
"group__dotdot__mqtt__attributes.html#ga876821d3f61891863168fea904e10c27",
"group__dotdot__mqtt__attributes.html#ga9b7ec37944076634ce4a72ac9a32baeb",
"group__dotdot__mqtt__attributes.html#gaae7299344046d64794fbf4b2898aa5a2",
"group__dotdot__mqtt__attributes.html#gac43895c41f24d2e3017a0e36f64e6e16",
"group__dotdot__mqtt__attributes.html#gad95c1411245ed94a79fe947440d81496",
"group__dotdot__mqtt__attributes.html#gaed09624411ada91b4ad4cd14e6dc6b09",
"group__dotdot__mqtt__command__helpers.html#ga0f2186d95f769f47a139af89100e001c",
"group__dotdot__mqtt__command__helpers.html#ga6603dcf8dc34f12c2cc01e83f677269b",
"group__dotdot__mqtt__command__helpers.html#gab9d556886649b5166d0cb0b06e495e05",
"group__dotdot__mqtt__generated__commands.html#ga139225303d16027daa423cfc0f456e49",
"group__dotdot__mqtt__generated__commands.html#gab4b78ded5a39fcc6cbcd0c5e4e4fabee",
"group__dotdot__mqtt__group__commands.html#ga14c7200bfaacfdb68b84a7c063314e79",
"group__dotdot__mqtt__group__commands.html#ga619bf63d977d784d6eb00b42ef865f77",
"group__dotdot__mqtt__group__commands.html#gaab219da98153c095f2c2479acb623db3",
"group__dotdot__mqtt__group__commands.html#gaf20a3cf1db93d2635f900177bff42728",
"group__dotdot__mqtt__helpers.html#ga47c7adde7e1c34266182be3720488f38",
"group__dotdot__mqtt__helpers.html#gaabe07058e0884ee30c30895544d8ec20",
"group__dotdot__mqtt__helpers__ccp.html#ga09d72bffbbbcf6ead8c709b76986cca1",
"group__dotdot__mqtt__helpers__ccp.html#ga618cb45811a7ec8c18f7e5692e1b289b",
"group__dotdot__mqtt__helpers__ccp.html#gac62efa593af5117f8d76b6b9fa04dbdf",
"group__dotdot__mqtt__hpp.html#ga14b134f3aa6c622b8923e6646d23f86c",
"group__dotdot__mqtt__hpp.html#ga448ddeb02d6e51dd448cd995b5dce974",
"group__dotdot__mqtt__hpp.html#ga736b209d0e75f58b34a92600e36d1434",
"group__dotdot__mqtt__hpp.html#gaa0a35dd169d3345374a61a52cb7ac109",
"group__dotdot__mqtt__hpp.html#gad263809abb78f68c1f43a15b5e927bb8",
"group__dotdot__mqtt__send__commands.html#ga077e45fd25faeece7b4c45391175def2",
"group__dotdot__mqtt__send__commands.html#ga61096a7a23a329921c6c046b1a1965f0",
"group__dotdot__mqtt__send__commands.html#gab91b4b865bb057170482217cc48c2950",
"group__dotdot__mqtt__supported__generated__commands.html#ga5419ed1cfd8643ba240597a3a3c04bac",
"group__list.html#ga89eaee4d473383726c7899f70d244cfe",
"group__process.html#ga52a36a03fb08b1708d84b421d2b3b962",
"group__timer.html#gaa10c14f8082ea8b10dfbefb81e1fa2e0",
"group__unify__config.html#ga814585e9e2b41f2f341c6368c39ba0c7",
"group__unify__dotdot__attribute__store__helpers.html#ga02557143c332b96fbc21427d08988a0c",
"group__unify__dotdot__attribute__store__helpers.html#ga078becb3f929ee616da79b507420101c",
"group__unify__dotdot__attribute__store__helpers.html#ga0c9deb03b2efc58ed5e9ae52dd4ea7eb",
"group__unify__dotdot__attribute__store__helpers.html#ga10fe9e67863e418f0b525e6c23897758",
"group__unify__dotdot__attribute__store__helpers.html#ga16f3271d446c9bc06971c6dbcb5470ec",
"group__unify__dotdot__attribute__store__helpers.html#ga1d2d04cf74a0a58027f5fca5f841a013",
"group__unify__dotdot__attribute__store__helpers.html#ga2373afc0f97e72b534922e348ac216cd",
"group__unify__dotdot__attribute__store__helpers.html#ga293d8d8e9c3d71b264acb6f9fcd52fe6",
"group__unify__dotdot__attribute__store__helpers.html#ga2eb5a8559211eea8bc3717e38725b8bb",
"group__unify__dotdot__attribute__store__helpers.html#ga33e23a4c7abb11e9913e82ebb4ac455d",
"group__unify__dotdot__attribute__store__helpers.html#ga3974b57e8a820556d6a30934e42495b6",
"group__unify__dotdot__attribute__store__helpers.html#ga3f133f569c88fc685fbf1efaea41b2cc",
"group__unify__dotdot__attribute__store__helpers.html#ga44f9471575ffa365c330588d84e5d546",
"group__unify__dotdot__attribute__store__helpers.html#ga4a4a074ffc8e3f66166e6abd2a0dffd8",
"group__unify__dotdot__attribute__store__helpers.html#ga4f3db567215d4aa5ca7c815b32e8ece5",
"group__unify__dotdot__attribute__store__helpers.html#ga553c467ee47887786aa436e21edb4bec",
"group__unify__dotdot__attribute__store__helpers.html#ga5ae1e4c1282df8d522d6daf1caa0242e",
"group__unify__dotdot__attribute__store__helpers.html#ga6076362cc65be3b58382cbd5844ebe21",
"group__unify__dotdot__attribute__store__helpers.html#ga668d250fcd706e1f39519ba88ef48021",
"group__unify__dotdot__attribute__store__helpers.html#ga6c3930141e0213fc9a8ca82389f780c2",
"group__unify__dotdot__attribute__store__helpers.html#ga70b2344a9928df90fae8eae29aeb9295",
"group__unify__dotdot__attribute__store__helpers.html#ga766fca2cdb1f7f800489154f4cada484",
"group__unify__dotdot__attribute__store__helpers.html#ga7c7819c56eb180002904a0c189a0b250",
"group__unify__dotdot__attribute__store__helpers.html#ga827ee52028a1beed79e6c781a522bd17",
"group__unify__dotdot__attribute__store__helpers.html#ga87093377d04239341ceb82dceceac92a",
"group__unify__dotdot__attribute__store__helpers.html#ga8d26d1d01d7af8d03a26ac553b70061b",
"group__unify__dotdot__attribute__store__helpers.html#ga915aeb2db23a9d1b372d969c8d623902",
"group__unify__dotdot__attribute__store__helpers.html#ga96f58cdcecc48a5180553071980a50ee",
"group__unify__dotdot__attribute__store__helpers.html#ga9c6bfe9e11b44ecf977de42efcb6dac4",
"group__unify__dotdot__attribute__store__helpers.html#gaa244f28690d12001f851a1fe68a6bf0e",
"group__unify__dotdot__attribute__store__helpers.html#gaa7ea4953093334cd83dfb6fc62c5df4f",
"group__unify__dotdot__attribute__store__helpers.html#gaacd815d82b013b35687871fd09164e9a",
"group__unify__dotdot__attribute__store__helpers.html#gab21677757bb1dbae7080f1c8e4c860ce",
"group__unify__dotdot__attribute__store__helpers.html#gab7eeb5c31cdbdb0ccf13a490f6899b93",
"group__unify__dotdot__attribute__store__helpers.html#gabce3712cb318667ba9bd07f6392f483c",
"group__unify__dotdot__attribute__store__helpers.html#gac35da6b06156c1f1b5735227eff53918",
"group__unify__dotdot__attribute__store__helpers.html#gac8fa6cfe4beb6d85b97f4feb2a663a7b",
"group__unify__dotdot__attribute__store__helpers.html#gacdb12e7aee4aeeba6f3a50b72bb9445f",
"group__unify__dotdot__attribute__store__helpers.html#gad36731a771f175f26d98a223761f591d",
"group__unify__dotdot__attribute__store__helpers.html#gad9347a4bcf24d2d368741cfe2f3f0b33",
"group__unify__dotdot__attribute__store__helpers.html#gadf012d0f6a834a7b21d743cc5abf80df",
"group__unify__dotdot__attribute__store__helpers.html#gae50794abb6ffc2358a9e594f82d84c7d",
"group__unify__dotdot__attribute__store__helpers.html#gaeadd4e2e21a77522ec466f671bf588b6",
"group__unify__dotdot__attribute__store__helpers.html#gaefca486f0cacda2d02f4557b8fd3c4e1",
"group__unify__dotdot__attribute__store__helpers.html#gaf56099b2e9ebc50f44e2033974c4f1c3",
"group__unify__dotdot__attribute__store__helpers.html#gaf9d68dd15478df4a401b4a5486cc0180",
"group__unify__enums.html#ga76d6526a5645a6986da09a98951a51c1",
"group__zap.html#ga10462f2c3a5a0824d72af3fc0375540c",
"group__zap.html#ga3c620882538a25a532ffe5a679397b54",
"group__zap.html#ga70c80a6710e678259f89689fb5053a30",
"group__zap.html#ga9d5444f67d3947e1a5f792fb9bd56599",
"group__zap.html#gacd0f161c72085fa7d189b631808cd57b",
"group__zap.html#gafa8d6fe3148e1fd5ddf296d880cad5bf",
"group__zap.html#gga1c2c15a01e1865ed963d376f77be646ea51b5367190252b2081f08228c224d784",
"group__zap.html#gga40ac8fb1660228196bd93446589455a6a828e5ee5030f41869d1dcf6b80f73ed1",
"group__zap.html#gga695680232fc4c33f7538696a47e1e56da7791737d737467d04bf30607fd6dd97f",
"group__zap.html#ggaa28a09e2ea7706b0a839eb599dce5731a245e0c11f66103e5dd32bf52523af396",
"group__zap.html#ggac7333edb654ee11b168cea5fbab852edaba271156820e1366e48c960a07d61c3c",
"group__zap.html#ggaed3bac47dc4e9d2702d525fd3b22d9d9a7daa3775899032ee617dd8721e379bd1",
"group__zigpc__common__zigbee.html#ga9b0796d928306f50c1f3d84341f180c2",
"group__zigpc__datastore__fixture.html#ga0ca51df2207735a1ec90283401cab46d",
"group__zigpc__group__mgmt__mqtt.html#gaa8bf47bdaaa1aa8eb43d7f764ca00980",
"group__zigpc__zcl__definitions.html#gga0d3a350256591a6dc3e8c8839a1cca72abf999cc334987d7bc7205414ae86eb9e",
"group__zigpc__zcl__definitions.html#gga3e6f3ce5edbbb40bc788f8904dd65eafa11efabb0d3c4d2811839ca728e76b130",
"group__zigpc__zcl__definitions.html#gga9c6f4d01fe3bbd394c751f5b132addb2a08078abe0abb337f68aa99cc92e57d42",
"group__zigpc__zcl__definitions.html#ggadb6b63bfa2aeeab84a233a7028c85bb0a21e9210e0729f538175e8f8fecea7704",
"mqtt__wrapper_8h.html#a206aa0050b08f5b5fd0f70be6c4c666f",
"namespacezigpc__ucl_1_1mqtt.html#a0ae3f2434936eef3e9855e9437c03292a0cde6a64c24ecd02af7c7f7807b6a351",
"struct__IasaceZoneStatusRecord.html",
"struct__uic__mqtt__dotdot__groups__supported__commands__.html#a8337106737e98b4ce74c0dbda568249b",
"structmessage__queue__element__t.html",
"structuic__mqtt__dotdot__application__monitoring__updated__state__t.html#ab916437040554dcae0807c819c8c4d87",
"structuic__mqtt__dotdot__color__control__command__step__hue__fields__t.html#a954d817e8a17e3ef30593c57b4bc12dc",
"structuic__mqtt__dotdot__door__lock__command__clear__year__day__schedule__fields__t.html#a163a7690b2f84ce4466fa6e480431e12",
"structuic__mqtt__dotdot__electrical__conductivity__measurement__updated__state__t.html#a73134f1420917fa24b56c8e766b017d7",
"structuic__mqtt__dotdot__electrical__measurement__updated__state__t.html#ad62158001694777457753cdcdb98e55d",
"structuic__mqtt__dotdot__metering__updated__state__t.html#a36a0a0f5367e136f2eb4431a962bd2f9",
"structuic__mqtt__dotdot__pressure__measurement__updated__state__t.html#af092f2298f3164674d5c348ba90d7be0",
"structuic__mqtt__dotdot__system__metrics__state__t.html#a8e54226f5bd761145f9b4c0000496f32",
"structuic__mqtt__dotdot__window__covering__state__t.html#ad19f9b95ba2a91a7ea4aa81a20e7ab7f",
"structzigpc__zclcmdparse__door__lock__get__weekday__schedule__response__t.html",
"unify__dotdot__attribute__store__command__callbacks__color__control_8c.html#a070ad9f4649cbbf7bbdbeef3b79d736d",
"unionzigpc__zclcmdparse__callback__data__t.html#a0b25dd27a6c1616a4978f2d07464e516",
"zigpc__gateway__int_8h.html#a0f79885a40129c92a337ef0afa34c931"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';