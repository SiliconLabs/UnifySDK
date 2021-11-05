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
    [ "Z3Gateway API Readme", "md_applications_zigpc_components_z3gateway_README.html", [
      [ "File structure", "md_applications_zigpc_components_z3gateway_README.html#autotoc_md11", null ],
      [ "Modifications to Z3Gateway Project", "md_applications_zigpc_components_z3gateway_README.html#autotoc_md12", [
        [ "Makefile changes", "md_applications_zigpc_components_z3gateway_README.html#autotoc_md13", null ]
      ] ],
      [ "GSDK EmberAF Host File Changes", "md_applications_zigpc_components_z3gateway_README.html#autotoc_md14", [
        [ "GSDK Linux Serial File Changes", "md_applications_zigpc_components_z3gateway_README.html#autotoc_md15", null ],
        [ "Remove CLI interface functionality", "md_applications_zigpc_components_z3gateway_README.html#autotoc_md16", null ],
        [ "List of GSDK Zigbee Plugins Used in API exposed", "md_applications_zigpc_components_z3gateway_README.html#autotoc_md17", null ]
      ] ]
    ] ],
    [ "ZigPC (Beta) Developer Guide", "md_applications_zigpc_readme_developer.html", [
      [ "Build Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md31", [
        [ "Set up Gecko SDK", "md_applications_zigpc_readme_developer.html#autotoc_md32", null ],
        [ "Cross Compiling for Raspberry Pi Using Docker", "md_applications_zigpc_readme_developer.html#autotoc_md33", null ],
        [ "Compiling", "md_applications_zigpc_readme_developer.html#autotoc_md34", null ],
        [ "Advanced Feature - Run Raspberry Pi Unit Tests in Docker on the Host", "md_applications_zigpc_readme_developer.html#autotoc_md35", null ]
      ] ],
      [ "Install Instructions", "md_applications_zigpc_readme_developer.html#autotoc_md36", null ],
      [ "Reading Console Logs", "md_applications_zigpc_readme_developer.html#autotoc_md37", [
        [ "Changing Log Levels", "md_applications_zigpc_readme_developer.html#autotoc_md38", null ]
      ] ],
      [ "Using Zigbee Network Analyzer to Monitor PAN", "md_applications_zigpc_readme_developer.html#autotoc_md39", [
        [ "Installing Simplicity Studio v5", "md_applications_zigpc_readme_developer.html#autotoc_md40", null ],
        [ "Detecting Devices in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md41", [
          [ "Connection via Ethernet Interface", "md_applications_zigpc_readme_developer.html#autotoc_md42", null ],
          [ "Connection via Serial USB Interface", "md_applications_zigpc_readme_developer.html#autotoc_md43", null ]
        ] ],
        [ "Capturing Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md44", null ],
        [ "Decrypting Network Traffic in Simplicity Studio", "md_applications_zigpc_readme_developer.html#autotoc_md45", null ]
      ] ]
    ] ],
    [ "ZigPC (Beta) User's Guide", "md_applications_zigpc_readme_user.html", [
      [ "Overview", "md_applications_zigpc_readme_user.html#autotoc_md47", null ],
      [ "Hardware Prerequisites", "md_applications_zigpc_readme_user.html#autotoc_md48", [
        [ "Gateway Host Device: Raspberry Pi", "md_applications_zigpc_readme_user.html#autotoc_md49", null ],
        [ "Gateway NCP Device", "md_applications_zigpc_readme_user.html#autotoc_md50", null ],
        [ "PAN Zigbee Device", "md_applications_zigpc_readme_user.html#autotoc_md51", null ]
      ] ],
      [ "Installing ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md52", [
        [ "Description of Installed Components", "md_applications_zigpc_readme_user.html#autotoc_md53", [
          [ "Mosquitto MQTT System Service", "md_applications_zigpc_readme_user.html#autotoc_md54", null ],
          [ "Mosquitto MQTT Clients: mosquitto_pub, mosquitto_sub", "md_applications_zigpc_readme_user.html#autotoc_md55", null ],
          [ "libuic.so", "md_applications_zigpc_readme_user.html#autotoc_md56", null ],
          [ "uic-upvl", "md_applications_zigpc_readme_user.html#autotoc_md57", null ],
          [ "uic-angel", "md_applications_zigpc_readme_user.html#autotoc_md58", null ],
          [ "uic-image-provider", "md_applications_zigpc_readme_user.html#autotoc_md59", null ],
          [ "ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md60", null ]
        ] ]
      ] ],
      [ "Starting ZigPC on the Command Line", "md_applications_zigpc_readme_user.html#autotoc_md61", null ],
      [ "Understanding ZigPC Functions", "md_applications_zigpc_readme_user.html#autotoc_md62", [
        [ "Device Addressing", "md_applications_zigpc_readme_user.html#autotoc_md63", null ],
        [ "Zigbee Node Commissioning", "md_applications_zigpc_readme_user.html#autotoc_md64", [
          [ "Z3 Install Code-Based Provisioning", "md_applications_zigpc_readme_user.html#autotoc_md65", null ]
        ] ],
        [ "Device State Updates", "md_applications_zigpc_readme_user.html#autotoc_md66", null ],
        [ "Device Discovery", "md_applications_zigpc_readme_user.html#autotoc_md67", null ],
        [ "Device Command Relaying", "md_applications_zigpc_readme_user.html#autotoc_md68", null ],
        [ "Device Attribute Updates", "md_applications_zigpc_readme_user.html#autotoc_md69", null ],
        [ "Group Message Handling", "md_applications_zigpc_readme_user.html#autotoc_md70", null ],
        [ "Zigbee OTA Firmware Upgrade Support", "md_applications_zigpc_readme_user.html#autotoc_md71", null ],
        [ "Zigbee Sleepy End Device Support", "md_applications_zigpc_readme_user.html#autotoc_md72", null ]
      ] ],
      [ "Getting Started with Including Zigbee 3.0 Device to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md73", [
        [ "Adding Zigbee Device to the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md74", null ],
        [ "Discovering UCL Cluster Command Support for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md75", null ],
        [ "Discovering UCL Cluster Attribute Updates for Z3Device", "md_applications_zigpc_readme_user.html#autotoc_md76", null ],
        [ "Sending a UCL Cluster Command to Z3Device to see Attribute Update", "md_applications_zigpc_readme_user.html#autotoc_md77", null ],
        [ "Removing a Device Already in the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md78", null ],
        [ "Updating Zigbee Device Firmware already on the ZigPC Network", "md_applications_zigpc_readme_user.html#autotoc_md79", [
          [ "Instructions on how to start an OTA update:", "md_applications_zigpc_readme_user.html#autotoc_md80", null ],
          [ "Once the Unify Image Provider recognizes changes to images.json, the following actions will occur:", "md_applications_zigpc_readme_user.html#autotoc_md81", null ]
        ] ]
      ] ],
      [ "Technical Specifications", "md_applications_zigpc_readme_user.html#autotoc_md82", [
        [ "Device UNID Format", "md_applications_zigpc_readme_user.html#autotoc_md83", null ],
        [ "Recognized SmartStart DSK Format", "md_applications_zigpc_readme_user.html#autotoc_md84", null ],
        [ "Zigbee Device Support", "md_applications_zigpc_readme_user.html#autotoc_md85", null ]
      ] ],
      [ "Appendix", "md_applications_zigpc_readme_user.html#autotoc_md86", [
        [ "Setting Up the Zigbee NCP Connected to ZigPC", "md_applications_zigpc_readme_user.html#autotoc_md87", [
          [ "Using Pre-built NCP FW Images", "md_applications_zigpc_readme_user.html#autotoc_md88", null ],
          [ "Building NCP FW Images Using Studio", "md_applications_zigpc_readme_user.html#autotoc_md89", null ]
        ] ],
        [ "Setting Up the Sample Zigbee Z3Light FW Image", "md_applications_zigpc_readme_user.html#autotoc_md90", null ],
        [ "Flashing Zigbee Devices", "md_applications_zigpc_readme_user.html#autotoc_md91", null ]
      ] ]
    ] ],
    [ "ZigPC (Beta) Release Notes", "md_applications_zigpc_release_notes.html", [
      [ "[1.0.3] - Nov 2021", "md_applications_zigpc_release_notes.html#autotoc_md93", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md94", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md95", null ],
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md96", null ]
      ] ],
      [ "[1.0.2] - Oct 2021", "md_applications_zigpc_release_notes.html#autotoc_md97", null ],
      [ "[1.0.1] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md98", [
        [ "Fixed", "md_applications_zigpc_release_notes.html#autotoc_md99", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zigpc_release_notes.html#autotoc_md100", [
        [ "Added", "md_applications_zigpc_release_notes.html#autotoc_md101", null ],
        [ "Changed", "md_applications_zigpc_release_notes.html#autotoc_md102", null ]
      ] ],
      [ "Known Issues", "md_applications_zigpc_release_notes.html#autotoc_md103", null ],
      [ "Zigbee Certification information", "md_applications_zigpc_release_notes.html#autotoc_md105", null ]
    ] ],
    [ "CMake coding standard", "md_doc_standards_cmake-standard.html", null ],
    [ "Unify Coding Standard", "md_doc_standards_coding-standard.html", [
      [ "1 Introduction", "md_doc_standards_coding-standard.html#autotoc_md109", [
        [ "1.1 Purpose of this document", "md_doc_standards_coding-standard.html#autotoc_md110", null ],
        [ "1.2 Latest version of this document", "md_doc_standards_coding-standard.html#autotoc_md111", null ],
        [ "1.3 Changing this document", "md_doc_standards_coding-standard.html#autotoc_md112", [
          [ "Zigbee Devices Used for Testing", "md_applications_zigpc_release_notes.html#autotoc_md104", null ],
          [ "1.3.1 Versioning", "md_doc_standards_coding-standard.html#autotoc_md113", null ],
          [ "1.3.2 Review committee", "md_doc_standards_coding-standard.html#autotoc_md114", null ]
        ] ],
        [ "1.4 Structure and wording of this document", "md_doc_standards_coding-standard.html#autotoc_md115", [
          [ "1.4.1 Source Code", "md_doc_standards_coding-standard.html#autotoc_md116", null ],
          [ "1.4.2 Required", "md_doc_standards_coding-standard.html#autotoc_md117", null ],
          [ "1.4.3 Recommended", "md_doc_standards_coding-standard.html#autotoc_md118", null ],
          [ "1.4.4 Should", "md_doc_standards_coding-standard.html#autotoc_md119", null ]
        ] ],
        [ "1.5 Application of this standard", "md_doc_standards_coding-standard.html#autotoc_md120", [
          [ "1.5.1 New modules", "md_doc_standards_coding-standard.html#autotoc_md121", null ],
          [ "1.5.2 Existing modules", "md_doc_standards_coding-standard.html#autotoc_md122", null ]
        ] ]
      ] ],
      [ "2 General Guidelines", "md_doc_standards_coding-standard.html#autotoc_md123", [
        [ "2.1 General Guidelines", "md_doc_standards_coding-standard.html#autotoc_md124", [
          [ "2.1.1 Use peer review", "md_doc_standards_coding-standard.html#autotoc_md125", null ],
          [ "2.1.3 Be a Boy Scout", "md_doc_standards_coding-standard.html#autotoc_md126", null ],
          [ "2.1.4 Avoid reinventing the Wheel", "md_doc_standards_coding-standard.html#autotoc_md127", null ],
          [ "2.1.5 Refactor code", "md_doc_standards_coding-standard.html#autotoc_md128", null ]
        ] ],
        [ "2.2 Testing", "md_doc_standards_coding-standard.html#autotoc_md129", [
          [ "2.2.1 Testability", "md_doc_standards_coding-standard.html#autotoc_md130", null ],
          [ "2.2.2 Regression Test", "md_doc_standards_coding-standard.html#autotoc_md131", null ]
        ] ],
        [ "2.3 Embedded Coding", "md_doc_standards_coding-standard.html#autotoc_md132", [
          [ "2.3.1 Write energy friendly code", "md_doc_standards_coding-standard.html#autotoc_md133", null ],
          [ "2.3.2 Interrupts", "md_doc_standards_coding-standard.html#autotoc_md134", null ]
        ] ]
      ] ],
      [ "3 C Specific Guidelines", "md_doc_standards_coding-standard.html#autotoc_md135", [
        [ "3.1 Compilers", "md_doc_standards_coding-standard.html#autotoc_md136", [
          [ "3.1.1 C standards version (Required)", "md_doc_standards_coding-standard.html#autotoc_md137", null ],
          [ "3.1.1.1 C++ standards version (Required)", "md_doc_standards_coding-standard.html#autotoc_md138", null ],
          [ "3.1.2 Support mixing C++ and C (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md139", null ],
          [ "3.1.3 Multiple build tool support (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md140", null ],
          [ "3.1.4 Compile with warnings (Required)", "md_doc_standards_coding-standard.html#autotoc_md141", null ],
          [ "3.1.5 Treat warnings as errors (Required)", "md_doc_standards_coding-standard.html#autotoc_md142", null ]
        ] ],
        [ "3.2 Preprocessor", "md_doc_standards_coding-standard.html#autotoc_md143", [
          [ "3.2.1 Use header file guards (Required)", "md_doc_standards_coding-standard.html#autotoc_md144", null ],
          [ "3.2.2 Preprocessor conditional compilation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md145", null ],
          [ "3.2.3 Use of preprocessor directive #error (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md146", null ],
          [ "3.2.4 Use of preprocessor directive #include (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md147", null ]
        ] ],
        [ "3.2.5 Use of preprocessor directive <tt>#undef</tt> (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md148", null ],
        [ "3.2.6 Use of preprocessor 'stringify' and concatenation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md149", null ],
        [ "3.2.7 General use of preprocessor  (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md150", null ],
        [ "3.3 Standard Libraries", "md_doc_standards_coding-standard.html#autotoc_md151", [
          [ "3.3.1 Use of stdint.h and stdbool.h (Required)", "md_doc_standards_coding-standard.html#autotoc_md152", null ],
          [ "3.3.2 Use of stdlib (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md153", null ],
          [ "3.3.3 Use of dynamic memory / the heap (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md154", null ],
          [ "3.3.4 Do not use the standard C assert() (Required)", "md_doc_standards_coding-standard.html#autotoc_md155", null ]
        ] ],
        [ "3.4 Keywords", "md_doc_standards_coding-standard.html#autotoc_md156", [
          [ "3.4.1 The \"volatile\" keyword (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md157", null ],
          [ "3.4.2 The \"inline\" keyword (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md158", null ],
          [ "3.4.3 The \"static\" keyword (Required)", "md_doc_standards_coding-standard.html#autotoc_md159", null ]
        ] ],
        [ "3.5 Data Types", "md_doc_standards_coding-standard.html#autotoc_md160", [
          [ "3.5.1 Avoid enums in arrays, structures, or unions (Required)", "md_doc_standards_coding-standard.html#autotoc_md161", null ],
          [ "3.5.2 Avoid bitfields in structures/unions (Required)", "md_doc_standards_coding-standard.html#autotoc_md162", null ]
        ] ],
        [ "3.6 Variables", "md_doc_standards_coding-standard.html#autotoc_md163", [
          [ "3.6.1 Using global variables (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md164", null ]
        ] ],
        [ "3.7 Functions", "md_doc_standards_coding-standard.html#autotoc_md165", [
          [ "3.7.1 Prototype functions (Required)", "md_doc_standards_coding-standard.html#autotoc_md166", null ],
          [ "3.7.2 Functions return data types (Required)", "md_doc_standards_coding-standard.html#autotoc_md167", [
            [ "Exceptions", "md_doc_standards_coding-standard.html#autotoc_md168", null ]
          ] ],
          [ "3.7.3 Return values should always be checked and propagated (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md169", null ],
          [ "3.7.4 Functions replicating a standard API can follow that API (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md170", null ]
        ] ],
        [ "3.8 Macros", "md_doc_standards_coding-standard.html#autotoc_md171", [
          [ "3.8.1 Macros with side-effects (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md172", null ],
          [ "3.8.2 Macros with statement(s) (Required)", "md_doc_standards_coding-standard.html#autotoc_md173", null ],
          [ "3.8.3 Functional macros with argument(s) (Required)", "md_doc_standards_coding-standard.html#autotoc_md174", null ]
        ] ],
        [ "3.9 goto statements", "md_doc_standards_coding-standard.html#autotoc_md175", [
          [ "3.9.1 <tt>goto</tt> statements should only be used for cleanup purposes or early exit in case of an error, when there is no simple workaround (Required)", "md_doc_standards_coding-standard.html#autotoc_md176", null ],
          [ "3.9.2 <tt>goto</tt>s should only refer to a label declared after them (Required)", "md_doc_standards_coding-standard.html#autotoc_md177", null ],
          [ "3.9.3 <tt>goto</tt>s should only refer to a static label located in the same function (Required)", "md_doc_standards_coding-standard.html#autotoc_md178", null ],
          [ "3.9.4 Any label referenced by a <tt>goto</tt> need to be declared in the same block or a block enclosing the <tt>goto</tt> (Required)", "md_doc_standards_coding-standard.html#autotoc_md179", null ]
        ] ],
        [ "3.10 Libraries", "md_doc_standards_coding-standard.html#autotoc_md180", [
          [ "3.10.1 Dependencies (Required)", "md_doc_standards_coding-standard.html#autotoc_md181", null ],
          [ "3.10.2 Including third party code (Required)", "md_doc_standards_coding-standard.html#autotoc_md182", null ],
          [ "3.10.3 Configuring libraries in source form (Required)", "md_doc_standards_coding-standard.html#autotoc_md183", null ],
          [ "3.10.4 Configuring libraries in binary form (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md184", null ]
        ] ],
        [ "3.11 Misc", "md_doc_standards_coding-standard.html#autotoc_md185", [
          [ "3.11.1 Avoid embedding assignments in expressions (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md186", null ]
        ] ]
      ] ],
      [ "4 Coding style and formatting", "md_doc_standards_coding-standard.html#autotoc_md187", [
        [ "4.1 General formatting", "md_doc_standards_coding-standard.html#autotoc_md188", [
          [ "4.1.1 Use spaces, not tabs (Required)", "md_doc_standards_coding-standard.html#autotoc_md189", null ],
          [ "4.1.2 Indents are 2 spaces per level (Required)", "md_doc_standards_coding-standard.html#autotoc_md190", [
            [ "4.1.2.1 Preprocessor indentation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md191", null ]
          ] ],
          [ "4.1.3 Lines should not be longer than 80 characters (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md192", null ],
          [ "4.1.4 Line endings (Required)", "md_doc_standards_coding-standard.html#autotoc_md193", null ],
          [ "4.1.5 Use only plain ASCII or UTF-8 (Required)", "md_doc_standards_coding-standard.html#autotoc_md194", null ],
          [ "4.1.6 Use ISO8601 formatting for dates (Required)", "md_doc_standards_coding-standard.html#autotoc_md195", null ],
          [ "4.1.7 Inserting empty and blank lines (Required)", "md_doc_standards_coding-standard.html#autotoc_md196", null ],
          [ "4.1.8 Use parentheses liberally (Required)", "md_doc_standards_coding-standard.html#autotoc_md197", null ],
          [ "4.1.9 Break up long expressions (Required)", "md_doc_standards_coding-standard.html#autotoc_md198", null ],
          [ "4.1.10 <tt>goto</tt> labels should be on column 1 (Required)", "md_doc_standards_coding-standard.html#autotoc_md199", null ]
        ] ],
        [ "4.2 Commenting code (Required)", "md_doc_standards_coding-standard.html#autotoc_md200", [
          [ "4.2.1 Use C++ style comments (\"//\")", "md_doc_standards_coding-standard.html#autotoc_md201", null ]
        ] ],
        [ "4.3 Bracing style (Required)", "md_doc_standards_coding-standard.html#autotoc_md202", null ],
        [ "4.4 Switch statements and labels", "md_doc_standards_coding-standard.html#autotoc_md203", [
          [ "4.4.1 Using labels (Required)", "md_doc_standards_coding-standard.html#autotoc_md204", null ],
          [ "4.4.2 Labels with block (Required)", "md_doc_standards_coding-standard.html#autotoc_md205", null ],
          [ "4.4.3 Switch statements (Required)", "md_doc_standards_coding-standard.html#autotoc_md206", null ]
        ] ],
        [ "4.5 Functions, operators and C keywords", "md_doc_standards_coding-standard.html#autotoc_md207", [
          [ "4.5.1 Listing function parameters (Required)", "md_doc_standards_coding-standard.html#autotoc_md208", null ],
          [ "4.5.2 Using function parentheses (Required)", "md_doc_standards_coding-standard.html#autotoc_md209", null ],
          [ "4.5.3 Binary and ternary operators (Required)", "md_doc_standards_coding-standard.html#autotoc_md210", null ],
          [ "4.5.4 Use a single space after C keywords (Required)", "md_doc_standards_coding-standard.html#autotoc_md211", null ],
          [ "4.5.5 Additional space within expressions", "md_doc_standards_coding-standard.html#autotoc_md212", null ],
          [ "4.5.6 Identify do-nothing code (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md213", null ],
          [ "4.5.7 Pointer asterisk position (Required)", "md_doc_standards_coding-standard.html#autotoc_md214", null ],
          [ "4.5.8 Don't mix pointer and value type declarations (Required)", "md_doc_standards_coding-standard.html#autotoc_md215", null ]
        ] ],
        [ "4.6 Naming", "md_doc_standards_coding-standard.html#autotoc_md216", [
          [ "4.6.1 General Considerations", "md_doc_standards_coding-standard.html#autotoc_md217", [
            [ "4.6.1.1 Use meaningful names (Required)", "md_doc_standards_coding-standard.html#autotoc_md218", null ],
            [ "4.6.1.2 Avoid abbreviations (Required)", "md_doc_standards_coding-standard.html#autotoc_md219", null ]
          ] ],
          [ "4.6.2 Namespaces (Required)", "md_doc_standards_coding-standard.html#autotoc_md220", null ],
          [ "4.6.3 Naming (Required)", "md_doc_standards_coding-standard.html#autotoc_md221", [
            [ "4.6.3.1 Casing (Required)", "md_doc_standards_coding-standard.html#autotoc_md222", null ],
            [ "4.6.3.2 Casing and acronyms (Required)", "md_doc_standards_coding-standard.html#autotoc_md223", null ]
          ] ],
          [ "4.6.4 Naming functions and variables", "md_doc_standards_coding-standard.html#autotoc_md224", [
            [ "4.6.4.1 Functions and variables names have the form verb_noun (Required)", "md_doc_standards_coding-standard.html#autotoc_md225", null ],
            [ "4.6.4.2 Function parameters should not have any prefix (Required)", "md_doc_standards_coding-standard.html#autotoc_md226", null ],
            [ "4.6.4.3 Function-local variables should not have any prefix (Required)", "md_doc_standards_coding-standard.html#autotoc_md227", null ],
            [ "4.6.4.4 Variable and Function Names Indicate Units for Time (Required)", "md_doc_standards_coding-standard.html#autotoc_md228", null ],
            [ "4.6.4.5 Functions/stubs called on specific events/callbacks should start 'on' in their name (Required)", "md_doc_standards_coding-standard.html#autotoc_md229", null ],
            [ "4.6.4.6 Interrupt handlers should be suffixed by 'IRQHandler' or 'irq_handler' (Required)", "md_doc_standards_coding-standard.html#autotoc_md230", null ],
            [ "4.6.4.7 Non-blocking functions executed periodically in a main loop should be suffixed by 'step' (Required)", "md_doc_standards_coding-standard.html#autotoc_md231", null ],
            [ "4.6.4.8 Functions that are tasks should be suffixed by 'task' (Required)", "md_doc_standards_coding-standard.html#autotoc_md232", null ]
          ] ],
          [ "4.6.5 Naming constants", "md_doc_standards_coding-standard.html#autotoc_md233", [
            [ "4.6.5.1 Constants should use upper case (Required)", "md_doc_standards_coding-standard.html#autotoc_md234", null ]
          ] ],
          [ "4.6.6 Naming function-like macros", "md_doc_standards_coding-standard.html#autotoc_md235", [
            [ "4.6.6.1 Follow the naming convention for regular functions (Required)", "md_doc_standards_coding-standard.html#autotoc_md236", null ],
            [ "4.6.6.2 Use all caps for macros that can't be functions (Required)", "md_doc_standards_coding-standard.html#autotoc_md237", null ]
          ] ],
          [ "4.6.7 Naming types", "md_doc_standards_coding-standard.html#autotoc_md238", [
            [ "4.6.7.1 Public typedefs (Required)", "md_doc_standards_coding-standard.html#autotoc_md239", null ],
            [ "4.6.7.2 Structure fields should be snake_case, without any prefixes (Required)", "md_doc_standards_coding-standard.html#autotoc_md240", null ],
            [ "4.6.7.3 Type from typedef (Optional)", "md_doc_standards_coding-standard.html#autotoc_md241", null ]
          ] ],
          [ "4.6.8 Files and directory structure", "md_doc_standards_coding-standard.html#autotoc_md242", [
            [ "4.6.8.1 Filenames and directories use lower case (Required)", "md_doc_standards_coding-standard.html#autotoc_md243", null ],
            [ "4.6.8.2 Avoid duplicate filenames", "md_doc_standards_coding-standard.html#autotoc_md244", null ],
            [ "4.6.8.3 File names (Required)", "md_doc_standards_coding-standard.html#autotoc_md245", null ],
            [ "4.6.8.4 Directory names (Required)", "md_doc_standards_coding-standard.html#autotoc_md246", null ]
          ] ]
        ] ]
      ] ],
      [ "5 Documentation", "md_doc_standards_coding-standard.html#autotoc_md247", [
        [ "5.1 General", "md_doc_standards_coding-standard.html#autotoc_md248", null ],
        [ "5.2 Comments should answer the question \"Why?\" (Required)", "md_doc_standards_coding-standard.html#autotoc_md249", null ]
      ] ],
      [ "6 Doxygen Coding Style Guide", "md_doc_standards_coding-standard.html#autotoc_md250", [
        [ "6.1 File comment header (Required)", "md_doc_standards_coding-standard.html#autotoc_md251", null ],
        [ "6.2 Grouping modules (@addtogroup) (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md252", null ],
        [ "6.3 File/module level documentation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md253", null ],
        [ "6.4 Function documentation (Required)", "md_doc_standards_coding-standard.html#autotoc_md254", null ],
        [ "6.5 Variable documentation (Required)", "md_doc_standards_coding-standard.html#autotoc_md255", null ],
        [ "6.6 Header file vs. implementation file (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md256", null ],
        [ "6.7 Do not document sections (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md257", null ],
        [ "6.8 Do not release sections (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md258", null ]
      ] ],
      [ "7 Licensing", "md_doc_standards_coding-standard.html#autotoc_md259", [
        [ "7.1 Silicon Labs Licenses", "md_doc_standards_coding-standard.html#autotoc_md260", null ],
        [ "7.2 Third-Party Code", "md_doc_standards_coding-standard.html#autotoc_md261", null ]
      ] ],
      [ "8 C++ Specific Guidelines", "md_doc_standards_coding-standard.html#autotoc_md262", null ]
    ] ],
    [ "Known Abbreviations", "md_doc_standards_known-abbreviations.html", null ],
    [ "Unify SDK Developer Readme", "md_doc_readme_developer.html", [
      [ "Directory Structure and File Naming Conventions", "md_doc_readme_developer.html#autotoc_md265", [
        [ "Rules", "md_doc_readme_developer.html#autotoc_md266", null ],
        [ "Structure of Components (Best Practice)", "md_doc_readme_developer.html#autotoc_md267", null ],
        [ "Structure", "md_doc_readme_developer.html#autotoc_md268", null ]
      ] ],
      [ "Component Templates", "md_doc_readme_developer.html#autotoc_md269", [
        [ "Demo", "md_doc_readme_developer.html#autotoc_md270", null ]
      ] ],
      [ "Clang-Format (autoformat code)", "md_doc_readme_developer.html#autotoc_md271", [
        [ "Install on Mac", "md_doc_readme_developer.html#autotoc_md272", null ],
        [ "Install on Ubuntu", "md_doc_readme_developer.html#autotoc_md273", null ],
        [ "Visual Studio Code", "md_doc_readme_developer.html#autotoc_md274", null ],
        [ "Command Line", "md_doc_readme_developer.html#autotoc_md275", null ]
      ] ],
      [ "Pre-commit Hooks", "md_doc_readme_developer.html#autotoc_md276", null ],
      [ "CMake", "md_doc_readme_developer.html#autotoc_md277", [
        [ "Defining new uic-components", "md_doc_readme_developer.html#autotoc_md278", null ],
        [ "Mock uic-components", "md_doc_readme_developer.html#autotoc_md279", [
          [ "Additional headers", "md_doc_readme_developer.html#autotoc_md280", null ]
        ] ],
        [ "Unit-testing", "md_doc_readme_developer.html#autotoc_md281", [
          [ "DEPENDS", "md_doc_readme_developer.html#autotoc_md282", null ],
          [ "NAME", "md_doc_readme_developer.html#autotoc_md283", null ]
        ] ],
        [ "EXCLUDE", "md_doc_readme_developer.html#autotoc_md284", null ],
        [ "Stub", "md_doc_readme_developer.html#autotoc_md285", null ],
        [ "Do's and dont's", "md_doc_readme_developer.html#autotoc_md286", null ],
        [ "CMake File Formatting", "md_doc_readme_developer.html#autotoc_md287", null ],
        [ "Example", "md_doc_readme_developer.html#autotoc_md288", null ],
        [ "References", "md_doc_readme_developer.html#autotoc_md289", null ],
        [ "Code Coverage", "md_doc_readme_developer.html#autotoc_md290", [
          [ "Dependencies", "md_doc_readme_developer.html#autotoc_md291", null ]
        ] ]
      ] ]
    ] ],
    [ "Unify SDK Library Overview", "md_doc_overview.html", [
      [ "Base Platform", "md_doc_overview.html#autotoc_md293", [
        [ "Main loop", "md_doc_overview.html#autotoc_md294", null ],
        [ "Config System", "md_doc_overview.html#autotoc_md295", null ],
        [ "Logging System", "md_doc_overview.html#autotoc_md296", null ],
        [ "MQTT Client", "md_doc_overview.html#autotoc_md297", null ],
        [ "DotDot Serializer", "md_doc_overview.html#autotoc_md298", null ],
        [ "DotDot Attributes", "md_doc_overview.html#autotoc_md299", null ],
        [ "Datastore", "md_doc_overview.html#autotoc_md300", null ],
        [ "Attribute Store", "md_doc_overview.html#autotoc_md301", null ],
        [ "Attribute Mapper", "md_doc_overview.html#autotoc_md302", null ],
        [ "Console Interface", "md_doc_overview.html#autotoc_md303", null ],
        [ "SmartStart", "md_doc_overview.html#autotoc_md304", null ],
        [ "Node State Topic", "md_doc_overview.html#autotoc_md305", null ],
        [ "Protocol Controller MQTT", "md_doc_overview.html#autotoc_md306", null ],
        [ "OTA MQTT Client", "md_doc_overview.html#autotoc_md307", null ]
      ] ],
      [ "Test Framework", "md_doc_overview.html#autotoc_md308", null ],
      [ "Build System", "md_doc_overview.html#autotoc_md309", null ],
      [ "Packaging Framework", "md_doc_overview.html#autotoc_md310", null ],
      [ "Coding Standard", "md_doc_overview.html#autotoc_md311", null ]
    ] ],
    [ "Unify Dockerfile", "md_docker_readme_developer.html", [
      [ "Install Docker", "md_docker_readme_developer.html#autotoc_md313", null ],
      [ "Build Dockerfile Image", "md_docker_readme_developer.html#autotoc_md314", null ]
    ] ],
    [ "README to Make Debian Package for UIC Component (for Example ZPC)", "md_doc_README_debian_packaging.html", [
      [ "Function arguments", "md_doc_README_debian_packaging.html#autotoc_md316", null ],
      [ "1. PKG_NAME      component package Name", "md_doc_README_debian_packaging.html#autotoc_md317", null ],
      [ "2. PKG_FILE_NAME Debian package filename for the component", "md_doc_README_debian_packaging.html#autotoc_md318", null ],
      [ "3. PKG_DEPNDS    Component debian package depenedencies.", "md_doc_README_debian_packaging.html#autotoc_md319", null ],
      [ "4. PKG_EXTRA     Component debian package Control extra info", "md_doc_README_debian_packaging.html#autotoc_md320", null ],
      [ "See https://cmake.org/cmake/help/v3.6/module/CPackDeb.html for more information", "md_doc_README_debian_packaging.html#autotoc_md321", null ]
    ] ],
    [ "Unify SDK Library Developer Guide", "md_doc_readme_uic_application_developer.html", [
      [ "Creating a New Application", "md_doc_readme_uic_application_developer.html#autotoc_md323", null ],
      [ "Unify SDK Library (Unify Main) and IPC", "md_doc_readme_uic_application_developer.html#autotoc_md324", null ]
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
"af-main-host_8c.html",
"attribute__resolver__rule__internal_8h_source.html",
"classattribute__store__node.html#ab100340b6f1986b20cb04fab7aa4db86",
"contiki-conf_8h.html#aca1ff60ac76bc69388fc1c5adc31bc7f",
"device-table_8h.html#a4ca7acdd277aee0da9515bbf3a1fb09f",
"functions_vars_g.html",
"group__attribute__store__callbacks.html#ga1fab6588a92e807e06c92619abedf0e3",
"group__datastore.html#gaf5b54b66e8ded7f7ed747fd984b38a67",
"group__mqtt__client.html#ggaca6b39d8cd48e315ea885c3f715a7ff3a49d464ea759a6d1bec813df82f897130",
"group__ringbuf.html#ga5e40b0459813787a4e67fceb0aab5fd6",
"group__sl__status.html#gae87dd784423a5570abe6203d3a6bf904",
"group__z3gateway.html#gae0af3e528d757e64f6c87b4b060ec8e0",
"group__zigpc__common__zigbee.html#gab38787eaf2b5c3a95b418cb63b9273b4",
"group__zigpc__gateway.html#gaaf18d750c515daba01b6ee589903ca41",
"group__zigpc__zcl__util.html",
"md_doc_overview.html#autotoc_md303",
"namespaceimage__file__handler.html",
"struct____attribute____.html#aab3283ffa89d1092d571b22570fbd9ad",
"structz3gatewayDeviceTableStorageEntry.html#aa63f83e8db6942e72ab288bcb22561e6",
"uic__mqtt_8c.html#a93d71c4598a61093a8ff94ea21956731",
"zigpc__gateway__int_8h.html#a0d0fc19d35645beade8646d9369383be"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';