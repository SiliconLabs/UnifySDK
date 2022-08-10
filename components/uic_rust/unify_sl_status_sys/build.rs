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

use convert_case::{Case, Casing};
use quote::{format_ident, ToTokens, __private::TokenStream};
use std::{
    env,
    fs::File,
    io::{BufRead, BufReader, LineWriter, Write},
    path::PathBuf,
};

use regex::Regex;

fn main() {
    let mut status_codes: Vec<(String, String)> = Vec::new();
    let re = Regex::new(r"^#define\s+(SL_STATUS_\w+).+(0[xX][0-9a-fA-F]+)").unwrap();

    let file = File::open("../../../include/sl_status.h").unwrap();
    let reader = BufReader::new(file);
    for line in reader.lines() {
        for cap in re.captures_iter(&line.unwrap()) {
            status_codes.push((cap[1].to_string(), cap[2].to_string()));
        }
    }

    write_file(status_codes).expect("error writing out content to sl_status.rs file");
}

fn write_file(status_codes: Vec<(String, String)>) -> std::io::Result<()> {
    let mut out_file = PathBuf::from(env::var("OUT_DIR").unwrap());
    out_file.push("sl_status.rs");
    let rust_status_file = File::create(out_file).expect("error creating file sl_status.rs");

    let mut file = LineWriter::new(rust_status_file);
    file.write_all(b"// Generated SL_STATUS struct from the C header sl_status.h\n")?;
    file.write_all(b"// This file is auto generated. hence do not modify.\n")?;

    // write out sl status values
    for (name, val) in &status_codes {
        let line = format!("pub const {}: u32 = {};\n", name, val);
        file.write_all(line.as_bytes())?;
    }

    // write out conversion from u32 to string
    file.write_all(b"\npub fn sl_status_to_string(val: u32) -> &'static str {\n match val {\n")?;
    for (name, val) in &status_codes {
        let line = format!("\t\t{} => \"{}\",\n", val, name);
        file.write_all(line.as_bytes())?;
    }

    let line = format!("\t\t _ => \"UNKNOWN_STATUS_CODE\",\n");
    file.write_all(line.as_bytes())?;
    file.write_all(b"\t}\t}")?;

    file.write_all(generate_map_functions(status_codes).as_bytes())?;

    Ok(())
}

fn generate_map_functions(status_codes: Vec<(String, String)>) -> String {
    let mut tokens = TokenStream::new();

    status_codes
        .iter()
        .map(|(name, val)| {
            (
                format_ident!("map_to_{}", name.to_case(Case::Snake)),
                if val.chars().any(|c| c.is_ascii_alphabetic()) {
                    u32::from_str_radix(val.trim_start_matches("0x"), 16)
                        .expect("hex value to be valid")
                } else {
                    val.parse::<u32>().expect("expected an integer value")
                },
            )
        })
        .for_each(|(method_name, val)| {
            let method = quote::quote! {
                pub fn #method_name<T>(_:T) -> u32{
                    #val
                }
            };
            method.to_tokens(&mut tokens);
        });

    tokens.to_string()
}
