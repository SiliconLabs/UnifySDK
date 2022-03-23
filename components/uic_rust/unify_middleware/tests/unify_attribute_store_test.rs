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

use serde::{Deserialize, Serialize};
use serial_test::serial;
use std::ffi::CString;
use std::sync::Once;
use unify_middleware::{
    Attribute, AttributeStoreError, AttributeStoreExtTrait, AttributeStoreTrait,
};

extern "C" {
    pub fn datastore_init(database: *const std::os::raw::c_char) -> u32;
    //pub fn datastore_teardown() -> u32;
}

fn setup_datastore() {
    unsafe {
        let database_name = CString::new(":memory:").unwrap();
        assert_eq!(0, datastore_init(database_name.as_ptr()));
    }
}
/*
fn teardown() {
    unsafe {
        assert_eq!(0, datastore_teardown());
    }
} */
static INIT: Once = Once::new();

pub fn initialize() {
    INIT.call_once(|| {
        setup_datastore();
    });
}

#[test]
#[serial]
fn get_root() {
    initialize();
    let root = Attribute::root().unwrap();
    assert_eq!(1, root.type_of());
}

#[test]
#[serial]
fn add_node() {
    initialize();
    let root = Attribute::root().unwrap();
    assert_eq!(
        Err(AttributeStoreError::FailedToGet(root.clone())),
        root.get_desired::<u32>()
    );
    let child42 = root.add::<u32>(42, None, None).unwrap();
    assert_eq!(42, child42.type_of());
    assert_eq!(root, child42.get_parent().unwrap());
    assert_eq!(
        Err(AttributeStoreError::FailedToGet(child42.clone())),
        child42.get_desired::<u32>()
    );
    let child43 = child42.add(43, Some(321_u32), Some(123_u32)).unwrap();
    assert_eq!(43, child43.type_of());
    assert_eq!(123_u32, child43.get_desired().unwrap());
    assert_eq!(321_u32, child43.get_reported().unwrap());
    assert_eq!(child42, child43.get_parent().unwrap());
}

#[test]
#[serial]
fn attribute_type_string() {
    initialize();
    let root = Attribute::root().unwrap();
    let child = root.add::<u32>(100, None, None).unwrap();
    let desired = "Hello Attribute Store".to_string();
    child.set_desired(Some(desired.clone())).unwrap();
    let result: String = child.get_desired().unwrap();
    assert_eq!(desired, result);

    let reported = "hello world str";
    child.set_reported(Some(reported.clone())).unwrap();
    let result: String = child.get_reported().unwrap();
    assert_eq!(reported, result);
}

#[derive(Serialize, Deserialize, Clone, PartialEq, Debug)]
struct TestType1 {
    t1: u8,
    t2: u16,
    t3: u32,
}

#[test]
#[serial]
fn attribute_type_test_type1() {
    initialize();
    let root = Attribute::root().unwrap();
    let child = root.add::<TestType1>(100, None, None).unwrap();
    let desired = TestType1 {
        t1: 42,
        t2: 1337,
        t3: 0xdeadbeef,
    };
    child.set_desired(Some(desired.clone())).unwrap();
    let result: TestType1 = child.get_desired().unwrap();
    assert_eq!(desired, result);
}

#[test]
#[serial]
fn attribute_children() {
    initialize();
    let root = Attribute::root().unwrap();
    root.delete().unwrap();
    let _ = root
        .add::<TestType1>(100, None, None)
        .expect("adding a child to root should not fail");
    let _ = root
        .add::<TestType1>(100, None, None)
        .expect("adding a second child to root should not fail");

    let chlds = root.get_children_by_type(100).unwrap();
    assert_eq!(2, chlds.len());
}
