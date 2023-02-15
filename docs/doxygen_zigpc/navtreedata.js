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
      [ "Getting the NCP WSTK going :", "md_applications_zigpc_Getting_Started.html#autotoc_md19", null ],
      [ "Debugging with visual studio code, Cleaning the code", "md_applications_zigpc_Getting_Started.html#autotoc_md20", null ]
    ] ],
    [ "ZigPC Developer Guide", "md_applications_zigpc_readme_developer.html", [
      [ "Build Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md22", [
        [ "Set up Gecko SDK", "md_applications_zigpc_readme_developer.html#autotoc_md23", null ]
      ] ],
      [ "Modifying Underlying SLC-based Zigbee Host Library", "md_applications_zigpc_readme_developer.html#autotoc_md24", [
        [ "Cross Compiling for Raspberry Pi Using Docker", "md_applications_zigpc_readme_developer.html#autotoc_md25", null ],
        [ "Compiling", "md_applications_zigpc_readme_developer.html#autotoc_md26", null ],
        [ "Advanced Feature - Run Raspberry Pi Unit Tests in Docker on the Host", "md_applications_zigpc_readme_developer.html#autotoc_md27", null ]
      ] ],
      [ "Install Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md28", null ],
      [ "Reading Console Logs", "md_applications_zigpc_readme_developer.html#autotoc_md29", [
        [ "Changing Log Levels", "md_applications_zigpc_readme_developer.html#autotoc_md30", null ]
      ] ],
      [ "Using Zigbee Network Analyzer to Monitor PAN", "md_applications_zigpc_readme_developer.html#autotoc_md31", [
        [ "Installing Simplicity Studio v5", "md_applications_zigpc_readme_developer.html#autotoc_md32", null ],
        [ "Detecting Devices in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md33", [
          [ "Connection via Ethernet Interface", "md_applications_zigpc_readme_developer.html#autotoc_md34", null ],
          [ "Connection via Serial USB Interface", "md_applications_zigpc_readme_developer.html#autotoc_md35", null ]
        ] ],
        [ "Capturing Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md36", null ],
        [ "Decrypting Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md37", null ],
        [ "Adding custom clusters", "md_applications_zigpc_readme_developer.html#autotoc_md38", null ]
      ] ]
    ] ],
    [ "ZigPC User's Guide", "md_applications_zigpc_readme_user.html", [
      [ "Overview", "md_applications_zigpc_readme_user.html#autotoc_md40", null ],
      [ "Hardware Prerequisites", "md_applications_zigpc_readme_user.html#autotoc_md41", [
        [ "Gateway Host Device: Raspberry Pi", "md_applications_zigpc_readme_user.html#autotoc_md42", null ],
        [ "Gateway NCP Device", "md_applications_zigpc_readme_user.html#autotoc_md43", null ],
        [ "PAN Zigbee Device", "md_applications_zigpc_readme_user.html#autotoc_md44", null ]
      ] ],
      [ "Installing ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md45", [
        [ "Description of Installed Components", "md_applications_zigpc_readme_user.html#autotoc_md46", [
          [ "Mosquitto MQTT System Service", "md_applications_zigpc_readme_user.html#autotoc_md47", null ],
          [ "Mosquitto MQTT Clients: mosquitto_pub, mosquitto_sub", "md_applications_zigpc_readme_user.html#autotoc_md48", null ],
          [ "uic-upvl", "md_applications_zigpc_readme_user.html#autotoc_md49", null ],
          [ "uic-gms", "md_applications_zigpc_readme_user.html#autotoc_md50", null ],
          [ "uic-image-provider", "md_applications_zigpc_readme_user.html#autotoc_md51", null ],
          [ "zigpc", "md_applications_zigpc_readme_user.html#autotoc_md52", null ]
        ] ]
      ] ],
      [ "Starting ZigPC on the Command Line", "md_applications_zigpc_readme_user.html#autotoc_md53", null ],
      [ "Using ZigPC in a multiprotocol configuration", "md_applications_zigpc_readme_user.html#autotoc_md54", [
        [ "NOTE", "md_applications_zigpc_readme_user.html#autotoc_md55", null ]
      ] ],
      [ "Understanding ZigPC Functions", "md_applications_zigpc_readme_user.html#autotoc_md56", [
        [ "Device Addressing", "md_applications_zigpc_readme_user.html#autotoc_md57", null ],
        [ "Zigbee Node Commissioning", "md_applications_zigpc_readme_user.html#autotoc_md58", [
          [ "Z3 Install Code-Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md59", null ],
          [ "Well-Known-Key Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md60", null ]
        ] ],
        [ "Device State Updates", "md_applications_zigpc_readme_user.html#autotoc_md61", null ],
        [ "Device Discovery", "md_applications_zigpc_readme_user.html#autotoc_md62", null ],
        [ "Device Command Relaying", "md_applications_zigpc_readme_user.html#autotoc_md63", null ],
        [ "Device GeneratedCommand Updates", "md_applications_zigpc_readme_user.html#autotoc_md64", null ],
        [ "Device Attribute Updates", "md_applications_zigpc_readme_user.html#autotoc_md65", null ],
        [ "Group Message Handling", "md_applications_zigpc_readme_user.html#autotoc_md66", null ],
        [ "Zigbee OTA Firmware Upgrade Support", "md_applications_zigpc_readme_user.html#autotoc_md67", null ],
        [ "Zigbee Sleepy End Device Support", "md_applications_zigpc_readme_user.html#autotoc_md68", null ],
        [ "Protocol Controller Diagnostics", "md_applications_zigpc_readme_user.html#autotoc_md69", null ],
        [ "Updating NCP Firmware", "md_applications_zigpc_readme_user.html#autotoc_md70", null ]
      ] ],
      [ "Getting Started with Including Zigbee 3.0 Device to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md71", [
        [ "Adding Zigbee Device to the ZigPC Network via Install-Code Method", "md_applications_zigpc_readme_user.html#autotoc_md72", null ],
        [ "Adding Zigbee Device to the ZigPC Network using Well-Known Key", "md_applications_zigpc_readme_user.html#autotoc_md73", null ],
        [ "Discovering UCL Cluster Command Support for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md74", null ],
        [ "Discovering UCL Cluster Attribute Updates for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md75", null ],
        [ "Sending a UCL Cluster Command to Z3Device to see Attribute Update", "md_applications_zigpc_readme_user.html#autotoc_md76", null ],
        [ "Removing a Device Already in the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md77", null ],
        [ "Updating Zigbee Device Firmware already on the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md78", [
          [ "Instructions on how to start an OTA update", "md_applications_zigpc_readme_user.html#autotoc_md79", null ],
          [ "Once the Unify Image Provider recognizes changes to images.json, the following actions will occur", "md_applications_zigpc_readme_user.html#autotoc_md80", null ]
        ] ],
        [ "Retrieving Diagnostic information from ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md81", [
          [ "Steps to enable this behaviour", "md_applications_zigpc_readme_user.html#autotoc_md82", null ]
        ] ]
      ] ],
      [ "Technical Specifications", "md_applications_zigpc_readme_user.html#autotoc_md83", [
        [ "Device UNID Format", "md_applications_zigpc_readme_user.html#autotoc_md84", null ],
        [ "Recognized SmartStart DSK Format", "md_applications_zigpc_readme_user.html#autotoc_md85", null ],
        [ "Zigbee Device Support", "md_applications_zigpc_readme_user.html#autotoc_md86", null ]
      ] ],
      [ "Appendix", "md_applications_zigpc_readme_user.html#autotoc_md87", [
        [ "Setting Up the Zigbee NCP Connected to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md88", [
          [ "Using Pre-built NCP FW Images", "md_applications_zigpc_readme_user.html#autotoc_md89", null ],
          [ "Building NCP FW Images Using Studio", "md_applications_zigpc_readme_user.html#autotoc_md90", null ]
        ] ],
        [ "Setting Up the Sample Zigbee Z3Light FW Image", "md_applications_zigpc_readme_user.html#autotoc_md91", null ],
        [ "Flashing Zigbee Devices", "md_applications_zigpc_readme_user.html#autotoc_md92", null ]
      ] ]
    ] ],
    [ "ZigPC Release Notes", "md_applications_zigpc_release_notes.html", [
      [ "[1.3.0] - February 2023", "md_applications_zigpc_release_notes.html#autotoc_md94", [
        [ "Added (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md95", null ],
        [ "Changed (1.3.0)", "md_applications_zigpc_release_notes.html#autotoc_md96", null ]
      ] ],
      [ "[1.2.0] - August 2022", "md_applications_zigpc_release_notes.html#autotoc_md97", [
        [ "Added (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md98", null ],
        [ "Changed (1.2.0)", "md_applications_zigpc_release_notes.html#autotoc_md99", null ]
      ] ],
      [ "[1.1.1] - March 2022", "md_applications_zigpc_release_notes.html#autotoc_md100", [
        [ "Added (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md101", null ],
        [ "Changed (1.1.1)", "md_applications_zigpc_release_notes.html#autotoc_md102", null ]
      ] ],
      [ "[1.1.0] - Feb 2022", "md_applications_zigpc_release_notes.html#autotoc_md103", [
        [ "Added (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md104", null ],
        [ "Changed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md105", null ],
        [ "Removed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md106", null ],
        [ "Fixed (1.1.0)", "md_applications_zigpc_release_notes.html#autotoc_md107", null ]
      ] ],
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md108", [
        [ "Added (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md109", null ],
        [ "Changed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md110", null ],
        [ "Fixed (1.0.3)", "md_applications_zigpc_release_notes.html#autotoc_md111", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md112", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md113", [
        [ "Fixed (1.0.1)", "md_applications_zigpc_release_notes.html#autotoc_md114", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md115", [
        [ "Added (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md116", null ],
        [ "Changed (1.0.0)", "md_applications_zigpc_release_notes.html#autotoc_md117", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md118", [
        [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md119", null ]
      ] ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md120", null ]
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
"attribute__transitions__process_8h.html#aab9668e4e3f013669ffca12c8c6ea174",
"classattribute__store__node.html#aa4973c0b5c3057e07ae98bf9893e09eb",
"classzigpc__cpu__load__metric.html#a9753497baede2d78b12ef4db0154721a",
"dir_96a9893d53ad312a5db6d5573602d229.html",
"gecko_2uic__main__externals__platform_8c.html#ae1033412830b39863acb315244949a9f",
"group__attribute__poll.html#ga715a207c32f656bf97fd0865f73b37dd",
"group__attribute__store__validation.html#ga5ad72a1b835af5b98718003fddcc4e5a",
"group__datastore__attributes.html#gaa8fc59088848ce0bb08d78a8b0fca5d5",
"group__dotdot__attribute__id__definitions.html#ga23fde2c2872efff624ae6f0823c575e7",
"group__dotdot__attribute__id__definitions.html#ga4cfec568f904b4c4af5a3f0ba26ea662",
"group__dotdot__attribute__id__definitions.html#ga752ed94e5eb1fb8570d980621264aded",
"group__dotdot__attribute__id__definitions.html#gaa99359c0715b087293f116c00612990b",
"group__dotdot__attribute__id__definitions.html#gad43dd816f2ef67d17cf0943b418c39d1",
"group__dotdot__attribute__id__definitions.html#gafedff56744f1d28452cd01e32227469d",
"group__dotdot__cluster__command__id__definitions.html#ga8b6c17868cadb1d5747d0ab55407476b",
"group__dotdot__cluster__id__definitions.html#ga681396ef7af7fbc6b35c6928c67632db",
"group__dotdot__mqtt.html#ga079f8c30209f8319364ada2d7e13fd1c",
"group__dotdot__mqtt.html#ga11c0645e95d8c0cec0cabf3cf2925629",
"group__dotdot__mqtt.html#ga1b0b9f6bac39a735830904050d7fc40a",
"group__dotdot__mqtt.html#ga23a7472c8d6cd867524f76131a849369",
"group__dotdot__mqtt.html#ga2d2f5b011720deb82169f30749cf58eb",
"group__dotdot__mqtt.html#ga36b400c7e97c087560913a6d9e454d8a",
"group__dotdot__mqtt.html#ga4050509f87fbf6a89bf0fc4428a6422a",
"group__dotdot__mqtt.html#ga4a1b7fdb41c533c0943544de5ed1003d",
"group__dotdot__mqtt.html#ga53385ec3f4e158a897d6f6e6ad196fe9",
"group__dotdot__mqtt.html#ga5d54479e5ae588e6345e20f4fe114ae9",
"group__dotdot__mqtt.html#ga67ac4cb04252daaf8346250ab3c9221f",
"group__dotdot__mqtt.html#ga725b6aac2abaed550abcd02534bca314",
"group__dotdot__mqtt.html#ga7bdef345ebacb4bb5144c70af4ef6165",
"group__dotdot__mqtt.html#ga85799c9cbec8d4684ee56df94dcb2464",
"group__dotdot__mqtt.html#ga8d0fdeb144f5e0eeade68ee81a760ca0",
"group__dotdot__mqtt.html#ga96e51796ab19b2229651123be6974338",
"group__dotdot__mqtt.html#gaa1ec344d03c92406abed597ac4a811f1",
"group__dotdot__mqtt.html#gaac67ea84210de361c85850fa01d2ab7f",
"group__dotdot__mqtt.html#gab730c5deba064369a1c159ca5be92d98",
"group__dotdot__mqtt.html#gac019bcda3420751e4feeb22daeb65c88",
"group__dotdot__mqtt.html#gac8a054a025b514af22a048088c3dac9c",
"group__dotdot__mqtt.html#gad304e1b22a11360dcfc739287704c487",
"group__dotdot__mqtt.html#gadd3dd9bc3b5ce89124fc3737e99ae848",
"group__dotdot__mqtt.html#gae70c0faf9f2e56bd3473a37b602a269c",
"group__dotdot__mqtt.html#gaf01eac1c2636aa19b0d638d0acc17feb",
"group__dotdot__mqtt.html#gaf89cd1caacc6c55df7881d38b4611da0",
"group__dotdot__mqtt__attributes.html#ga013f40b7f058135df0921ee2156415a4",
"group__dotdot__mqtt__attributes.html#ga19dcae2f748239ca3a5f5b5824d40453",
"group__dotdot__mqtt__attributes.html#ga31d73582cd5053cc594b5955f3ef736f",
"group__dotdot__mqtt__attributes.html#ga472f690b70ba75c4c9831f143c62623a",
"group__dotdot__mqtt__attributes.html#ga5ce6088cfa2d4b4a481e859d32f9db5c",
"group__dotdot__mqtt__attributes.html#ga720c9ec90d217b03d742d34a37446b2c",
"group__dotdot__mqtt__attributes.html#ga89283b0b16fdf04c13cc2218df395cdf",
"group__dotdot__mqtt__attributes.html#ga9e20b4717ec2e6e08d5d9e2c1a14b8ef",
"group__dotdot__mqtt__attributes.html#gab0d832d357c60118c5cc2f6ec115aba0",
"group__dotdot__mqtt__attributes.html#gac6adb784762c7f5638443c79f78f1c8d",
"group__dotdot__mqtt__attributes.html#gadc373af19979d661d3e9762eb05007ee",
"group__dotdot__mqtt__attributes.html#gaf1750af44ab7b92806ca450a57aef3a6",
"group__dotdot__mqtt__command__helpers.html#ga19782ffac01123f17a1e0f4843c9ba97",
"group__dotdot__mqtt__command__helpers.html#ga71a3231583e54e2e48d90af3c769e6e2",
"group__dotdot__mqtt__command__helpers.html#gabc112c3548b284372ee3d43e99e9988e",
"group__dotdot__mqtt__generated__commands.html#ga16cce5eeb5037c3c7c9fdba594222919",
"group__dotdot__mqtt__generated__commands.html#gab54d0c286221077ac67319d2ce6ee0fe",
"group__dotdot__mqtt__group__commands.html#ga106e8b1078d6e4c3a3e9c0db4db737ed",
"group__dotdot__mqtt__group__commands.html#ga579a126cd4396ad8c92bffdb3e8f6cf0",
"group__dotdot__mqtt__group__commands.html#gaa1b4e538588b712673e1d04449c88e3c",
"group__dotdot__mqtt__group__commands.html#gae7d0b64c593438109d80aac070991f8e",
"group__dotdot__mqtt__helpers.html#ga39d799337544784443a97482d13c7dc4",
"group__dotdot__mqtt__helpers.html#ga9b8fd07ec9b5b4d6ed9cd99899e4a8f8",
"group__dotdot__mqtt__helpers.html#gaf99ec03b1849866e414787dcc5673512",
"group__dotdot__mqtt__helpers__ccp.html#ga4e94c770e1147e22f5c86c2315d0d434",
"group__dotdot__mqtt__helpers__ccp.html#gab9179a3969b00a01b0a14034d0241bd4",
"group__dotdot__mqtt__hpp.html#ga0991491bd9330d97c1fad192bd33bfd3",
"group__dotdot__mqtt__hpp.html#ga3f5bd7e3aa2f56ffa78b8b7c385e4b7a",
"group__dotdot__mqtt__hpp.html#ga6f8e3de4c3cb25f622ce8443b5d80e61",
"group__dotdot__mqtt__hpp.html#ga9ee972e3e6922036c5a44f8084446786",
"group__dotdot__mqtt__hpp.html#gacf295e9c48da7d248b4c85c46239c092",
"group__dotdot__mqtt__send__commands.html#ga0205ae41806cbcda480c9558032dc5a7",
"group__dotdot__mqtt__send__commands.html#ga54ab38ffc0acc91f87205dfe0d40df4c",
"group__dotdot__mqtt__send__commands.html#gaa760a0c9de63b971fb9f1f59a1dfb550",
"group__dotdot__mqtt__supported__generated__commands.html#ga0520bce1dbfbd109795fc5e535112d6d",
"group__image__listener.html",
"group__process.html#ga03302c412b77c7e2a8c5b3515e0ad93a",
"group__sl__log.html#ga0ef5fd78a051792b1a266b5ed90481c7",
"group__unify__attribute__timeouts.html",
"group__unify__dotdot__attribute__store__helpers.html#ga02a099de08e0d1e6721748136ad74844",
"group__unify__dotdot__attribute__store__helpers.html#ga091670497ce618072d6966c3c03a2baa",
"group__unify__dotdot__attribute__store__helpers.html#ga0f9f3787d8eb8cc7114e27d6bddf7f5e",
"group__unify__dotdot__attribute__store__helpers.html#ga172bbd5dee6597fa216136e7d7c491b1",
"group__unify__dotdot__attribute__store__helpers.html#ga1f25bd221ac841202d015f2a4759da9d",
"group__unify__dotdot__attribute__store__helpers.html#ga26d1934261cbca4ca880f481cdb0de89",
"group__unify__dotdot__attribute__store__helpers.html#ga2e70aef13c723e2f9da75565e90e79c6",
"group__unify__dotdot__attribute__store__helpers.html#ga3598a588fac962329e480502229dca38",
"group__unify__dotdot__attribute__store__helpers.html#ga3d8e049b7298fd5b2cf56e536b475dd6",
"group__unify__dotdot__attribute__store__helpers.html#ga44b6418f624122f898333b15880fc2b5",
"group__unify__dotdot__attribute__store__helpers.html#ga4c3ddf6cd13cf0bc02cdca068cc3c805",
"group__unify__dotdot__attribute__store__helpers.html#ga533a684eb27dc579e57fd1477a3de340",
"group__unify__dotdot__attribute__store__helpers.html#ga5a73bde2fdadf71d761c4ab3175cc4c7",
"group__unify__dotdot__attribute__store__helpers.html#ga61a67dbca5f8039d04b3ab7cdc9bedac",
"group__unify__dotdot__attribute__store__helpers.html#ga69a2c5493018bdf6ba844a15303daad0",
"group__unify__dotdot__attribute__store__helpers.html#ga7050099669f60dd2a1af2f2def04bd5f",
"group__unify__dotdot__attribute__store__helpers.html#ga7807d0e2062947be82ba5ec03ff5433b",
"group__unify__dotdot__attribute__store__helpers.html#ga7fdad20fcac64235c1a09d643669f7b3",
"group__unify__dotdot__attribute__store__helpers.html#ga86b3720bc0fc0cb9a9bf01946f632f8f",
"group__unify__dotdot__attribute__store__helpers.html#ga8e05f82b19a9309cbf9390478fd3caf1",
"group__unify__dotdot__attribute__store__helpers.html#ga956411adf14d68f74a1ab1540b3b6017",
"group__unify__dotdot__attribute__store__helpers.html#ga9c049ca37a0c51fe084ad9acc9686bcc",
"group__unify__dotdot__attribute__store__helpers.html#gaa3d7f1ecc0cc35cf0963106179c57bcf",
"group__unify__dotdot__attribute__store__helpers.html#gaab66b18adc95f1e0b4a8198708fdbae7",
"group__unify__dotdot__attribute__store__helpers.html#gab2333b1c3b78c9cd2f904059039bf0fb",
"group__unify__dotdot__attribute__store__helpers.html#gaba8f30af5286d249de59079d4521d3b5",
"group__unify__dotdot__attribute__store__helpers.html#gac13650ae8ab36b44ed42ebf0b4f5e42d",
"group__unify__dotdot__attribute__store__helpers.html#gac9617854887cb6a420abb276cd47ef7a",
"group__unify__dotdot__attribute__store__helpers.html#gacf84f0298c69aa8bd4a01de26b14528a",
"group__unify__dotdot__attribute__store__helpers.html#gad7fc081b0b589b164346678bcde52e90",
"group__unify__dotdot__attribute__store__helpers.html#gadf8d0ccab84df4240c48486b6d6d1072",
"group__unify__dotdot__attribute__store__helpers.html#gae745fc4259c551294ea52d316b908d5c",
"group__unify__dotdot__attribute__store__helpers.html#gaee9da60851edd77bea53fcd5ac328c4f",
"group__unify__dotdot__attribute__store__helpers.html#gaf5e42a4b4cdb03dee3050d07f023dd14",
"group__unify__dotdot__attribute__store__helpers.html#gafcb021a2e95e6a683dd82d2b61c124d8",
"group__unify__sl__status__codes.html#gadd61042225a3bf66c2cd41664e0b597a",
"group__zap.html#ga275bdcd58ed9b51562c1da9d757a9972",
"group__zap.html#ga5239853b2dc69399009569d3170f345f",
"group__zap.html#ga860f16ee7ed6daa0a168d3558fc6e8b7",
"group__zap.html#gab928c17e2b9156edfc9e3bc45b3412fd",
"group__zap.html#gae4fc2e3cdedc0c931b12835be59f5fac",
"group__zap.html#gga0703769274fa2e73c6ddd6ca5c6fd90cabfa659ade0404c2426554dd211418d3e",
"group__zap.html#gga40ac8fb1660228196bd93446589455a6a048bc95ddd1234f7bfc4dec694de20e2",
"group__zap.html#gga461021d9b02ff549602cfc67d97278eea7d125b289023b3716fee3382169eb384",
"group__zap.html#gga87e02574c9dd27418af62d6b9349b9aba20197189c18c33c5eba2ede8338ad95e",
"group__zap.html#ggab35de7cd6192eb0d3288dbaba7f39bf8ad48dcbd8ddece06d059f02500b60baed",
"group__zap.html#ggadf764cbdea00d65edcd07bb9953ad2b7a64b33c0bc1a1b6c7832cd4be7ccfb536",
"group__zigpc__common__zigbee.html#ga0a3a7102d7c9fe56015e848f2d0965ea",
"group__zigpc__common__zigbee.html#ggaa9947bc2df97f9a535a56b2da30c4ba9a23fcc8386d12ef28ff79345a494d26c2",
"group__zigpc__gateway.html#gaddae89c536ca2647bfdf8b9c024896a6",
"group__zigpc__stdin.html#gafbfaa6fc4f7d22ee5ec12a6373aae7f2",
"group__zigpc__zcl__definitions.html#gga22a23b1d6a8445410cc511764179b661a4d7406e530bbf4c66de44e566feda408",
"group__zigpc__zcl__definitions.html#gga548dfa4f1afc477f2655bae3e31fba6fa470bbaa1527794f7f6d468a7940e015b",
"group__zigpc__zcl__definitions.html#ggaa03d837e0450405c4857b29e7b64d894a3f4803f61a7975a1f8d57ab00c025480",
"md_applications_zigpc_readme_developer.html#autotoc_md25",
"namespaceuic__ota.html#a9f805f5bda1874c3b0f87a6db76f1596",
"procinit_8c.html#ab63dfcdd3c420e54251f2dbb067a1df5",
"struct__uic__mqtt__dotdot__color__control__supported__commands__.html#a2401e1c42763b9e7535d8468e7640806",
"structast_1_1assignment.html#af241ec288241c04879d9d47b4190835e",
"structuic__mqtt__dotdot__alarms__updated__state__t.html",
"structuic__mqtt__dotdot__carbon__monoxide__updated__state__t.html#a229f2eaa89794ebc35bd60c7be654a2f",
"structuic__mqtt__dotdot__device__temperature__configuration__state__t.html",
"structuic__mqtt__dotdot__door__lock__command__unlock__door__response__fields__t.html#a50f1d23ff6224ca65784234fa99055a8",
"structuic__mqtt__dotdot__electrical__measurement__updated__state__t.html",
"structuic__mqtt__dotdot__level__command__move__fields__t.html#ac0bdc1ae49c7df68149d5b12eaa37b8d",
"structuic__mqtt__dotdot__ota__upgrade__state__t.html#a810f68cc2fff682ac61563cc3e19a794",
"structuic__mqtt__dotdot__pump__configuration__and__control__updated__state__t.html#a0b454c994b86b1b65130b3039c1ff63d",
"structuic__mqtt__dotdot__thermostat__command__setpoint__raise__or__lower__fields__t.html#aff84ee2a15d32b5481b22e4fd1403bd2",
"structzigbee__node__t.html#af975a7c74ca9d85ef76cdb027a62987a",
"structzigpc__zclcmdparse__ias__zone__zone__status__change__notification__t.html#a5feed95476f53bb5bd8c2bf38350528d",
"unify__dotdot__attribute__store__force__read__attributes__command__callbacks_8c.html#a1caca4840e3888b611a78aa71e6225d4",
"zigpc__config_8c.html#a5242548808dc8bd7e4d5eb50fc8f0bdc",
"zigpc__ucl_8hpp.html#a0ae3f2434936eef3e9855e9437c03292aeb9d70b8abb8cc33ddb33743c528c28c"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';