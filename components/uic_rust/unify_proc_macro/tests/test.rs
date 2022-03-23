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
#[cfg(test)]
mod test {
    use unify_proc_macro::*;

    static mut INT32_CALLED: u32 = 0;
    static mut STR_CALLED: Option<String> = None;
    static mut STRING_CALLED: Option<String> = None;
    static mut DATA_CALLED: Vec<u8> = Vec::new();
    static mut DATA_CALLED2: Vec<u8> = Vec::new();

    #[generate_extern_c]
    fn test_func(arg: u32, data: &[u8], str_ref: &str, str: String, mutdata: &mut [u8]) {
        unsafe {
            INT32_CALLED = arg;
            STR_CALLED = Some(str_ref.to_string());
            STRING_CALLED = Some(str);
            DATA_CALLED.extend_from_slice(data);

            mutdata[0] = 0;
            DATA_CALLED2.extend_from_slice(mutdata);
        }
    }

    #[test]
    fn test_func_conversion() {
        unsafe {
            let cstr = std::ffi::CString::new("test_string").unwrap();
            let cstring = std::ffi::CString::new("string2").unwrap();
            let arr: [u8; 5] = [6; 5];
            let mut mutarr: [u8; 5] = [6; 5];

            test_func_c(
                666,
                arr.as_ptr(),
                arr.len() as u16,
                cstr.as_ptr(),
                cstring.as_ptr(),
                mutarr.as_mut_ptr(),
                mutarr.len() as u16,
            );

            mutarr[0] = 0;

            assert_eq!(INT32_CALLED, 666);
            assert_eq!(STR_CALLED.clone().unwrap(), "test_string".to_string());
            assert_eq!(STRING_CALLED.clone().unwrap(), "string2".to_string());
            assert_eq!(DATA_CALLED, arr.to_vec());
            assert_eq!(DATA_CALLED2, mutarr.to_vec());
        }
    }
}
