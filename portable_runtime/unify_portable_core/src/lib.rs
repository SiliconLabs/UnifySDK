#[cfg(any(target_arch = "arm", target_arch = "aarch64"))]
compile_error!("Neither ARM nor AARCH64 is supported");

#[macro_use]
extern crate lazy_static;

// See run_unify.py for features not yet ported.
#[cfg(target_os = "windows")]
use std::os::windows::process::CommandExt;

use file_matcher::FileNamed;
use port_selector::{is_free, Port};
use std::{
    env,
    error::Error,
    fmt,
    fs::{File, remove_file},
    io::{Write, stdout},
    net::IpAddr,
    path::{Path, PathBuf},
    process::{Child, Command, ExitStatus},
    str::FromStr,
};
use regex::Regex;
use bimap::BiMap;

const LOG_NAME: &str = "./portable_runtime.log";

const DOCKER_CTX: &str = "resources/docker-files";

#[cfg(target_os = "windows")]
lazy_static! { static ref SILINK_PATH: PathBuf = PathBuf::from(r#".\resources\silink\silink.exe"#); }
#[cfg(target_os = "linux")]
lazy_static! { static ref SILINK_PATH: PathBuf = PathBuf::from(r#"./resources/silink/silink"#); }
#[cfg(target_os = "macos")]
lazy_static! { static ref SILINK_PATH: PathBuf = PathBuf::from(r#"./resources/silink/silink"#); }

#[cfg(target_os = "windows")]
lazy_static! { static ref COMMANDER_PATH: PathBuf = PathBuf::from(r#".\resources\commander\commander.exe"#); }
#[cfg(target_os = "linux")]
lazy_static! { static ref COMMANDER_PATH: PathBuf = PathBuf::from(r#"./resources/commander/commander"#); }
#[cfg(target_os = "macos")]
lazy_static! { static ref COMMANDER_PATH: PathBuf = PathBuf::from(r#"/Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/adapter_packs/commander/Commander.app/Contents/MacOS/commander"#); }

macro_rules! create_cmd {
    ( $args:expr) => {{
        #[cfg(target_os = "windows")]
        let (shell, shell_arg) = ("cmd", "/C");
        #[cfg(not(target_os = "windows"))]
        let (shell, shell_arg) = ("sh", "-c");

        let mut cmd = Command::new(shell);
        cmd.arg(shell_arg).arg($args.join(" "));
        cmd.env("DOCKER_DEFAULT_PLATFORM", "linux/amd64");

        writeln!(get_log_file(), "{:#?}", cmd).unwrap();

        creation_flag(cmd)
    }};
}

macro_rules! run_cmd {
    ( $args:expr) => {{
        create_cmd!($args)
    }};
}

macro_rules! run_cmd_in_dir {
    ($args:expr, $working_dir:expr) => {{
        let mut cmd = create_cmd!($args);
        if $working_dir != "" {
            cmd.current_dir($working_dir);
        }
        cmd.stdout(get_log_file()).stderr(get_log_file());
        cmd
    }};
}

#[cfg(target_os = "windows")]
fn creation_flag(mut cmd: Command) -> Command {
    const CREATE_NO_WINDOW: u32 = 0x08000000;
    cmd.creation_flags(CREATE_NO_WINDOW);
    cmd
}

#[cfg(not(target_os = "windows"))]
fn creation_flag(cmd: Command) -> Command {
    cmd
}

fn get_log_file() -> File {
    File::options()
        .append(true)
        .open(LOG_NAME)
        .expect("Failed to open log")
}

pub fn init_log() {
    File::create(LOG_NAME).expect("Failed to create log");
}

pub fn is_docker_available() -> bool {
    match run_cmd_in_dir!(["docker", "info"], "").status() {
      Ok(status) => status.success(),
      Err(_) => false
    }
}

pub fn does_file_exists(input_path: String) -> bool {
  let path_obj = Path::new(&input_path);
  path_obj.exists()
}

pub fn is_docker_compose_installed() -> bool {
    match run_cmd_in_dir!(["docker-compose", "version"], "").status() {
      Ok(status) => status.success(),
      Err(_) => false
    }
}

pub fn get_host_ip_address() -> String {
    return local_ipaddress::get().unwrap();
}

pub fn get_busy_ports() -> Vec<u16> {
    let unify_ports = vec![1337, 1343, 1883, 3000, 3080];
    let mut busy_ports = Vec::new();
    for required_port in unify_ports.iter() {
        let check_port: Port = *required_port;
        if !is_free(check_port) {
            busy_ports.push(*required_port);
        }
    }
    return busy_ports;
}

pub fn identify_unify_debian_packages() -> Result<String, Box<dyn std::error::Error>> {
    let unify_zip_exists = FileNamed::wildmatch("unify_*.zip")
        .within(DOCKER_CTX)
        .find()?;

    return Ok(unify_zip_exists
        .file_name()
        .unwrap()
        .to_os_string()
        .into_string()
        .unwrap());
}

pub fn docker_compose_build(path: &str) -> Result<ExitStatus, std::io::Error> {
    println!("Building docker image with {}", path);

    run_cmd_in_dir!(
        ["docker-compose", "build", "--build-arg", &format!("DEBIAN_PACKAGES={}", path)],
        Path::new(DOCKER_CTX).to_str().unwrap()
    ).spawn()?
    .wait()
}

#[derive(Debug)]
struct SilinkError(String);

impl fmt::Display for SilinkError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "There is an error: {}", self.0)
    }
}

impl Error for SilinkError {}

pub fn silink_prepare() -> Result<&'static str, Box<dyn std::error::Error>> {
    #[cfg(target_os = "macos")] {
        run_cmd!([
            "xattr",
            "-dr",
            "com.apple.quarantine",
            SILINK_PATH.to_str().unwrap()
        ]).output()?;
        run_cmd!(["chmod", "+x", SILINK_PATH.to_str().unwrap()]).output()?;
    }

    #[cfg(target_os = "linux")] {
        run_cmd!(["chmod", "+x", SILINK_PATH.to_str().unwrap()]).output()?;
    }

    Ok(SILINK_PATH.to_str().unwrap())
}

pub fn silink_get_device_count() -> Result<i32, Box<dyn std::error::Error>> {
    _ = silink_prepare().expect("Failed to set execution permissions for Silink.");

    let cmd_res = run_cmd!([SILINK_PATH.to_str().unwrap(), "-list"]).output()?;
    let device_count_string = String::from_utf8_lossy(&cmd_res.stdout).into_owned();
    println!("devices: {}", device_count_string);
    let mut parts = device_count_string
        .split_whitespace()
        .map(|s| s.parse::<i32>());
    match parts.next() {
        Some(count) => Ok(count?),
        None => Ok(0),
    }
}

pub fn silink_automap(serial_number: &str) -> Result<Child, std::io::Error> {
    _ = silink_prepare().expect("Failed to set execution permissions for Silink.");

    if serial_number.is_empty() {
      run_cmd_in_dir!([SILINK_PATH.to_str().unwrap(), "-automap", "4900"], "").spawn()
    } else {
      run_cmd_in_dir!(
            [
                SILINK_PATH.to_str().unwrap(),
                "-sn",
                serial_number,
                "-automap",
                "4900"
            ],
            ""
        ).spawn()
    }
}

pub fn silink_inspect() -> Result<String, &'static str> {
    _ = silink_prepare().expect("Failed to set execution permissions for Silink.");
    match run_cmd!([SILINK_PATH.to_str().unwrap(), "-inspect"]).output() {
        Ok(result) => {
            Ok(String::from_utf8_lossy(&result.stdout).to_string())
        }
        Err(_) => Err("Failed to start Silink!")
    }
}

pub fn set_zpc_device(device_str: &str) {
    match IpAddr::from_str(device_str) {
        Ok(_)  => env::set_var("ZPC_DEVICE_IP", device_str),
        Err(_) => env::set_var("ZPC_DEVICE_TTY", device_str)
    }
}

pub fn set_zpc_config_args(arg_str: &str) {
    env::set_var("ZPC_CONFIG_ARGS", arg_str);
}

pub fn docker_compose_up() -> Result<ExitStatus, std::io::Error> {
    println!("Starting Unify Portable Runtime...");
    run_cmd_in_dir!(
        ["docker-compose", "up", "-d"],
        Path::new(DOCKER_CTX).to_str().unwrap()
    ).spawn()?
    .wait()
}

pub fn docker_compose_down() -> Result<ExitStatus, std::io::Error> {
    println!("Stopping the Unify Portable Runtime...");
    run_cmd_in_dir!(
        ["docker-compose", "down"],
        Path::new(DOCKER_CTX).to_str().unwrap()
    ).spawn()?
    .wait()
}

pub fn kill_process(pid: &str) {
    #[cfg(target_os = "windows")]
    run_cmd!(["taskkill", "/F", "/PID", pid]).output().unwrap();

    #[cfg(not(target_os = "windows"))]
    run_cmd!(["kill", "-9", pid]).output().unwrap();

}

#[derive(Debug)]
struct CommanderError(String);

pub struct DeviceInfo {
    pub serial_number: String,
    pub board_number: String,
    pub application_name: Option<String>,
    pub frequency: Option<String>
}

lazy_static! {
    pub static ref REGIONS: BiMap<i16, &'static str> = {
        BiMap::from_iter(vec![
            (0, "EU"),
            (1, "US"),
            (2, "ANZ"),
            (3, "HK"),
            (4, "MA"),
            (5, "IN"),
            (6, "IS"),
            (7, "RU"),
            (8, "CN"),
            (9, "US_LR"),
            (32, "JP"),
            (33, "KR"),
        ])
    };
}

impl fmt::Display for CommanderError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "There is an error: {}", self.0)
    }
}

impl Error for CommanderError {}

pub fn commander_prepare() -> Result<&'static str, Box<dyn std::error::Error>> {
    if COMMANDER_PATH.try_exists().is_err() {
      return Err("Failed to verify the 'commander' flashing tool to be available.\n\
                  Please go to Simplicity Studio's website [https://www.silabs.com/developers/simplicity-studio] and follow the instructions to download and install.".into());
    }

    #[cfg(target_os = "linux")]
    run_cmd!(["chmod", "+x", COMMANDER_PATH.to_str().unwrap()]).status()?;

    Ok(COMMANDER_PATH.to_str().unwrap())
}

pub fn flash_device(name_path: &str, serialnum: &str, proccesor_type: &str, region: i16) -> Result<(), &'static str> {

    println!("Flashing FW {} to device with serial no. {}", name_path, serialnum);

    let commander_path = commander_prepare().unwrap();

    println!("\t- Erasing flash");
    match run_cmd_in_dir!([commander_path, "device", "masserase", "-s", serialnum, "-d", proccesor_type], "").status() {
        Err(_) => return Err("Failed to mass erase!"),
        Ok(stat) => if ! stat.success() { return Err("Failed to mass erase!"); } else { () },
    }

    println!("\t- Flashing MFG token");
    match run_cmd_in_dir!([commander_path, "flash", "--tokengroup", "znet", "--token", &("MFG_ZWAVE_COUNTRY_FREQ:0x".to_owned() + &(format!("{:x}", region))), "-s", serialnum, "-d", proccesor_type], "").status() {
        Err(_) => return Err("Failed to flash manufacturer token!"),
        Ok(stat) => if ! stat.success() { return Err("Failed to flash manufacturer token!"); } else { () },
    }

    println!("\t- Flashing application firmware file");
    match run_cmd_in_dir!([commander_path, "flash", name_path, "-s", serialnum, "-d", proccesor_type], "").status() {
        Err(_) => return Err("Failed to flash the application!"),
        Ok(stat) =>
          if ! stat.success() {
              return Err("Failed to flash the application!");
          } else {
              println!("Flashing succeeded!");
              Ok(())
          },
    }
}

fn read_mfg_token(serial_number: &str, proccesor_type: Option<&str>, token: &str, regex: Regex) -> Result<String, Box<dyn std::error::Error>> {
    let commander_path = commander_prepare().unwrap();
    let mut command_parts = vec![commander_path, "tokendump", "--tokengroup", "znet", "--token", token, "-s", serial_number];
    if proccesor_type.is_some() {
        command_parts.push("-d");
        command_parts.push(proccesor_type.unwrap());
    }
    let qr_raw_cmd = run_cmd!(command_parts).output()?;
    let qr_raw_string = String::from_utf8_lossy(&qr_raw_cmd.stdout);
    let dsk_match_string = regex.captures(&qr_raw_string);

    match dsk_match_string {
        None => Ok(String::new()),
        Some(caps) => Ok(caps.get(1).unwrap().as_str().to_string()),
    }
}

pub fn read_dsk(serial_number: &str, proccesor_type: Option<&str>) -> Result<String, Box<dyn std::error::Error>> {
    read_mfg_token(serial_number, proccesor_type, "MFG_ZW_QR_CODE",
        Regex::new(r#"MFG_ZW_QR_CODE: "([0-9]*)""#).unwrap()
    )
}

fn read_freq(serial_number: &str, proccesor_type: Option<&str>) -> Option<String> {
    let freq_string = read_mfg_token(serial_number, proccesor_type, "MFG_ZWAVE_COUNTRY_FREQ",
        Regex::new(r#"MFG_ZWAVE_COUNTRY_FREQ: 0x([[:xdigit:]]{2})"#).unwrap()
    ).unwrap();
    let freq_value = i16::from_str_radix(&freq_string, 16).unwrap_or(254);
    match REGIONS.get_by_left(&freq_value) {
        Some(region) => Some(region.to_string()),
        None => None
    }
}

pub fn list_devices(silink_result_tmp: &str, print_output: bool) -> Vec<DeviceInfo> {
    let new_string = silink_result_tmp.to_string();

    let re_device = Regex::new(r#"(\{(.|\n)*?\})"#).unwrap();
    let re_serial = Regex::new(r#"serialNumber=([0-9]{9})"#).unwrap();
    let re_board  = Regex::new(r#"boardSerial\[0\]=([0-9]{9})"#).unwrap();

    if print_output {
        println!("{: >10} | {: <10} | {: <9} | {: <5} | {: <29}",
            "Device No.", "Serial No.", "Board No.", "Freq.", "Application name");
        println!("{: >10}-+-{: <10}-+-{: <9}-+-{: <5}-+-{: <29}",
            "-".repeat(10), "-".repeat(10), "-".repeat(9), "-".repeat(5), "-".repeat(29));
    }

    re_device.find_iter(&new_string)
        .enumerate()
        .map(move |(number, device)| {
            let device_string = device.as_str().to_owned();
            let serial_number = re_serial.captures(&device_string)
                .unwrap()[1].to_owned();
            let board_number = re_board.captures(&device_string)
                .unwrap()[1].to_owned();
            let frequency = read_freq(&serial_number, None);
            if print_output {
                print!("{: >10} | {: <10} | {: <9} | {: <5} | ",
                    number, serial_number, board_number,
                    frequency.clone().unwrap_or("?".to_string())
                );
                stdout().flush().expect("Couldn't flush stdout");
            }
            // FIXME - Setting  default_controller as appliaction name when get_name fails
            // FIXME - Ideally it should be read from device
            let mut application_name = get_name(&serial_number);
            if application_name.is_none()  {
                application_name = Some(String::from("unknown"));
            }

            if print_output {
                println!( "{: <29}",
                    application_name.clone().unwrap_or((&"unknown").to_string()).to_string()
                );
            }
            DeviceInfo{serial_number, board_number, application_name, frequency}
        })
        .collect()
}

pub fn print_dsk(dsk: &str){
    if dsk.chars().count() < 53 {
        eprintln!("No DSK key found. For the serial-api App, there is no dsk key. {}", dsk);
    } else{
        println!("Here is your device DSK value.\nCopy this value for later secure inclusion: {}-{}-{}-{}-{}-{}-{}-{}",
                 &dsk[12..17],
                 &dsk[17..22],
                 &dsk[22..27],
                 &dsk[27..32],
                 &dsk[32..37],
                 &dsk[37..42],
                 &dsk[42..47],
                 &dsk[47..52]);
    }
}

pub fn get_name(serial_number: &str) -> Option<String> {
    let commander_path = commander_prepare().unwrap();

    // Read NVM3 contents to a temporary file, parse, then delete it
    let nvm_file = format!("nvm_{}.s37", serial_number);
    run_cmd!([commander_path, "nvm3", "read", "-s", serial_number, "-o", &nvm_file]).output().ok();
    let command_result = run_cmd!([
        commander_path, "nvm3", "parse", &nvm_file, "--key", "0x4100c" ])
        .output().unwrap();
    remove_file(&nvm_file).ok();

    let commander_output = String::from_utf8_lossy(&command_result.stdout);
    if !commander_output.contains("Matching NVM3 objects:") {
        None
    } else {
        // Parse commander output, convert bytes to ASCII string
        let matcher = Regex::new(r"(?m)[[:xdigit:]]{8}: ((?: ?[[:xdigit:]]{2})+)").unwrap();
        let ascii_chars = matcher.captures_iter(&commander_output)
            .take(2)
            .map(|cap| cap[1].to_owned())
            .collect::<Vec<String>>()
            .join(" ") // Join lines to get space-delimited list of bytes, i.e. "01 23 45 ..."
            .split(" ")
            .map(|character| u8::from_str_radix(character, 16).unwrap())
            .filter(|character| character != &0) // Remove trailing '\0' characters
            .collect::<Vec<u8>>();
        let name = std::str::from_utf8(&ascii_chars).unwrap();
        Some(name.to_owned())
    }
}
