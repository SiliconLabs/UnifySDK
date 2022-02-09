export type EditableAttributeState = {
    ShowModal: boolean;
    Value: string;
}

export type EditableAttributeProps = {
    SocketServer: WebSocket;
    Unid: string;
    Cluster: any;
    ClusterName: string;
    FieldName: string;
    ReplaceNameWithUnid: boolean;
}