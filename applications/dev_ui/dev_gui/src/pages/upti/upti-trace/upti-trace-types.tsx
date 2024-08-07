import { UPTI, UPTIItem } from "../upti-list/upti-list-types"

export type UPTITraceProps = {
    IsConnected: boolean;
    SocketServer: WebSocket;
    SerialNumber: string;
    UPTI: UPTI;
}

export type UPTITraceState = {
    SelectedPTI: UPTIItem | undefined;
}

export type TraceItem = {
    TimeStamp: string;
    AdditionalData: AdditionalData,
    RawFrame: any
}

export type AdditionalData = {
    Mode: string,
    Protocol: number;
    RadioConfig: number,
    Rssi: number;
    Channel: number;
    Speed: number;
    Syncword: any,
    Version: number
    ErrorCode: number
    Payload: any;
}

export enum WakeUpBeamTypes {
    Impulse,
    Start,
    Stop,
}

export enum Frequencies {
    None,
    EU,
    US,
    AU_NZ,
    HK,
    MY,
    IN,
    JP,
    RU,
    IL,
    KR,
    CN,
    US_LR1,
    US_LR2,
    US_LR3,
    EU_LR1,
    EU_LR2,
    EU_LR3
}

export enum Modes {
    None = 0x00,
    RxSuccess = 0xF9,
    RxAbort = 0xFA,
    TxSuccess = 0xFD,
    TxAbort = 0xFE
}

export enum Speed {
    "9.6K",
    "40K",
    "100K",
    "100K LR"
}