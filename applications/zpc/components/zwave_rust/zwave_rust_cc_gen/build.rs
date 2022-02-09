use std::env;
use std::process::Command;

fn main() {
    let out_dir = env::var_os("OUT_DIR").unwrap();
    //TODO I'm not quite sure how to set this in a nice way
    //env::var_os("XML_FILE").unwrap();
    let xml_file = "../../zwave_command_classes/assets/ZWave_custom_cmd_classes.xml";

    Command::new("python3")
        .arg("zwave_rust_cc_gen.py")
        .arg("--xml")
        .arg(&xml_file)
        .arg("--outdir")
        .arg(&out_dir)
        .status()
        .unwrap();
    println!("cargo:rerun-if-changed=zwave_rust_cc_gen.py");
    println!("cargo:rerun-if-changed={:?}", &xml_file);
}
