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

use crate::zwave_controller_sys::zwave_controller_encapsulation_scheme_t;

/// The [ZwaveControllerEncapsulationScheme] is a rust version of
/// [crate::zwave_command_handler_sys::zwave_controller_encapsulation_scheme_t] hiding
/// logic and quirks needed for dealing with C/C++ types.
#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq)]
pub enum ZwaveControllerEncapsulationScheme {
    ZwaveControllerEncapsulationNetworkScheme,
    ZwaveControllerEncapsulationNone,
    ZwaveControllerEncapsulationSecurity0,
    ZwaveControllerEncapsulationSecurity2Unauthenticated,
    ZwaveControllerEncapsulationSecurity2Authenticated,
    ZwaveControllerEncapsulationSecurity2Access,
}

/// Implement the conversion between the binding generated pub enum
/// [zwave_controller_encapsulation_scheme_t] to the duplicated
/// [ZwaveControllerEncapsulationScheme]
impl From<zwave_controller_encapsulation_scheme_t> for ZwaveControllerEncapsulationScheme {
    fn from(typ: zwave_controller_encapsulation_scheme_t) -> Self {
        match typ {
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME => ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationNetworkScheme,
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NONE => ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationNone,
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0 => ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity0,
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED => ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity2Unauthenticated,
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED => ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity2Authenticated,
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS => ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity2Access,
        }
    }
}

/// Implement the conversion between the binding generated pub enum
/// [ZwaveControllerEncapsulationScheme] to the duplicated
/// [zwave_controller_encapsulation_scheme_t]
impl From<ZwaveControllerEncapsulationScheme> for zwave_controller_encapsulation_scheme_t {
    fn from(typ: ZwaveControllerEncapsulationScheme) -> Self {
        match typ {
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationNetworkScheme => zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationNone => zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity0 => zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity2Unauthenticated => zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity2Authenticated => zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED,
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity2Access => zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
            
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{zwave_controller_encapsulation_scheme_t, ZwaveControllerEncapsulationScheme};

    #[test]
    fn type_conversion() {
        // Conversion from binding encapsulation scheme to rust encapsulation scheme.
        let network_encapsulation_scheme = ZwaveControllerEncapsulationScheme::from(
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
        );
        assert_eq!(
            network_encapsulation_scheme,
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationNetworkScheme
        );

        let network_encapsulation_scheme = zwave_controller_encapsulation_scheme_t::from(
            ZwaveControllerEncapsulationScheme::ZwaveControllerEncapsulationSecurity0,
        );
        assert_eq!(
            network_encapsulation_scheme,
            zwave_controller_encapsulation_scheme_t::ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0
        );
    }
}
