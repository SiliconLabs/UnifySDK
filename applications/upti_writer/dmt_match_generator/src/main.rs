// License
// <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

use serde_json;
use std::fs;

const MAX_NAME_LENGTH: usize = 10;

fn main() {
    let path = "debug-message-type.json";
    let data = fs::read_to_string(path).expect("Unable to read file");
    let json_object: serde_json::Value = serde_json::from_str(&data).expect("Unable to parse");

    println!("");
    println!("// Message type CONST is generated from {}, feature date {}", path, json_object["featureDate"]);
    // Print CONST
    for (key, value) in json_object["types"].as_object().unwrap() {
        let message_type = key.to_string().replace("\"", "");
        let message_code = value["code"].to_string().replace("\"", "");

        println!("const {}: u16 = {};", message_type, message_code);
    }

    println!("");
    println!("// MASK and CATEGRY CONST");
    println!("const MASK_PACKET: u32 = 1 << 24;");
    println!("const MASK_AEM: u32 = 1 << 27;");
    println!("const CATEGORY_UNKNOWN: u32 = 0xFF << 16;");
    println!("const CATEGORY_BACKCHANNEL: u32 = 0x02 << 16;");

    println!("");
    println!("// Match is generated from {}, feature date {}", path, json_object["featureDate"]);
    println!("match debug_type {{");
    // Print match
    for (key, value) in json_object["types"].as_object().unwrap() {
        let message_type = key.to_string().replace("\"", "");
        let mut mask  = format!("CATEGORY_BACKCHANNEL | {} as u32", message_type);

        if message_type.starts_with("PACKET") {
            mask = format!("MASK_PACKET | CATEGORY_BACKCHANNEL | {} as u32", message_type);
        };

        if message_type.starts_with("AEM") {
            mask = format!("MASK_AEM | CATEGORY_BACKCHANNEL | {} as u32", message_type);
        };

        let mut description = value["shortDescription"].to_string().replace("\"", "");
        // Truncate description lenght to MAX_NAME_LENGTH 
        if description.len() > MAX_NAME_LENGTH {
            description.truncate(MAX_NAME_LENGTH);
        }
        println!("  {} => return (\"{}\", {}),", message_type, description, mask);
    }
    println!("  _ => return (\"Unknown\", {}),", "CATEGORY_UNKNOWN");
    println!("}};");
}
