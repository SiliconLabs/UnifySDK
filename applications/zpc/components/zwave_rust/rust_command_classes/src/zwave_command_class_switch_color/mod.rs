///////////////////////////////////////////////////////////////////////////////
// # License
// <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
///////////////////////////////////////////////////////////////////////////////
// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.
//
///////////////////////////////////////////////////////////////////////////////

use crate::ZwaveHandlerConfig;
use crate::{SL_STATUS_FAIL, SL_STATUS_NOT_SUPPORTED, SL_STATUS_OK};
use futures::stream::StreamExt;
use unify_attribute_store_sys::{attribute_resolver_register_rule, sl_status_t};
use unify_log_sys::*;
use unify_middleware::contiki::contiki_spawn;
use unify_middleware::{
    Attribute, AttributeEvent, AttributeEventType, AttributeStoreError, AttributeStoreExtTrait,
    AttributeStoreTrait, AttributeValueState,
};
use unify_proc_macro::{as_extern_c, generate_resolver_callback};
use zwave_command_classes_sys::zwave_command_class_get_endpoint_node;
///
/// This file implement the switch color command class
///
///
use zwave_rust_cc_gen::command_class_switch_color::*;
use zwave_rust_proc_macros::zwave_command_class;

/// Attribute definitions
const zwCOMMAND_CLASS_SWITCH_COLOR_VERSION: u32 = 0x3301;
const zwSWITCH_COLOR_COLOR_COMPONENT_COUNT: u32 = 0x3302;
const zwSWITCH_COLOR_COLOR_COMPONENT_ID: u32 = 0x3303;
const zwSWITCH_COLOR_VALUE: u32 = 0x3304;
const zwSWITCH_COLOR_SUPPORTED_COLOR_COMPONENT_MASK: u32 = 0x3305;
const zwSWITCH_COLOR_DURATION: u32 = 0x3306;
const zwSWITCH_COLOR_CURRENT_VALUE: u32 = 0x3307;
const zwSWITCH_COLOR_TARGET_VALUE: u32 = 0x3308;

declare_app_name!("zwave_cc_switch_color");

/////////////////////// Command Generators //////////////////////////

#[generate_resolver_callback]
fn switch_color_get(attr: Attribute) -> Result<(sl_status_t, Vec<u8>), AttributeStoreError> {
    let component = attr.get_parent()?;
    let get_frame = SwitchColorGetFrame {
        color_component_id: component.get_reported()?,
    };
    Ok((SL_STATUS_OK, get_frame.into()))
}

#[generate_resolver_callback]
fn switch_color_set(attr: Attribute) -> Result<(sl_status_t, Vec<u8>), AttributeStoreError> {
    let ep = (attr.get_parent()?).get_parent()?;
    let mut colors = Vec::new();
    // Gather data tor all color components
    for component in ep.get_children_by_type(zwSWITCH_COLOR_COLOR_COMPONENT_ID)? {
        let value_attribute = component.get_child_by_type(zwSWITCH_COLOR_TARGET_VALUE)?;
        let value: u32 = value_attribute.get_desired_or_reported()?;
        let color_value = SwitchColorSetVg1Vg {
            color_component_id: component.get_reported()?,
            value: value as u8,
        };
        colors.push(color_value);
    }
    let set_frame = SwitchColorSetFrame {
        properties1: SwitchColorSetProperties1Bits {
            color_component_count: 0, //The component count is updated by the frame generator
            reserved: 0,
        },
        vg1: colors,
        duration: SwitchColorSetDurationEnum::Instantly,
    };

    Ok((SL_STATUS_OK, set_frame.into()))
}

#[generate_resolver_callback]
fn switch_color_supported_get(
    attr: Attribute,
) -> Result<(sl_status_t, Vec<u8>), AttributeStoreError> {
    Ok((SL_STATUS_OK, (SwitchColorSupportedGetFrame {}).into()))
}
///////////////////////////// Version Update ///////////////////////////////

fn on_attribute_version(ep: Attribute, version: u8) -> Result<(), AttributeStoreError> {
    // Use any color component id at token for starting the supported get
    if version > 0 {
        ep.emplace::<u32>(zwSWITCH_COLOR_COLOR_COMPONENT_ID, None, None)?;
    }
    Ok(())
}

/////////////////////////// Command Handlers ///////////////////////////

fn switch_color_handle_supported_report(
    ep: Attribute,
    f: SwitchColorSupportedReportFrame,
) -> Result<sl_status_t, AttributeStoreError> {
    //delete old components such we don't end up with duplicates

    if let Ok(components) = ep.get_children_by_type(zwSWITCH_COLOR_COLOR_COMPONENT_ID) {
        for c in components {
            c.delete()?;
        }
    }

    // The bits bitmask is byte reversed order with LSB bits in the first byte
    let mask = (f.color_component_mask >> 8) | (f.color_component_mask & 0xff);
    for i in 0..15 {
        if (mask & (1 << i)) != 0 {
            let component =
                ep.emplace::<u8>(zwSWITCH_COLOR_COLOR_COMPONENT_ID, Some(i + 1), None)?;
            component.emplace::<u32>(zwSWITCH_COLOR_CURRENT_VALUE, None, None)?;
            component.emplace::<u32>(zwSWITCH_COLOR_TARGET_VALUE, None, None)?;
            component.emplace::<u32>(zwSWITCH_COLOR_DURATION, None, None)?;
        }
    }
    Ok(SL_STATUS_OK)
}

fn switch_color_handle_report(
    ep: Attribute,
    f: SwitchColorReportFrame,
) -> Result<sl_status_t, AttributeStoreError> {
    let component = ep.get_child_by_type_and_value(
        zwSWITCH_COLOR_COLOR_COMPONENT_ID,
        &Some(f.color_component_id),
    )?;
    component
        .get_child_by_type(zwSWITCH_COLOR_CURRENT_VALUE)?
        .set_reported(Some(f.current_value as u32))?;
    component
        .get_child_by_type(zwSWITCH_COLOR_TARGET_VALUE)?
        .set_reported(Some(f.target_value as u32))?;
    component
        .get_child_by_type(zwSWITCH_COLOR_DURATION)?
        .set_reported(Some(f.duration as u32))?;

    Ok(SL_STATUS_OK)
}

async fn on_switch_color_version_change_async() {
    let version_update = |event: &AttributeEvent| {
        (event.event_type == AttributeEventType::ATTRIBUTE_UPDATED)
            && (event.value_state == AttributeValueState::DESIRED_ATTRIBUTE)
            && (event.attribute.type_of() == zwCOMMAND_CLASS_SWITCH_COLOR_VERSION)
    };

    let mut attribute_update_stream = Attribute::attribute_changed(version_update);
    loop {
        if let Some(change) = attribute_update_stream.next().await {
            if let Ok(version) = change.attribute.get_reported::<u8>() {
                if let Err(e) =
                    on_attribute_version(change.attribute.get_parent().unwrap(), version)
                {
                    log_error!("error getting version information {}", e);
                }
            }
        } else {
            break;
        }
    }
}

///////////////////////// Command class registration ////////////////////

const CONFIG: ZwaveHandlerConfig = ZwaveHandlerConfig {
    minimal_scheme:
    zwave_command_handler_sys::zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
    command_class: COMMAND_CLASS_SWITCH_COLOR,
    command_class_name: "Color Switch\0",
    comments: "\0",
    version: COMMAND_CLASS_SWITCH_COLOR_VERSION,
    manual_security_validation: false,
};

#[zwave_command_class(CONFIG)]
#[derive(Default)]
pub struct ZwaveCommandClassSwitchColor {}

impl ZwaveCommandClassSwitchColor {
    fn handle_control(
        ep: Attribute,
        frame: CommandClassSwitchColorFrame,
    ) -> Result<sl_status_t, AttributeStoreError> {
        match frame {
            CommandClassSwitchColorFrame::SwitchColorSupportedReportFrame(f) => {
                switch_color_handle_supported_report(ep, f)
            }
            CommandClassSwitchColorFrame::SwitchColorReportFrame(f) => {
                switch_color_handle_report(ep, f)
            }
            _ => Ok(SL_STATUS_NOT_SUPPORTED),
        }
    }

    pub fn on_support(
        &self,
        info: zwave_controller_sys::zwave_controller_connection_info,
        frame: &[u8],
    ) -> sl_status_t {
        SL_STATUS_NOT_SUPPORTED
    }

    pub fn on_init(&self, status: sl_status_t) -> sl_status_t {
        attribute_resolver_register_rule(
            zwSWITCH_COLOR_COLOR_COMPONENT_ID,
            None,
            as_extern_c!(switch_color_supported_get),
        );

        attribute_resolver_register_rule(
            zwSWITCH_COLOR_CURRENT_VALUE,
            None,
            as_extern_c!(switch_color_get),
        );

        contiki_spawn(on_switch_color_version_change_async());
        SL_STATUS_OK
    }

    pub fn on_control(
        &self,
        info: zwave_command_classes_sys::zwave_controller_connection_info,
        frame: CommandClassSwitchColorFrame,
    ) -> sl_status_t {
        let ep = unsafe {
            let ep_node = zwave_command_class_get_endpoint_node(
                &info as *const zwave_command_classes_sys::zwave_controller_connection_info,
            );
            Attribute::new_from_handle(ep_node).unwrap()
        };
        match Self::handle_control(ep, frame) {
            Ok(status) => status,
            Err(e) => {
                log_error!("error handling control frame {}", e);
                SL_STATUS_FAIL
            }
        }
    }
}

//////////////////////////////////////// Tests ///////////////////////////////////
#[cfg(test)]
mod test {
    use super::*;
    use serial_test::serial;
    use std::ffi::CString;
    use std::sync::Once;

    extern "C" {
        pub fn datastore_init(database: *const std::os::raw::c_char) -> u32;
    }

    static INIT: Once = Once::new();
    pub fn initialize() {
        INIT.call_once(|| unsafe {
            let database_name = CString::new(":memory:").unwrap();
            assert_eq!(0, datastore_init(database_name.as_ptr()));
        });
        Attribute::root().unwrap().delete().unwrap();
    }

    #[test]
    #[serial]
    fn test_on_interview() {
        use std::convert::TryInto;
        initialize();
        let ep = Attribute::root().unwrap();

        //Check that we create the first color component id
        {
            on_attribute_version(ep, 2).expect("attribute store error");
            let components = ep
                .get_children_by_type(zwSWITCH_COLOR_COLOR_COMPONENT_ID)
                .unwrap();
            assert_eq!(1, components.len());
        }

        // Check that we can handle a SUPPORTED report
        let z = [0x33u8, 0x02u8, 0x03u8, 0x00u8];
        let f = (z[..]).try_into();
        ZwaveCommandClassSwitchColor::handle_control(ep, f.unwrap()).unwrap();

        let components = ep
            .get_children_by_type(zwSWITCH_COLOR_COLOR_COMPONENT_ID)
            .unwrap();
        //Expect that we get two color components, 1 and 2
        assert_eq!(2, components.len());
        assert_eq!(1, components[0].get_reported().unwrap());
        assert_eq!(2, components[1].get_reported().unwrap());

        //Check that we can do a get
        let target_value1 = components[0]
            .get_child_by_type(zwSWITCH_COLOR_TARGET_VALUE)
            .unwrap();
        let current_value1 = components[0]
            .get_child_by_type(zwSWITCH_COLOR_CURRENT_VALUE)
            .unwrap();

        let frame_result = switch_color_get_rust(target_value1).unwrap();
        assert_eq!((SL_STATUS_OK, vec![0x33u8, 0x03u8, 0x01u8]), frame_result);

        //Check that we can handle a V3 report
        let z = [0x33u8, 0x04u8, 0x01u8, 0x42u8, 0xaa, 0x00];
        let f = (z[..]).try_into();
        ZwaveCommandClassSwitchColor::handle_control(ep, f.unwrap()).unwrap();

        assert_eq!(0x42, current_value1.get_reported().unwrap());
        assert_eq!(0xaa, target_value1.get_reported().unwrap());

        //Check that we can handle a V1 report
        let z = [0x33u8, 0x04u8, 0x01u8, 0x42u8];
        let f = (z[..]).try_into();
        ZwaveCommandClassSwitchColor::handle_control(ep, f.unwrap()).unwrap();

        assert_eq!(0x42, current_value1.get_reported().unwrap());
        assert_eq!(0x0, target_value1.get_reported().unwrap());

        //Can we generate a set?
        components[0]
            .get_child_by_type(zwSWITCH_COLOR_TARGET_VALUE)
            .unwrap()
            .set_desired(Some(0xaa))
            .unwrap();
        components[1]
            .get_child_by_type(zwSWITCH_COLOR_TARGET_VALUE)
            .unwrap()
            .set_desired(Some(0xbb))
            .unwrap();
        let frame_result2 = switch_color_set_rust(target_value1).unwrap();
        assert_eq!(
            (
                SL_STATUS_OK,
                vec![0x33u8, 0x05u8, 0x02u8, 0x01u8, 0xaau8, 0x02u8, 0xbbu8, 0x00u8]
            ),
            frame_result2
        );
    }
}
