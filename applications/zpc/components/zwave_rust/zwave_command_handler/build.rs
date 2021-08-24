// License
// <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
use uic_bindgen::BindingsTarget;

fn main() {
    let bind_target = BindingsTarget::from_json(format!(
        "{}/zwave_command_handler_bindgen.json",
        env!("LIBUIC_DIR")
    ))
    .unwrap();
    bind_target.write_link_configuration();
}
