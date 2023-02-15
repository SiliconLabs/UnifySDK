#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

extern crate exitcode;
use exitcode::ExitCode;
use serde_repr::*;
use std::{env, sync::RwLock};
use tauri::Window;
use unify_portable_core::*;

#[derive(Clone, Debug, PartialEq, Serialize_repr)]
#[repr(u8)]
enum Status {
    Init = 0,
    Build = 1,
    SetDevice = 2,
    Run = 3,
    Complete = 4,
}

#[derive(Clone, Debug, PartialEq, Serialize_repr)]
#[repr(u8)]
enum BuildStatus {
    Init = 0,
    Waiting = 1,
    Pass = 2,
    Error = 3,
}

#[derive(Clone, serde::Serialize)]
#[allow(non_snake_case)]
struct BuildActions {
    Docker: BuildStatus,
    DockerCompose: BuildStatus,
    UnifyLibrary: BuildStatus,
    BuildDocker: BuildStatus,
    RunDocker: BuildStatus,
}

#[derive(Clone, serde::Serialize)]
#[allow(non_snake_case)]
struct State {
    Status: Status,
    BuildActions: BuildActions,
    ErrorMessage: String,
    ExitCode: ExitCode,
}

#[derive(Clone)]
#[allow(non_snake_case)]
struct TauriState {
    State: State,
    SilinkProcessId: u32,
}

const RUNNING_ON_ARM: bool = cfg!(any(target_arch = "arm", target_arch = "aarch64"));

#[tauri::command]
async fn init(
    window: Window,
    state: tauri::State<'_, RwLock<TauriState>>,
) -> Result<State, tauri::Error> {
    if silink_get_device_count().unwrap() > 0 {
        let device_list = silink_inspect();
        if device_list.is_ok() {
            window
                .emit(
                    "devices",
                    String::from_utf8_lossy(&device_list?.stdout).into_owned(),
                )
                .unwrap();
        }
    }

    return Ok(state.read().unwrap().State.clone());
}

#[tauri::command]
async fn build(
    window: Window,
    state: tauri::State<'_, RwLock<TauriState>>,
) -> Result<(), tauri::Error> {
    let mut app_state = state.write().unwrap();
    app_state.State.Status = Status::Build;
    app_state.State.BuildActions.Docker = BuildStatus::Waiting;
    window.emit("state", app_state.State.clone()).unwrap();

    if is_docker_available().is_err() {
        app_state.State.ExitCode = exitcode::UNAVAILABLE;
        app_state.State.BuildActions.Docker = BuildStatus::Error;
        app_state.State.ErrorMessage = "Please make sure Docker is installed! Go to https://www.docker.com/ and follow the instructions to download and install.".to_owned();
        window.emit("state", app_state.State.clone()).unwrap();
        return Ok(());
    }

    app_state.State.BuildActions.Docker = BuildStatus::Pass;
    app_state.State.BuildActions.DockerCompose = BuildStatus::Waiting;
    window.emit("state", app_state.State.clone()).unwrap();

    if is_docker_compose_installed().is_err() {
        app_state.State.ExitCode = exitcode::UNAVAILABLE;
        app_state.State.BuildActions.DockerCompose = BuildStatus::Error;
        app_state.State.ErrorMessage = "Please make sure Docker Compose is installed! Go to https://www.docker.com/ and follow the instructions to download and install.".to_owned();
        window.emit("state", app_state.State.clone()).unwrap();
        return Ok(());
    }

    app_state.State.BuildActions.DockerCompose = BuildStatus::Pass;
    app_state.State.BuildActions.UnifyLibrary = BuildStatus::Waiting;
    window.emit("state", app_state.State.clone()).unwrap();

    let unify_zip_exists = identify_unify_debian_packages();
    if unify_zip_exists.is_err() {
        let example_package = if cfg!(target_arch = "arm") {
            "unify_1.1.1_armhf.zip".to_string()
        } else {
            "unify_1.1.1_x86_64.zip".to_string()
        };
        let unify_release_url = "https://github.com/SiliconLabs/UnifySDK/releases";
        app_state.State.ExitCode = exitcode::NOINPUT;
        app_state.State.BuildActions.UnifyLibrary = BuildStatus::Error;
        app_state.State.ErrorMessage = format!("Could not find Unify zip file in the docker-files directory! You can find the Unify packages here: {}. Make sure to login to verify that you have access to the Unify SDK. Download the Zip package of Unify and place it in the 'docker-files' folder. Choose the one that matches your architecture - in your case {}.", unify_release_url, &example_package);
        window.emit("state", app_state.State.clone()).unwrap();
        return Ok(());
    }

    app_state.State.BuildActions.UnifyLibrary = BuildStatus::Pass;
    app_state.State.BuildActions.BuildDocker = BuildStatus::Waiting;
    window.emit("state", app_state.State.clone()).unwrap();

    if docker_compose_build(&unify_zip_exists.unwrap()).is_err() {
        app_state.State.ExitCode = exitcode::TEMPFAIL;
        app_state.State.BuildActions.BuildDocker = BuildStatus::Error;
        app_state.State.ErrorMessage = "Failed to run 'docker compose build'!".to_owned();
        window.emit("state", app_state.State.clone()).unwrap();
        return Ok(());
    }

    app_state.State.BuildActions.BuildDocker = BuildStatus::Pass;
    app_state.State.Status = Status::SetDevice;
    window.emit("state", app_state.State.clone()).unwrap();
    Ok(())
}

#[tauri::command]
async fn run(
    device: String,
    source_type: String,
    data_type: String,
    region: String,
    window: Window,
    state: tauri::State<'_, RwLock<TauriState>>,
) -> Result<(), tauri::Error> {
    println!("run unify params:\r\n data_type: {data_type}, name: {device}, region: {region}");

    let mut app_state = state.write().unwrap();
    app_state.State.Status = Status::Run;
    app_state.State.BuildActions.RunDocker = BuildStatus::Waiting;
    window.emit("state", app_state.State.clone()).unwrap();

    let zpc_device_string = if source_type == "Detected" {
        local_ipaddress::get().unwrap()
    } else {
        device.clone()
    };

    set_zpc_device(&zpc_device_string);
    let region_str = String::from("--zpc.rf_region ") + &region;
    set_zpc_config_args(&region_str);

    let serial = if source_type == "Detected" {
        device.clone()
    } else {
        String::from("")
    };
    let silink_handle = silink_automap(serial.as_str());

    if silink_handle.is_err() && !RUNNING_ON_ARM {
        app_state.State.ErrorMessage = "Failed to start Silink!".to_owned();
        app_state.State.ExitCode = exitcode::TEMPFAIL;
        app_state.State.BuildActions.RunDocker = BuildStatus::Error;
        window.emit("state", app_state.State.clone()).unwrap();
        return Ok(());
    }
    app_state.SilinkProcessId = silink_handle.unwrap().id();

    if docker_compose_up().is_err() {
        app_state.State.ErrorMessage = "Failed to start Portable Runtime".to_owned();
        app_state.State.ExitCode = exitcode::TEMPFAIL;
        app_state.State.BuildActions.RunDocker = BuildStatus::Error;
        window.emit("state", app_state.State.clone()).unwrap();
        return Ok(());
    }
    app_state.State.BuildActions.RunDocker = BuildStatus::Pass;
    app_state.State.Status = Status::Complete;
    window.emit("state", app_state.State.clone()).unwrap();
    Ok(())
}

#[tauri::command]
async fn exit(state: tauri::State<'_, RwLock<TauriState>>) -> Result<(), tauri::Error> {
    println!("exit");
    let tauri_state = state.read().unwrap();
    if tauri_state.SilinkProcessId != 0 {
        let pid = tauri_state.SilinkProcessId.to_string();
        kill_process(&pid.as_str());
        docker_compose_down().unwrap();
    }
    std::process::exit(tauri_state.State.ExitCode);
}

fn main() {
    if RUNNING_ON_ARM {
        eprintln!("Running on ARM/AARCH64 which is not a supported architecture just yet.");
        std::process::exit(exitcode::OSERR);
    }

    let mut tauri_state = TauriState {
        State: State {
            Status: Status::Init,
            BuildActions: BuildActions {
                Docker: BuildStatus::Init,
                DockerCompose: BuildStatus::Init,
                UnifyLibrary: BuildStatus::Init,
                BuildDocker: BuildStatus::Init,
                RunDocker: BuildStatus::Init,
            },
            ErrorMessage: String::new(),
            ExitCode: exitcode::OK,
        },
        SilinkProcessId: 0,
    };
    let busy_ports = get_busy_ports();
    if busy_ports.len() > 0 {
        tauri_state.State.ErrorMessage = format!("Another process is using port(s): {:?}. Please make sure to stop that service before trying to launch Unify.", busy_ports);
        tauri_state.State.ExitCode = exitcode::NOHOST;
    }
    init_log();
    tauri::Builder::default()
        .manage(RwLock::new(tauri_state))
        .invoke_handler(tauri::generate_handler![init, build, run, exit])
        .run(tauri::generate_context!())
        .expect("error while running Unify SDK application");
}
