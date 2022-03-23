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

/// This module is used to define the mapping between primitive C types to Rust types.
/// The types are exported by the cargo-bindgen project and depending on
/// the platform a corresponding matching type is set in this module. This is required
/// since cargo bindgen exports code as `no_std` which means we dont have access
/// to the types that are present in `std::ffi`. Hence we need to define them
/// ourselfs. As a basis the same mapping as the types inside the `std::ffi` is
/// chosen.
#[allow(dead_code, non_snake_case, non_camel_case_types)]
pub mod ctypes {
    #[cfg(any(
        all(
            target_os = "linux",
            any(
                target_arch = "aarch64",
                target_arch = "arm",
                target_arch = "hexagon",
                target_arch = "powerpc",
                target_arch = "powerpc64",
                target_arch = "s390x",
                target_arch = "riscv64",
                target_arch = "riscv32"
            )
        ),
        all(
            target_os = "android",
            any(target_arch = "aarch64", target_arch = "arm")
        ),
        all(target_os = "l4re", target_arch = "x86_64"),
        all(
            target_os = "freebsd",
            any(
                target_arch = "aarch64",
                target_arch = "arm",
                target_arch = "powerpc",
                target_arch = "powerpc64"
            )
        ),
        all(
            target_os = "netbsd",
            any(target_arch = "aarch64", target_arch = "arm", target_arch = "powerpc")
        ),
        all(target_os = "openbsd", target_arch = "aarch64"),
        all(
            target_os = "vxworks",
            any(
                target_arch = "aarch64",
                target_arch = "arm",
                target_arch = "powerpc64",
                target_arch = "powerpc"
            )
        ),
        all(target_os = "fuchsia", target_arch = "aarch64")
    ))]
    pub type c_char = u8;
    #[cfg(not(any(
        all(
            target_os = "linux",
            any(
                target_arch = "aarch64",
                target_arch = "arm",
                target_arch = "hexagon",
                target_arch = "powerpc",
                target_arch = "powerpc64",
                target_arch = "s390x",
                target_arch = "riscv64",
                target_arch = "riscv32"
            )
        ),
        all(
            target_os = "android",
            any(target_arch = "aarch64", target_arch = "arm")
        ),
        all(target_os = "l4re", target_arch = "x86_64"),
        all(
            target_os = "freebsd",
            any(
                target_arch = "aarch64",
                target_arch = "arm",
                target_arch = "powerpc",
                target_arch = "powerpc64"
            )
        ),
        all(
            target_os = "netbsd",
            any(target_arch = "aarch64", target_arch = "arm", target_arch = "powerpc")
        ),
        all(target_os = "openbsd", target_arch = "aarch64"),
        all(
            target_os = "vxworks",
            any(
                target_arch = "aarch64",
                target_arch = "arm",
                target_arch = "powerpc64",
                target_arch = "powerpc"
            )
        ),
        all(target_os = "fuchsia", target_arch = "aarch64")
    )))]
    pub type c_char = i8;
    pub type c_schar = i8;
    pub type c_uchar = u8;
    pub type c_ushort = u16;
    pub type c_short = i16;
    pub type c_void = core::ffi::c_void;
    pub type c_int = i32;
    pub type c_uint = u32;
    #[cfg(any(target_pointer_width = "32", windows))]
    pub type c_long = i32;
    #[cfg(any(target_pointer_width = "32", windows))]
    pub type c_ulong = u32;
    #[cfg(all(target_pointer_width = "64", not(windows)))]
    pub type c_long = i64;
    #[cfg(all(target_pointer_width = "64", not(windows)))]
    pub type c_ulong = u64;
    pub type c_longlong = i64;
    pub type c_ulonglong = u64;
    pub type c_float = f32;
    pub type c_double = f64;
    pub type c_size_t = usize;
    pub type c_ptrdiff_t = isize;
}
