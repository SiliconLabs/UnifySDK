
# unify_node_state_monitor

The role of the node state monitor is to subscribe to the
ucl/by-unid/<unid>/State topic. It will check when nodes a are changing state,
and create UnifyNode object, populated with Endpoints, state and security
information. When a new node is found / deleted or updated a callback is fired
containing the node object. The node state monitor should only fire a callback
once all information has been collected.


# matter_node_state_monitor

This class will, when unify devices are created / deleted or updated generate a
corresponding matter device and assign the Unify endpoint a matter endpoint id.
The Device translator will keep a translation table between unify between a
matter endpoint identifier.

The Device translator has as a scheme for translating a list for supported attributes
and clusters to a matter device type. It will fill in a structure as follows:

EmberAfEndpointType
- EmberAfCluster[]
-- EmberAfAttributeMetadata[]

And register this with emberAfSetDynamicEndpoint.

# matter_cluster_translator

The purpose of the cluster translator is to handle read/write attribute calls as
well as matter commands. Since the matter data model and the UIC data model are
very similar we will generally be able to do a 1-1 translation.

The translator has a ZAP generated handler, which implements a generic handler
for all clusters which is able to send translate Matter commands into mqtt
messages. The translator uses `InteractionModelEngine::RegisterCommandHandler`
to register itself with the matter application framework. As the default
behavior the translator will directly translate the matter command into a unify
mqtt command.

The translator will also handle attribute read and attribute writes. For
attributes the translator uses the system `registerAttributeAccessOverride, when
an attribute read is requested the Unify Reported value should be reported when
an attribute write is requested the corresponding /WriteAttribute command is
published on the mqtt side.

The Unify Cluster translator is not required to check the capabilities of a node
before sending WriteAttributes or other commands.

The cluster translator must have a feature which allows for overriding the ZAP 
generated handling.

## Special Clusters

There are some special clusters which may need a custom implementation, mostly
due to changes in the data model structure. The dotdot data models 

- The group cluster need special handling.
- The scene cluster also needs special handling. 
- The binding cluster

