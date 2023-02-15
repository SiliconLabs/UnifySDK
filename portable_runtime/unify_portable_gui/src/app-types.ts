export enum Status {
    Init,
    Build,
    SetDevice,
    Run,
    Complete
}

export enum BuildStatus {
    Init,
    Waiting,
    Pass,
    Error
}

export enum SourceType {
    Detected,
    Manual
}

export type BuildActions = {
    Docker: BuildStatus,
    DockerCompose: BuildStatus,
    UnifyLibrary: BuildStatus,
    BuildDocker: BuildStatus,
    RunDocker: BuildStatus
}

export type AppState = {
    ApplicationState: ApplicationState,
    TargetDevice: string,
    DeviceList: string[],
    SourceType: SourceType,
    Region: string,
    DataType: string
}

export type ApplicationState = {
    Status: Status,
    BuildActions: BuildActions,
    ErrorMessage: string
}