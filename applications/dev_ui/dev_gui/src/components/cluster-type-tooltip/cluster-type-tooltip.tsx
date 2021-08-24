import { Tooltip } from '@material-ui/core';
import * as React from 'react';
import { NodeTypesList } from '../../cluster-types/cluster-types';
import { ClusterViewOverrides } from '../../pages/base-clusters/cluster-view-overrides';
import { ClusterTypeTooltipProps } from './cluster-type-tooltip-types';
import * as GrIcons from 'react-icons/gr';


class ClusterTypeTooltip extends React.Component<ClusterTypeTooltipProps, {}> {
    getTooltip(type: any) {
        if (type === NodeTypesList.ProtocolController)
            return <Tooltip title="Protocol Controller"><span className="icon cursor-defult"><GrIcons.GrNodes /></span></Tooltip>
        return (ClusterViewOverrides as any)[type]?.NodesTooltip || (NodeTypesList as any)[type] || "Unknown";
    }

    render() {
        return (
            this.props.ClusterTypes.map((i: any, index: number) => { return <div key={index} className="col-md-12"> {this.getTooltip(i)}</div> })
        );
    }
}

export default ClusterTypeTooltip