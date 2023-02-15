import * as React from 'react';
import { Button } from 'react-bootstrap';
import { MenuItem, TextField } from '@mui/material';
import { AddAttributeProps, AddAttributeState } from './add-attribute-types';
import { ClusterTypeAttrs } from '../../cluster-types/cluster-type-attributes';

class AddAttribute extends React.Component<AddAttributeProps, AddAttributeState> {
    constructor(props: AddAttributeProps) {
        super(props);
        let attrs = this.getAttrList(this.props.Cluster);
        this.state = {
            Attr: !attrs || !attrs.length ? "" : attrs[0],
            GetAttrList: this.getAttrList
        };
    }

    static getDerivedStateFromProps(props: AddAttributeProps, state: AddAttributeState) {
        let attrs = state.GetAttrList(props.Cluster);
        return {
            Attr: state.Attr && attrs.indexOf(state.Attr) > -1 ? state.Attr : (!attrs || !attrs.length ? "" : attrs[0])
        };
    }

    getAttrList = (cluster: string) => {
        return ClusterTypeAttrs[cluster]?.server?.attributes?.filter((i: any) => i.sceneRequired === true && (!this.props.AttrList || !Object.keys(this.props.AttrList).find((attr: string) => attr === i.name)))
    }

    render() {
        let attrs = this.getAttrList(this.props.Cluster);
        return attrs && attrs.length ? <>
            <Button variant="primary" className="float-right mr-3" size='sm' disabled={!(this.state.Attr && Object.keys(this.state.Attr).length)} onClick={() => this.props.AddAttributeFunc(this.props.Cluster, this.state.Attr)}>Add Attribute</Button>
            <TextField size='small' className="col-sm-4 float-right mr-3 text-field-small" fullWidth={true} select label="Attribute" name="Attr" value={this.state.Attr} onChange={(event: any) => this.setState({ Attr: event?.target?.value })} variant="outlined">
                {attrs.map((attr: any, index: number) => {
                    return <MenuItem key={index} value={attr}>
                        {attr.name}
                    </MenuItem>
                })}
            </TextField>
        </>
            : <></>
    }
}

export default AddAttribute