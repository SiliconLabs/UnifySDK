// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

fn main() {
    if let Ok(bind_target) = uic_bindgen::BindingsTarget::from_json(format!(
        "{}/uic_angel_bindgen.json",
        env!("LIBUIC_DIR")
    )) {
        bind_target.write_link_configuration();
    } else {
        println!("cargo:warning= uic_angel will not link against native libs");
    }
}
