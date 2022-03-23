export type EditableAttributeState = {
    ShowModal: boolean;
    Value: string;
    IsAssignToAll: boolean;
}

export type EditableAttributeProps = {
    SocketServer: WebSocket;
    Node: any;
    EpName: string;
    Cluster: any;
    ClusterName: string;
    FieldName: string;
    ReplaceNameWithUnid: boolean;
    GetOptions?: any;
    Disabled: boolean;
}