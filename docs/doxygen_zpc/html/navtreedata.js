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
  [ "Z-Wave Protocol Controller", "index.html", [
    [ "Introduction", "index.html#intro", null ],
    [ "References", "index.html#References", [
      [ "Z-Wave Alliance, Z-Wave Device Class Specification", "index.html#device_class_spec", null ],
      [ "Z-Wave Alliance, Application Command Class Specification", "index.html#application_cc_spec", null ],
      [ "Z-Wave Alliance, Management Command Class Specification", "index.html#management_cc_spec", null ],
      [ "Z-Wave Alliance, Transport-Encapsulation Command Class Specification", "index.html#transport_cc_spec", null ],
      [ "Z-Wave Alliance, Network-Protocol Command Class Specification", "index.html#network_cc_spec", null ],
      [ "Z-Wave Alliance, Z-Wave Plus Role Types Specification", "index.html#role_type_spec", null ],
      [ "Z-Wave Alliance, Z-Wave Plus Device Types Specification", "index.html#device_type_spec", null ],
      [ "Z-Wave Alliance, Z-Wave Plus v2 Device Type Specification", "index.html#device_type_spec_v2", null ],
      [ "Z-Wave Alliance, Z-Wave and Z-Wave Long Range Network Layer Specification", "index.html#zwave_nwk_spec", null ]
    ] ],
    [ "model_structure", "md_applications_zpc_components_zwave_command_classes_model_structure.html", null ],
    [ "generator.py", "md_applications_zpc_components_zwave_command_classes_README.html", null ],
    [ "ZPC Developer's Guide", "md_applications_zpc_readme_developer.html", [
      [ "Build Instructions", "md_applications_zpc_readme_developer.html#autotoc_md6", [
        [ "Cross Compiling for Raspberry Pi Using Docker", "md_applications_zpc_readme_developer.html#autotoc_md7", null ],
        [ "Advanced Feature - Run Raspberry Pi Unit Tests in Docker on the Host", "md_applications_zpc_readme_developer.html#autotoc_md8", null ]
      ] ],
      [ "Install Instructions", "md_applications_zpc_readme_developer.html#autotoc_md9", null ]
    ] ],
    [ "ZPC User's Guide", "md_applications_zpc_readme_user.html", [
      [ "Installation", "md_applications_zpc_readme_user.html#autotoc_md11", [
        [ "Files", "md_applications_zpc_readme_user.html#autotoc_md12", [
          [ "Configuration File", "md_applications_zpc_readme_user.html#autotoc_md13", null ],
          [ "Database File", "md_applications_zpc_readme_user.html#autotoc_md14", null ]
        ] ]
      ] ],
      [ "Running ZPC", "md_applications_zpc_readme_user.html#autotoc_md15", [
        [ "Running ZPC Using Systemd", "md_applications_zpc_readme_user.html#autotoc_md16", null ],
        [ "Running ZPC From the Command Line", "md_applications_zpc_readme_user.html#autotoc_md17", [
          [ "(Advanced) Configuring ZPC to Use TLS Certificate-Based Encryption", "md_applications_zpc_readme_user.html#autotoc_md18", [
            [ "Self Signed Certificate Generation", "md_applications_zpc_readme_user.html#autotoc_md19", null ],
            [ "Broker Configurations", "md_applications_zpc_readme_user.html#autotoc_md20", null ],
            [ "ZPC command line options", "md_applications_zpc_readme_user.html#autotoc_md21", null ]
          ] ]
        ] ]
      ] ],
      [ "Understanding how the ZPC Works", "md_applications_zpc_readme_user.html#autotoc_md22", null ],
      [ "Dotdot Cluster to Z-Wave Command Class Mapping", "md_applications_zpc_readme_user.html#autotoc_md23", null ],
      [ "Migrating to ZPC from Z/IP Gateway", "md_applications_zpc_readme_user.html#autotoc_md24", [
        [ "ZPC Database JSON Example", "md_applications_zpc_readme_user.html#autotoc_md25", null ]
      ] ],
      [ "Performing Backup and Restore", "md_applications_zpc_readme_user.html#autotoc_md26", null ],
      [ "Datastore Versioning", "md_applications_zpc_readme_user.html#autotoc_md27", [
        [ "Recovering a Database", "md_applications_zpc_readme_user.html#autotoc_md28", null ]
      ] ],
      [ "Performing Firmware Updates", "md_applications_zpc_readme_user.html#autotoc_md29", [
        [ "UIID Construction", "md_applications_zpc_readme_user.html#autotoc_md30", null ],
        [ "UIID/UNID Association", "md_applications_zpc_readme_user.html#autotoc_md31", null ],
        [ "Version String Calculation", "md_applications_zpc_readme_user.html#autotoc_md32", null ],
        [ "Triggering a Firmware Update", "md_applications_zpc_readme_user.html#autotoc_md33", null ],
        [ "Aborting a Firmware Update", "md_applications_zpc_readme_user.html#autotoc_md34", null ],
        [ "After a Successful Firmware Update", "md_applications_zpc_readme_user.html#autotoc_md35", null ],
        [ "Examples", "md_applications_zpc_readme_user.html#autotoc_md36", null ],
        [ "Possible Errors", "md_applications_zpc_readme_user.html#autotoc_md37", null ]
      ] ],
      [ "Z-Wave Certification Information", "md_applications_zpc_readme_user.html#autotoc_md38", [
        [ "Z-Wave Protocol Interoperability", "md_applications_zpc_readme_user.html#autotoc_md39", null ],
        [ "Device Type and Role Type", "md_applications_zpc_readme_user.html#autotoc_md40", null ],
        [ "Manufacturer-Specific Information", "md_applications_zpc_readme_user.html#autotoc_md41", null ],
        [ "Z-Wave Plus Info Information", "md_applications_zpc_readme_user.html#autotoc_md42", null ],
        [ "Network Management Information", "md_applications_zpc_readme_user.html#autotoc_md43", [
          [ "Network Management", "md_applications_zpc_readme_user.html#autotoc_md44", [
            [ "Adding a Node", "md_applications_zpc_readme_user.html#autotoc_md45", null ],
            [ "Removing a Node", "md_applications_zpc_readme_user.html#autotoc_md46", null ],
            [ "Management Operations for Individual Nodes", "md_applications_zpc_readme_user.html#autotoc_md47", null ]
          ] ]
        ] ],
        [ "Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md48", [
          [ "Association / Multi Channel Association Command Classes Information", "md_applications_zpc_readme_user.html#autotoc_md49", [
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md50", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md51", null ]
          ] ],
          [ "Basic Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md52", [
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md53", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md54", null ]
          ] ],
          [ "Battery Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md55", [
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md56", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md57", null ]
          ] ],
          [ "Binary Switch Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md58", [
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md59", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md60", null ]
          ] ],
          [ "Device Reset Locally Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md61", [
            [ "Door Lock Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md62", null ],
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md63", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md64", null ]
          ] ],
          [ "Indicator Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md65", [
            [ "Support", "md_applications_zpc_readme_user.html#autotoc_md66", null ],
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md67", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md68", null ]
          ] ],
          [ "Multilevel Switch Command Class Information", "md_applications_zpc_readme_user.html#autotoc_md69", [
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md70", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md71", null ]
          ] ],
          [ "Security 0 - Security 2 Command Classes Information", "md_applications_zpc_readme_user.html#autotoc_md72", [
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md73", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md74", null ]
          ] ],
          [ "Thermostat Setpoint and Thermostat Mode Command Classes Information", "md_applications_zpc_readme_user.html#autotoc_md75", [
            [ "Node status", "md_applications_zpc_readme_user.html#autotoc_md76", null ],
            [ "Sending commands", "md_applications_zpc_readme_user.html#autotoc_md77", null ]
          ] ]
        ] ],
        [ "SmartStart Information", "md_applications_zpc_readme_user.html#autotoc_md78", [
          [ "SmartStart List Format", "md_applications_zpc_readme_user.html#autotoc_md79", null ],
          [ "Modifying SmartStart List Entries", "md_applications_zpc_readme_user.html#autotoc_md80", null ],
          [ "Removing SmartStart List Entries", "md_applications_zpc_readme_user.html#autotoc_md81", null ],
          [ "Z-Wave Long Range Support", "md_applications_zpc_readme_user.html#autotoc_md82", null ],
          [ "SmartStart Supported Protocol Detection", "md_applications_zpc_readme_user.html#autotoc_md83", null ]
        ] ]
      ] ],
      [ "Transitioning from Z/IP Gateway + Z-Ware", "md_applications_zpc_readme_user.html#autotoc_md84", null ]
    ] ],
    [ "ZPC Release Notes", "md_applications_zpc_release_notes.html", [
      [ "[1.0.2] - Oct 2021", "md_applications_zpc_release_notes.html#autotoc_md86", [
        [ "Added", "md_applications_zpc_release_notes.html#autotoc_md87", null ],
        [ "Changed", "md_applications_zpc_release_notes.html#autotoc_md88", null ],
        [ "Fixed", "md_applications_zpc_release_notes.html#autotoc_md89", null ]
      ] ],
      [ "[1.0.1] - Aug 2021", "md_applications_zpc_release_notes.html#autotoc_md90", [
        [ "Fixed", "md_applications_zpc_release_notes.html#autotoc_md91", null ]
      ] ],
      [ "[1.0.0] - Aug 2021", "md_applications_zpc_release_notes.html#autotoc_md92", [
        [ "Added", "md_applications_zpc_release_notes.html#autotoc_md93", null ],
        [ "Changed", "md_applications_zpc_release_notes.html#autotoc_md94", null ]
      ] ],
      [ "Known Bugs", "md_applications_zpc_release_notes.html#autotoc_md95", null ],
      [ "Z-Wave Certification information", "md_applications_zpc_release_notes.html#autotoc_md96", null ]
    ] ],
    [ "CMake coding standard", "md_doc_standards_cmake-standard.html", null ],
    [ "Unify Coding Standard", "md_doc_standards_coding-standard.html", [
      [ "1 Introduction", "md_doc_standards_coding-standard.html#autotoc_md100", [
        [ "1.1 Purpose of this document", "md_doc_standards_coding-standard.html#autotoc_md101", null ],
        [ "1.2 Latest version of this document", "md_doc_standards_coding-standard.html#autotoc_md102", null ],
        [ "1.3 Changing this document", "md_doc_standards_coding-standard.html#autotoc_md103", [
          [ "1.3.1 Versioning", "md_doc_standards_coding-standard.html#autotoc_md104", null ],
          [ "1.3.2 Review committee", "md_doc_standards_coding-standard.html#autotoc_md105", null ]
        ] ],
        [ "1.4 Structure and wording of this document", "md_doc_standards_coding-standard.html#autotoc_md106", [
          [ "1.4.1 Source Code", "md_doc_standards_coding-standard.html#autotoc_md107", null ],
          [ "1.4.2 Required", "md_doc_standards_coding-standard.html#autotoc_md108", null ],
          [ "1.4.3 Recommended", "md_doc_standards_coding-standard.html#autotoc_md109", null ],
          [ "1.4.4 Should", "md_doc_standards_coding-standard.html#autotoc_md110", null ]
        ] ],
        [ "1.5 Application of this standard", "md_doc_standards_coding-standard.html#autotoc_md111", [
          [ "1.5.1 New modules", "md_doc_standards_coding-standard.html#autotoc_md112", null ],
          [ "1.5.2 Existing modules", "md_doc_standards_coding-standard.html#autotoc_md113", null ]
        ] ]
      ] ],
      [ "2 General Guidelines", "md_doc_standards_coding-standard.html#autotoc_md114", [
        [ "2.1 General Guidelines", "md_doc_standards_coding-standard.html#autotoc_md115", [
          [ "2.1.1 Use peer review", "md_doc_standards_coding-standard.html#autotoc_md116", null ],
          [ "2.1.3 Be a Boy Scout", "md_doc_standards_coding-standard.html#autotoc_md117", null ],
          [ "2.1.4 Avoid reinventing the Wheel", "md_doc_standards_coding-standard.html#autotoc_md118", null ],
          [ "2.1.5 Refactor code", "md_doc_standards_coding-standard.html#autotoc_md119", null ]
        ] ],
        [ "2.2 Testing", "md_doc_standards_coding-standard.html#autotoc_md120", [
          [ "2.2.1 Testability", "md_doc_standards_coding-standard.html#autotoc_md121", null ],
          [ "2.2.2 Regression Test", "md_doc_standards_coding-standard.html#autotoc_md122", null ]
        ] ],
        [ "2.3 Embedded Coding", "md_doc_standards_coding-standard.html#autotoc_md123", [
          [ "2.3.1 Write energy friendly code", "md_doc_standards_coding-standard.html#autotoc_md124", null ],
          [ "2.3.2 Interrupts", "md_doc_standards_coding-standard.html#autotoc_md125", null ]
        ] ]
      ] ],
      [ "3 C Specific Guidelines", "md_doc_standards_coding-standard.html#autotoc_md126", [
        [ "3.1 Compilers", "md_doc_standards_coding-standard.html#autotoc_md127", [
          [ "3.1.1 C standards version (Required)", "md_doc_standards_coding-standard.html#autotoc_md128", null ],
          [ "3.1.1.1 C++ standards version (Required)", "md_doc_standards_coding-standard.html#autotoc_md129", null ],
          [ "3.1.2 Support mixing C++ and C (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md130", null ],
          [ "3.1.3 Multiple build tool support (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md131", null ],
          [ "3.1.4 Compile with warnings (Required)", "md_doc_standards_coding-standard.html#autotoc_md132", null ],
          [ "3.1.5 Treat warnings as errors (Required)", "md_doc_standards_coding-standard.html#autotoc_md133", null ]
        ] ],
        [ "3.2 Preprocessor", "md_doc_standards_coding-standard.html#autotoc_md134", [
          [ "3.2.1 Use header file guards (Required)", "md_doc_standards_coding-standard.html#autotoc_md135", null ],
          [ "3.2.2 Preprocessor conditional compilation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md136", null ],
          [ "3.2.3 Use of preprocessor directive #error (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md137", null ],
          [ "3.2.4 Use of preprocessor directive #include (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md138", null ]
        ] ],
        [ "3.2.5 Use of preprocessor directive <tt>#undef</tt> (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md139", null ],
        [ "3.2.6 Use of preprocessor 'stringify' and concatenation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md140", null ],
        [ "3.2.7 General use of preprocessor  (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md141", null ],
        [ "3.3 Standard Libraries", "md_doc_standards_coding-standard.html#autotoc_md142", [
          [ "3.3.1 Use of stdint.h and stdbool.h (Required)", "md_doc_standards_coding-standard.html#autotoc_md143", null ],
          [ "3.3.2 Use of stdlib (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md144", null ],
          [ "3.3.3 Use of dynamic memory / the heap (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md145", null ],
          [ "3.3.4 Do not use the standard C assert() (Required)", "md_doc_standards_coding-standard.html#autotoc_md146", null ]
        ] ],
        [ "3.4 Keywords", "md_doc_standards_coding-standard.html#autotoc_md147", [
          [ "3.4.1 The \"volatile\" keyword (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md148", null ],
          [ "3.4.2 The \"inline\" keyword (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md149", null ],
          [ "3.4.3 The \"static\" keyword (Required)", "md_doc_standards_coding-standard.html#autotoc_md150", null ]
        ] ],
        [ "3.5 Data Types", "md_doc_standards_coding-standard.html#autotoc_md151", [
          [ "3.5.1 Avoid enums in arrays, structures, or unions (Required)", "md_doc_standards_coding-standard.html#autotoc_md152", null ],
          [ "3.5.2 Avoid bitfields in structures/unions (Required)", "md_doc_standards_coding-standard.html#autotoc_md153", null ]
        ] ],
        [ "3.6 Variables", "md_doc_standards_coding-standard.html#autotoc_md154", [
          [ "3.6.1 Using global variables (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md155", null ]
        ] ],
        [ "3.7 Functions", "md_doc_standards_coding-standard.html#autotoc_md156", [
          [ "3.7.1 Prototype functions (Required)", "md_doc_standards_coding-standard.html#autotoc_md157", null ],
          [ "3.7.2 Functions return data types (Required)", "md_doc_standards_coding-standard.html#autotoc_md158", [
            [ "Exceptions", "md_doc_standards_coding-standard.html#autotoc_md159", null ]
          ] ],
          [ "3.7.3 Return values should always be checked and propagated (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md160", null ],
          [ "3.7.4 Functions replicating a standard API can follow that API (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md161", null ]
        ] ],
        [ "3.8 Macros", "md_doc_standards_coding-standard.html#autotoc_md162", [
          [ "3.8.1 Macros with side-effects (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md163", null ],
          [ "3.8.2 Macros with statement(s) (Required)", "md_doc_standards_coding-standard.html#autotoc_md164", null ],
          [ "3.8.3 Functional macros with argument(s) (Required)", "md_doc_standards_coding-standard.html#autotoc_md165", null ]
        ] ],
        [ "3.9 goto statements", "md_doc_standards_coding-standard.html#autotoc_md166", [
          [ "3.9.1 <tt>goto</tt> statements should only be used for cleanup purposes or early exit in case of an error, when there is no simple workaround (Required)", "md_doc_standards_coding-standard.html#autotoc_md167", null ],
          [ "3.9.2 <tt>goto</tt>s should only refer to a label declared after them (Required)", "md_doc_standards_coding-standard.html#autotoc_md168", null ],
          [ "3.9.3 <tt>goto</tt>s should only refer to a static label located in the same function (Required)", "md_doc_standards_coding-standard.html#autotoc_md169", null ],
          [ "3.9.4 Any label referenced by a <tt>goto</tt> need to be declared in the same block or a block enclosing the <tt>goto</tt> (Required)", "md_doc_standards_coding-standard.html#autotoc_md170", null ]
        ] ],
        [ "3.10 Libraries", "md_doc_standards_coding-standard.html#autotoc_md171", [
          [ "3.10.1 Dependencies (Required)", "md_doc_standards_coding-standard.html#autotoc_md172", null ],
          [ "3.10.2 Including third party code (Required)", "md_doc_standards_coding-standard.html#autotoc_md173", null ],
          [ "3.10.3 Configuring libraries in source form (Required)", "md_doc_standards_coding-standard.html#autotoc_md174", null ],
          [ "3.10.4 Configuring libraries in binary form (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md175", null ]
        ] ],
        [ "3.11 Misc", "md_doc_standards_coding-standard.html#autotoc_md176", [
          [ "3.11.1 Avoid embedding assignments in expressions (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md177", null ]
        ] ]
      ] ],
      [ "4 Coding style and formatting", "md_doc_standards_coding-standard.html#autotoc_md178", [
        [ "4.1 General formatting", "md_doc_standards_coding-standard.html#autotoc_md179", [
          [ "4.1.1 Use spaces, not tabs (Required)", "md_doc_standards_coding-standard.html#autotoc_md180", null ],
          [ "4.1.2 Indents are 2 spaces per level (Required)", "md_doc_standards_coding-standard.html#autotoc_md181", [
            [ "4.1.2.1 Preprocessor indentation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md182", null ]
          ] ],
          [ "4.1.3 Lines should not be longer than 80 characters (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md183", null ],
          [ "4.1.4 Line endings (Required)", "md_doc_standards_coding-standard.html#autotoc_md184", null ],
          [ "4.1.5 Use only plain ASCII or UTF-8 (Required)", "md_doc_standards_coding-standard.html#autotoc_md185", null ],
          [ "4.1.6 Use ISO8601 formatting for dates (Required)", "md_doc_standards_coding-standard.html#autotoc_md186", null ],
          [ "4.1.7 Inserting empty and blank lines (Required)", "md_doc_standards_coding-standard.html#autotoc_md187", null ],
          [ "4.1.8 Use parentheses liberally (Required)", "md_doc_standards_coding-standard.html#autotoc_md188", null ],
          [ "4.1.9 Break up long expressions (Required)", "md_doc_standards_coding-standard.html#autotoc_md189", null ],
          [ "4.1.10 <tt>goto</tt> labels should be on column 1 (Required)", "md_doc_standards_coding-standard.html#autotoc_md190", null ]
        ] ],
        [ "4.2 Commenting code (Required)", "md_doc_standards_coding-standard.html#autotoc_md191", [
          [ "4.2.1 Use C++ style comments (\"//\")", "md_doc_standards_coding-standard.html#autotoc_md192", null ]
        ] ],
        [ "4.3 Bracing style (Required)", "md_doc_standards_coding-standard.html#autotoc_md193", null ],
        [ "4.4 Switch statements and labels", "md_doc_standards_coding-standard.html#autotoc_md194", [
          [ "4.4.1 Using labels (Required)", "md_doc_standards_coding-standard.html#autotoc_md195", null ],
          [ "4.4.2 Labels with block (Required)", "md_doc_standards_coding-standard.html#autotoc_md196", null ],
          [ "4.4.3 Switch statements (Required)", "md_doc_standards_coding-standard.html#autotoc_md197", null ]
        ] ],
        [ "4.5 Functions, operators and C keywords", "md_doc_standards_coding-standard.html#autotoc_md198", [
          [ "4.5.1 Listing function parameters (Required)", "md_doc_standards_coding-standard.html#autotoc_md199", null ],
          [ "4.5.2 Using function parentheses (Required)", "md_doc_standards_coding-standard.html#autotoc_md200", null ],
          [ "4.5.3 Binary and ternary operators (Required)", "md_doc_standards_coding-standard.html#autotoc_md201", null ],
          [ "4.5.4 Use a single space after C keywords (Required)", "md_doc_standards_coding-standard.html#autotoc_md202", null ],
          [ "4.5.5 Additional space within expressions", "md_doc_standards_coding-standard.html#autotoc_md203", null ],
          [ "4.5.6 Identify do-nothing code (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md204", null ],
          [ "4.5.7 Pointer asterisk position (Required)", "md_doc_standards_coding-standard.html#autotoc_md205", null ],
          [ "4.5.8 Don't mix pointer and value type declarations (Required)", "md_doc_standards_coding-standard.html#autotoc_md206", null ]
        ] ],
        [ "4.6 Naming", "md_doc_standards_coding-standard.html#autotoc_md207", [
          [ "4.6.1 General Considerations", "md_doc_standards_coding-standard.html#autotoc_md208", [
            [ "4.6.1.1 Use meaningful names (Required)", "md_doc_standards_coding-standard.html#autotoc_md209", null ],
            [ "4.6.1.2 Avoid abbreviations (Required)", "md_doc_standards_coding-standard.html#autotoc_md210", null ]
          ] ],
          [ "4.6.2 Namespaces (Required)", "md_doc_standards_coding-standard.html#autotoc_md211", null ],
          [ "4.6.3 Naming (Required)", "md_doc_standards_coding-standard.html#autotoc_md212", [
            [ "4.6.3.1 Casing (Required)", "md_doc_standards_coding-standard.html#autotoc_md213", null ],
            [ "4.6.3.2 Casing and acronyms (Required)", "md_doc_standards_coding-standard.html#autotoc_md214", null ]
          ] ],
          [ "4.6.4 Naming functions and variables", "md_doc_standards_coding-standard.html#autotoc_md215", [
            [ "4.6.4.1 Functions and variables names have the form verb_noun (Required)", "md_doc_standards_coding-standard.html#autotoc_md216", null ],
            [ "4.6.4.2 Function parameters should not have any prefix (Required)", "md_doc_standards_coding-standard.html#autotoc_md217", null ],
            [ "4.6.4.3 Function-local variables should not have any prefix (Required)", "md_doc_standards_coding-standard.html#autotoc_md218", null ],
            [ "4.6.4.4 Variable and Function Names Indicate Units for Time (Required)", "md_doc_standards_coding-standard.html#autotoc_md219", null ],
            [ "4.6.4.5 Functions/stubs called on specific events/callbacks should start 'on' in their name (Required)", "md_doc_standards_coding-standard.html#autotoc_md220", null ],
            [ "4.6.4.6 Interrupt handlers should be suffixed by 'IRQHandler' or 'irq_handler' (Required)", "md_doc_standards_coding-standard.html#autotoc_md221", null ],
            [ "4.6.4.7 Non-blocking functions executed periodically in a main loop should be suffixed by 'step' (Required)", "md_doc_standards_coding-standard.html#autotoc_md222", null ],
            [ "4.6.4.8 Functions that are tasks should be suffixed by 'task' (Required)", "md_doc_standards_coding-standard.html#autotoc_md223", null ]
          ] ],
          [ "4.6.5 Naming constants", "md_doc_standards_coding-standard.html#autotoc_md224", [
            [ "4.6.5.1 Constants should use upper case (Required)", "md_doc_standards_coding-standard.html#autotoc_md225", null ]
          ] ],
          [ "4.6.6 Naming function-like macros", "md_doc_standards_coding-standard.html#autotoc_md226", [
            [ "4.6.6.1 Follow the naming convention for regular functions (Required)", "md_doc_standards_coding-standard.html#autotoc_md227", null ],
            [ "4.6.6.2 Use all caps for macros that can't be functions (Required)", "md_doc_standards_coding-standard.html#autotoc_md228", null ]
          ] ],
          [ "4.6.7 Naming types", "md_doc_standards_coding-standard.html#autotoc_md229", [
            [ "4.6.7.1 Public typedefs (Required)", "md_doc_standards_coding-standard.html#autotoc_md230", null ],
            [ "4.6.7.2 Structure fields should be snake_case, without any prefixes (Required)", "md_doc_standards_coding-standard.html#autotoc_md231", null ],
            [ "4.6.7.3 Type from typedef (Optional)", "md_doc_standards_coding-standard.html#autotoc_md232", null ]
          ] ],
          [ "4.6.8 Files and directory structure", "md_doc_standards_coding-standard.html#autotoc_md233", [
            [ "4.6.8.1 Filenames and directories use lower case (Required)", "md_doc_standards_coding-standard.html#autotoc_md234", null ],
            [ "4.6.8.2 Avoid duplicate filenames", "md_doc_standards_coding-standard.html#autotoc_md235", null ],
            [ "4.6.8.3 File names (Required)", "md_doc_standards_coding-standard.html#autotoc_md236", null ],
            [ "4.6.8.4 Directory names (Required)", "md_doc_standards_coding-standard.html#autotoc_md237", null ]
          ] ]
        ] ]
      ] ],
      [ "5 Documentation", "md_doc_standards_coding-standard.html#autotoc_md238", [
        [ "5.1 General", "md_doc_standards_coding-standard.html#autotoc_md239", null ],
        [ "5.2 Comments should answer the question \"Why?\" (Required)", "md_doc_standards_coding-standard.html#autotoc_md240", null ]
      ] ],
      [ "6 Doxygen Coding Style Guide", "md_doc_standards_coding-standard.html#autotoc_md241", [
        [ "6.1 File comment header (Required)", "md_doc_standards_coding-standard.html#autotoc_md242", null ],
        [ "6.2 Grouping modules (@addtogroup) (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md243", null ],
        [ "6.3 File/module level documentation (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md244", null ],
        [ "6.4 Function documentation (Required)", "md_doc_standards_coding-standard.html#autotoc_md245", null ],
        [ "6.5 Variable documentation (Required)", "md_doc_standards_coding-standard.html#autotoc_md246", null ],
        [ "6.6 Header file vs. implementation file (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md247", null ],
        [ "6.7 Do not document sections (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md248", null ],
        [ "6.8 Do not release sections (Recommended)", "md_doc_standards_coding-standard.html#autotoc_md249", null ]
      ] ],
      [ "7 Licensing", "md_doc_standards_coding-standard.html#autotoc_md250", [
        [ "7.1 Silicon Labs Licenses", "md_doc_standards_coding-standard.html#autotoc_md251", null ],
        [ "7.2 Third-Party Code", "md_doc_standards_coding-standard.html#autotoc_md252", null ]
      ] ],
      [ "8 C++ Specific Guidelines", "md_doc_standards_coding-standard.html#autotoc_md253", null ]
    ] ],
    [ "Known Abbreviations", "md_doc_standards_known-abbreviations.html", null ],
    [ "Unify SDK Developer Readme", "md_doc_readme_developer.html", [
      [ "Directory Structure and File Naming Conventions", "md_doc_readme_developer.html#autotoc_md256", [
        [ "Rules", "md_doc_readme_developer.html#autotoc_md257", null ],
        [ "Structure of Components (Best Practice)", "md_doc_readme_developer.html#autotoc_md258", null ],
        [ "Structure", "md_doc_readme_developer.html#autotoc_md259", null ]
      ] ],
      [ "Component Templates", "md_doc_readme_developer.html#autotoc_md260", [
        [ "Demo", "md_doc_readme_developer.html#autotoc_md261", null ]
      ] ],
      [ "Clang-Format (autoformat code)", "md_doc_readme_developer.html#autotoc_md262", [
        [ "Install on Mac", "md_doc_readme_developer.html#autotoc_md263", null ],
        [ "Install on Ubuntu", "md_doc_readme_developer.html#autotoc_md264", null ],
        [ "Visual Studio Code", "md_doc_readme_developer.html#autotoc_md265", null ],
        [ "Command Line", "md_doc_readme_developer.html#autotoc_md266", null ]
      ] ],
      [ "Pre-commit Hooks", "md_doc_readme_developer.html#autotoc_md267", null ],
      [ "CMake", "md_doc_readme_developer.html#autotoc_md268", [
        [ "Defining new uic-components", "md_doc_readme_developer.html#autotoc_md269", null ],
        [ "Mock uic-components", "md_doc_readme_developer.html#autotoc_md270", [
          [ "Additional headers", "md_doc_readme_developer.html#autotoc_md271", null ]
        ] ],
        [ "Unit-testing", "md_doc_readme_developer.html#autotoc_md272", [
          [ "DEPENDS", "md_doc_readme_developer.html#autotoc_md273", null ],
          [ "NAME", "md_doc_readme_developer.html#autotoc_md274", null ]
        ] ],
        [ "EXCLUDE", "md_doc_readme_developer.html#autotoc_md275", null ],
        [ "Stub", "md_doc_readme_developer.html#autotoc_md276", null ],
        [ "Do's and dont's", "md_doc_readme_developer.html#autotoc_md277", null ],
        [ "CMake File Formatting", "md_doc_readme_developer.html#autotoc_md278", null ],
        [ "Example", "md_doc_readme_developer.html#autotoc_md279", null ],
        [ "References", "md_doc_readme_developer.html#autotoc_md280", null ],
        [ "Code Coverage", "md_doc_readme_developer.html#autotoc_md281", [
          [ "Dependencies", "md_doc_readme_developer.html#autotoc_md282", null ]
        ] ]
      ] ]
    ] ],
    [ "Unify SDK Library Overview", "md_doc_overview.html", [
      [ "Base Platform", "md_doc_overview.html#autotoc_md284", [
        [ "Main loop", "md_doc_overview.html#autotoc_md285", null ],
        [ "Config System", "md_doc_overview.html#autotoc_md286", null ],
        [ "Logging System", "md_doc_overview.html#autotoc_md287", null ],
        [ "MQTT Client", "md_doc_overview.html#autotoc_md288", null ],
        [ "DotDot Serializer", "md_doc_overview.html#autotoc_md289", null ],
        [ "DotDot Attributes", "md_doc_overview.html#autotoc_md290", null ],
        [ "Datastore", "md_doc_overview.html#autotoc_md291", null ],
        [ "Attribute Store", "md_doc_overview.html#autotoc_md292", null ],
        [ "Attribute Mapper", "md_doc_overview.html#autotoc_md293", null ],
        [ "Console Interface", "md_doc_overview.html#autotoc_md294", null ],
        [ "SmartStart", "md_doc_overview.html#autotoc_md295", null ],
        [ "Node State Topic", "md_doc_overview.html#autotoc_md296", null ],
        [ "Protocol Controller MQTT", "md_doc_overview.html#autotoc_md297", null ],
        [ "OTA MQTT Client", "md_doc_overview.html#autotoc_md298", null ]
      ] ],
      [ "Test Framework", "md_doc_overview.html#autotoc_md299", null ],
      [ "Build System", "md_doc_overview.html#autotoc_md300", null ],
      [ "Packaging Framework", "md_doc_overview.html#autotoc_md301", null ],
      [ "Coding Standard", "md_doc_overview.html#autotoc_md302", null ]
    ] ],
    [ "Unify Dockerfile", "md_docker_readme_developer.html", [
      [ "Install Docker", "md_docker_readme_developer.html#autotoc_md304", null ],
      [ "Build Dockerfile Image", "md_docker_readme_developer.html#autotoc_md305", null ]
    ] ],
    [ "README to Make Debian Package for UIC Component (for Example ZPC)", "md_doc_README_debian_packaging.html", [
      [ "Function arguments", "md_doc_README_debian_packaging.html#autotoc_md307", null ],
      [ "1. PKG_NAME      component package Name", "md_doc_README_debian_packaging.html#autotoc_md308", null ],
      [ "2. PKG_FILE_NAME Debian package filename for the component", "md_doc_README_debian_packaging.html#autotoc_md309", null ],
      [ "3. PKG_DEPNDS    Component debian package depenedencies.", "md_doc_README_debian_packaging.html#autotoc_md310", null ],
      [ "4. PKG_EXTRA     Component debian package Control extra info", "md_doc_README_debian_packaging.html#autotoc_md311", null ],
      [ "See https://cmake.org/cmake/help/v3.6/module/CPackDeb.html for more information", "md_doc_README_debian_packaging.html#autotoc_md312", null ]
    ] ],
    [ "Unify SDK Library Developer Guide", "md_doc_readme_uic_application_developer.html", [
      [ "Creating a New Application", "md_doc_readme_uic_application_developer.html#autotoc_md314", null ],
      [ "Unify SDK Library (Unify Main) and IPC", "md_doc_readme_uic_application_developer.html#autotoc_md315", null ]
    ] ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Modules", "modules.html", "modules" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
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
        [ "Variables", "globals_vars.html", "globals_vars" ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", "globals_eval" ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"ZW__typedefs_8h.html",
"classast_1_1dep__eval__path.html#af61f19399df9411eedd227cad1068c0c",
"datastore__attributes_8h_source.html",
"functions_b.html",
"group__ZWAPI__BASIS.html#ga7a2885626cba3bc9d38f21aca3de085e",
"group__ZWAPI__CONTROLLER.html#ga6e50a93518af078f107923bca22a6880",
"group__ZWAPI__INIT.html#ga8760e50d33250308039ff09a9e8a1674",
"group__ZWAPI__TRANSPORT.html#gae3301f35fd3b25ac67d4730e29a481c0",
"group__attribute__store__value__helpers.html#ga2179ceede1afad2f87c6dbe35809b6ab",
"group__datastore.html#ga3d4cfb0e60a2bbe38b06cd0a18c44e32",
"group__memb.html#gab8f8dad927b501d8b4ed4340d1bcf4fb",
"group__notification__command__class.html#ga674be361d495594dc0c154e0553fea8c",
"group__process.html#ga958ff4131a37eaf12b920da16749e3b7",
"group__sl__status.html#ga3a0900f840eb105f6c112201c46aaf84",
"group__uic__fixt.html#ga487efe216dc6c7d92ce248961b3ddef4",
"group__zpc__utils.html#gae80c42e828f74022c10bf530a612d22e",
"group__zwave__command__class__wake__up.html#ga11866ec0935bee3fba386bc3ed28e0b6",
"group__zwave__network__management.html#gac01bf38badf30f94eac9a04b486cb967",
"group__zwave__tx__interface.html#ga4b1785eddaa2ebfc6a702d9b4d0b27a3",
"md_applications_zpc_readme_user.html#autotoc_md19",
"md_doc_standards_coding-standard.html#autotoc_md212",
"ota_8hpp.html#ae6ebb5b487964684aa56019a2730acfc",
"structassociation__set__frame.html#a72a96addbddfb364c5c30c3e67ac7b4e",
"structpower__level__test.html",
"structzwave__controller__callbacks__t.html#a8c2f475607fadb0664fe8630a03c7304",
"ucl__network__management_8h_source.html",
"zpc__stdin__fixt_8h.html#a5d4732560e00bb1905ddfdbc5ce12279",
"zwapi__internal_8h.html#a9e34a8eec16bffc48c5056c3b0f392dd",
"zwave__api__demo__commands_8c.html#af766cec3395e596e458c07df1583a32d",
"zwave__command__class__notification__types_8h.html#a46fa465f195518e100bce9e92b9bfa4c",
"zwave__command__class__transport__service_8h_source.html",
"zwave__rx__internals_8h.html#a5271693400e97e0f87571a5868dcec92",
"zwave__s2__network_8h_source.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';