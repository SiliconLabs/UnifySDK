export type PreferredProtocolsDlgState = {
    SmartStartItem: any,
    ShowModal: boolean,
    PreferredProtocols: Protocol[]
}

export type PreferredProtocolsDlgProps = {
    SaveFunc: any
}

export type Protocol = {
    Name: string,
    Enabled: boolean
}