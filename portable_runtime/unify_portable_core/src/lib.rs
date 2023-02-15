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
    fs::File,
    io::Write,
    path::{Path, PathBuf},
    process::{Child, Command, ExitStatus, Output},
    collections::HashSet,
};
use regex::Regex;

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
      Ok(_) => true,
      Err(_) => false
    }
}

pub fn is_docker_compose_installed() -> bool {
    match run_cmd_in_dir!(["docker-compose", "version"], "").status() {
      Ok(_) => true,
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

pub fn silink_inspect() -> Result<Output, std::io::Error> {
    _ = silink_prepare().expect("Failed to set execution permissions for Silink.");
    run_cmd!([SILINK_PATH.to_str().unwrap(), "-inspect"]).output()
}

pub fn set_zpc_device(device_str: &str) {
    env::set_var("ZPC_DEVICE", device_str);
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

pub fn flash_device(name_path: &str, serialnum: &str, proccesor_type: &str, region: i8) -> Result<(), std::io::Error> {

    println!("Flashing FW for {} to device with serial no. {}", name_path, serialnum);

    let commander_path = commander_prepare().unwrap();

    // Erase flash
    //  TODO: Check the succes erase: Flash was erased successfully
    let erase_out = run_cmd!([commander_path, "device", "masserase", "-s", serialnum, "-d", proccesor_type]).output()?;
    let erase_out_string = String::from_utf8_lossy(&erase_out.stdout);
    println!("{}", erase_out.status);
    println!("{}", erase_out_string);

    // Flash MFG token
    let mfgtoken_out = run_cmd!([commander_path, "flash", "--tokengroup", "znet", "--token",
        &("MFG_ZWAVE_COUNTRY_FREQ:0x".to_owned() + &(format!("{:x}", region))), "-s", serialnum, "-d", proccesor_type]).output()?;
    let mfgtoken_out_string = String::from_utf8_lossy(&mfgtoken_out.stdout);
    println!("{}", mfgtoken_out_string);

    // Flash the downloaded hex file
    let flash_out = run_cmd!([commander_path, "flash", name_path, "-s", serialnum, "-d", proccesor_type]).output()?;
    let flash_out_string = String::from_utf8_lossy(&flash_out.stdout);
    println!("{}", flash_out_string);
    Ok(())
}

pub fn read_dsk(serialnum: &str, proccesor_type: &str) -> Result<String, Box<dyn std::error::Error>> {
    let commander_path = commander_prepare().unwrap();
    // Read DSK value
    let qr_raw_cmd = run_cmd!([commander_path, "tokendump", "--tokengroup", "znet", "--token", "MFG_ZW_QR_CODE", "-s", serialnum, "-d", proccesor_type]).output()?;
    let qr_raw_string = String::from_utf8_lossy(&qr_raw_cmd.stdout);

    let re = Regex::new(r#"MFG_ZW_QR_CODE: "([0-9]*)""#).unwrap();
    let dsk_match_string = re.captures(&qr_raw_string);

    match dsk_match_string {
        None => Ok(String::new()),
        Some(caps) => Ok(caps.get(1).unwrap().as_str().to_string()),
}
}

pub fn get_serialnum(silink_result_tmp: &str) -> Vec<String> {

    let mut vec_return = Vec::new();

    let re_device = Regex::new(r#"(\{(.|\n)*?\})"#).unwrap();
    let re_serial = Regex::new(r#"(serialNumber=)([0-9]{9})"#).unwrap();
    let re_board  = Regex::new(r#"(boardSerial\[0\]=)([0-9]{9})"#).unwrap();
    let new_string:String = silink_result_tmp.to_string().clone();
    let vec_device:HashSet<&str> = re_device.find_iter(&new_string).map(|mat| mat.as_str()).collect();

    let mut act_device_num = 0;

    for act_device in vec_device {

        let device_string_tmp:String = act_device.to_string().clone();

        println!("Device: {}", act_device_num);
        let vec_serial:HashSet<&str> = re_serial.find_iter(&device_string_tmp).map(|mat| mat.as_str()).collect();
        for act_serial in vec_serial {
            let serial_formated:String = act_serial[act_serial.len() - 9..].to_string();
            println!("\t Serialnumber: {}", serial_formated);
            vec_return.push(serial_formated);
        }

        let vec_board:HashSet<&str>  = re_board.find_iter(&device_string_tmp).map(|mat| mat.as_str()).collect();
        for act_board in vec_board {
            println!("\t Boardnumber: {}", act_board[act_board.len() - 9..].to_string());
        }

        act_device_num = act_device_num + 1;
    }

   return vec_return;
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
