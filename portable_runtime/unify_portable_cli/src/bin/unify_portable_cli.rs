extern crate exitcode;
use clap::{Parser, Subcommand};
use ctrlc;
use std::net::IpAddr;
use std::process;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use unify_portable_core::*;
use std::collections::HashMap;

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
    /// Available option for --app-name are: switch-on-off, multilevel-sensor, serial-api
    /// Two arguments can be specified optionally before the subcommand from the main
    /// arguments: `--serial-no` and `--zwave-rf-region`.
    /// In case of serial number is not specified, the choice will be prompted.
    /// In case of rf region is not specified, the default EU region frequency will be used.
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
    /// In case of only one device is connected, serial-no argument is optional.
    ReadDsk {
        #[clap(short = 'c', long, forbid_empty_values = false)]
        /// Chip type. If you not give any then the default: ZGM230.
        chip_type: Option<String>,
    },
    /// Lists the info of the connected devices.
    /// With boardSerial[0] can identify physically the board. With serialNumber you can flash.
    ListDevices {
    }
}

#[derive(clap::ArgEnum, Clone, Debug, PartialEq, Eq, Hash)]
enum AppName {
   SwitchOnOff,
   MultilevelSensor,
   SerialAPI,
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
    let mut arg_count = 0;
    let mut zpc_device_string = get_host_ip_address(); // Default device string if we use Silink to map a device.
    let mut silink_sn = String::new();

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

            let mut proc_type = String::from("ZGM230");
            if chip_type.is_none() {
                // Set default value, if not set any.
               println!("Chip type is not given as parameter. Using default ZGM230.");
            }
            else {
                proc_type.push_str(&chip_type.clone().unwrap());
            }

            silink_sn = match get_serial_no(parsed_args.serial_no) {
                Ok(sn) => sn.to_string(),
                Err(e) => {
                    eprintln!("{}", e);
                    process::exit(exitcode::NOINPUT);
                }
            };

            match read_dsk(&silink_sn, &proc_type) {
                Ok(dsk) => print_dsk(&dsk),
                Err(e) => {
                  eprintln!("{}", e);
                  process::exit(exitcode::NOINPUT);
              }
            }

            process::exit(exitcode::OK);
        }
        Some(Commands::ListDevices {}) => {

            let silink_result = silink_inspect();
            if silink_result.is_err() {
                eprintln!("Failed to start Silink!");
            } else {

                let silink_result_tmp = silink_result.unwrap();
                let silink_result_string = String::from_utf8_lossy(&silink_result_tmp.stdout);
                println!("{}", silink_result_string);
            }

            process::exit(exitcode::OK);
        }
        None => {
            // Intentionally blank block
            // Default arguments, continue with default unify portable cli argument processing
        }
    }

    // Default unify portable cli argument processing

    if parsed_args.device != None {
        arg_count += 1;
        zpc_device_string = parsed_args.device.unwrap();
    }

    if parsed_args.serial_no != None {
        arg_count += 1;
        silink_sn = parsed_args.serial_no.unwrap().to_string();
        // Use default value of zpc_device_string
    }

    if parsed_args.ip_address != None {
        arg_count += 1;
        zpc_device_string = parsed_args.ip_address.unwrap().to_string();
    }

    if arg_count > 1 {
        eprintln!("You must supply only one of the options device, serial-no or ip-address or none at all.");
        process::exit(exitcode::USAGE);
    }

    set_zpc_device(&zpc_device_string);

    if parsed_args.zwave_rf_region != None {
        let region_string =
            String::from("--zpc.rf_region ") + &parsed_args.zwave_rf_region.unwrap();
        set_zpc_config_args(&region_string);
    }

    println!("Verifying Docker installation");

    if !is_docker_available() || !is_docker_compose_installed() {
        eprintln!("Call to Docker failed!\n\
                   Please make sure Docker and Docker Compose is installed and running.\n\
                   Go to https://www.docker.com/ and follow the instructions to download and install.");

        process::exit(exitcode::UNAVAILABLE);
    }

    if arg_count == 0 {
        let device_count = silink_get_device_count().unwrap();
        if device_count == 0 {
            eprintln!("Could not find any connected devices. Please verify that you have a Silabs device connected!");
            process::exit(exitcode::USAGE);
        } else if device_count == 1 {
            println!("Attempting to automap the connected device...");
        } else {
            eprintln!("Multiple devices detected and no device string was given.");
            eprintln!("Please provide the device serial number of the controller device eg. '-s 440262195'.");
            eprintln!("Here is a list of connected devices:");
            let silink_result = silink_inspect();
            if silink_result.is_err() {
                eprintln!("Failed to start Silink!");
            } else {
                silink_result.unwrap();
            }
            process::exit(exitcode::USAGE);
        }
    }

    println!("Mapping device with Silink...");

    let silink = silink_automap(&silink_sn);
    let silink_is_running = !silink.is_err();
    let mut silink_process = silink.unwrap();
    if !silink_is_running || silink_process.try_wait().is_err() {
        eprintln!("Failed to start Silink!");
        process::exit(exitcode::TEMPFAIL);
    }

    println!("Building Unify Portable Runtime...");

    let unify_zip_exists = identify_unify_debian_packages();
    if unify_zip_exists.is_err() {
        let unify_release_url = "https://github.com/SiliconLabs/UnifySDK/releases";
        let example_package = "unify_1.1.1_x86_64.zip".to_string();
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

    docker_compose_up().expect("Failed to start Portable Runtime");

    ctrlc::set_handler(move || {
        if silink_is_running {
            silink_process.kill().unwrap();
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
        Ok(p) => final_app_path = p,
        Err(e) => return Err(e),
    }

    let silink_sn;
    match get_serial_no(serial_no) {
        Ok(sn) => silink_sn = sn,
        Err(e) => return Err(e),
    }

    let proc_type;
    if chip.is_none() {
        // Set default value, if not set any.
        proc_type = String::from("ZGM230");
        println!("Processor type is not given as parameter. Using default ZGM230.");
    } else {
        proc_type = String::from(&chip.clone().unwrap());
    }

    let region = get_region(zwave_rf_region);

    println!("Flash application under from {}", final_app_path);
    flash_device(&final_app_path, &silink_sn, &proc_type, region).expect("Failed to flash!");

    // Read DSK value
    match read_dsk(&silink_sn, &proc_type){
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

fn get_serial_no(serial_no: Option<i32>) -> Result<String, &'static str> {
    if serial_no != None {
        return Ok(serial_no.unwrap().to_string());
    } else {
        println!("Serial number is not specified.");

        let silink_result = silink_inspect();
        if silink_result.is_err() {
            return Err("Failed to start Silink!");
        } else {

            let silink_result_tmp = silink_result.unwrap();
            let silink_result_string = String::from_utf8_lossy(&silink_result_tmp.stdout);

            let vec_parsed = get_serialnum(&silink_result_string);
            if vec_parsed.len() == 0 {
              return Err("Found no connected boards!\nPlease check the board's connection and try again.");
            }

            // TODO: Check input
            let mut line = String::new();
            println!("Please enter the number of the device to flash.");
            eprintln!("Be aware that the Device no. is not always the same when multiple boards are connected!!!");
            println!("Press 0, 1, ... or x to cancel:");
            std::io::stdin().read_line(&mut line).unwrap();

            if line.trim() == "x" {
                return Err("");
            }

            match line.trim().parse::<usize>() {
                Ok(n) => match vec_parsed.get(n) {
                    Some(n_get) => Ok(n_get.to_string()),
                    None => Err("No board found matching that number!")
                },
                Err(_) => Err("Unrecognized input!")
            }
        }
    }
}

fn get_region(zwave_rf_region: Option<String>) -> i8 {
    if zwave_rf_region.is_none(){
        println!("RF region not given as parameter. Using default EU.");
        return 0x00;
    } else {
        let mut rf_region_map: HashMap<String, i8> = HashMap::new();
        rf_region_map.insert("EU".to_string(), 0);
        rf_region_map.insert("US".to_string(), 1);
        rf_region_map.insert("US_LR".to_string(), 9);
        rf_region_map.insert("ANZ".to_string(), 2);
        rf_region_map.insert("HK".to_string(), 3);
        rf_region_map.insert("MA".to_string(), 4);
        rf_region_map.insert("IN".to_string(), 5);
        rf_region_map.insert("IS".to_string(), 6);
        rf_region_map.insert("RU".to_string(), 7);
        rf_region_map.insert("CN".to_string(), 8);
        rf_region_map.insert("JP".to_string(), 32);
        rf_region_map.insert("KR".to_string(), 7);

        match rf_region_map.get(&zwave_rf_region.unwrap()) {
            Some(r) => return *r,
            None => return 0x00,
        }
    }
}
