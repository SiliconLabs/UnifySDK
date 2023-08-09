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

            silink_sn = match get_serial_no(parsed_args.serial_no) {
                Ok(sn) => sn.to_string(),
                Err(e) => {
                    eprintln!("{}", e);
                    process::exit(exitcode::NOINPUT);
                }
            };

            match read_dsk(&silink_sn, chip_type.as_deref()) {
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
            let silink_result_string = silink_inspect().unwrap();
            let device = &list_devices(&silink_result_string, false)[0];
            if device.application_name.as_ref().unwrap().eq(&"unknown") ||
                device.application_name.as_ref().unwrap().eq(&"serial_api_controller") {
                println!("Identified as {} Serial API Controller ", device.serial_number);
            } else {
                eprintln!("Found a connected device, but could not identify it as Serial API Controller.");
                eprintln!("Try resetting or re-flashing it!");
                process::exit(exitcode::UNAVAILABLE);
            }
        } else {
            println!("Attempting to identify the Serial API Controller...");
            let silink_result_string = silink_inspect().unwrap();
            let devices = list_devices(&silink_result_string, false);
            match devices.iter().find(|device|
                device.application_name.is_some() &&
                device.application_name.as_ref().unwrap().eq(&"serial_api_controller")
            ) {
                None => {
                    eprintln!("Identification failed.");
                    silink_sn = match get_serial_no(parsed_args.serial_no) {
                        Ok(sn) => sn,
                        Err(e) => {
                            eprintln!("{}", e);
                            process::exit(exitcode::NOINPUT);
                        }
                    };
                },
                Some(device) => {
                    println!("Identified {} as a controller.", device.serial_number);
                    silink_sn = device.serial_number.clone();
                }
            }
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
        Ok(p) => if does_file_exists(p.clone()) {
                    final_app_path = p
                  } else {
                    return Err("Firmware not found")
                  },
        Err(e) => return Err(e),
    }

    let silink_sn;
    match get_serial_no(serial_no) {
        Ok(sn) => silink_sn = sn,
        Err(e) => return Err(e),
    }

    if chip.is_none() {
        println!("Processor type is not given as parameter. Using default ZGM230.");
    }

    let region = get_region(zwave_rf_region);

    println!("Flash application from {}", final_app_path);
    flash_device(&final_app_path, &silink_sn,
        &chip.clone().unwrap_or((&"ZGM230S").to_string()), region)?;

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

fn get_serial_no(serial_no: Option<i32>) -> Result<String, &'static str> {
    if serial_no != None {
        return Ok(serial_no.unwrap().to_string());
    } else {
        println!("Serial number is not specified.\n");

        match silink_inspect() {
            Ok(silink_result_string) => {
                let vec_parsed = list_devices(&silink_result_string, true);
                if vec_parsed.len() == 0 {
                return Err("Found no connected boards!\nPlease check the board's connection and try again.");
                }

                // TODO: Check input
                let mut line = String::new();
                println!("\nPlease select a device.");
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
