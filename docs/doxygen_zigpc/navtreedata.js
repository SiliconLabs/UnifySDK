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
        [ "Added (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md86", null ],
        [ "Changed (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md87", null ]
      ] ],
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md88", [
        [ "Added (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md89", null ],
        [ "Changed (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md90", null ]
      ] ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md91", [
        [ "Added (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md92", null ],
        [ "Changed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md93", null ],
        [ "Removed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md94", null ],
        [ "Fixed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md95", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md96", [
        [ "Added (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md97", null ],
        [ "Changed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md98", null ],
        [ "Fixed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md99", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md100", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md101", [
        [ "Fixed (1.0.1)", "md_applications_zigpc_release_notes.html#autotoc_md102", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md103", [
        [ "Added (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md104", null ],
        [ "Changed (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md105", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md106", [
        [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md107", null ]
      ] ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md108", null ]
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
"cc_8h.html#a79624e93060c0dbee5ba2542d79cb442",
"classmqtt__client__fsm__disconnected.html#ae537271051121623995b00ba72327ec0",
"classzigpc__discovery_1_1events_1_1EndpointDiscoverRequest.html#aaaba75ad9d906e8d3d225c8529ad344d",
"dir_f5fefb264d93c1374fa462a4eb0f63a6.html",
"globals_func.html",
"group__attribute__store__api.html#ga9b1c2d32e839ca14103b5997e1c85b7e",
"group__attribute__store__value__helpers.html#gafbd8072f005dacc2ee797b01f8b46089",
"group__dotdot__cluster__command__id__definitions.html#ga3e169913e4deb8bd82149d9537d90b73",
"group__dotdot__cluster__command__id__definitions.html#gacf7710c252e10d056b08ac6b75cd9bbb",
"group__dotdot__mqtt.html#ga039a37db9500d0c5aa28422fa9ce0d9b",
"group__dotdot__mqtt.html#ga182c4957a0d001527289b93f26f3a4d2",
"group__dotdot__mqtt.html#ga2b10586207ce8ba204386c91f13b1faf",
"group__dotdot__mqtt.html#ga4050509f87fbf6a89bf0fc4428a6422a",
"group__dotdot__mqtt.html#ga56a4346ad8da1f560d30c683d304464d",
"group__dotdot__mqtt.html#ga6bd734261255e0e2526602132788e7f5",
"group__dotdot__mqtt.html#ga801377ebf87fc1016a0174d087be0e1f",
"group__dotdot__mqtt.html#ga9365ea3ddf5b76e408fce943ac58b819",
"group__dotdot__mqtt.html#gaa9cc96536601ccbc15efa9ab69d15d42",
"group__dotdot__mqtt.html#gabfe2d30fe6326d6bfa69f70f45fabd89",
"group__dotdot__mqtt.html#gad6da7680e09d035802317459503dc028",
"group__dotdot__mqtt.html#gaec5a2266a1895375e8cb48787d501a41",
"group__dotdot__mqtt.html#gaffe765a4569c90f83386b76a00bb6900",
"group__dotdot__mqtt__attributes.html#ga15fcffe6bea18a1f999b2734325a10eb",
"group__dotdot__mqtt__attributes.html#ga2ce41f0613479df5a0c6a765cc537993",
"group__dotdot__mqtt__attributes.html#ga44559be19e83a92ce064a1d240746dd8",
"group__dotdot__mqtt__attributes.html#ga5b7d74b3f350851a569e8b68620b771e",
"group__dotdot__mqtt__attributes.html#ga6eca2150a2da9073b97a7c5ddd4ef8f9",
"group__dotdot__mqtt__attributes.html#ga876821d3f61891863168fea904e10c27",
"group__dotdot__mqtt__attributes.html#ga9be6f010b24749e1e3fcb925502896e5",
"group__dotdot__mqtt__attributes.html#gaae9c49cdbeafac6474d44d23c30b031e",
"group__dotdot__mqtt__attributes.html#gac4a0a166b3ee9bd442f79c88b2986af0",
"group__dotdot__mqtt__attributes.html#gada5dd6d11f4dfc55dcbdf112824b954e",
"group__dotdot__mqtt__attributes.html#gaedda5cddfc5236470b44981e5cd5fa38",
"group__dotdot__mqtt__command__helpers.html#ga12e5a494dbc1add7dce99e0d4d081378",
"group__dotdot__mqtt__command__helpers.html#ga691ea1f9f355b97d6771936b85c701cf",
"group__dotdot__mqtt__command__helpers.html#gab8da66f5d68fea4995f7e9ac99c35b53",
"group__dotdot__mqtt__generated__commands.html#ga0ceff17cd19dafe08b29a1fdee0b6d16",
"group__dotdot__mqtt__generated__commands.html#gaa74fab3963de957fd83493041c4e6b15",
"group__dotdot__mqtt__group__commands.html#ga0bd4b460e0219619e0d40467784a4613",
"group__dotdot__mqtt__group__commands.html#ga53f8f1fdcd64b784f16253e79699b2e6",
"group__dotdot__mqtt__group__commands.html#ga9e924e417542bcb84bb8f0f931c0ea8e",
"group__dotdot__mqtt__group__commands.html#gae3d39bb4198c233978438ae99f112288",
"group__dotdot__mqtt__helpers.html#ga30c26fe3a227a2b9974c407a3bf2c04b",
"group__dotdot__mqtt__helpers.html#ga94c86a16b6aa42c4b3ff7f0dc2b24afd",
"group__dotdot__mqtt__helpers.html#gaf12d81e888f31d80700b5326e68f1cf9",
"group__dotdot__mqtt__helpers__ccp.html#ga42af5f50ca30d1c9f9c0a8e8f0edb7df",
"group__dotdot__mqtt__helpers__ccp.html#gaa78ceb0e24a4ce4669937d8633140976",
"group__dotdot__mqtt__helpers__ccp.html#gafedf1a0836f21b22df7970560a1f1504",
"group__dotdot__mqtt__hpp.html#ga310090a074a1476340b05a3068b9db44",
"group__dotdot__mqtt__hpp.html#ga61c2675bd6eeb272ca3da48d03c1bbd8",
"group__dotdot__mqtt__hpp.html#ga907570cd3965895b3162d702b2be1b10",
"group__dotdot__mqtt__hpp.html#gac327b9f5f707eb16dd40c0487d2f319f",
"group__dotdot__mqtt__hpp.html#gaf58a528b9e31605484fab0cf5435e4b9",
"group__dotdot__mqtt__send__commands.html#ga389f2e57a85b7625c8d365c40caabe95",
"group__dotdot__mqtt__send__commands.html#ga91184afe450eb327ed5aef451f2192ce",
"group__dotdot__mqtt__send__commands.html#gae7e6a8ea901f6232991f36300adc7071",
"group__dotdot__mqtt__supported__generated__commands.html#gaffe9bad263bdd4b612def9f727d9d839",
"group__ota__zigbee__int.html#ga220c936d32c304ae5365705f5ff51a51",
"group__rt.html#gabd8aed15a76f89a2b3ff298acd3ca938",
"group__unify__config.html#ga3f4fb187fb7daba01f86175d1f1e791d",
"group__zap.html#ga021245238d4767e0b51e3a6a0d90d1bb",
"group__zap.html#ga06fc87d81c62e9abb8790b6e5713c55bac170879671ab39aac16609ebc54368c9",
"group__zap.html#ga25d13cc5e5554906dc67adc01817c613",
"group__zap.html#ga4cd11b716f0d4678d57011c070b35abf",
"group__zap.html#ga7e6aff90f7d8b445844d15c24ba23676",
"group__zap.html#gaafc46de1558bd2fff69049fd2e7b21ca",
"group__zap.html#gadb27e0679e6e191b04201aced7f282af",
"group__zap.html#gga0703769274fa2e73c6ddd6ca5c6fd90ca13f544c66c634ffa1a2f8ecc866dc244",
"group__zap.html#gga34210d70a37ec4c7afbdfb4ed1d7d032ac1d65d8b9882bc6a0057fcb7716199b3",
"group__zap.html#gga40ac8fb1660228196bd93446589455a6ae53ff2775a3d5c7cb9c30bb91aac139d",
"group__zap.html#gga6e0ef59f77db82b77121159ead810f84ab0a9f9b76f2e6c64395b0f1115daa664",
"group__zap.html#ggaa3ff17a179862e659a861c5c8a23eaf7ac054b12e7459b0ae25a33cd9c0628bca",
"group__zap.html#ggacf7123a18858fd623d4445be23ad34acaa970d3c0db08905b49c2fec64626efd1",
"group__zigpc__common__zigbee.html#ga208ae04359d1f884fb2b2110d3c8cfca",
"group__zigpc__datastore.html#ga10faddb37f581f12a69757138e00be40",
"group__zigpc__gateway__process__send.html#ga680359ee568dff338095bd8dcfc32c0f",
"group__zigpc__zcl__definitions.html#gga08072ae07724a3bca8be16e010e8175eacc599b7a8471e086da9779379a0e858b",
"group__zigpc__zcl__definitions.html#gga22a23b1d6a8445410cc511764179b661ad7b18b9222430618bbd7f16e8a85b6ad",
"group__zigpc__zcl__definitions.html#gga548dfa4f1afc477f2655bae3e31fba6fae193e3b477333e4c55a496926fd8f390",
"group__zigpc__zcl__definitions.html#ggaa03d837e0450405c4857b29e7b64d894aec3be83a11fad811672cb285531f149d",
"mqtt__test__helper_8h.html",
"rust__process_8c.html#a327067cd8fc2b53dbd6e9d15c1c83429",
"struct__uic__mqtt__dotdot__door__lock__supported__commands__.html#a6019398f9aed7b3cce8f5e82dc0eb25b",
"structast_1_1signed__.html",
"structuic__mqtt__dotdot__aox__position__estimation__updated__state__t.html",
"structuic__mqtt__dotdot__color__control__command__move__to__color__temperature__fields__t.html#a7a25ae35f6e1f4afd374033a9a9f3375",
"structuic__mqtt__dotdot__diagnostics__updated__state__t.html#a7d4013a21544a92a2019d55018cf70fc",
"structuic__mqtt__dotdot__door__lock__updated__state__t.html#a564a60c5b09326639df9e04b06675362",
"structuic__mqtt__dotdot__electrical__measurement__updated__state__t.html#a7aeff0a971126f677f1b29b139b3ad65",
"structuic__mqtt__dotdot__level__updated__state__t.html#ac1fc44650d118317829421318afb426c",
"structuic__mqtt__dotdot__power__configuration__state__t.html#a3a0473d5f7ce8ea5dc84c2353d49fe79",
"structuic__mqtt__dotdot__scenes__command__enhanced__view__scene__response__fields__t.html",
"structuic__mqtt__dotdot__thermostat__updated__state__t.html#a598aec4ea53251e5791b910b8ea027e7",
"structzigpc__group__member__t.html",
"uic__mqtt_8c.html#a93d71c4598a61093a8ff94ea21956731",
"zigpc__discovery_8hpp.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';