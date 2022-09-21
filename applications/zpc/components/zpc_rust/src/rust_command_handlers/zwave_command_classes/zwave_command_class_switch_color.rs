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
#![allow(
    missing_docs,
    dead_code,
    non_snake_case,
    non_camel_case_types,
    non_upper_case_globals
)]
use crate::rust_command_handlers::{
    zwave_command_class_definitions::{ConnectionInfoTrait, ZwaveControllerEncapsulationScheme},
    zwave_command_class_interface::{
        CommandClassConfigurationTrait, FrameControlSupportTrait, ZwaveVersionChangedTrait,
    },
};
use crate::zwave_controller_sys::zwave_controller_endpoint_t;
use rust_command_class_frame_types::command_class_switch_color::*;
use unify_attribute_store_sys::{
    attribute_resolver_register_rule, attribute_resolver_restart_set_resolution,
    is_node_pending_set_resolution, sl_status_t,
};
use unify_log_sys::*;
use unify_middleware::{
    attribute_callback_by_type_and_state, attribute_store_or_return_with, Attribute,
    AttributeEvent, AttributeEventType, AttributeStoreError, AttributeTrait, AttributeValueState,
};
use unify_proc_macro::{as_extern_c, generate_resolver_callback};
use unify_sl_status_sys::{map_to_sl_status_fail, SL_STATUS_FAIL, SL_STATUS_OK};
use zwave_rust_proc_macros::zwave_command_class;

/// Attribute definitions
const ATTRIBUTE_ENDPOINT_ID: u32 = 0x4;
const ATTRIBUTE_VERSION: u32 = 0x3301;
const ATTRIBUTE_SUPPORTED_COLOR_COMPONENT_MASK: u32 = 0x3302;
const ATTRIBUTE_STATE: u32 = 0x3303;
const ATTRIBUTE_COLOR_COMPONENT_ID: u32 = 0x3304;
const ATTRIBUTE_COLOR_VALUE: u32 = 0x3305;
const ATTRIBUTE_COLOR_DURATION: u32 = 0x3306;

// Attribute types, copy from zwave_command_class_color_switch_types.h
type color_component_bitmask_t = u16;
type color_component_id_t = u8;
type color_component_id_value_t = u32;
type color_component_id_duration_t = u32;

type command_status_values_t = u32;
const FINAL_STATE: command_status_values_t = 0;
const NEEDS_ONE_COMMAND: command_status_values_t = 1;
const NEEDS_MULTIPLE_COMMANDS: command_status_values_t = 2;

declare_app_name!("zwave_command_class_switch_color_control");

fn on_value_update(event: &AttributeEvent<Attribute>) {
    if event.event_type != AttributeEventType::ATTRIBUTE_UPDATED {
        return;
    }
    // Verify if any of the values has a desired/reported mismatch under the state.
    let mut command_needed = false;

    let state_node = match event.attribute.get_first_parent_with_type(ATTRIBUTE_STATE) {
        Some(state) => state,
        None => {
            log_warning!("Cannot find state parent attribute for {}", event.attribute);
            return;
        }
    };

    let component_nodes = state_node.get_children_by_type(ATTRIBUTE_COLOR_COMPONENT_ID);

    // Verify if we have any mismatch
    for component_node in &component_nodes {
        let value_node = component_node.get_child_by_type(ATTRIBUTE_COLOR_VALUE);
        if true == value_node.is_desired_set() && false == value_node.is_value_matched() {
            command_needed = true;
            break;
        }
    }

    if command_needed == true {
        let _ = state_node
            .set_reported(Some(FINAL_STATE))
            .and(state_node.set_desired(Some(NEEDS_ONE_COMMAND)));
        if is_node_pending_set_resolution(state_node.handle) {
            attribute_resolver_restart_set_resolution(state_node.handle);
        }
    } else {
        let _ = state_node
            .set_reported(Some(FINAL_STATE))
            .and(state_node.set_desired(Some(FINAL_STATE)));
    }
}

///////////////////////// Command class registration ////////////////////
#[zwave_command_class]
impl CommandClassConfigurationTrait for CommandClassSwitchColorFrame {
    const COMMAND_CLASS_NAME: &'static str = "Color Switch";
    const MINIMAL_SCHEME: ZwaveControllerEncapsulationScheme =
        ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationNetworkScheme;
    const COMMENTS: &'static str = "Partial Control:<br>1. Durations are ignored<br>2. No support for Start/Stop Level Change<br>3. No support for other Color Component IDs than 0,1,2,3,4";
    const MANUAL_SECURITY_VALIDATION: bool = false;
    const INCLUDE_SUPPORT: bool = false;
    const INCLUDE_CONTROL: bool = true;

    fn on_init() -> sl_status_t {
        attribute_resolver_register_rule(
            ATTRIBUTE_SUPPORTED_COLOR_COMPONENT_MASK,
            None,
            as_extern_c!(switch_color_supported_get),
        );

        attribute_resolver_register_rule(ATTRIBUTE_STATE, as_extern_c!(switch_color_set), None);

        attribute_resolver_register_rule(
            ATTRIBUTE_COLOR_VALUE,
            None,
            as_extern_c!(switch_color_get),
        );

        attribute_callback_by_type_and_state(
            ATTRIBUTE_COLOR_VALUE,
            AttributeValueState::DESIRED_ATTRIBUTE,
            on_value_update,
        );
        attribute_callback_by_type_and_state(
            ATTRIBUTE_COLOR_DURATION,
            AttributeValueState::DESIRED_ATTRIBUTE,
            on_value_update,
        );

        SL_STATUS_OK
    }
}

///////////////////////////// Version Update ///////////////////////////////
#[zwave_command_class]
impl ZwaveVersionChangedTrait for CommandClassSwitchColorFrame {
    const ATTRIBUTE_STORE_TYPE: u32 = ATTRIBUTE_VERSION;

    fn on_attribute_version(ep: Attribute, version: u8) {
        // Use any color component id at token for starting the supported get
        if version > 0 {
            if !ep
                .get_child_by_type(ATTRIBUTE_SUPPORTED_COLOR_COMPONENT_MASK)
                .valid()
            {
                let _ = ep.add::<color_component_id_t>(
                    ATTRIBUTE_SUPPORTED_COLOR_COMPONENT_MASK,
                    None,
                    None,
                );
            }
        }
    }
}

/////////////////////////// Command Handlers ///////////////////////////
impl FrameControlSupportTrait for SwitchColorSupportedReportFrame {
    fn on_control(&self, info: &dyn ConnectionInfoTrait) -> Result<(), sl_status_t> {
        let ep = info.get_endpoint();

        // Save the bitmask of supported component IDs in the attribute store
        if let Err(e) = ep
            .get_child_by_type(ATTRIBUTE_SUPPORTED_COLOR_COMPONENT_MASK)
            .set_reported::<color_component_bitmask_t>(Some(self.color_component_mask))
        {
            log_warning!("could not set bitmask for color component {}", e);
        }

        let mut state_node = ep.get_child_by_type(ATTRIBUTE_STATE);
        if !state_node.valid() {
            state_node = ep
                .add::<command_status_values_t>(ATTRIBUTE_STATE, None, None)
                .map_err(map_to_sl_status_fail)?;
        }

        // Create the duration under the State node, if does not exist
        if !state_node
            .get_child_by_type(ATTRIBUTE_COLOR_DURATION)
            .valid()
        {
            state_node
                .add::<color_component_id_duration_t>(ATTRIBUTE_COLOR_DURATION, None, None)
                .map_err(map_to_sl_status_fail)?;
        }

        // The bits bitmask is byte reversed order with LSB bits in the first byte
        let mask = (self.color_component_mask >> 8) | (self.color_component_mask & 0xff);
        for i in 0..15 {
            if (mask & (1 << i)) != 0 {
                // Create all the attributes under the state (component IDs and their value/duration)
                let component = state_node
                    .emplace::<color_component_id_t>(ATTRIBUTE_COLOR_COMPONENT_ID, Some(i), None)
                    .map_err(map_to_sl_status_fail)?;

                // Create the value under the component, if does not exist
                if !component.get_child_by_type(ATTRIBUTE_COLOR_VALUE).valid() {
                    component
                        .add::<color_component_id_value_t>(ATTRIBUTE_COLOR_VALUE, None, None)
                        .map_err(map_to_sl_status_fail)?;
                }
            }
        }
        Ok(())
    }
}

impl FrameControlSupportTrait for SwitchColorReportFrame {
    fn on_control(&self, info: &dyn ConnectionInfoTrait) -> Result<(), sl_status_t> {
        let ep = info.get_endpoint();

        let state_node = ep.get_child_by_type(ATTRIBUTE_STATE);
        let component_node = state_node.get_child_by_type_and_value(
            ATTRIBUTE_COLOR_COMPONENT_ID,
            &Some(self.color_component_id),
        );
        let value_node = component_node.get_child_by_type(ATTRIBUTE_COLOR_VALUE);
        let duration_node = state_node.get_child_by_type(ATTRIBUTE_COLOR_DURATION);

        if !value_node.valid() || !duration_node.valid() {
            return Err(SL_STATUS_FAIL);
        }

        value_node.stop_transition();

        // UIC-1927 : change value_node.set_desired(None) to
        // .set_desired::<color_component_id_value_t>(Some(self.target_value.into()))
        // This will trigger the Rust callbacks to invoke on_value_update() later on.
        // which provokes that we try to set this desired value again.

        // Set all the values in the attribute store
        value_node
            .set_desired::<color_component_id_value_t>(None)
            .and(
                value_node
                    .set_reported::<color_component_id_value_t>(Some(self.current_value.into())),
            )
            .and(duration_node.set_desired::<color_component_id_duration_t>(None))
            .and(duration_node.set_reported(Some(self.duration as color_component_id_duration_t)))
            .map_err(map_to_sl_status_fail)?;

        // FIXME: UIC-1926 Here when we get the real duration, we want to
        // do like in Multilevel switch:
        // clock_time_t duration_time = zwave_duration_to_time(self.duration);
        // value_node.start_transition(duration_time);

        // FIXME: UIC-1927 See above. this is normally how we prevent to resolve
        // after updating the desired value ourselves.
        // Make sure not to resolve Color Switch values if the node tells us its desired state
        let _ = state_node
            .set_reported(Some(FINAL_STATE))
            .and(state_node.set_desired(Some(FINAL_STATE)));

        log_debug!(
            "NodeID {}:{} reported Color ID {} set to {}",
            unsafe { info.get_remote().__bindgen_anon_1.node_id },
            info.get_remote().endpoint_id,
            self.color_component_id,
            self.current_value
        );

        Ok(())
    }
}

/////////////////////// Command Generators //////////////////////////

#[generate_resolver_callback]
fn switch_color_get(value_node: Attribute) -> Result<(sl_status_t, Vec<u8>), AttributeStoreError> {
    let component_node = value_node
        .get_first_parent_with_type(ATTRIBUTE_COLOR_COMPONENT_ID)
        .ok_or(AttributeStoreError::InvalidType)?;
    let get_frame = SwitchColorGetFrame {
        color_component_id: component_node.get_reported::<color_component_id_t>()?,
    };
    Ok((SL_STATUS_OK, get_frame.into()))
}

#[generate_resolver_callback]
fn switch_color_set(state_node: Attribute) -> Result<(sl_status_t, Vec<u8>), AttributeStoreError> {
    let mut colors = Vec::new();
    // FIXME: UIC-1926 Default should be 0xFF if we have no desired value in the
    // attribute store. I really can't use SwitchColorSetDurationEnum::Default, which is 0
    // the "real" default value is 0xFF
    let mut _duration: color_component_id_duration_t = 0xFF as color_component_id_duration_t;
    // Check if this component has a desired duration
    match state_node
        .get_child_by_type(ATTRIBUTE_COLOR_DURATION)
        .get_desired::<color_component_id_duration_t>()
    {
        Ok(desired_duration) => _duration = desired_duration,
        Err(_) => (),
    }
    // Gather data tor all color components
    for component in state_node.get_children_by_type(ATTRIBUTE_COLOR_COMPONENT_ID) {
        let value_attribute = component.get_child_by_type(ATTRIBUTE_COLOR_VALUE);
        let value: u32 = value_attribute.get_desired_or_reported::<color_component_id_value_t>()?;
        let color_value = SwitchColorSetVg1Vg {
            color_component_id: component.get_reported::<color_component_id_t>()?,
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
        // FIXME: UIC-1926 This SwitchColorSetFrame wants a SwitchColorSetDurationEnum for
        // the duration, which prevents me to use my duration found in the attribute store.
        duration: SwitchColorSetDurationEnum::Instantly,
    };

    Ok((SL_STATUS_OK, set_frame.into()))
}

#[generate_resolver_callback]
fn switch_color_supported_get(_: Attribute) -> Result<(sl_status_t, Vec<u8>), AttributeStoreError> {
    Ok((SL_STATUS_OK, (SwitchColorSupportedGetFrame {}).into()))
}

//////////////////////////////////////// Tests ///////////////////////////////////
#[cfg(test)]
mod test {
    use super::*;
    use crate::zwave_command_class_definitions::MockConnectionInfoTrait;
    use serial_test::serial;
    use std::ffi::CString;
    use std::sync::Once;
    use unify_middleware::AttributeStore;
    // use zwave_controller_sys::zwave_controller_endpoint_t;
    use unify_middleware::AttributeStoreTrait;

    extern "C" {
        pub fn datastore_init(database: *const std::os::raw::c_char) -> u32;
        pub fn zpc_attribute_store_test_helper_create_network();
        pub fn attribute_store_log();
    }

    static INIT: Once = Once::new();
    pub fn initialize() {
        INIT.call_once(|| unsafe {
            let database_name = CString::new(":memory:").unwrap();
            assert_eq!(0, datastore_init(database_name.as_ptr()));
        });
        AttributeStore::new().unwrap().root().delete().unwrap();
    }

    #[test]
    #[serial]
    fn test_on_interview() {
        use std::convert::TryInto;

        initialize();
        let ep = AttributeStore::new().unwrap().root();

        //Check that we create the first color component id
        CommandClassSwitchColorFrame::on_attribute_version(ep, 2);

        // Check that we can handle a SUPPORTED report
        let z = [0x33u8, 0x02u8, 0x03u8, 0x00u8];
        let f: CommandClassSwitchColorFrame = (z[..]).try_into().unwrap();

        let mut mock_connection_info = MockConnectionInfoTrait::new();
        mock_connection_info
            .expect_get_endpoint()
            .times(1)
            .return_const(ep);

        let status = f.on_control(&mock_connection_info);
        assert_eq!(Ok(()), status);

        //Expect that we get two color components, 0 and 1
        let state = ep.get_child_by_type(ATTRIBUTE_STATE);
        let components = state.get_children_by_type(ATTRIBUTE_COLOR_COMPONENT_ID);

        assert_eq!(2, components.len());
        assert_eq!(
            0,
            components[0]
                .get_reported::<color_component_id_t>()
                .unwrap()
        );
        assert_eq!(
            1,
            components[1]
                .get_reported::<color_component_id_t>()
                .unwrap()
        );

        //Check that we can do a get
        let value0 = components[0].get_child_by_type(ATTRIBUTE_COLOR_VALUE);
        let duration = state.get_child_by_type(ATTRIBUTE_COLOR_DURATION);
        let value1 = components[1].get_child_by_type(ATTRIBUTE_COLOR_VALUE);

        let mut frame_result = switch_color_get_rust(value0).unwrap();
        assert_eq!((SL_STATUS_OK, vec![0x33u8, 0x03u8, 0x00u8]), frame_result);

        frame_result = switch_color_get_rust(value1).unwrap();
        assert_eq!((SL_STATUS_OK, vec![0x33u8, 0x03u8, 0x01u8]), frame_result);

        // Check that we can handle a V3 report
        // FIXME: UIC-1926 Frame parsing dies if duration is not one of these
        // 3 values : SwitchColorReportDurationEnum
        // We should be very tolerant in frame parsing and accept quirky values (event when they are out of spec)
        // so stopping the frame parsing when a value is wrong is a bit strong here.
        let z = [
            0x33u8, 0x04u8, 0x01u8, 0x42u8, 0xaau8, 0xFEu8, 0x00u8, 0x00u8, 0x00u8,
        ];
        let f: CommandClassSwitchColorFrame = (z[..]).try_into().unwrap();

        mock_connection_info
            .expect_get_endpoint()
            .times(1)
            .return_const(ep);
        let mock_controller_endpoint_info: zwave_controller_endpoint_t = Default::default();
        mock_connection_info
            .expect_get_remote()
            .times(2)
            .return_const(mock_controller_endpoint_info);

        let status = f.on_control(&mock_connection_info);
        assert_eq!(Ok(()), status);

        // Verify that the attribute store was updated
        assert_eq!(
            0x42,
            value1.get_reported::<color_component_id_value_t>().unwrap()
        );
        // UIC-1927 : Reactivate this test
        //assert_eq!(
        //    0xaa,
        //    value1.get_desired::<color_component_id_value_t>().unwrap()
        //);
        // FIXME: UIC-1926 Frame parser fail. Here we receive 0xFE in the report and saves value 0x01
        // SwitchColorReportDurationEnum::UnknownDuration in the attribute store
        /*
        assert_eq!(
            0xFEu32,
            duration
                .get_reported::<color_component_id_duration_t>()
                .unwrap()
        );
        */

        //Check that we can handle a V1 report
        let z = [0x33u8, 0x04u8, 0x01u8, 0x52u8];
        let f: CommandClassSwitchColorFrame = (z[..]).try_into().unwrap();
        // ZwaveCommandClassSwitchColor::handle_control(ep, f.unwrap()).unwrap();

        mock_connection_info
            .expect_get_endpoint()
            .times(1)
            .return_const(ep);
        mock_connection_info
            .expect_get_remote()
            .times(2)
            .return_const(mock_controller_endpoint_info);

        let status = f.on_control(&mock_connection_info);
        assert_eq!(Ok(()), status);
        assert_eq!(
            0x52,
            value1.get_reported::<color_component_id_value_t>().unwrap()
        );
        // UIC-1927 : Reactivate this test
        //assert_eq!(
        //    0x00,
        //    value1.get_desired::<color_component_id_value_t>().unwrap()
        //);

        let test_event = AttributeEvent {
            attribute: value0,
            event_type: AttributeEventType::ATTRIBUTE_UPDATED,
            value_state: AttributeValueState::DESIRED_ATTRIBUTE,
        };

        //Can we generate a set?
        value0
            .set_desired(Some(0xaa as color_component_id_value_t))
            .unwrap();
        value1
            .set_desired(Some(0xbb as color_component_id_value_t))
            .unwrap();
        duration
            .set_desired(Some(0x12 as color_component_id_value_t))
            .unwrap();

        // Trigger the attribute store callback, now the state node should
        // indicate that we need a set command
        assert_eq!(true, state.is_value_matched());
        on_value_update(&test_event);
        assert_eq!(false, state.is_value_matched());

        frame_result = switch_color_set_rust(state).unwrap();
        assert_eq!(
            (
                SL_STATUS_OK,
                vec![0x33u8, 0x05u8, 0x02u8, 0x00u8, 0xaau8, 0x01u8, 0xbbu8, 0x00u8]
            ),
            frame_result
        );

        // Align all the values
        assert_eq!(true, value0.set_desired_as_reported());
        assert_eq!(true, value1.set_desired_as_reported());
        // State attribute gets updated accordingly:
        assert_eq!(false, state.is_value_matched());
        on_value_update(&test_event);
        assert_eq!(true, state.is_value_matched());
    }
}
