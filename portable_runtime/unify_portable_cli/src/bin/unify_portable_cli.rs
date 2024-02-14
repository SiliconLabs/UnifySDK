extern crate exitcode;
use clap::{Parser, Subcommand};
use ctrlc;
use std::net::IpAddr;
use std::process;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use unify_portable_core::*;
use std::collections::HashMap;
use std::process::Child;

/// Argument parsing for windows
#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]

struct Args {
    #[clap(short = 'd', long, forbid_empty_values = true)]
    /// Path to device that should be directly mapped eg. -d /dev/ttyUSB0.
    device: Option<String>,
    #[clap(short = 's', long, forbid_empty_values = true)]
    /// Serial number of device that can be mapped using Silink eg. -s 440262195.
    serial_no: Option<i32>,
    #[clap(short = 'i', long, forbid_empty_values = true)]
    /// IP Address for a direct IP connection to WSTK board eg. -i 10.0.0.12.
    ip_address: Option<IpAddr>,
    #[clap(long, forbid_empty_values = true)]
    /// Path to device that should be directly mapped eg. --zigpc-device /dev/ttyUSB0.
    zigpc_device: Option<String>,
    #[clap(long, forbid_empty_values = true)]
    /// Serial number of device that can be mapped using Silink eg. --zigpc-serial-no 440262195.
    zigpc_serial_no: Option<i32>,
    #[clap(long, forbid_empty_values = true)]
    /// IP Address for a direct IP connection to WSTK board eg. --zigpc-ip-address 10.0.0.12.
    zigpc_ip_address: Option<IpAddr>,
    #[clap(long, forbid_empty_values = true)]
    /// Path to device that should be directly mapped eg. --aoxpc-device /dev/ttyUSB0.
    aoxpc_device: Option<String>,
    #[clap(long, forbid_empty_values = true)]
    /// Serial number of device that can be mapped using Silink eg. --aoxpc-serial-no 440262195.
    aoxpc_serial_no: Option<i32>,
    #[clap(long, forbid_empty_values = true)]
    /// IP Address for a direct IP connection to WSTK board eg. --aoxpc-ip-address 10.0.0.12.
    aoxpc_ip_address: Option<IpAddr>,
    #[clap(long, forbid_empty_values = true)]
    /// Path to device that should be directly mapped eg. --multiprotocol-device /dev/ttyUSB0.
    multiprotocol_device: Option<String>,
    #[clap(long, forbid_empty_values = true)]
    /// IP Address for a direct IP connection to WSTK board eg. --multiprotocol-ip-address 10.0.0.12.
    multiprotocol_ip_address: Option<IpAddr>,
    #[clap(long, forbid_empty_values = true)]
    /// Serial number of device that can be mapped using Silink eg. --multiprotocol-serial-no 440262195.
    multiprotocol_serial_no: Option<i32>,
    #[clap(short = 'p', long, forbid_empty_values = true, value_delimiter=',', value_parser=PROTCOL_MODULE_LIST)]
    /// Selections of protocol services to run eg. -p "Z-Wave". 
    protocols: Option<Vec<String>>,
    #[clap(short = 'm', long, forbid_empty_values = true, value_delimiter=',', value_parser=HELPER_MODULE_LIST)]
    /// Selections of helper services to run eg. -m "NAL,GMS". 
    modules: Option<Vec<String>>,
    #[clap(short = 'a', long, forbid_empty_values = true, value_delimiter=',', value_parser=APPLICATIONS_LIST)]
    /// Selections of applications to run eg. -a "UMB".
    applications: Option<Vec<String>>,
    #[clap(long, forbid_empty_values = true)]
    /// Region for the Z-Wave controller to operate in eg. --zwave-rf-region US.
    zwave_rf_region: Option<String>,
    #[clap(subcommand)]
    /// Additional Subcommands. For subcommands' usage, use the help of the subcommands.
    command: Option<Commands>,
}

#[derive(Subcommand, Debug)]
enum Commands {
    /// Command for flash an application to the connected device.
    /// One of the --app-path or --app-name argument is mandatory.
    /// The available options for --app-name are:
    /// multilevel-sensor, power-strip, sensor-pir, serial-api, switch-on-off, wall-controller
    /// Two arguments can be specified optionally before the subcommand from the main
    /// arguments: `--serial-no` and `--zwave-rf-region`.
    /// In case the serial number is not specified, the choice will be prompted.
    /// In case the rf region is not specified, the default EU region frequency will be used.
    FlashApp {
        #[clap(short = 'p', long, forbid_empty_values = true)]
        /// Relative file path to be flashed. App_path or app_name is mandatory. Example: unify_portable_cli.exe flash-app -s 440262138 -p zwave_soc_switch_on_off-brd2603a-us.hex
        app_path: Option<String>,

        #[clap(short = 'n', long, arg_enum, forbid_empty_values = true, conflicts_with = "app-path")]
        /// Application name to be flashed. App_path or app_name is mandatory. Example: unify_portable_cli.exe flash-app -n switch-on-off
        app_name: Option<AppName>,

        #[clap(short = 'c', long, forbid_empty_values = true)]
        /// Chip type. If you not give any then the default: ZGM230.
        chip_type: Option<String>,
    },
    /// Command for read DSK (Device Specific Key) value for Smart Start.
    /// If the serial number is not specified, you will be prompted to select
    /// one of the connnected devices.
    ReadDsk {
        #[clap(short = 'c', long, forbid_empty_values = false)]
        /// Chip type. If you not give any then the default: ZGM230.
        chip_type: Option<String>,
    },
    /// Lists the info of the connected devices.
    /// With boardSerial[0] can identify physically the board. With serialNumber you can flash.
    ListDevicesRaw {
    },
    /// Lists serial number, board number, configured transmit region,
    /// and installed application for each device.
    ListDevices {
    }
}

#[derive(clap::ArgEnum, Clone, Debug, PartialEq, Eq, Hash)]
enum AppName {
   SwitchOnOff,
   MultilevelSensor,
   SerialAPI,
   SensorPIR,
   WallController,
   PowerStrip,
}

fn handle_device_args(
    arg_count: &mut usize,
    device_string: &mut String,
    device_silink_sn: &mut String,
    device_ip_configured: &mut Option<bool>,
    protocol_device: Option<String>,
    protocol_ip: Option<IpAddr>,
    protocol_sn: Option<i32>,
    protocol_type: &str,
) {
    if protocol_device != None {
        *arg_count += 1;
        *device_string = protocol_device.unwrap();
    }

    if protocol_ip != None {
        // zpc uses the default port configuration and doesn't require explicit port setup.
        *arg_count += 1;
        *device_ip_configured = Some(true);
        *device_string = protocol_ip.as_ref().unwrap().to_string();
    }

    if protocol_sn != None {
        *arg_count += 1;
        *device_silink_sn = protocol_sn.unwrap().to_string();
    }

    if *arg_count > 1 {
        eprintln!(
            "Only one option should be passed for the {}: either device, serial-no, or ip-address.",
            protocol_type
        );
        process::exit(exitcode::USAGE);
    }
}

// Retrieve the port w.r.t to the protocol.
fn get_port_env_var(protocol: &str) -> i32 {
    let port = match PORTS.get(protocol) {
        Some(&p) => p,
        None => "Unsupported protocol",
    };

    port.parse::<i32>().unwrap() + 1
}

fn main() {
    let busy_ports = get_busy_ports();
    if busy_ports.len() > 0 {
        eprintln!(
            "Another process is using port(s): {:?}.\n\
                       Please make sure to stop that service before trying to launch Unify.",
            busy_ports
        );
        std::process::exit(exitcode::NOHOST);
    }

    init_log();

    let parsed_args = Args::parse();
    let mut zpc_arg_count = 0;
    let mut zigpc_arg_count = 0;
    let mut aoxpc_arg_count = 0;
    let mut multiprotocol_arg_count = 0;
    // Default device string if we use Silink to map a device.
    let mut zpc_device_string = get_host_ip_address();
    let mut zigpc_device_string = get_host_ip_address();
    let mut aoxpc_device_string = get_host_ip_address();
    let mut multiprotocol_device_string = get_host_ip_address();

    let mut silink_sn = String::new();
    let mut zpc_silink_sn = String::new();
    let mut zigpc_silink_sn = String::new();
    let mut aoxpc_silink_sn = String::new();
    let mut multiprotocol_silink_sn = String::new();

    // Remote ip configured flag
    let mut zigpc_remote_ip_configured: Option<bool> = Some(false);
    let mut aoxpc_remote_ip_configured: Option<bool> = Some(false);
    let mut multiprotocol_remote_ip_configured: Option<bool> = Some(false);

    // Creating a Vector of silink process data, to be used to kill the process while exit.
    let mut silink_process_list: Vec<Child> = Vec::new();

    match &parsed_args.command {
        Some(Commands::FlashApp { app_path, app_name, chip_type }) => {
            // flash-app subcommand is used
            match handle_flash_app_command(app_path.clone(), app_name.clone(), chip_type.clone(),
                parsed_args.serial_no, parsed_args.zwave_rf_region.clone()) {
                    Ok(()) => process::exit(exitcode::OK),
                    Err(e) => {
                        eprintln!("{}", e);
                        process::exit(exitcode::NOINPUT);
                    }
                }
        }
        Some(Commands::ReadDsk { chip_type }) => {
            silink_sn = match get_serial_no(parsed_args.serial_no, "".to_string(), Vec::new()) {
                Ok(sn) => sn.to_string(),
                Err(e) => {
                    eprintln!("{}", e);
                    process::exit(exitcode::NOINPUT);
                }
            };

            let chip;

            match check_chip_type(&chip_type, &silink_sn) {
                Ok(c_type) => chip = Some(c_type),
                Err(e) => {
                    eprintln!("{}", e);
                    process::exit(exitcode::CONFIG); 
                }
            }

            match read_dsk(&silink_sn, chip.as_deref()) {
                Ok(dsk) => print_dsk(&dsk),
                Err(e) => {
                    eprintln!("{}", e);
                    process::exit(exitcode::NOINPUT);
                }
            }

            process::exit(exitcode::OK);
        }
        Some(Commands::ListDevicesRaw {}) => {
            match silink_inspect() {
                Err(error) => {
                    eprintln!("{}", error);
                    process::exit(exitcode::UNAVAILABLE)
                }
                Ok(silink_result_string) => {
                    println!("{}", silink_result_string);
                    process::exit(exitcode::OK);
                }
            }
        }
        Some(Commands::ListDevices {}) => {
            match silink_inspect() {
                Err(error) => {
                    eprintln!("{}", error);
                    process::exit(exitcode::UNAVAILABLE);
                }
                Ok(silink_result_string) => {
                    list_devices(&silink_result_string, true);
                    process::exit(exitcode::OK);
                }
            }
        }
        None => {
            // Intentionally blank block
            // Default arguments, continue with default unify portable cli argument processing
        }
    }

    // Processing ZPC CLI arguments
    handle_device_args(
        &mut zpc_arg_count,
        &mut zpc_device_string,
        &mut zpc_silink_sn,
        &mut None,
        parsed_args.device,
        parsed_args.ip_address,
        parsed_args.serial_no,
        "zpc",
    );

    set_zpc_device(&zpc_device_string);

    if parsed_args.zwave_rf_region != None {
        let region_string =
            String::from("--zpc.rf_region ") + &parsed_args.zwave_rf_region.unwrap();
        set_zpc_config_args(&region_string);
    }

    // Processing ZigPC CLI arguments
    handle_device_args(
        &mut zigpc_arg_count,
        &mut zigpc_device_string,
        &mut zigpc_silink_sn,
        &mut zigpc_remote_ip_configured,
        parsed_args.zigpc_device,
        parsed_args.zigpc_ip_address,
        parsed_args.zigpc_serial_no,
        "zigpc",
    );

    if zigpc_remote_ip_configured == Some(true) {
        set_env_var(ZIGPC_PORT_KEY, SILINK_DEFAULT_PORT);
    }

    set_zigpc_device(&zigpc_device_string);

    // Processing AoXPC CLI arguments
    handle_device_args(
        &mut aoxpc_arg_count,
        &mut aoxpc_device_string,
        &mut aoxpc_silink_sn,
        &mut aoxpc_remote_ip_configured,
        parsed_args.aoxpc_device,
        parsed_args.aoxpc_ip_address,
        parsed_args.aoxpc_serial_no,
        "aoxpc",
    );

    if aoxpc_remote_ip_configured == Some(true) {
        set_env_var(AOXPC_PORT_KEY, SILINK_DEFAULT_PORT);
    }

    set_aoxpc_device(&aoxpc_device_string);

    // Processing MultiProtocol CLI arguments
    handle_device_args(
        &mut multiprotocol_arg_count,
        &mut multiprotocol_device_string,
        &mut multiprotocol_silink_sn,
        &mut multiprotocol_remote_ip_configured,
        parsed_args.multiprotocol_device,
        parsed_args.multiprotocol_ip_address,
        parsed_args.multiprotocol_serial_no,
        "multiprotocol",
    );

    if multiprotocol_remote_ip_configured == Some(true) {
        set_env_var(MULTIPROTOCOL_PORT_KEY, SILINK_DEFAULT_PORT);
    }

    set_multiprotocol_device(&multiprotocol_device_string);

    let mut sel_list_string = "".to_string();
    let mut prot_sel_list_string = "".to_string();
    if let Some(mut protocols) = parsed_args.protocols {
        // Sorting protocols vector, so to use the dedup to remove duplicate entries.
        protocols.sort();
        protocols.dedup();
        prot_sel_list_string = protocols.join(",");
    }

    let mut mod_sel_list_string = "".to_string();
    if let Some(mut modules) = parsed_args.modules {
        // Sorting modules vector, so to use the dedup to remove duplicate entries.
        modules.sort();
        modules.dedup();
        mod_sel_list_string = modules.join(",");
        sel_list_string = prot_sel_list_string.clone() + "," + mod_sel_list_string.as_str();
    }
    else {
        sel_list_string = prot_sel_list_string.clone();
    }

    let mut app_sel_list_string = "".to_string();
    if let Some(mut applications) = parsed_args.applications {
        // Sorting applications vector, so to use the dedup to remove duplicate entries.
        applications.sort();
        applications.dedup();
        app_sel_list_string = applications.join(",");
        sel_list_string = prot_sel_list_string.clone() + "," + mod_sel_list_string.as_str() + "," + app_sel_list_string.as_str();
    }

    if prot_sel_list_string == "" {
        prot_sel_list_string = "Z-Wave".to_string();
    }

    println!("Verifying Docker installation");

    if !is_docker_available() || !is_docker_compose_installed() {
        eprintln!("Call to Docker failed!\n\
                   Please make sure Docker and Docker Compose is installed and running.\n\
                   Go to https://www.docker.com/ and follow the instructions to download and install.");

        process::exit(exitcode::UNAVAILABLE);
    }

    let bind = Arc::new(prot_sel_list_string.clone());

    let protocols: Vec<String> = bind.split(',').map(|s| s.trim().to_string()).collect();

    let mut device_count = silink_get_device_count().unwrap();

    // Vector to store the devices mapped.
    let mut silink_automapped_devices: Vec<String> = Vec::new();

    for protocol in protocols {
        silink_sn = String::new();

        match protocol.as_str() {
            "Z-Wave" =>{
                // If serial number is not configured but other device info is configured, we continue without creating silink connection instance.
                if zpc_silink_sn == "" && zpc_arg_count != 0 {
                    continue;
                } else {
                    silink_sn = zpc_silink_sn.clone();
                    // zpc uses the default port configuration and doesn't require explicit port setup.
                }
            },
            "Zigbee-NCP" => {
                // If serial number is not configured but other device info is configured, we continue without creating silink connection instance.
                if zigpc_silink_sn == "" && zigpc_arg_count != 0 {
                    continue;
                } else {
                    silink_sn = zigpc_silink_sn.clone();

                    // Setting the PORT env w.r.t silink mapping
                    let remote_port = get_port_env_var(protocol.as_str());
                    set_env_var(ZIGPC_PORT_KEY, &remote_port.to_string());
                }
            },
            "AoX-Bluetooth" => {
                // If serial number is not configured but other device info is configured, we continue without creating silink connection instance.
                if aoxpc_silink_sn == "" && aoxpc_arg_count != 0 {
                    continue;
                } else {
                    silink_sn = aoxpc_silink_sn.clone();

                    // Setting the PORT env w.r.t silink mapping
                    let remote_port = get_port_env_var(protocol.as_str());
                    set_env_var(AOXPC_PORT_KEY, &remote_port.to_string());
                }
            },
            "MultiProtocol" => {
                if multiprotocol_silink_sn == "" && multiprotocol_arg_count != 0 {
                    continue;
                } else {
                    silink_sn = multiprotocol_silink_sn.clone();

                    // Setting the PORT env w.r.t silink mapping
                    let remote_port = get_port_env_var(protocol.as_str());
                    set_env_var(MULTIPROTOCOL_PORT_KEY, &remote_port.to_string());
                }
            },
            _ =>{
                println!("Not a valid protocol!");
            }
        }

        // If silink_sn is empty, then no options are used to configure the device.
        // So, here we list the devices connected and prompts user to select the device.
        if silink_sn == "" {

            if device_count == 0 {
                eprintln!("Could not find any connected devices. Please verify that you have enough Silabs devices connected!");
                process::exit(exitcode::USAGE);
            } else if device_count == 1 {
                // Auto selection if single device is connected to host.
                let silink_result_string = silink_inspect().unwrap();
                // Filtering out the previously mapped devices from the device list to avoid confusion.
                let filtered_result_string = filtered_mapped_devices(&silink_result_string, silink_automapped_devices.clone()).unwrap();
                let device = &list_devices(&filtered_result_string, false)[0];
                if device.application_name.as_ref().unwrap().eq(&"unknown") ||
                    device.application_name.as_ref().unwrap().eq(&"serial_api_controller") {
                        println!("Identified as {} Serial API Controller ", device.serial_number);
                    silink_sn = device.serial_number.clone();
                } else {
                    eprintln!("Found a connected device, but could not identify it as Controller.");
                    eprintln!("Try resetting or re-flashing it!");
                    process::exit(exitcode::UNAVAILABLE);
                }
            } else {
                println!("Attempting to identify the Serial API Controller...");
                let silink_result_string = silink_inspect().unwrap();
                let devices = list_devices(&silink_result_string, false);
                match devices.iter().find(|device|
                    device.application_name.is_some() && 
                    protocol.as_str() == "Z-Wave" &&
                    device.application_name.as_ref().unwrap().eq(&"serial_api_controller")
                ) {
                    None => {
                        eprintln!("Identification failed.");
                        silink_sn = match get_serial_no(None, protocol.clone(), silink_automapped_devices.clone()) {
                            Ok(sn) => sn,
                            Err(e) => {
                                eprintln!("{}", e);
                                process::exit(exitcode::NOINPUT);
                            }
                        };
                        println!("Setting up serial no {} device as a {} controller.", silink_sn, protocol);
                    },
                    Some(device) => {
                        println!("Identified {} as a controller.", device.serial_number);
                        silink_sn = device.serial_number.clone();
                    }
                }
            }
        }
        println!("Mapping device with Silink...");
    
        let silink = silink_automap(&silink_sn, protocol.as_str());
        let silink_is_running = !silink.is_err();
        let mut silink_process = silink.unwrap();
        if !silink_is_running || silink_process.try_wait().is_err() {
            eprintln!("Failed to start Silink!");
            process::exit(exitcode::TEMPFAIL);
        }

        // To store mapped devices to silink automapped devices Vector.
        silink_automapped_devices.push(silink_sn);
        device_count -= 1;
        silink_process_list.push(silink_process);
    }

    let conflicts = validate_configuration(&sel_list_string);
    if  !conflicts.is_empty() {
        eprintln!(
            "There seems to be some conflicts or problem with current selection or configuration!\n\
                Check and retry again with proper configuration!\nConflicts:");
        for conflict in conflicts {
            eprintln!("\t{} : {}", conflict.0, conflict.1);
        }
        process::exit(exitcode::CONFIG);
    }

    println!("Building Unify Portable Runtime...");

    let unify_zip_exists = identify_unify_debian_packages();
    if unify_zip_exists.is_err() {
        let unify_release_url = "https://github.com/SiliconLabs/UnifySDK/releases";
        let example_package = "unify_1.3.1_x86_64.zip".to_string();
        eprintln!(
            "Could not find Unify zip file in the docker-files directory!\n\
                   You can find the Unify packages here: {}.\n\
                   Make sure to login to verify that you have access to the Unify SDK.\n\
                   Download the Zip package of Unify and place it in the 'docker-files' folder.\n\
                   Choose the one that matches your architecture - in your case {}.",
            unify_release_url, example_package
        );
        process::exit(exitcode::NOINPUT);
    }

    docker_compose_build(&unify_zip_exists.unwrap()).expect("Failed to run 'docker-compose build'!");

    docker_compose_up(&sel_list_string).expect("Failed to start Portable Runtime");

    ctrlc::set_handler(move || {

        for process in silink_process_list.iter_mut(){
            process.kill().unwrap();
        }

        docker_compose_down().expect("Failed stopping Docker containers");

        process::exit(exitcode::OK);
    })
    .expect("Error setting Ctrl-C handler");

    println!("Unify is running!");
    println!("Please visit http://localhost:3080 in your browser.");
    println!("Change the 'Host' field to 'mqtt-broker' and press 'Connect'.");
    println!("");
    println!("Press Ctrl-C to stop...");

    let running = Arc::new(AtomicBool::new(true));
    while running.load(Ordering::SeqCst) {}
}

fn handle_flash_app_command(app_path: Option<String>, app_name:  Option<AppName>, chip: Option<String>,
    serial_no: Option<i32>, zwave_rf_region: Option<String>) -> Result<(), &'static str> {

    let final_app_path;
    match get_app_firmware_path(app_path.clone(), app_name.clone()) {
        Ok(p) => if does_file_exists(p.clone()) {
                    final_app_path = p
                  } else {
                    return Err("Firmware not found")
                  },
        Err(e) => return Err(e),
    }

    let silink_sn;
    match get_serial_no(serial_no, "".to_string(), Vec::new()) {
        Ok(sn) => silink_sn = sn,
        Err(e) => return Err(e),
    }

    let chip_type;

    match check_chip_type(&chip, &silink_sn) {
        Ok(c_type) => chip_type = c_type,
        Err(e) => return Err(e),
    }

    let region = get_region(zwave_rf_region);

    println!("Flash application from {}", final_app_path);
    flash_device(
        &final_app_path,
        &silink_sn,
        &chip_type,
        region
    )?;

    // Read DSK value
    match read_dsk(&silink_sn, chip.as_deref()){
        Ok(dsk) => print_dsk(&dsk),
        Err(e) => {
            eprintln!("{}", e);
            process::exit(exitcode::NOINPUT);
        }
    }

    Ok(())
}

fn get_app_firmware_path(app_path: Option<String>, app_name:  Option<AppName>) -> Result<String, &'static str> {
    let mut app_rul_map: HashMap<AppName, &'static str> = HashMap::new();
    app_rul_map.insert(AppName::SwitchOnOff, "resources/app-binaries/zwave_soc_switch_on_off-brd2603a-eu.hex");
    app_rul_map.insert(AppName::MultilevelSensor, "resources/app-binaries/zwave_soc_multilevel_sensor-brd2603a-eu.hex");
    app_rul_map.insert(AppName::SerialAPI, "resources/app-binaries/zwave_ncp_serial_api_controller-brd2603a-eu.hex");
    app_rul_map.insert(AppName::SensorPIR, "resources/app-binaries/zwave_soc_sensor_pir-brd2603a-eu.hex");
    app_rul_map.insert(AppName::WallController, "resources/app-binaries/zwave_soc_wall_controller-brd2603a-eu.hex");
    app_rul_map.insert(AppName::PowerStrip, "resources/app-binaries/zwave_soc_power_strip-brd2603a-eu.hex");

    if app_name.is_some() {
        let file_path = app_rul_map.get(&app_name.unwrap()).expect("Failed to get file path from AppName!");
        return Ok(file_path.to_string());
    } else {
        if app_path.is_some() {
            return Ok(String::from(&app_path.unwrap()));
        } else {
            return Err("Missing app_path or app-name.");
        }
    }
}

fn get_serial_no(serial_no: Option<i32>, protocol: String, silink_automapped_devices: Vec<String>) -> Result<String, &'static str> {
    if serial_no != None {
        return Ok(serial_no.unwrap().to_string());
    } else {
        println!("Serial number is not specified.\n");

        match silink_inspect() {
            Ok(silink_result_string) => {
                // Filtering out the previously mapped devices from the available list to avoid confusion.
                let filtered_result_string = filtered_mapped_devices(&silink_result_string, silink_automapped_devices.clone()).unwrap();
                let vec_parsed = list_devices(&filtered_result_string, true);
                if vec_parsed.len() == 0 {
                return Err("Found no connected boards!\nPlease check the board's connection and try again.");
                }

                // TODO: Check input
                let mut line = String::new();
                println!("\nPlease select a device for {}.", protocol);
                println!("Note that the device's number may change when multiple boards are connected!");
                println!("Type 0, 1, ... or x to cancel:");
                std::io::stdin().read_line(&mut line).unwrap();

                if line.trim() == "x" {
                    return Err("");
                }

                match line.trim().parse::<usize>() {
                    Ok(n) => match vec_parsed.into_iter().enumerate().find(|(index, _)| index == &n ) {
                        Some(n_get) => Ok(n_get.1.serial_number.to_owned()),
                        None => Err("No board found matching that number!")
                    },
                    Err(_) => Err("Unrecognized input!")
                }
            }
            Err(error) => Err(error)
        }
    }
}

fn get_region(zwave_rf_region: Option<String>) -> i16 {
    match zwave_rf_region {
        None => {
            println!("RF region not given as parameter. Using default EU.");
            0i16
        },
        Some(region) => {
            *REGIONS.get_by_right(&region as &str).unwrap_or(&0i16)
        }
    }
}
