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

use std::io::{self, BufRead};
use std::net::{Shutdown, TcpStream, UdpSocket};
use std::str::FromStr;
use std::string::ToString;
use stoppable_thread;
use unify_config_sys::*;
use unify_log_sys::*;
use unify_middleware::unify_mqtt_client::{sl_status_t, MqttClientTrait, UnifyMqttClient};

use get_if_addrs;
use std::collections::HashMap;
use std::sync::Mutex;
use std::time::Duration;
use std::{net::IpAddr, net::Ipv4Addr};

#[allow(non_snake_case)]
#[derive(Debug)]
pub struct AdapterItem {
    IPAddress: String,
    SerialNumber: String,
}

#[derive(Debug, PartialEq, PartialOrd)]
#[allow(non_snake_case)]
pub enum Modes {
    None = 0x00,
    RxSuccess = 0xF9,
    RxAbort = 0xFA,
    TxSuccess = 0xFD,
    TxAbort = 0xFE,
}

impl std::fmt::Display for Modes {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl From<u8> for Modes {
    fn from(val: u8) -> Self {
        match val {
            0xF9 => return Modes::RxSuccess,
            0xFA => return Modes::RxAbort,
            0xFD => return Modes::TxSuccess,
            0xFE => return Modes::TxAbort,
            _ => return Modes::None,
        };
    }
}

#[derive(Debug)]
#[allow(non_snake_case)]
pub struct TraceItem {
    Mode: Modes,
    Protocol: u8,
    RadioConfig: u8,
    Rssi: i16,
    Channel: u8,
    Speed: u8,
    Syncword: Vec<u8>,
    Version: u8,
    ErrorCode: u8,
    DataLength: usize,
    Payload: Vec<u8>,
}

#[allow(non_snake_case)]
pub struct MapEntry {
    IPAddress: String,
    Handle: Option<stoppable_thread::StoppableHandle<()>>,
    Enabled: bool,
    GetOTA: bool,
    GetAdditionalData: bool,
}

pub struct Address {
    ip: IpAddr,
    broadcast: IpAddr,
}

#[derive(Default, Clone)]
pub struct UptiConfig {
    upti_ips: Vec<String>,
    enable_discovery: bool,
}

#[derive(Debug, Default)]
pub struct AppInfo {
    is_rx: bool,
    is_syncword: bool,
    is_radio_conf: bool,
    is_rssi: bool,
    version: u8,
}

static CHANNEL_REGIONS_BAUD_LR: [u32; 4] = [
    (0x03 << 8) + 0x0C,
    (0x03 << 8) + 0x0D,
    (0x00 << 8) + 0x0E,
    (0x01 << 8) + 0x0E,
]; // CH3+US_LR1, CH3+US_LR2, CH0+US_LR3, CH1+US_LR3

static CHANNEL_REGIONS_BAUD9600: [u32; 11] = [
    (0x02 << 8) + 0x01, // European Union
    (0x02 << 8) + 0x02, // United States
    (0x02 << 8) + 0x03, // Australia/New Zealand
    (0x02 << 8) + 0x04, // Hong Kong
    (0x02 << 8) + 0x05, // Malaysia
    (0x02 << 8) + 0x06, // India
    (0x02 << 8) + 0x08, // Russian Federation
    (0x02 << 8) + 0x09, // Israel
    (0x02 << 8) + 0x0B, // China
    (0x02 << 8) + 0x0C, // United States, Long Range 1
    (0x02 << 8) + 0x0D, // United States, Long Range 2
];

static CHANNEL_REGIONS_BAUD40_K: [u32; 11] = [
    (0x01 << 8) + 0x01, // European Union
    (0x01 << 8) + 0x02, // United States
    (0x01 << 8) + 0x03, // Australia/New Zealand
    (0x01 << 8) + 0x04, // Hong Kong
    (0x01 << 8) + 0x05, // Malaysia
    (0x01 << 8) + 0x06, // India
    (0x01 << 8) + 0x08, // Russian Federation
    (0x01 << 8) + 0x09, // Israel
    (0x01 << 8) + 0x0B, // China
    (0x01 << 8) + 0x0C, // United States, Long Range 1
    (0x01 << 8) + 0x0D, // United States, Long Range 2
];

static CHANNEL_REGIONS_BAUD100_K: [u32; 17] = [
    (0x00 << 8) + 0x01, // European Union
    (0x00 << 8) + 0x02, // United States
    (0x00 << 8) + 0x03, // Australia/New Zealand
    (0x00 << 8) + 0x04, // Hong Kong
    (0x00 << 8) + 0x05, // Malaysia
    (0x00 << 8) + 0x06, // India
    (0x00 << 8) + 0x07, // Japan
    (0x01 << 8) + 0x07, // Japan
    (0x02 << 8) + 0x07, // Japan
    (0x00 << 8) + 0x08, // Russian Federation
    (0x00 << 8) + 0x09, // Israel
    (0x00 << 8) + 0x0A, // Korea
    (0x01 << 8) + 0x0A, // Korea
    (0x02 << 8) + 0x0A, // Korea
    (0x00 << 8) + 0x0B, // China
    (0x00 << 8) + 0x0C, // United States, Long Range 1
    (0x00 << 8) + 0x0D, // United States, Long Range 2
];

static OPEN_BRACKET: u8 = 0x5B; // '[' symbol
static CLOSE_BRACKET: u8 = 0x5D; // ']' symbol
static RSSI_COMPENSATION: i16 = 0x32;

static RX_START: u8 = 0xF8;
static TX_START: u8 = 0xFC;
static RX_SUCCESS: u8 = 0xF9;
static TX_SUCCESS: u8 = 0xFD;

static OTA_PACKET_DATA: u8 = 0x55;

declare_app_name!("upti_cap_streamer");

static ZWAVE_PROTOCOL: u8 = 0x06;
#[allow(dead_code)]
static BLE_PROTOCOL: u8 = 0x03;
#[allow(dead_code)]
static ZIGBEE_PROTOCOL: [u8; 5] = [0x01, 0x02, 0x05, 0x07, 0x08];

impl ConfigLoader for UptiConfig {
    fn from_config() -> Result<Self, unify_config_sys::config_status_t>
    where
        Self: Sized,
    {
        let mut ips: Vec<String> = vec![];
        let mut to_discover: bool = true;
        match config_get_as_bool("upti.enable_discovery") {
            Ok(en_discovery) => {
                to_discover = en_discovery;
            }
            Err(_) => {}
        }
        match config_get_as_string("upti.ips") {
            Ok(upti_ips) => {
                if !upti_ips.is_empty() {
                    ips = upti_ips
                        .replace(" ", "")
                        .split(',')
                        .map(String::from)
                        .collect();
                }
            }
            Err(_) => {}
        }
        Ok(UptiConfig {
            upti_ips: ips,
            enable_discovery: to_discover,
        })
    }
}

// Use a HashMap to store the adapters data
lazy_static! {
    static ref ADAPTERS_MAP: Mutex<HashMap<String, MapEntry>> = {
        let m = HashMap::new();
        Mutex::new(m)
    };
}

/// Discovers WSTK boards (adapters) in network and updates ADAPTERS_MAP content
pub fn discovery_adapters() -> Result<(), sl_status_t> {
    let devices = get_supported_devices();
    let mut map = ADAPTERS_MAP.lock().unwrap();
    for device in &devices {
        let key = device.SerialNumber.to_string();
        if !map.contains_key(&key) {
            let entry = MapEntry {
                IPAddress: device.IPAddress.to_string(),
                Handle: None,
                Enabled: false,
                GetOTA: false,
                GetAdditionalData: false,
            };
            map.insert(key, entry);
        }
    }
    let mut to_remove = Vec::new();
    for (key, _value) in &*map {
        if devices.iter().any(|v| v.SerialNumber == *key) {
            continue;
        } else {
            to_remove.push(key.to_owned());
        }
    }
    for key in to_remove.iter() {
        map.remove(key);
    }

    drop(map); // unlock mutex
    publish_list();
    Ok(())
}

/// Controls capture sessions. Each capture session publishes data that it receives from debug channel
/// to corresponding MQTT topic
pub fn toggle_adapter(
    json: json::JsonValue,
    serial_number: String,
) -> std::result::Result<(), String> {
    // Mandatory keys
    let enable = json["Enable"]
        .as_bool()
        .ok_or(format!("could not parse 'Enable':{}", json))?;

    // Optional keys. Set value to `false` if key not exists
    let get_ota = match json["GetOTA"].as_bool() {
        Some(get_ota) => get_ota,
        None => {
            log_error!(
                "could not parse or find 'GetOTA' key: {}, use `false` as default value",
                json
            );
            false
        }
    };
    let get_add_data = match json["GetAdditionalData"].as_bool() {
        Some(get_add_data) => get_add_data,
        None => {
            log_error!(
                "could not parse or find 'GetAdditionalData' key: {}, use `true` as default value",
                json
            );
            false
        }
    };

    let mut map = ADAPTERS_MAP.lock().unwrap();
    // Check if adapter serial number in SupportedDevices
    if map.contains_key(&serial_number) {
        log_info!(
            "Action {} requested for {}",
            if enable { "Enable" } else { "Disable" },
            serial_number
        );
        let entry = map.get(&serial_number);
        match entry {
            Some(entry) => {
                // We have entry in map
                if enable != entry.Enabled {
                    if enable {
                        // Try to start thread and save thread handle in the map
                        if let Some(handle) = on_pti_enable(
                            entry.IPAddress.to_string(),
                            serial_number.to_string(),
                            get_ota,
                            get_add_data,
                        ) {
                            (*map.get_mut(&serial_number).unwrap()).Handle = Some(handle);
                            (*map.get_mut(&serial_number).unwrap()).Enabled = true;
                            (*map.get_mut(&serial_number).unwrap()).GetOTA = get_ota;
                            (*map.get_mut(&serial_number).unwrap()).GetAdditionalData =
                                get_add_data;
                        }
                    } else {
                        // Read thread handle and send stop message to thread
                        // take() method allows to destroy thread from other thread
                        if let Some(handle) = (*map.get_mut(&serial_number).unwrap()).Handle.take()
                        {
                            let join_handle = handle.stop();
                            join_handle.join().unwrap();
                        }
                        (*map.get_mut(&serial_number).unwrap()).Enabled = false;
                        (*map.get_mut(&serial_number).unwrap()).Handle = None;
                    }
                }
            }
            None => {}
        };
    } else {
        log_info!(
            "Action {} requested for non existing adapter {}",
            if enable { "Enable" } else { "Disable" },
            serial_number
        );
    }
    drop(map); // unlock mutex
    publish_list();
    Ok(())
}

/// Connects to adapter, creates new thread, reads trace packages from TCP stream,
/// publishes packages to ucl/UPTIcap/<serial_number>/TracePackage topic
/// Parsing OTA packets not implemented
fn on_pti_enable(
    pti_address: String,
    serial: String,
    _get_ota: bool,
    get_add_data: bool,
) -> Option<stoppable_thread::StoppableHandle<()>> {
    let addr = format!("{}:4905", pti_address);
    // Try to connect and get stream first and then start thread
    if let Ok(stream) = TcpStream::connect_timeout(&addr.parse().unwrap(), Duration::new(1, 0)) {
        log_info!("Connected to adapter. IP: {}", addr);
        // Some emulation of non-blocking reading - just set read timeout to 1 sec
        stream.set_read_timeout(Some(Duration::new(1, 0))).unwrap();
        // Wrap the stream in a BufReader, so we can use the BufRead methods
        let mut reader = io::BufReader::new(stream.try_clone().expect("Cannot clone stream"));

        // Build topic name based on adapter serial number
        let topic = format!("ucl/UPTICap/{}/TracePackage", serial);
        let mqttclient = UnifyMqttClient::default();

        let handle = Some(stoppable_thread::spawn(move |stopped| {
            while !stopped.get() {
                // Read data from the TcpStream
                match reader.fill_buf() {
                    Ok(buf) => {
                        // Process received data
                        let received_data = buf.to_vec();
                        // log_debug!("TCP Reader data: {:?}", received_data);
                        let (consumed, rawframe) = get_raw_frame(&received_data);
                        if let Some(raw_data) = rawframe {
                            let message = build_trace_message(raw_data, get_add_data);
                            mqttclient
                                .publish(&topic, message.dump().as_bytes(), false)
                                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))
                                .unwrap();
                        }
                        // log_debug("Consumed: {} bytes", consumed);
                        reader.consume(consumed);
                    }
                    Err(_) => {
                        // If no data to read in Buffer "Resource temporarily unavailable (os error 11)" error
                        // generated and there is no difference if TCP connection exists or not
                    }
                }
            }
            log_info!("Stream shutdown for {} {}", pti_address, serial);
            stream.shutdown(Shutdown::Both).unwrap();
        }));
        handle
    } else {
        log_error!("Could not connect to adapter. IP: {}", addr);
        None
    }
}

/// Build TracePackage message payload
fn build_trace_message(raw_data: Vec<u8>, get_add_data: bool) -> json::JsonValue {
    let mut message = json::object! {
        "TimeStamp"=> chrono::offset::Local::now().to_rfc3339(),
    };
    if get_add_data {
        if let Some(data) = get_pti_data_frame(&raw_data) {
            let frame = parse_data(&data);
            let add_data = json::object! {
                "Mode"=>frame.Mode.to_string(),
                "Protocol"=>frame.Protocol,
                "RadioConfig"=>frame.RadioConfig,
                "Rssi"=> frame.Rssi,
                "Channel"=> frame.Channel,
                "Speed"=> frame.Speed,
                "Syncword"=>frame.Syncword,
                "Version"=>frame.Version,
                "ErrorCode"=>frame.ErrorCode,
                "Payload"=> frame.Payload
            };
            message.insert("AdditionalData", add_data).unwrap();
        }
    }
    message.insert("RawFrame", raw_data).unwrap();
    message
}

/// Discovers WSTK boards with broadcast to port 19020 on all network interfaces.
fn get_supported_devices() -> Vec<AdapterItem> {
    log_info!("Start adapters discovery...");

    let mut adapters: Vec<AdapterItem> = vec![];
    let mut address_list: Vec<Address> = vec![];
    let mut enable_local_discovery: bool = true;
    let mut preconfigured_ips: Vec<String> = vec![];
    // Get list of network interfaces
    let ifaces = get_if_addrs::get_if_addrs().unwrap();

    for iface in ifaces.iter() {
        if iface.is_loopback() {
            continue;
        }
        match &iface.addr {
            get_if_addrs::IfAddr::V4(get_if_addrs::Ifv4Addr {
                ip: _,
                netmask: _,
                broadcast: None,
            }) => {
                log_info!("ip:catch");
                continue;
            }
            get_if_addrs::IfAddr::V4(addr) => {
                let address = Address {
                    ip: std::net::IpAddr::V4(addr.ip),
                    broadcast: match addr.broadcast {
                        // If no broadcast address provided for interface
                        // use 255.255.255.255
                        None => std::net::IpAddr::V4(Ipv4Addr::new(255, 255, 255, 255)),
                        Some(broadcast) => (std::net::IpAddr::V4(broadcast)),
                    },
                };
                address_list.push(address);
            }
            get_if_addrs::IfAddr::V6(_) => {
                continue;
            }
        }
    }

    match UptiConfig::from_config() {
        Ok(config_ips) => {
            preconfigured_ips.extend(config_ips.upti_ips);
            enable_local_discovery = config_ips.enable_discovery;
        }
        _ => {}
    }

    log_info!("Preconfigured devices IP list {:?}", preconfigured_ips);
    log_info!(
        "Device discovery in local network is {}",
        if enable_local_discovery {
            "enabled"
        } else {
            "disabled"
        }
    );

    // Try to find device on each interface
    if !preconfigured_ips.is_empty() {
        for address in &address_list {
            for ip_item in &preconfigured_ips {
                match std::net::IpAddr::from_str(&ip_item) {
                    Ok(broadcast) => {
                        let addr = Address {
                            ip: address.ip,
                            broadcast: broadcast,
                        };
                        if let Some(discovered_adapters) = udp_discovery(addr) {
                            adapters.extend(discovered_adapters);
                        }
                    }
                    Err(_) => {
                        continue;
                    }
                }
            }
        }
    }

    if enable_local_discovery {
        for address in address_list {
            if let Some(discovered_adapters) = udp_discovery(address) {
                adapters.extend(discovered_adapters);
            }
        }
    }

    log_info!("Discovery is over.");
    adapters
}

fn udp_discovery(address: Address) -> Option<Vec<AdapterItem>> {
    // ip:0 - 0 means random UDP port number
    let addr = format!("{}:{}", address.ip, 0);

    let socket = match UdpSocket::bind(&addr) {
        Ok(socket) => socket,
        Err(_) => {
            log_info!("{} [can't bind to socket]", address.ip);
            return None;
        }
    };

    socket
        .set_read_timeout(Some(Duration::new(2, 0)))
        .expect("[can`t set timeout]");
    socket.set_broadcast(true).expect("[can`t set broadcast]");

    let discovery = String::from("Discover");
    let mut cmd_bytes: [u8; 64] = [0; 64]; // Should be exactly 64 bytes
    cmd_bytes[..discovery.len()].copy_from_slice(&discovery.as_bytes());

    // Zniffer application always uses broadcast_addr "255.255.255.255:19020"
    // But OS informs us about broadcast address for each network interface
    // so lets try to use OS information
    let broadcast_addr = format!("{}:{}", address.broadcast, 19020);
    if let Err(err) = socket.send_to(&cmd_bytes, broadcast_addr) {
        log_info!("{} [can`t send broadcast command] {}", address.ip, err);
        return None;
    }

    let mut adapters: Vec<AdapterItem> = vec![];

    loop {
        let mut buf: [u8; 128] = [0; 128]; // Discover answer has 128 bytes
        let end_point_ip;
        let message_len;
        match socket.recv_from(&mut buf) {
            Ok((msg_len, src_addr)) => {
                end_point_ip = src_addr.ip();
                message_len = msg_len;
            }
            Err(err) => {
                if err.kind() == io::ErrorKind::WouldBlock || err.kind() == io::ErrorKind::TimedOut
                {
                    break;
                } else {
                    log_info!("{} [can`t read from socket] {}", address.ip, err);
                    break;
                }
            }
        };

        if message_len == 128 && buf.starts_with("Found".as_bytes()) {
            let mut serial: [u8; 4] = [0; 4];
            serial.copy_from_slice(&buf[48..52]);
            serial.reverse();
            let serial_number: u32 = ((serial[0] as u32) << 24)
                | ((serial[1] as u32) << 16)
                | ((serial[2] as u32) << 8)
                | ((serial[3] as u32) << 0);

            log_info!(
                "{} - IP: {}, SN: {}",
                address.ip,
                end_point_ip,
                serial_number
            );

            let adapter_list_item = AdapterItem {
                IPAddress: end_point_ip.to_string(),
                SerialNumber: serial_number.to_string(),
            };

            adapters.push(adapter_list_item);
        } else {
            log_info!("{} [not supported]", address.ip);
        }
    }

    if adapters.is_empty() {
        return None;
    } else {
        return Some(adapters);
    }
}

fn publish_list() {
    let map = ADAPTERS_MAP.lock().unwrap();
    let mut device_array = json::JsonValue::new_array();

    for key in map.keys() {
        let entry = map.get(key);
        match entry {
            Some(entry) => {
                let device_item = json::object! {
                    "PTIAddress" => entry.IPAddress.to_string(),
                    "SerialNumber" => key.to_string(),
                    "Enabled" => entry.Enabled
                };
                device_array
                    .push(device_item)
                    .map_err(|e| format!("JSON arrary building error : {}", e))
                    .unwrap();
            }
            None => {}
        }
    }

    let list = json::object! {
        "value" => device_array
    };

    let mqttclient = UnifyMqttClient::default();
    let topic = "ucl/UPTICap/List";
    mqttclient
        .publish(&topic, list.dump().as_bytes(), true)
        .map_err(|e| format!("error publishing topic '{}' : {}", topic, e))
        .unwrap();
}

/// Removes WSTK device from ADAPTERS_MAP, stops all running threads,
/// publishes unretain message to ucl/UPTICap/List
pub fn remove_list() -> Result<(), sl_status_t> {
    let mut map = ADAPTERS_MAP.lock().unwrap();

    if !map.is_empty() {
        // Stop all running threads
        for (_, val) in map.iter_mut() {
            if let Some(handle) = val.Handle.take() {
                let join_handle = handle.stop();
                join_handle.join().unwrap();
            }
        }
        // Remove all entry from map
        map.clear();
        // Send unretain message
        let mqttclient = UnifyMqttClient::default();
        let topic = "ucl/UPTICap/List";
        let _msg_id = mqttclient
            .publish(&topic, &[], true)
            .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
    }
    Ok(())
}

/// Identify RawFrame in TCP stream
fn get_raw_frame(received_data: &Vec<u8>) -> (usize, Option<Vec<u8>>) {
    let rd_len = received_data.len();
    if rd_len > 4 {
        let mut index: usize = 1;
        while index < rd_len {
            // Find Start symbol
            if received_data[index - 1] == OPEN_BRACKET {
                // If we have enough data for RawFrame
                if index + (received_data[index] as usize) < rd_len {
                    // If we have End Symbol
                    if received_data[index + (received_data[index] as usize)] == CLOSE_BRACKET {
                        // Bingo! We have full frame! Return data and number of consumed bytes
                        let start_pos = index - 1;
                        let end_pos = index + 1 + received_data[index] as usize;
                        // [RawFrame]
                        let data = &received_data[start_pos..end_pos];
                        return (end_pos, Some(data.to_vec()));
                    }
                } else {
                    return (0, None);
                }
            }
            index = index + 1;
        }
        return (index, None);
    }
    return (0, None);
}

/// Check if frame is PTI
fn get_pti_data_frame(received_data: &Vec<u8>) -> Option<Vec<u8>> {
    let rd_len = received_data.len();
    if rd_len > 4 {
        let index: usize = 1;
        let start_pos = index + 2;
        let end_pos = index + received_data[index] as usize;
        let data = &received_data[start_pos..end_pos];
        if data.len() > 11 + 6 {
            if ((data[11] == RX_START) && (data[data.len() - 6] == RX_SUCCESS))
                || ((data[11] == TX_START) && (data[data.len() - 5] == TX_SUCCESS))
                || ((data[11] == RX_START) && (data[12] == OTA_PACKET_DATA))
                || ((data[11] == TX_START) && (data[12] == OTA_PACKET_DATA))
            {
                return Some(data.to_vec());
            }
        }
    }
    return None;
}

fn parse_data(data: &Vec<u8>) -> TraceItem {
    let _length_index_in_header: usize = 7;
    let _hw_end_try_offset: [usize; 2] = [5, 6];
    let _syncword_length: usize = 4;
    let _before_data_length: usize = 12;
    let mut trace_item = TraceItem {
        Channel: 0,
        Rssi: 0,
        Speed: 0,
        DataLength: 0,
        RadioConfig: 0,
        Protocol: 0,
        Payload: Vec::default(),
        Mode: Modes::None,
        Syncword: Vec::default(),
        Version: 0,
        ErrorCode: 0,
    };

    if let Some(app_info) = get_appended_info(data) {
        let mut data_index: usize = data.len() - 2;
        let status = data[data_index];
        data_index = data_index - 1;
        let radio_info = data[data_index];
        let mut radio_conf: u8 = 0;
        if app_info.is_radio_conf {
            data_index = data_index - 1;
            radio_conf = data[data_index];
        }
        let mut sync_word: Vec<u8> = Vec::default();
        if app_info.is_syncword {
            data_index = data_index - 4;
            sync_word = data[data_index..data_index + 4].to_vec();
        }
        let mut rssi: i16 = 0;
        if app_info.is_rssi {
            data_index = data_index - 1;
            // From Version 1 onwards, RSSI (Rx only) under the APPENDED_INFO will now have to compensated
            // by subtracting 0x32 to get actual RSSI. (RAIL_LIB-5072)
            rssi = if app_info.version >= 1 {
                data[data_index] as i16 - RSSI_COMPENSATION
            } else {
                data[data_index] as i16
            };
        }
        data_index = data_index - 1;
        trace_item = TraceItem {
            Channel: radio_info & 0x3F,
            Rssi: rssi,
            Speed: 0,
            DataLength: 0,
            RadioConfig: radio_conf,
            Protocol: status & 0x0F,
            Payload: Vec::default(),
            Mode: Modes::from(data[data_index]),
            Syncword: sync_word,
            Version: app_info.version,
            ErrorCode: status & 0xF0,
        };

        if trace_item.Protocol == ZWAVE_PROTOCOL {
            let region = radio_conf & 0x1F;
            let key = ((trace_item.Channel as usize) << 8) + region as usize;
            if CHANNEL_REGIONS_BAUD9600.contains(&(key as u32)) {
                trace_item.Speed = 0;
            } else if CHANNEL_REGIONS_BAUD40_K.contains(&(key as u32)) {
                trace_item.Speed = 1;
            } else if CHANNEL_REGIONS_BAUD100_K.contains(&(key as u32)) {
                trace_item.Speed = 2;
            } else if CHANNEL_REGIONS_BAUD_LR.contains(&(key as u32)) {
                trace_item.Speed = 3;
            }
        }

        let packet_length = data_index - _before_data_length;
        if packet_length > 4 {
            if _length_index_in_header < packet_length
                && data[_before_data_length + _length_index_in_header] as usize <= packet_length
            {
                trace_item.DataLength =
                    data[_before_data_length + _length_index_in_header] as usize;
            } else {
                trace_item.DataLength = packet_length;
            }
        } else {
            trace_item.DataLength = data.len() - _before_data_length - data_index + 1;
        }
        trace_item.Payload =
            data[_before_data_length..trace_item.DataLength + _before_data_length].to_vec();
    } else {
        trace_item.DataLength = data.len();
        trace_item.Payload = data.to_vec();
    }

    // log_debug!("trace: {:?}", trace_item);
    return trace_item;
}

/// Appended Info decoder
/// For Versions 0 and 1, APPENDED_INFO_CFG can take on these values (v bit is the version):
///   0b1 0 0 000 00v = 0x100 or 0x101 = Tx, SYNCWORD  absent, RADIO_CONFIG  absent, RSSI  absent
///   0b1 0 0 001 00v = 0x108 or 0x109 = Tx, SYNCWORD  absent, RADIO_CONFIG present, RSSI  absent
///   0b1 0 0 010 00v = 0x110 or 0x111 = reserved
///   0b1 0 0 011 00v = 0x118 or 0x119 = reserved
///   0b1 0 0 100 00v = 0x120 or 0x121 = Tx, SYNCWORD present, RADIO_CONFIG  absent, RSSI  absent
///   0b1 0 0 101 00v = 0x128 or 0x129 = Tx, SYNCWORD present, RADIO_CONFIG present, RSSI  absent
///   0b1 0 0 110 00v = 0x130 or 0x131 = reserved
///   0b1 0 0 111 00v = 0x138 or 0x139 = reserved
///   0b1 0 1 000 00v = 0x140 or 0x141 = reserved
///   0b1 0 1 001 00v = 0x148 or 0x149 = Rx, SYNCWORD  absent, RADIO_CONFIG  absent, RSSI present
///   0b1 0 1 010 00v = 0x150 or 0x151 = Rx, SYNCWORD  absent, RADIO_CONFIG present, RSSI present
///   0b1 0 1 011 00v = 0x158 or 0x159 = reserved
///   0b1 0 1 100 00v = 0x160 or 0x161 = reserved
///   0b1 0 1 101 00v = 0x168 or 0x169 = Rx, SYNCWORD present, RADIO_CONFIG  absent, RSSI present
///   0b1 0 1 110 00v = 0x170 or 0x171 = Rx, SYNCWORD present, RADIO_CONFIG present, RSSI present
///   0b1 0 1 111 00v = 0x178 or 0x179 = reserved
// In essence, b5 indicates presence of SYNCWORD for both Rx and Tx;
// b3 indicates presence of RADIO_CONFIG for Tx only; and b4 indicates presence of RADIO_CONFIG for Rx only.

fn get_appended_info(data: &Vec<u8>) -> Option<AppInfo> {
    let app_info = data[data.len() - 1];
    let mut info = AppInfo::default();
    info.version = app_info & 0x01;
    match app_info {
        0x00 | 0x01 => {}
        0x08 | 0x09 => {
            info.is_radio_conf = true;
        }
        0x20 | 0x21 => {
            info.is_syncword = true;
        }
        0x28 | 0x29 => {
            info.is_syncword = true;
            info.is_radio_conf = true;
        }
        0x48 | 0x49 => {
            info.is_rx = true;
            info.is_rssi = true;
        }
        0x50 | 0x51 => {
            info.is_rx = true;
            info.is_radio_conf = true;
            info.is_rssi = true;
        }
        0x68 | 0x69 => {
            info.is_rx = true;
            info.is_syncword = true;
            info.is_rssi = true;
        }
        0x70 | 0x71 => {
            info.is_rx = true;
            info.is_syncword = true;
            info.is_radio_conf = true;
            info.is_rssi = true;
        }
        _ => {
            return None;
        }
    }
    return Some(info);
}

// Test TCP frames
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn full_frame_test() {
        // Define some non-diagnostic frame
        let non_diagnostic_frame: Vec<u8> = vec![
            91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93,
        ];

        // Check if pti frame in buffer
        let (consumed, frame) = get_raw_frame(&non_diagnostic_frame);
        assert_eq!(consumed, non_diagnostic_frame.len());
        assert!(frame.is_some());
    }

    #[test]
    fn random_data_frame_test() {
        // Define some non-diagnostic frame
        let random_frame: Vec<u8> = vec![
            01, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 02,
        ];

        // Check if pti frame in buffer
        let (consumed, frame) = get_raw_frame(&random_frame);
        assert_eq!(consumed, random_frame.len());
        assert!(frame.is_none());
    }

    #[test]
    fn partial_frame_test_1() {
        // Define some non-diagnostic frame
        let partial_frame: Vec<u8> = vec![
            91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81,
        ];

        // Check if pti frame in buffer
        let (consumed, frame) = get_raw_frame(&partial_frame);

        assert_eq!(consumed, 0);
        assert!(frame.is_none());
    }

    #[test]
    fn full_frame_test_2() {
        // Define some frame with simbols before '[' and after ']'
        let full_frame: Vec<u8> = vec![
            0, 0, 0, 91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1,
            1, 1, 13, 255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93, 0, 0,
        ];

        let raw_frame: Vec<u8> = vec![
            91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93,
        ];

        // Check if pti frame in buffer
        let (consumed, frame) = get_raw_frame(&full_frame);

        assert_eq!(consumed, raw_frame.len() + 3);
        assert!(frame.is_some());
        match frame {
            Some(data) => {
                assert_eq!(&data, &raw_frame);
            }
            None => {
                panic!("No RawFrame detected!");
            }
        }
    }

    #[test]
    fn consecutive_frames() {
        // Define two consecutive frames
        let full_frame: Vec<u8> = vec![
            91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93, 91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 43,
            0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13, 255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93,
        ];

        let raw_frame: Vec<u8> = vec![
            91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93,
        ];

        // Check if pti frame in buffer
        let (consumed, frame) = get_raw_frame(&full_frame);

        assert_eq!(consumed, raw_frame.len());
        assert!(frame.is_some());
        match frame {
            Some(data) => {
                assert_eq!(&data, &raw_frame);
            }
            None => {
                panic!("No raw frame detected!");
            }
        }
    }

    #[test]
    fn broken_and_valid_consecutive_frames() {
        // Define two consecutive frames
        let full_frame: Vec<u8> = vec![
            91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 43, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0,
            1, 248, 196, 230, 184, 133, 1, 1, 1, 13, 255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93,
        ];

        let raw_frame: Vec<u8> = vec![
            91, 33, 0, 2, 0, 49, 71, 17, 133, 0, 0, 42, 0, 1, 248, 196, 230, 184, 133, 1, 1, 1, 13,
            255, 1, 8, 5, 31, 249, 32, 1, 2, 6, 81, 93,
        ];

        // Check if pti frame in buffer
        let (consumed, frame) = get_raw_frame(&full_frame);

        assert_eq!(consumed, full_frame.len());
        assert!(frame.is_some());
        match frame {
            Some(data) => {
                assert_eq!(&data, &raw_frame);
            }
            None => {
                panic!("No raw frame detected!");
            }
        }
    }
}
