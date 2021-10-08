use super::*;
use serde::Deserialize;
use serial_test::serial;
use std::{ffi::CString, fmt::Debug};
use uic_proc_macro::{as_extern_c, generate_extern_c};

extern "C" {
    pub fn datastore_init(database: *const std::os::raw::c_char) -> sl_status_t;
    pub fn datastore_teardown() -> sl_status_t;
}

fn setup_datastore() {
    unsafe {
        let database_name = CString::new(":memory:").unwrap();
        assert_eq!(0, datastore_init(database_name.into_raw()));
        attribute_store::init().unwrap();
    }
}
fn teardown() {
    unsafe {
        assert_eq!(0, datastore_teardown());
    }
}

#[test]
#[serial]
fn add_simple_node_to_datastore() {
    setup_datastore();
    unsafe {
        let node_id = attribute_store_add_node(42, attribute_store_get_root());
        assert_ne!(0, node_id);
    }
    teardown();
}

// test function that writes the desired value into the attribute store,
// reads it again and compares it against the initial value.
fn read_write_assert_eq<T>(desired: T) -> u32
where
    T: Serialize + DeserializeOwned + Clone + PartialEq + Debug + Any,
{
    let node_id = attribute_store::add_node(42, unsafe { attribute_store_get_root() }).unwrap();
    attribute_store::write_attribute(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        desired.clone(),
    )
    .unwrap();
    let readback: T = attribute_store::read_attribute(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    )
    .unwrap();
    assert_eq!(readback, desired);
    node_id
}

#[test]
#[serial]
fn add_write_and_read() {
    setup_datastore();

    // test u8
    let test_val: u8 = 13;
    read_write_assert_eq(test_val);

    //test i32
    let desired_i32: i32 = 65212444;
    read_write_assert_eq(desired_i32);

    // test string
    // read as string
    // read as byte array
    let str = String::from("CD");
    let node_id = read_write_assert_eq(str);
    let readback_bytes2: Vec<u8> = attribute_store::read_attribute(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    )
    .unwrap();
    assert_eq!(readback_bytes2, vec![67, 68, 0]);
    let as_str: String = attribute_store::read_attribute(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    )
    .unwrap();
    assert_eq!("CD".to_string(), as_str);

    // test string from byte vec
    let desired_bytes: Vec<u8> = vec![65, 66, 0];
    let node_id = read_write_assert_eq(desired_bytes);
    let as_str: String = attribute_store::read_attribute(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    )
    .unwrap();
    assert_eq!("AB".to_string(), as_str);

    // test custom enum as u8
    #[repr(u8)]
    #[derive(Serialize, Deserialize, Clone, PartialEq, Debug)]
    enum TestEnum {
        test1 = 0,
        test2 = 1,
    }
    read_write_assert_eq(TestEnum::test2);

    teardown();
}

#[test]
#[serial]
fn overflowing_types() {
    setup_datastore();

    let node_id = attribute_store::add_node(42, unsafe { attribute_store_get_root() }).unwrap();
    attribute_store::write_attribute::<u32>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        0xFFFFFFFF,
    )
    .unwrap();
    let res = attribute_store::read_attribute::<u8>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    );
    assert!(res.is_err());

    teardown();
}

#[test]
#[serial]
fn test_empty_string() {
    setup_datastore();
    let empty = String::new();
    let node_id = attribute_store::add_node(42, unsafe { attribute_store_get_root() }).unwrap();

    attribute_store::write_attribute::<String>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        empty.clone(),
    )
    .unwrap();

    let res = attribute_store::read_attribute::<String>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    );
    assert_eq!(empty, res.unwrap());
}

#[test]
#[serial]
fn test_null_termination_string_error() {
    setup_datastore();
    // test1: not null terminated
    let data: [u8; 4] = [0x74, 0x65, 0x73, 0x74];
    let node_id = attribute_store::add_node(42, unsafe { attribute_store_get_root() }).unwrap();

    attribute_store::write_attribute::<[u8; 4]>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        data,
    )
    .unwrap();

    let res = attribute_store::read_attribute::<String>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    );

    if let Err(AttributeStoreError::NulTerminationError(_)) = res {
        // good!
    } else {
        panic!("unexpected error: {:?}", res);
    }

    // test2: null in string
    let data: [u8; 5] = [0x74, 0x0, 0x73, 0x74, 0x0];
    let node_id = attribute_store::add_node(42, unsafe { attribute_store_get_root() }).unwrap();

    attribute_store::write_attribute::<[u8; 5]>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
        data,
    )
    .unwrap();

    let res = attribute_store::read_attribute::<String>(
        node_id,
        attribute_store_node_value_state_t::DESIRED_ATTRIBUTE,
    );

    if let Err(AttributeStoreError::NulTerminationError(_)) = res {
        // good!
    } else {
        panic!("unexpected error: {:?}", res);
    }
}

#[generate_extern_c]
fn set_func(_node: u32, _data: *mut u8, _length: *mut u16) -> sl_status_t {
    0
}

#[generate_extern_c]
fn get_func(_node: u32, _data: *mut u8, _length: *mut u16) -> sl_status_t {
    0
}

#[test]
#[serial]
/// Test that attribute_resolver_register_rule is callable and doesn't return an error
fn attribute_resolver() {
    setup_datastore();
    assert_eq!(
        0,
        attribute_resolver_register_rule(42, as_extern_c!(set_func), as_extern_c!(get_func))
    );
    teardown();
}

static mut NODE_ID: u32 = 666;
static mut NODE_TYP: attribute_store_change_t = attribute_store_change_t::ATTRIBUTE_DELETED;
#[generate_extern_c]
fn callback_stub(node: u32, typ: attribute_store_change_t) {
    unsafe {
        NODE_ID = node;
        NODE_TYP = typ;
    }
}

static mut TYPE_CB_CALLED: bool = false;
#[generate_extern_c]
fn callback_type(_node: u32, _typ: attribute_store_change_t) {
    unsafe {
        TYPE_CB_CALLED = true;
    }
}

#[test]
#[serial]
fn test_callback_node() {
    setup_datastore();

    unsafe {
        attribute_store_register_callback(as_extern_c!(callback_stub));
        attribute_store_register_callback_by_type(as_extern_c!(callback_type), 42);
        let node_id = attribute_store_add_node(42, attribute_store_get_root());
        assert_eq!(NODE_ID, node_id);
        assert_eq!(NODE_TYP, attribute_store_change_t::ATTRIBUTE_CREATED);
        assert!(TYPE_CB_CALLED);
    }

    teardown();
}
