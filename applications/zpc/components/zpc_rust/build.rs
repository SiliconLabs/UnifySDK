///////////////////////////////////////////////////////////////////////////////
// # License
// <b>Copyright 2022  Silicon Laboratories Inc. www.silabs.com</b>
///////////////////////////////////////////////////////////////////////////////
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
//
///////////////////////////////////////////////////////////////////////////////

use anyhow::Result;
use std::env;
use std::path::PathBuf;
use std::process::Command;
use unify_build_utils::load_environment;
use unify_build_utils::*;

fn main() -> Result<()> {
    let out_dir = env::var_os("OUT_DIR").unwrap();
    let xml_file = "../zwave_command_classes/assets/ZWave_custom_cmd_classes.xml";
    Command::new("python3")
        .arg("zwave_rust_cc_framework_gen.py")
        .arg("--xml")
        .arg(&xml_file)
        .arg("--outdir")
        .arg(&out_dir)
        .status()
        .unwrap();
    println!("cargo:rerun-if-changed=zwave_rust_cc_framework_gen.py");
    println!("cargo:rerun-if-changed={:?}", &xml_file);

    let link_dependencies = load_environment("applications/zpc")?;

    let out_dir =
        PathBuf::from(std::env::var("OUT_DIR").expect("OUT_DIR is always set during build stage"));
    let components_path = PathBuf::from(concat!(env!("CARGO_MANIFEST_DIR"), "/.."));

    generate_bindings(
        &out_dir.join("firmware.rs"),
        &link_dependencies.include_directories,
        Some(".*firmware_.*"),
        Some("attribute_store_node_t"),
        Some(&[format!(
            "{}/zwave_command_classes/src/zwave_command_class_firmware_update_internals.h",
            components_path.to_string_lossy()
        )]),
    )?;

    generate_bindings(
        &out_dir.join("zwave_command_classes.rs"),
        &link_dependencies.include_directories,
        Some(".*zwave_command_class.*|zwave_firmware.*"),
        Some("zwave_controller.*"),
        Some(&[format!(
            "{}/zwave_command_classes/include/*.h",
            components_path.to_string_lossy()
        )]),
    )?;

    generate_bindings(
        &out_dir.join("zwave_command_handler.rs"),
        &link_dependencies.include_directories,
        Some(".*zwave_command_handler.*"),
        Some("zwave_controller.*"),
        Some(&[format!(
            "{}/zwave_command_handler/include/*.h",
            components_path.to_string_lossy()
        )]),
    )?;

    generate_bindings(
        &out_dir.join("zwave_controller.rs"),
        &link_dependencies.include_directories,
        Some(".*zwave_controller.*|zwave_node_.*"),
        None,
        Some(&[
            format!(
                "{}/zwave/zwave_controller/include/*.h",
                components_path.to_string_lossy()
            ),
            format!(
                "{}/zpc_attribute_store/include/zwave_utils.h",
                components_path.to_string_lossy()
            ),
        ]),
    )?;

    generate_bindings(
        &out_dir.join("zwave_tx.rs"),
        &link_dependencies.include_directories,
        Some(".*zwave_tx.*|ZWAVE_TX.*"),
        Some(".*zwave_controller.*"),
        Some(&[format!(
            "{}/zwave/zwave_tx/include/*.h",
            components_path.to_string_lossy()
        )]),
    )?;

    generate_bindings(
        &out_dir.join("zpc_attribute_store.rs"),
        &link_dependencies.include_directories,
        Some("get_zpc_.*"),
        Some(".*zwave_controller.*"),
        Some(&[format!(
            "{}/zpc_attribute_store/include/*.h",
            components_path.to_string_lossy()
        )]),
    )
}
