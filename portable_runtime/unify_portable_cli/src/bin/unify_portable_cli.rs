use std::process;
use unify_portable_core::{get_host_ip_address, system_verification, identify_unify_debian_packages, docker_compose_build, docker_compose_up, docker_compose_down, silink_get_device_count, silink_automap, silink_inspect, set_zpc_device, set_zpc_config_args};
use ctrlc;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use clap::Parser;
use std::net::IpAddr;

/// Argument parsing for windows
#[derive(Parser, Debug)]
#[clap(author, version, about, long_about = None)]

struct Args {

    #[clap(short, long, forbid_empty_values = true)]
    /// Path to device that should be directly mapped eg. -d /dev/ttyUSB0.
    device: Option<String>,
    #[clap(short, long, forbid_empty_values = true)]
    /// Serial number of device that can be mapped using Silink eg. -s 440262195.
    serial_no: Option<i32>,
    #[clap(short, long, forbid_empty_values = true)]
    /// IP Address for a direct IP connection to WSTK board eg. -i 10.0.0.12. 
    ip_address: Option<IpAddr>,
    #[clap(long, forbid_empty_values = true)]
    /// Region for the Z-Wave controller to operate in eg. --zwave-rf-region US.
    zwave_rf_region: Option<String>
}

fn main() {
    const RUNNING_ON_ARM: bool = cfg!(any(target_arch = "arm", target_arch = "aarch64"));

    if RUNNING_ON_ARM {
        eprintln!("Running on ARM/AARCH64 which is not a supported architecture just yet.");
        process::exit(19);
    }

    let parsed_args = Args::parse();
    let mut arg_count = 0;
    let mut zpc_device_string = get_host_ip_address(); // Default device string if we use Silink to map a device.
    let mut silink_sn = String::new();

    if parsed_args.device != None {
        arg_count += 1;
        zpc_device_string = parsed_args.device.unwrap();
    }

    if parsed_args.serial_no != None {
        arg_count += 1;
        silink_sn = parsed_args.serial_no.unwrap().to_string();
        // Use default value of zpc_device_string
    }

    if parsed_args.ip_address != None  {
        arg_count += 1;
        zpc_device_string = parsed_args.ip_address.unwrap().to_string();
    }

    if arg_count > 1 {
        eprintln!("You must supply only one of the options device, serial-no or ip-address or none at all.");
        process::exit(18);
    }

    set_zpc_device(&zpc_device_string);
    
    if parsed_args.zwave_rf_region != None  {
        let region_string = String::from("--zpc.rf_region ") + &parsed_args.zwave_rf_region.unwrap();
        set_zpc_config_args(&region_string);
    }

    if arg_count == 0 {
        let device_count = silink_get_device_count().unwrap();
        if device_count == 0 {
            eprintln!("Could not find any connected devices. Please verify that you have a Silabs device connected!");
            process::exit(18);
        } else if device_count == 1 {
            println!("Attempting to automap the connected device...");
        } else {
            eprintln!("Multiple devices detected and no device string was given.");
            eprintln!("Please provide the device serial number of the controller device eg. '-d 440262195'.");
            eprintln!("Here is a list of connected devices:");
            let silink_result = silink_inspect();
            if silink_result.is_err() && !RUNNING_ON_ARM {
                eprintln!("Failed to start Silink!");
            } else {
                silink_result.unwrap().wait().unwrap();
            }
            process::exit(19);
        }
    }

    println!("Mapping device with Silink...");

    let silink_handle = silink_automap(&silink_sn);

    if silink_handle.is_err() && !RUNNING_ON_ARM {
        eprintln!("Failed to start Silink!");
        process::exit(4);
    }

    let silink_is_running = !silink_handle.is_err();
    let silink_process = silink_handle.unwrap();

    println!("Verifying Docker installation");

    system_verification();

    println!("Building Unify Portable Runtime...");

    let example_package = identify_unify_debian_packages();
    if docker_compose_build(&example_package).unwrap().wait().is_err() {
        eprintln!("Failed to run 'docker-compose build'!");
        process::exit(4);
    }

    println!("Starting Unify Portable Runtime...");

    if docker_compose_up().unwrap().try_wait().is_err() {
        eprintln!("Failed to start Portable Runtime");
        process::exit(8);
    }

    ctrlc::set_handler(move || {
        println!("Stopping the Unify Portable Runtime...");

        if silink_is_running {
            silink_process.kill().unwrap();
        }

        if docker_compose_down().unwrap().try_wait().is_err() {
            eprintln!("Failed stopping Docker containers");
            process::exit(8);
        }
        process::exit(0);
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
