#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

extern crate exitcode;
use exitcode::ExitCode;
use serde_repr::*;
use std::{sync::RwLock, thread};
use tauri::{Window, WindowEvent};
use unify_portable_core::*;

#[derive(Clone, Debug, PartialEq, Serialize_repr, serde::Deserialize)]
#[repr(u8)]
enum BuildStatus {
    Init = 0,
    Waiting = 1,
    Pass = 2,
}

#[derive(Clone, Debug, PartialEq, Serialize_repr, serde::Deserialize)]
#[repr(u8)]
enum Status {
    PreRequisitesCheck = 0,
    Protocol = 1,
    Framework = 2,
    Configuration = 3,
    Review = 4,
    Running = 5,
    Error = 6,
}

#[derive(Clone, serde::Serialize, serde::Deserialize, Debug)]
#[allow(non_snake_case)]
struct PreRequisitesCheck {
  Docker: BuildStatus,
  DockerCompose: BuildStatus,
  UnifyLibrary: BuildStatus,
}

#[derive(Clone, serde::Serialize, serde::Deserialize, Debug)]
#[allow(non_snake_case)]
struct State {
    Status: Status,
    ErrorMessage: String,
    ServiceList: String,
    ProtocolList: Vec<ProtocolItem>,
    PreRequisitesCheck: PreRequisitesCheck,
}

#[derive(Clone, serde::Serialize, serde::Deserialize, Debug)]
#[allow(non_snake_case)]
struct ProtocolItem {
    Protocol: String,
    Device: String,
    IsManual: bool,
}

#[derive(Clone)]
#[allow(non_snake_case)]
struct TauriState {
    State: State,
    SilinkProcessList: Vec<u32>,
    ExitCode: ExitCode,
}

const RUNNING_ON_ARM: bool = cfg!(any(target_arch = "arm", target_arch = "aarch64"));

#[tauri::command]
async fn init(
    window: Window,
    state: tauri::State<'_, RwLock<TauriState>>,
) -> Result<State, tauri::Error> {
    let mut app_state = state.write().unwrap();
    app_state.State.Status = Status::PreRequisitesCheck;
    app_state.State.PreRequisitesCheck.Docker = BuildStatus::Waiting;
    window.emit("req-check", app_state.State.PreRequisitesCheck.clone()).unwrap();

    if !is_docker_available() {
        app_state.ExitCode = exitcode::UNAVAILABLE;
        app_state.State.Status = Status::Error;
        app_state.State.ErrorMessage = "Please make sure Docker is installed! Go to https://www.docker.com/ and follow the instructions to download and install.".to_owned();
        window
            .emit("error", app_state.State.ErrorMessage.clone())
            .unwrap();
    }

    app_state.State.PreRequisitesCheck.Docker = BuildStatus::Pass;
    app_state.State.PreRequisitesCheck.DockerCompose = BuildStatus::Waiting;
    window.emit("req-check", app_state.State.PreRequisitesCheck.clone()).unwrap();

    if !is_docker_compose_installed() {
        app_state.ExitCode = exitcode::UNAVAILABLE;
        app_state.State.Status = Status::Error;
        app_state.State.ErrorMessage = "Please make sure Docker Compose is installed! Go to https://www.docker.com/ and follow the instructions to download and install.".to_owned();
        window
            .emit("error", app_state.State.ErrorMessage.clone())
            .unwrap();
    }

    app_state.State.PreRequisitesCheck.DockerCompose = BuildStatus::Pass;
    app_state.State.PreRequisitesCheck.UnifyLibrary = BuildStatus::Waiting;
    window.emit("req-check", app_state.State.PreRequisitesCheck.clone()).unwrap();

    let unify_zip_exists = identify_unify_debian_packages();
    if unify_zip_exists.is_err() {
        let example_package = if cfg!(target_arch = "arm") {
            "unify_1.1.1_armhf.zip".to_string()
        } else {
            "unify_1.1.1_x86_64.zip".to_string()
        };
        let unify_release_url = "https://github.com/SiliconLabs/UnifySDK/releases";
        app_state.ExitCode = exitcode::NOINPUT;
        app_state.State.Status = Status::Error;
        app_state.State.ErrorMessage = format!("Could not find Unify zip file in the docker-files directory! You can find the Unify packages here: {}. Make sure to login to verify that you have access to the Unify SDK. Download the Zip package of Unify and place it in the 'docker-files' folder. Choose the one that matches your architecture - in your case {}.", unify_release_url, &example_package);
        window
            .emit("error", app_state.State.ErrorMessage.clone())
            .unwrap();
    }

    app_state.State.PreRequisitesCheck.UnifyLibrary = BuildStatus::Pass;
    window.emit("req-check", app_state.State.PreRequisitesCheck.clone()).unwrap();

    return Ok(app_state.State.clone());
}

#[tauri::command]
async fn validate(
    service_list: String,
    protocol_list: Vec<ProtocolItem>,
    region: String,
    window: Window,
    state: tauri::State<'_, RwLock<TauriState>>,
) -> Result<Vec<(String, String)>, tauri::Error> {
    println!("validate configuration {:?}", protocol_list);
    let mut conflicts: Vec<(String, String)> = vec![];
    let mut app_state = state.write().unwrap();

    protocol_list.iter().for_each(|item| {
        let mut silink_sn = String::new();
        match item.Protocol.as_str() {
            "Z-Wave" => {
                if item.IsManual {
                    set_zpc_device(&item.Device);
                } else {
                    set_zpc_device(&get_host_ip_address());
                    silink_sn = item.Device.clone();
                }
                let region_str = String::from("--zpc.rf_region ") + &region;
                set_zpc_config_args(&region_str);
            }
            "AoX-Bluetooth" => {
                if item.IsManual {
                    set_aoxpc_device(&item.Device);
                    set_env_var(AOXPC_PORT_KEY, SILINK_DEFAULT_PORT);
                } else {
                    set_aoxpc_device(&get_host_ip_address());
                    silink_sn = item.Device.clone();
                    let port =
                        get_port(item.Protocol.clone(), window.clone(), app_state.to_owned());
                    set_env_var(AOXPC_PORT_KEY, port.as_str());
                }
            }
            "Zigbee-NCP" => {
                if item.IsManual {
                    set_zigpc_device(&item.Device);
                    set_env_var(ZIGPC_PORT_KEY, SILINK_DEFAULT_PORT);
                } else {
                    set_zigpc_device(&get_host_ip_address());
                    silink_sn = item.Device.clone();
                    let port =
                        get_port(item.Protocol.clone(), window.clone(), app_state.to_owned());
                    set_env_var(ZIGPC_PORT_KEY, port.as_str());
                }
            }
            "MultiProtocol" => {
                if item.IsManual {
                    set_multiprotocol_device(&item.Device);
                    set_env_var(MULTIPROTOCOL_PORT_KEY, SILINK_DEFAULT_PORT);
                } else {
                    set_multiprotocol_device(&get_host_ip_address());
                    silink_sn = item.Device.clone();
                    let port =
                        get_port(item.Protocol.clone(), window.clone(), app_state.to_owned());
                    set_env_var(MULTIPROTOCOL_PORT_KEY, port.as_str());
                }
            }
            _ => {
                app_state.State.Status = Status::Error;
                app_state.State.ErrorMessage = "Unsupported protocol".to_string();
                app_state.ExitCode = exitcode::TEMPFAIL;
                window
                    .emit("error", app_state.State.ErrorMessage.clone())
                    .unwrap();
            }
        };

        if silink_sn != "" {
            println!("Mapping device with Silink...");
            let silink_handle = silink_automap(&silink_sn, item.Protocol.as_str());
            if silink_handle.is_err() && !RUNNING_ON_ARM {
                app_state.State.Status = Status::Error;
                app_state.State.ErrorMessage = "Failed to start Silink!".to_owned();
                app_state.ExitCode = exitcode::TEMPFAIL;
                window
                    .emit("error", app_state.State.ErrorMessage.clone())
                    .unwrap();
            }

            // To store mapped devices to silink automapped devices Vector.
            app_state
                .SilinkProcessList
                .push(silink_handle.unwrap().id());
        }
    });

    app_state.State.ServiceList = service_list.clone();
    app_state.State.ProtocolList = protocol_list.clone();
    let results = validate_configuration(service_list.as_str());
    results
        .iter()
        .for_each(|x| conflicts.push((x.0.to_string(), x.1.to_string())));
    println!("Conflicts: {:?}", conflicts);
    return Ok(conflicts);
}

#[tauri::command]
async fn discover(window: Window) -> Result<String, tauri::Error> {
    let mut list = String::from("");
    if silink_get_device_count().unwrap() > 0 {
        match silink_inspect() {
            Ok(device_list) => {
                list = String::from(&device_list);
            }
            Err(error) => {
                window.emit("error", String::from(error)).unwrap();
            }
        }
    }
    return Ok(list);
}

#[tauri::command]
async fn run(
    window: Window,
    state: tauri::State<'_, RwLock<TauriState>>,
) -> Result<(), tauri::Error> {
    let mut app_state = state.write().unwrap();
    app_state.State.Status = Status::Running;

    let unify_zip_exists = identify_unify_debian_packages();

    if docker_compose_build(&unify_zip_exists.unwrap()).is_err() {
        app_state.State.Status = Status::Error;
        app_state.ExitCode = exitcode::TEMPFAIL;
        app_state.State.ErrorMessage = "Failed to run 'docker compose build'!".to_owned();
        window
            .emit("error", app_state.State.ErrorMessage.clone())
            .unwrap();
        return Ok(());
    }

    app_state.State.Status = Status::Review;

    if docker_compose_up(&app_state.State.ServiceList).is_err() {
        app_state.State.Status = Status::Error;
        app_state.State.ErrorMessage = "Failed to start Portable Runtime".to_owned();
        app_state.ExitCode = exitcode::TEMPFAIL;
        window
            .emit("error", app_state.State.ErrorMessage.clone())
            .unwrap();
        return Ok(());
    }

    thread::spawn(move || loop {
        match docker_retriev_status() {
            Ok(status) => {
                // Converting the result string into array of objects
                // ex: converts result from {"Names":"name","State":"running","Status":"Up time"} {"Names":"name","State":"stopped","Status":"Up time"}
                // to array of objects [{"Names":"name","State":"running","Status":"Up time"}, {"Names":"name","State":"stopped","Status":"Up time"}]
                let result = status.split('}').map(|s| s.trim()).collect::<Vec<&str>>().join("},\n");
                let json_array = format!("[{}]", result.trim_end_matches(",\n"));
                window.emit("status", String::from(json_array)).unwrap();
            }
            Err(error) => {
                window.emit("error", String::from(error)).unwrap();
            }
        }
        thread::sleep(std::time::Duration::from_secs(10));
    });

    println!("Application Started...");

    Ok(())
}

#[tauri::command]
async fn exit(
    window: Window,
    state: tauri::State<'_, RwLock<TauriState>>,
) -> Result<(), tauri::Error> {
    let app_state = state.write().unwrap();
    _exit(window, &app_state.to_owned());
    std::process::exit(app_state.ExitCode);
}

fn _exit(window: Window, state: &TauriState) {
    println!("exit code: {}", state.ExitCode);
    let _ = window.hide();
    if state.SilinkProcessList.len() > 0 {
        for process in state.SilinkProcessList.iter() {
            kill_process(process.to_string().as_str());
        }
    }
    docker_compose_down().unwrap();
}

fn get_port(protocol: String, window: Window, mut app_state: TauriState) -> String {
    let port = match PORTS.get(protocol.as_str()) {
        Some(&p) => p,
        None => {
            app_state.State.Status = Status::Error;
            app_state.State.ErrorMessage =
                format!("Failed to start Silink! Unsupported {protocol} protocol.");
            app_state.ExitCode = exitcode::TEMPFAIL;
            window
                .emit("error", app_state.State.ErrorMessage.clone())
                .unwrap();
            "Unsupported protocol"
        }
    };
    (port.parse::<i32>().unwrap() + 1).to_string()
}

fn main() {
    if RUNNING_ON_ARM {
        eprintln!("Running on ARM/AARCH64 which is not a supported architecture just yet.");
        std::process::exit(exitcode::OSERR);
    }

    let mut tauri_state = TauriState {
        State: State {
            Status: Status::PreRequisitesCheck,
            PreRequisitesCheck: PreRequisitesCheck {
              Docker: BuildStatus::Init,
              DockerCompose: BuildStatus::Init,
              UnifyLibrary: BuildStatus::Init,
            },
            ErrorMessage: String::new(),
            ServiceList: String::from(""),
            ProtocolList: vec![],
        },
        ExitCode: exitcode::OK,
        SilinkProcessList: vec![],
    };
    let busy_ports = get_busy_ports();
    if busy_ports.len() > 0 {
        tauri_state.State.Status = Status::Error;
        tauri_state.State.ErrorMessage = format!("Another process is using port(s): {:?}. Please make sure to stop that service before trying to launch Unify.", busy_ports);
        tauri_state.ExitCode = exitcode::NOHOST;
    }
    init_log();
    let state = tauri_state.clone();
    tauri::Builder::default()
        .manage(RwLock::new(tauri_state))
        .on_window_event(move |event| match event.event() {
            WindowEvent::CloseRequested { .. } => {
                let _ = _exit(event.window().clone(), &state);
            }
            _ => {}
        })
        .invoke_handler(tauri::generate_handler![
            init, discover, validate, run, exit
        ])
        .run(tauri::generate_context!())
        .expect("error while running Unify SDK application");
}
