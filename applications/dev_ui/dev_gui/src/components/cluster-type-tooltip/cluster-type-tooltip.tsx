import { Tooltip } from '@material-ui/core';
import * as React from 'react';
import { NodeTypesList } from '../../cluster-types/cluster-types';
import { ClusterViewOverrides } from '../../pages/base-clusters/cluster-view-overrides';
import { ClusterTypeTooltipProps } from './cluster-type-tooltip-types';
import * as GrIcons from 'react-icons/gr';


class ClusterTypeTooltip extends React.Component<ClusterTypeTooltipProps, {}> {
    getTooltip(endpoint: string, type: string, attr: any) {
        if (type === NodeTypesList.ProtocolController)
            return <Tooltip title="Protocol Controller" ><span className="icon cursor-default"><GrIcons.GrNodes /></span></Tooltip>
        return (ClusterViewOverrides as any)[type]?.NodesTooltip(endpoint, attr) || (NodeTypesList as any)[type] || "Unknown";
    }

    render() {
        let eps = this.props.Ep && Object.keys(this.props.Ep);
        if (eps && eps.length) {
            return eps.map((ep: any, index: number) => {
                let clusters = this.props.Ep[ep].Clusters && Object.keys(this.props.Ep[ep].Clusters);
                if (clusters && clusters.length)
                    return clusters.map((cluster: any, clIndex: number) => {
                        return <div key={`${index}.${clIndex}`}> {this.getTooltip(ep, cluster, this.props.Ep[ep].Clusters[cluster].Attributes)}</div>
                    })
                else return (<div key={index}></div>)
            })
        } else {
            return (<></>)
        }
    }
}

export default ClusterTypeTooltip