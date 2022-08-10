// See run_unify.py for features not yet ported.

use duct::{cmd, Handle};
use std::env;
use std::error::Error;
use std::fmt;
use std::process;
use file_matcher::FileNamed;
use port_selector::{is_free, Port};

pub fn is_docker_installed() -> Result<(), Box<dyn std::error::Error>> {
    cmd!("docker").stdout_null().stderr_null().run()?;
    Ok(())
}

pub fn is_docker_compose_installed() -> Result<(), Box<dyn std::error::Error>> {
    match cmd!("docker", "compose").stdout_null().stderr_null().run() {
        Err(..) => {
            cmd!("docker-compose").stdout_null().stderr_null().run()?;
            Ok(())
        }
        _ => Ok(())
    }
}

pub fn get_docker_version() {
    // Meh - Use serde_json to parse the output of cmd!("docker", "version", "-f", "json").run()?;
}

pub fn get_host_ip_address() -> String{
    return local_ipaddress::get().unwrap();
}

pub fn system_verification() {
    if is_docker_installed().is_err() || is_docker_compose_installed().is_err() {
        eprintln!("Please make sure Docker and Docker Compose is installed!\n\
                   Go to https://www.docker.com/ and follow the instructions to download and install.");

        process::exit(1);
    }

    let unify_ports = vec![1337, 1883, 3080];

    for required_port in unify_ports.iter() {
        let check_port: Port = *required_port;
        if !is_free(check_port) {
            eprintln!("Another process is using port {}.\n\
                       Please make sure to stop that service before trying to launch Unify.",
                       required_port);

            process::exit(1);
        }
    }
}

pub fn identify_unify_debian_packages() -> String {
    let unify_zip_exists = FileNamed::wildmatch("unify_*.zip").within("resources/docker-files").find();

    let example_package = if cfg!(target_arch = "arm") 
                            { "unify_1.1.1_armhf.zip".to_string() }
                          else
                            { "unify_1.1.1_x86_64.zip".to_string() };

    if unify_zip_exists.is_err() {
        let unify_release_url = "https://github.com/SiliconLabs/UnifySDK/releases";

        eprintln!("Could not find Unify zip file in the docker-files directory!\n\
                   You can find the Unify packages here: {}.\n\
                   Make sure to login to verify that you have access to the Unify Framework.\n\
                   Download the Zip package of Unify and place it in the 'docker-files' folder.\n\
                   Choose the one that matches your architecture - in your case {}.",
                   unify_release_url,
                   example_package);
        process::exit(2);
    }

    return unify_zip_exists.unwrap().file_name().unwrap().to_os_string().into_string().unwrap()
}

pub fn docker_compose_build(path: &str) -> Result<Handle, Box<dyn std::error::Error>> {
    println!("Building docker image with {}", path);
    let handle = cmd!("docker-compose", "build", "--build-arg",
                                                 format!("DEBIAN_PACKAGES={}", path)).dir("resources/docker-files")
                                                                                     .stderr_to_stdout()
                                                                                     .stdout_path("docker.log")
                                                                                     .start()
                                                                                     .unwrap();
    return Ok(handle);
}

#[derive(Debug)]
struct SilinkError(String);

impl fmt::Display for SilinkError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "There is an error: {}", self.0)
    }
}

impl Error for SilinkError {}

fn silink_prepare() -> Result<String, Box<dyn std::error::Error>> {
    if cfg!(target_arch = "arm") || cfg!(target_arch = "aarch64") {
        return Err(Box::new(SilinkError("Unsupported OS or architecture for Silink.".into())))
    }

    let mut silink_cmd = String::from("resources/silink/silink");

    if cfg!(target_os = "windows") {
        silink_cmd = String::from("resources/silink/silink.exe");
    } else if cfg!(target_os = "macos") {
        cmd!("xattr", "-dr", "com.apple.quarantine", "resources/silink/silink").run().unwrap();
        cmd!("chmod", "+x", "resources/silink/silink").run().unwrap();
    } else if cfg!(target_os = "linux") {
        cmd!("chmod", "+x", "resources/silink/silink").run().unwrap();
    }

    Ok(silink_cmd)
}

pub fn silink_get_device_count() -> Result<i32, Box<dyn std::error::Error>> {
    let silink_cmd = silink_prepare().unwrap();

    let device_count_string = cmd!(silink_cmd, "-list").read()?;

    let mut parts = device_count_string.split_whitespace().map(|s| s.parse::<i32>());
    let device_count = parts.next().unwrap().unwrap();

    return Ok(device_count);
}

pub fn silink_automap(serial_number: &str) -> Result<Handle, std::io::Error> {
    let silink_cmd = silink_prepare().unwrap();
    if serial_number.is_empty() {
        return cmd!(silink_cmd, "-automap", "4900").stderr_to_stdout()
                                                                  .stdout_path("silink.log")
                                                                  .start();
    } else {
        return cmd!(silink_cmd, "-sn", serial_number, "-automap", "4900").stderr_to_stdout()
                                                   .stdout_path("silink.log")
                                                   .start();
    }
}

pub fn silink_inspect() -> Result<Handle, std::io::Error> {
    let silink_cmd = silink_prepare().unwrap();

    return cmd!(silink_cmd, "-inspect").start();
}


pub fn set_zpc_device(device_str: &str) {
    env::set_var("ZPC_DEVICE", device_str);
}

pub fn set_zpc_config_args(arg_str: &str) {
    env::set_var("ZPC_CONFIG_ARGS", arg_str);
}

pub fn docker_compose_up() -> Result<Handle, Box<dyn std::error::Error>> {
    let handle = cmd!("docker-compose", "up", "-d").dir("resources/docker-files")
                                                   .stderr_to_stdout()
                                                   .stdout_path("docker.log")
                                                   .start()
                                                   .unwrap();
    return Ok(handle);
}

pub fn docker_compose_down() -> Result<Handle, Box<dyn std::error::Error>> {
    let handle = cmd!("docker-compose", "down").dir("resources/docker-files")
                                               .stderr_to_stdout()
                                               .stdout_path("docker.log")
                                               .start()
                                               .unwrap();
    return Ok(handle);
}
