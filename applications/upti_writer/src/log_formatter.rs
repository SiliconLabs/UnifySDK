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

// Message type CONST is generated from debug-message-type.json, feature date "2022.1.8"
const TIME_SYNC: u16 = 0x0000;
const RESET_INFO: u16 = 0x0001;
const PRINTF: u16 = 0x0002;
const API_TRACE: u16 = 0x0003;
const ASSERT: u16 = 0x0004;
const CORE_DUMP: u16 = 0x0005;
const PHY_RX: u16 = 0x0006;
const API_RX: u16 = 0x0007;
const PHY_TX: u16 = 0x0008;
const API_TX: u16 = 0x0009;
const PACKET_TRACE: u16 = 0x000A;
const ERROR: u16 = 0x000B;
const STATS: u16 = 0x000C;
const TIME_SYNC_TEST: u16 = 0x000D;
const RADIO_REBOOT_COUNT: u16 = 0x000E;
const VIRTUAL_UART_TX: u16 = 0x0011;
const VIRTUAL_UART_RX: u16 = 0x0012;
const PACKET_TRACE_EM2420_TX: u16 = 0x0020;
const PACKET_TRACE_EM2420_RX: u16 = 0x0021;
const PACKET_TRACE_EM2XX_TX: u16 = 0x0022;
const PACKET_TRACE_EM2XX_RX: u16 = 0x0023;
const PACKET_TRACE_EM3XX_TX: u16 = 0x0024;
const PACKET_TRACE_EM3XX_RX: u16 = 0x0025;
const PACKET_TRACE_PRO2P_TX: u16 = 0x0026;
const PACKET_TRACE_PRO2P_RX: u16 = 0x0027;
const PACKET_TRACE_PRO2P_OTHER: u16 = 0x0028;
const PACKET_TRACE_EFR_TX: u16 = 0x0029;
const PACKET_TRACE_EFR_RX: u16 = 0x002A;
const PACKET_TRACE_EFR_OTHER: u16 = 0x002B;
const FLASH_READ_REQUEST: u16 = 0x0030;
const FLASH_READ_RESPONSE: u16 = 0x0031;
const EEPROM_READ_REQUEST: u16 = 0x0032;
const EEPROM_READ_RESPONSE: u16 = 0x0033;
const EEPROM_WRITE_REQUEST: u16 = 0x0034;
const EEPROM_WRITE_RESPONSE: u16 = 0x0035;
const RAM_READ_REQUEST: u16 = 0x0036;
const RAM_READ_RESPONSE: u16 = 0x0037;
const RAM_WRITE_REQUEST: u16 = 0x0038;
const RAM_WRITE_RESPONSE: u16 = 0x0039;
const INFO_REQUEST: u16 = 0x003A;
const INFO_RESPONSE: u16 = 0x003B;
const EZSP: u16 = 0x003C;
const EZSP_UART: u16 = 0x003D;
const DAG_TRACE: u16 = 0x003E;
const SIMULATED_EZSP_CALLBACK_READY: u16 = 0x003F;
const SIMULATED_WAKEUP_NCP: u16 = 0x0040;
const SIMULATED_NCP_IS_AWAKE: u16 = 0x0041;
const INFO_ZNET_VERSION: u16 = 0x0042;
const INFO_ZIP_VERSION: u16 = 0x0043;
const TIME: u16 = 0x0044;
const HEAP_STACK: u16 = 0x0045;
const MUSTANG: u16 = 0x0046;
const LATENCY: u16 = 0x0047;
const TMSP: u16 = 0x0048;
const AEM_SAMPLE: u16 = 0x0050;
const AEM_COUNTER: u16 = 0x0051;
const AEM_REQUEST: u16 = 0x0060;
const AEM_RESPONSE: u16 = 0x0061;
const AEM_CURRENT_PACKET: u16 = 0x0062;
const AEM_CURRENT_PACKET_V2: u16 = 0x0063;
const PC_SAMPLE_PACKET: u16 = 0x0064;
const EXCEPTION_PACKET: u16 = 0x0065;
const CPU_USAGE: u16 = 0x0070;
const USER_COMMAND: u16 = 0xFFFE;
const USER_RESPONSE: u16 = 0xFFFF;

// MASK and CATEGRY CONST
const MASK_PACKET: u32 = 1 << 24;
const MASK_AEM: u32 = 1 << 27;
const CATEGORY_UNKNOWN: u32 = 0xFF << 16;
const CATEGORY_BACKCHANNEL: u32 = 0x02 << 16;

// Debug frame Initial and End symbols
static OPEN_BRACKET: u8 = 0x5B; // '[' symbol
static CLOSE_BRACKET: u8 = 0x5D; // ']' symbol

#[derive(PartialEq)]
#[derive(Debug)]
struct DebugData {
    network_time: u64,
    duration: u16,
    debug_type: u16,
    seq_number: u16,
    content: Vec<u8>
}

/// Constructs formatted log record from the raw data
pub fn log_data_from_raw(raw_data: Vec<u8>, originator: String) -> Option<String> {
    // try to parse data
    if let Some(debug_data) = parse_raw(raw_data) {
        let (dm_name, dm_type) = get_full_debug_type(debug_data.debug_type);
        let content_hex_string = debug_data.content.iter().fold(String::new(), |acc, &num| acc + &format!("{:02X} ", num));
        let record = format!("[{} {} {} {} {:02X}] [{}] [{}]\n", 
            debug_data.network_time, 
            debug_data.duration,
            dm_type,
            dm_name, 
            debug_data.seq_number, 
            originator,
            content_hex_string.trim());
        return Some(record);
    } else {
        return None;
    }
}

/// Constructs a DebugData object from the raw message bytes. Assumes that the
/// message has been processed by the backchannel board (so that it is in the
/// ethernet format rather than the serial format) and that the framing and
/// length bytes have been removed.
///
/// Version 2.0 Message: 
/// 0 version number \ LSB, direction (0 = node to pc, 1 =  pc to node) 
/// 1 version number / MSB 
/// 2 timestamp byte 0 \ LSB, microsecond tics
/// 3 timestamp byte 1 | 
/// 4 timestamp byte 2 | 
/// 5 timestamp byte 3 | 
/// 6 timestamp byte 4 | 
/// 7 timestamp byte 5 / MSB 
/// 8 Debug Message Type \ LSB (coresponds to single byte message type). 
/// 9 Debug Message Type / MSB 10 sequence number ...
/// Data dependant on the Message Type
///
///
/// Version 3.0 Message: 
/// 0 version number \ LSB, direction (0 = node to pc, 1 =pc to node) 
/// 1 version number / MSB 
/// 2 timestamp byte 0 \ LSB, nanosecond tics
/// 3 timestamp byte 1 | 
/// 4 timestamp byte 2 | 
/// 5 timestamp byte 3 | 
/// 6 timestamp byte 4 | 
/// 7 timestamp byte 5 | 
/// 8 timestamp byte 6 | 
/// 9 timestamp byte 7 / MSB
/// 10 Debug Message Type \ LSB (corresponds to double byte message type. 
/// 11 Debug Message Type / MSB 
/// 12 Flags \ LSB 
/// 13 Flags | 
/// 14 Flags | 
/// 15 Flags / MSB
/// 16 sequence number 
/// 17 sequence number ... 
/// Data dependent on the Message Type
fn parse_raw(raw_data: Vec<u8>) -> Option<DebugData> {
    if raw_data[0] != OPEN_BRACKET {return None};
    let data_len: usize = (raw_data[2] as usize) << 8 | raw_data[1] as usize;
    if raw_data.len() != data_len+2 {return None};
    if raw_data[(data_len+1) as usize] != CLOSE_BRACKET {return None};
    // deframed data
    let raw = &raw_data[3..(1 + data_len)];

    if raw[0] == 3 {
        if raw.len() < 18 {return None};
        let network_time: u64 =(((raw[9] as u64) << 56)
            | ((raw[8] as u64) << 48)
            | ((raw[7] as u64) << 40)
            | ((raw[6] as u64) << 32)
            | ((raw[5] as u64) << 24)
            | ((raw[4] as u64) << 16)
            | ((raw[3] as u64) << 8)
            | ((raw[2] as u64) << 0)) / 1000;
        let debug_type: u16 = ((raw[11] as u16) << 8)
            | ((raw[10] as u16) << 0);

        // Flag bytes not parsed, reserved for future use
        let seq_number: u16 = ((raw[17] as u16) << 8) 
            | ((raw[16] as u16) << 0);
        let content: Vec<u8>= raw[18..(raw.len())].to_vec();

        return Some(DebugData{
            network_time: network_time,
            duration: 800, // durtion value is hardcoded
            debug_type: debug_type,
            seq_number: seq_number,
            content: content
        });
    } else {
    // We're assuming version 2, since in the code we occasionally parsed
    // random junk as version 2.
        if raw.len() < 11 {return None};
        let network_time: u64 = ((raw[7] as u64) << 40)
            | ((raw[6] as u64) << 32)
            | ((raw[5] as u64) << 24)
            | ((raw[4] as u64) << 16)
            | ((raw[3] as u64) << 8)
            | ((raw[2] as u64) << 0);
        let debug_type: u16 = ((raw[9] as u16) << 8)
            | ((raw[8] as u16) << 0);
        
        let seq_number: u16 = (raw[10] & 0xFF) as u16;
        let content: Vec<u8>= raw[11..(raw.len())].to_vec();

        return Some(DebugData{
            network_time: network_time,
            duration: 800, // durtion value is hardcoded
            debug_type: debug_type,
            seq_number: seq_number,
            content: content
        });
    }
}

/// Returns the event name and event id based on the event type.
/// Match is generated from debug-message-type.json, feature date "2022.1.8" 
fn get_full_debug_type(debug_type: u16) -> (&'static str, u32) {
    match debug_type {
        TIME_SYNC => return ("TimeSync", CATEGORY_BACKCHANNEL | TIME_SYNC as u32),
        RESET_INFO => return ("Reset", CATEGORY_BACKCHANNEL | RESET_INFO as u32),
        PRINTF => return ("Printf", CATEGORY_BACKCHANNEL | PRINTF as u32),
        API_TRACE => return ("APITrace", CATEGORY_BACKCHANNEL | API_TRACE as u32),
        ASSERT => return ("Assert", CATEGORY_BACKCHANNEL | ASSERT as u32),
        CORE_DUMP => return ("CoreDump", CATEGORY_BACKCHANNEL | CORE_DUMP as u32),
        PHY_RX => return ("phy_rx", CATEGORY_BACKCHANNEL | PHY_RX as u32),
        API_RX => return ("api_rx", CATEGORY_BACKCHANNEL | API_RX as u32),
        PHY_TX => return ("phy_tx", CATEGORY_BACKCHANNEL | PHY_TX as u32),
        API_TX => return ("api_tx", CATEGORY_BACKCHANNEL | API_TX as u32),
        PACKET_TRACE => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE as u32),
        ERROR => return ("Error", CATEGORY_BACKCHANNEL | ERROR as u32),
        STATS => return ("Stats", CATEGORY_BACKCHANNEL | STATS as u32),
        TIME_SYNC_TEST => return ("time_sync_", CATEGORY_BACKCHANNEL | TIME_SYNC_TEST as u32),
        RADIO_REBOOT_COUNT => return ("radio_rebo", CATEGORY_BACKCHANNEL | RADIO_REBOOT_COUNT as u32),
        VIRTUAL_UART_TX => return ("UART", CATEGORY_BACKCHANNEL | VIRTUAL_UART_TX as u32),
        VIRTUAL_UART_RX => return ("UART", CATEGORY_BACKCHANNEL | VIRTUAL_UART_RX as u32),
        PACKET_TRACE_EM2420_TX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EM2420_TX as u32),
        PACKET_TRACE_EM2420_RX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EM2420_RX as u32),
        PACKET_TRACE_EM2XX_TX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EM2XX_TX as u32),
        PACKET_TRACE_EM2XX_RX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EM2XX_RX as u32),
        PACKET_TRACE_EM3XX_TX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EM3XX_TX as u32),
        PACKET_TRACE_EM3XX_RX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EM3XX_RX as u32),
        PACKET_TRACE_PRO2P_TX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_PRO2P_TX as u32),
        PACKET_TRACE_PRO2P_RX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_PRO2P_RX as u32),
        PACKET_TRACE_PRO2P_OTHER => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_PRO2P_OTHER as u32),
        PACKET_TRACE_EFR_TX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EFR_TX as u32),
        PACKET_TRACE_EFR_RX => return ("Packet", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EFR_RX as u32),
        PACKET_TRACE_EFR_OTHER => return ("PTI", MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EFR_OTHER as u32),
        FLASH_READ_REQUEST => return ("flash_read", CATEGORY_BACKCHANNEL | FLASH_READ_REQUEST as u32),
        FLASH_READ_RESPONSE => return ("flash_read", CATEGORY_BACKCHANNEL | FLASH_READ_RESPONSE as u32),
        EEPROM_READ_REQUEST => return ("eeprom_rea", CATEGORY_BACKCHANNEL | EEPROM_READ_REQUEST as u32),
        EEPROM_READ_RESPONSE => return ("eeprom_rea", CATEGORY_BACKCHANNEL | EEPROM_READ_RESPONSE as u32),
        EEPROM_WRITE_REQUEST => return ("eeprom_wri", CATEGORY_BACKCHANNEL | EEPROM_WRITE_REQUEST as u32),
        EEPROM_WRITE_RESPONSE => return ("eeprom_wri", CATEGORY_BACKCHANNEL | EEPROM_WRITE_RESPONSE as u32),
        RAM_READ_REQUEST => return ("ram_read_r", CATEGORY_BACKCHANNEL | RAM_READ_REQUEST as u32),
        RAM_READ_RESPONSE => return ("ram_read_r", CATEGORY_BACKCHANNEL | RAM_READ_RESPONSE as u32),
        RAM_WRITE_REQUEST => return ("ram_write_", CATEGORY_BACKCHANNEL | RAM_WRITE_REQUEST as u32),
        RAM_WRITE_RESPONSE => return ("ram_write_", CATEGORY_BACKCHANNEL | RAM_WRITE_RESPONSE as u32),
        INFO_REQUEST => return ("info_reque", CATEGORY_BACKCHANNEL | INFO_REQUEST as u32),
        INFO_RESPONSE => return ("NodeInfo", CATEGORY_BACKCHANNEL | INFO_RESPONSE as u32),
        EZSP => return ("EZSP", CATEGORY_BACKCHANNEL | EZSP as u32),
        EZSP_UART => return ("ASH", CATEGORY_BACKCHANNEL | EZSP_UART as u32),
        DAG_TRACE => return ("DagTrace", CATEGORY_BACKCHANNEL | DAG_TRACE as u32),
        SIMULATED_EZSP_CALLBACK_READY => return ("EZSP_RDY", CATEGORY_BACKCHANNEL | SIMULATED_EZSP_CALLBACK_READY as u32),
        SIMULATED_WAKEUP_NCP => return ("NCP_WAKEUP", CATEGORY_BACKCHANNEL | SIMULATED_WAKEUP_NCP as u32),
        SIMULATED_NCP_IS_AWAKE => return ("NCP_AWAKE", CATEGORY_BACKCHANNEL | SIMULATED_NCP_IS_AWAKE as u32),
        INFO_ZNET_VERSION => return ("ZnetVer", CATEGORY_BACKCHANNEL | INFO_ZNET_VERSION as u32),
        INFO_ZIP_VERSION => return ("ZipVer", CATEGORY_BACKCHANNEL | INFO_ZIP_VERSION as u32),
        TIME => return ("Time", CATEGORY_BACKCHANNEL | TIME as u32),
        HEAP_STACK => return ("MemInfo", CATEGORY_BACKCHANNEL | HEAP_STACK as u32),
        MUSTANG => return ("Mustang", CATEGORY_BACKCHANNEL | MUSTANG as u32),
        LATENCY => return ("latency", CATEGORY_BACKCHANNEL | LATENCY as u32),
        TMSP => return ("tmsp", CATEGORY_BACKCHANNEL | TMSP as u32),
        AEM_SAMPLE => return ("AEMSample", MASK_AEM | CATEGORY_BACKCHANNEL | AEM_SAMPLE as u32),
        AEM_COUNTER => return ("AEMCounter", MASK_AEM | CATEGORY_BACKCHANNEL | AEM_COUNTER as u32),
        AEM_REQUEST => return ("AEMRequest", MASK_AEM | CATEGORY_BACKCHANNEL | AEM_REQUEST as u32),
        AEM_RESPONSE => return ("AEMRespons", MASK_AEM | CATEGORY_BACKCHANNEL | AEM_RESPONSE as u32),
        AEM_CURRENT_PACKET => return ("AEMPacket", MASK_AEM | CATEGORY_BACKCHANNEL | AEM_CURRENT_PACKET as u32),
        AEM_CURRENT_PACKET_V2 => return ("aem_curren", MASK_AEM | CATEGORY_BACKCHANNEL | AEM_CURRENT_PACKET_V2 as u32),
        PC_SAMPLE_PACKET => return ("pc_sample_", CATEGORY_BACKCHANNEL | PC_SAMPLE_PACKET as u32),
        EXCEPTION_PACKET => return ("exception_", CATEGORY_BACKCHANNEL | EXCEPTION_PACKET as u32),
        CPU_USAGE => return ("cpu_usage", CATEGORY_BACKCHANNEL | CPU_USAGE as u32),
        USER_COMMAND => return ("UserCmd", CATEGORY_BACKCHANNEL | USER_COMMAND as u32),
        USER_RESPONSE => return ("UserResp", CATEGORY_BACKCHANNEL | USER_RESPONSE as u32),
        _ => return ("Unknown", CATEGORY_UNKNOWN),
    };
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_debug_type() {
        const FAKE_EVENT: u16 = 0x0077;
        let (dm_name, dm_type) = get_full_debug_type(FAKE_EVENT);
        assert_eq!(dm_type, CATEGORY_UNKNOWN);
        assert_eq!(dm_name, "Unknown");

        let (dm_name, dm_type) = get_full_debug_type(PACKET_TRACE_EFR_RX);
        assert_eq!(dm_type, MASK_PACKET | CATEGORY_BACKCHANNEL | PACKET_TRACE_EFR_RX as u32);
        assert_eq!(dm_name, "Packet");
    }

    #[test]
    fn test_invalid_data() {
        // Frame with wrong Initial framing symbol
        let invalid_data_1: Vec<u8> = vec![
            0x5A, // Wrong Initial framing symbol
            0x1C, 0x00, 
            0x02, 0x00, 
            0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, 
            0x2A, 0x00, 
            0x03, 
            0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 0x00, 0x16, 0x51,
            0x5D 
        ];

        // Frame with wrong End framing symbol
        let invalid_data_2: Vec<u8> = vec![
            0x5B, 
            0x1C, 0x00, 
            0x02, 0x00, 
            0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, 
            0x2A, 0x00, 
            0x03, 
            0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 0x00, 0x16, 0x51,
            0x5A // Wrong End framing symbol
        ];

        // Frame with wrong lenght
        let invalid_data_3: Vec<u8> = vec![
            0x5B, 
            0x1D, 0x00, // Wrong lenght
            0x02, 0x00, 
            0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, 
            0x2A, 0x00, 
            0x03, 
            0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 0x00, 0x16, 0x51,
            0x5D
        ];

        // V2 Frame without content
        let invalid_data_4: Vec<u8> = vec![
            0x5B, 
            0x1C-0x0F, 0x00, //0x1C - 15
            0x02, 0x00, 
            0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, 
            0x2A, 0x00, 
            0x03, 
            // No content
            0x5D
        ];

        // V2 Frame without content and sequence number
        let invalid_data_5: Vec<u8> = vec![
            0x5B, 
            0x1C-0x10, 0x00, //0x1C - 16
            0x02, 0x00, 
            0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, 
            0x2A, 0x00, 
            // No sequence number 
            // No content
            0x5D
        ];

        // V3 Frame without content
        let invalid_data_6: Vec<u8> = vec![
            0x5B,
            0x23-0x0F, 0x00, //0x23 - 15
            0x03, 0x00, 
            0xF8, 0x95, 0x53, 0xFB, 0x4A, 0x00, 0x00, 0x00,
            0x2A, 0x00, 
            0x00, 0x00, 0x00, 0x00,
            0x03, 0x03,
            // No content
            0x5D 
        ]; 

        // V3 Frame without content and not full sequence number
        let invalid_data_7: Vec<u8> = vec![
            0x5B,
            0x23-0x10, 0x00, //0x23 - 16
            0x03, 0x00, 
            0xF8, 0x95, 0x53, 0xFB, 0x4A, 0x00, 0x00, 0x00,
            0x2A, 0x00, 
            0x00, 0x00, 0x00, 0x00,
            0x03, // Not full sequence number
            // No content
            0x5D 
        ]; 

        assert!(parse_raw(invalid_data_1).is_none());
        assert!(parse_raw(invalid_data_2).is_none());
        assert!(parse_raw(invalid_data_3).is_none());
        assert!(parse_raw(invalid_data_4).is_some());
        assert!(parse_raw(invalid_data_5).is_none());
        assert!(parse_raw(invalid_data_6).is_some());
        assert!(parse_raw(invalid_data_7).is_none());
    }

    #[test]
    fn test_v2_v3_debug_data() {
        // v2 debug message 
        let raw_data_v2: Vec<u8> = vec![
            0x5B, // Initial framing: '['
            0x1C, 0x00, // Lenght includes everything except the framing '[' and ']' characters.
            0x02, 0x00, // Version
            0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, // Timestamp 0x133200EB = 322044139
            0x2A, 0x00, // Debug type
            0x03, // Sequence number
            0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 0x00, 0x16, 0x51, // Content
            0x5D // End framing:  ']'
        ];

        // v3 debug message
        let raw_data_v3: Vec<u8> = vec![
            0x5B, // Initial framing: '['
            0x23, 0x00, // Lenght includes everything except the framing '[' and ']' characters.
            0x03, 0x00, // Version
            0xF8, 0x95, 0x53, 0xFB, 0x4A, 0x00, 0x00, 0x00, // Timestamp 0x4AFB5395F8/1000 = 322044139  
            0x2A, 0x00, // Debug type
            0x00, 0x00, 0x00, 0x00, // Flags
            0x03, 0x03, // Sequence number
            0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 0x00, 0x16, 0x51, // Content
            0x5D // End framing:  ']'
        ]; // 35

        let reference_data_v2 = DebugData {
            network_time: 322044139 as u64,
            duration: 800 as u16,
            debug_type: PACKET_TRACE_EFR_RX,
            seq_number: 0x0003 as u16,
            content: vec![0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 0x00, 0x16, 0x51]
        };
        let reference_data_v3 = DebugData {
            network_time: 322044139 as u64,
            duration: 800 as u16,
            debug_type: PACKET_TRACE_EFR_RX,
            seq_number: 0x0303 as u16,
            content: vec![0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 0x00, 0x16, 0x51]
        };
        let debug_data_v2 = parse_raw(raw_data_v2);
        let debug_data_v3 = parse_raw(raw_data_v3);

        assert!(debug_data_v2.is_some());
        assert!(debug_data_v3.is_some());
        
        match debug_data_v2 {
            Some(debug_data) => {
                assert_eq!(debug_data, reference_data_v2);
            }
            None => {
                panic!("No debug data in raw data v2!");
            }
        };

        match debug_data_v3 {
            Some(debug_data) => {
                assert_eq!(debug_data, reference_data_v3);
            }
            None => {
                panic!("No debug data in raw data v3!");
            }
        };
    }

    #[test]
    fn test_log_data_from_raw() {
        let raw_data_v2: Vec<u8> = vec![
            0x5B, 0x1C, 0x00, 0x02, 0x00, 0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, 0x2A, 0x00, 
            0x03, 0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 
            0x00, 0x16, 0x51, 0x5D
        ];

        let raw_data_v2_corrupted: Vec<u8> = vec![
            0x5B, 0x1C, 0x00, 0x02, 0x00, 0xEB, 0x00, 0x32, 0x13, 0x00, 0x00, 0x2A, 0x00, 
            0x03, 0xF8, 0x00, 0x14, 0x32, 0x62, 0xA4, 0x88, 0x21, 0xD1, 0xFA, 0xD5, 0x01, 
            0x00, 0x16, 0x51, 0x5E
        ];

        let record_data = String::from(
            "[322044139 800 16908330 Packet 03] [440138076] [F8 00 14 32 62 A4 88 21 D1 FA D5 01 00 16 51]\n"
        );

        let originator_1 = String::from("440138076");
        let originator_2 = String::from("440138077");
        let record = log_data_from_raw(raw_data_v2, originator_1);

        assert!(record.is_some());
        match record {
            Some(data) => {
                assert_eq!(data, record_data);
            }
            None => {
                panic!("No data to log in RawFrame!");
            }
        }
        assert!(log_data_from_raw(raw_data_v2_corrupted, originator_2).is_none());
    }
}