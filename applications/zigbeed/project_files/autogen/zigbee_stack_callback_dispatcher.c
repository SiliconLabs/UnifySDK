/*****************************************************************************/
/**
 * Copyright 2020 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// *** Generated file. Do not edit! ***
//

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "zigbee_stack_callback_dispatcher.h"



#if defined(EMBER_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)
// Incoming Message
bool sli_zb_af_support_incoming_message(
      // The aps Frame
      EmberApsFrame *apsFrame)
{
(void) apsFrame;
return (
false
);
}
#endif // defined(EMBER_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT)


#ifdef EZSP_HOST
// Incoming Message
void sli_zb_af_incoming_message(
      // Incoming message type
      EmberIncomingMessageType type,
      // The aps Frame
      EmberApsFrame *apsFrame,
      // The link quality
      uint8_t lastHopLqi,
      // The energy level
      int8_t lastHopRssi,
      // The sender of the message.
      EmberNodeId sender,
      // The index of a binding that matches the message
      // matching binding.
      uint8_t bindingIndex,
      // The index of the entry in the address table
      uint8_t addressIndex,
      // The message length
      uint8_t messageLength,
      // message
      uint8_t *messageContents)
{
(void) type;
(void) apsFrame;
(void) lastHopLqi;
(void) lastHopRssi;
(void) sender;
(void) bindingIndex;
(void) addressIndex;
(void) messageLength;
(void) messageContents;

sli_zigbee_af_incoming_message_callback(type, apsFrame, lastHopLqi, lastHopRssi, sender, bindingIndex, addressIndex, messageLength, messageContents);
}
#else // !EZSP_HOST
// Incoming Message
void sli_zb_af_incoming_message(
      // Incoming message type
      EmberIncomingMessageType type,
      // The aps Frame
      EmberApsFrame *apsFrame,
      // message
      EmberMessageBuffer message)
{
(void)type;
(void)apsFrame;
(void)message;

sli_zigbee_af_incoming_message_callback(type, apsFrame, message);
}
#endif // EZSP_HOST


// Message Sent
#ifdef EZSP_HOST
void sli_zigbee_af_message_sent(
      // Outgoing message type
      EmberOutgoingMessageType type,
      // Index or destination of the message
      uint16_t indexOrDestination,
      // The aps Frame
      EmberApsFrame *apsFrame,
      // Message tag
      uint8_t messageTag,
      // Message delivery status
      EmberStatus status,
      // The length of the messageContents
      uint8_t messageLength,
      // Message data
      uint8_t *messageContents)
{
(void)type;
(void)indexOrDestination;
(void)apsFrame;
(void)messageTag;
(void)status;
(void) messageLength;
(void) messageContents;

sli_zigbee_af_message_sent_callback(type, indexOrDestination, apsFrame, messageTag, status, messageLength, messageContents);
}
#else // !EZSP_HOST
void sli_zigbee_af_message_sent(
      // Outgoing message type
      EmberOutgoingMessageType type,
      // Index or destination of the message
      uint16_t indexOrDestination,
      // The aps Frame
      EmberApsFrame *apsFrame,
      // message
      EmberMessageBuffer message,
      // Message delivery status
      EmberStatus status)
{
(void)type;
(void)indexOrDestination;
(void)apsFrame;
(void)message;
(void)status;

sli_zigbee_af_message_sent_callback(type, indexOrDestination, apsFrame, message, status);
}
#endif // EZSP_HOST



// Trust Center Join
#ifdef EZSP_HOST
void sli_zigbee_af_trust_center_join(
      // Joining node's id
      EmberNodeId newNodeId,
      // Joining node's Eui64
      EmberEUI64 newNodeEui64,
      // Status
      EmberDeviceUpdate status,
      // Policy Decision
      EmberJoinDecision policyDecision,
      // Parent Id of the joining node
      EmberNodeId parentOfNewNode)
{
(void)newNodeId;
(void)newNodeEui64;
(void)status;
(void)policyDecision;
(void)parentOfNewNode;

}
#else // !EZSP_HOST
void sli_zigbee_af_trust_center_join(
      // Joining node's id
      EmberNodeId newNodeId,
      // Joining node's Eui64
      EmberEUI64 newNodeEui64,
      // Status
      EmberDeviceUpdate status,
      // Parent Id of the joining node
      EmberNodeId parentOfNewNode)
{
(void)newNodeId;
(void)newNodeEui64;
(void)status;
(void)parentOfNewNode;

}
#endif // EZSP_HOST


// Mark Buffers
void sli_zigbee_af_mark_buffers(void)
{

sli_zigbee_af_mark_buffers_callback();
}



// Packet Handoff Incoming
void sli_zigbee_af_packet_handoff_incoming(
      EmberZigbeePacketType packetType,
      EmberMessageBuffer packetBuffer,
      uint8_t index,
      // Return: 
      void *data)
{
(void)packetType;
(void)packetBuffer;
(void)index;
(void)data;

}



// Packet Handoff Outgoing
void sli_zigbee_af_packet_handoff_outgoing(
      EmberZigbeePacketType packetType,
      EmberMessageBuffer packetBuffer,
      uint8_t index,
      // Return: 
      void *data)
{
(void)packetType;
(void)packetBuffer;
(void)index;
(void)data;

}



// Incoming Mfg Test Message
void sli_zigbee_af_incoming_mfg_test_message(
      uint8_t messageType,
      uint8_t dataLength,
      // Return: 
      uint8_t *data)
{
(void)messageType;
(void)dataLength;
(void)data;

}



// Override Incoming Route Record
void sli_zigbee_af_override_incoming_route_record(
      EmberNodeId source,
      EmberEUI64 sourceEui,
      uint8_t relayCount,
      EmberMessageBuffer header,
      uint8_t relayListIndex,
      // Return: 
      bool *consumed)
{
(void)source;
(void)sourceEui;
(void)relayCount;
(void)header;
(void)relayListIndex;
(void)consumed;

sli_zigbee_af_override_incoming_route_record_callback(source, sourceEui, relayCount, header, relayListIndex, consumed);
}



// Override Append Source Route
// Return: Added bytes
void sli_zigbee_af_override_append_source_route(
      EmberNodeId destination,
      // Return: 
      EmberMessageBuffer *header,
      // Return: 
      bool *consumed)
{
(void)destination;
(void)header;
(void)consumed;

}



// Override Furthur Index For Source Route Add Entry
void sli_zigbee_af_override_furthur_index_for_source_route_add_entry(
      EmberNodeId id,
      // Return: 
      uint8_t *furtherIndex)
{
(void)id;
(void)furtherIndex;

sli_zigbee_af_override_furthur_index_for_source_route_add_entry_callback(id, furtherIndex);
}



// Stack Status
void sli_zigbee_af_stack_status(
      // Stack status
      EmberStatus status)
{
(void)status;

sli_zigbee_af_stack_status_callback(status);
}


// Redirect Outgoing Message
void sli_zigbee_af_redirect_outgoing_message(
      // The mac index used to redirect messages
      uint8_t mac_index,
      // Packet buffer header
      EmberMessageBuffer header,
      // Transmit priority
      uint8_t priority)
{
(void)mac_index;
(void)header;
(void)priority;

}



// Energy Scan Result
void sli_zigbee_af_energy_scan_result(
      // The 802.15.4 channel number that was scanned.
      uint8_t channel,
      // The maximum RSSI value found on the channel.
      int8_t maxRssiValue)
{
(void)channel;
(void)maxRssiValue;

sli_zigbee_af_energy_scan_result_callback(channel, maxRssiValue);
}



// Network Found
void sli_zigbee_af_network_found(
      // The parameters associated with the network found.
      EmberZigbeeNetwork *networkFound,
      // The link quality indication of the network found.
      uint8_t lqi,
      // The received signal strength indication of the network found.
      int8_t rssi)
{
(void)networkFound;
(void)lqi;
(void)rssi;

sli_zigbee_af_network_found_callback(networkFound, lqi, rssi);
}



// Scan Complete
void sli_zigbee_af_scan_complete(
      // The channel on which the current error occurred. Undefined for the case
      // of EMBER_SUCCESS.
      uint8_t channel,
      // The error condition that occurred on the current channel. Value will be
      // EMBER_SUCCESS when the scan has completed.
      EmberStatus status)
{
(void)channel;
(void)status;

sli_zigbee_af_scan_complete_callback(channel, status);
}



// Unused Pan Id Found
void sli_zigbee_af_unused_pan_id_found(
      // The unused panID which has been found.
      EmberPanId panId,
      // The channel that the unused panID was found on.
      uint8_t channel)
{
(void)panId;
(void)channel;

sli_zigbee_af_unused_pan_id_found_callback(panId, channel);
}



// Child Join
#ifdef EZSP_HOST
void sli_zigbee_af_child_join(
      // The index of the child of interest.
      uint8_t index,
      // True if the child is joining. False the child is leaving.
      bool joining,
      // The node ID of the child.
      EmberNodeId childId,
      // The EUI64 of the child.
      EmberEUI64 childEui64,
      // The node type of the child.
      EmberNodeType childType)
{
(void)index;
(void)joining;
(void)childId;
(void)childEui64;
(void)childType;

sli_zigbee_af_child_joinCallback(index, joining, childId, childEui64, childType);
}
#else // !EZSP_HOST
void sli_zigbee_af_child_join(
      // The index of the child of interest.
      uint8_t index,
      // True if the child is joining. False the child is leaving.
      bool joining)
{
(void)index;
(void)joining;

sli_zigbee_af_child_joinCallback(index, joining);
}
#endif // EZSP_HOST



// Duty Cycle
#ifdef EZSP_HOST
void sli_zigbee_af_duty_cycle(
      // The channel page whose duty cycle state has changed.
      uint8_t channelPage,
      // The channel number whose duty cycle state has changed.
      uint8_t channel,
      // The current duty cycle state.
      EmberDutyCycleState state,
      // The total number of connected end devices that are being monitored for
      // duty cycle.
      uint8_t totalDevices,
      // Consumed duty cycles of end devices that are being monitored. The first
      // entry always be the local stack's nodeId, and thus the total aggregate
      // duty cycle for the device.
      EmberPerDeviceDutyCycle *arrayOfDeviceDutyCycles)
{
(void)channelPage;
(void)channel;
(void)state;
(void)totalDevices;
(void)arrayOfDeviceDutyCycles;

sli_zigbee_af_duty_cycle_callback(channelPage, channel, state, totalDevices, arrayOfDeviceDutyCycles);
}
#else // !EZSP_HOST
void sli_zigbee_af_duty_cycle(
      // The channel page whose duty cycle state has changed.
      uint8_t channelPage,
      // The channel number whose duty cycle state has changed.
      uint8_t channel,
      // The current duty cycle state.
      EmberDutyCycleState state)
{
(void)channelPage;
(void)channel;
(void)state;

sli_zigbee_af_duty_cycle_callback(channelPage, channel, state);
}
#endif // EZSP_HOST



// Remote Set Binding
// Return: ZDO response status.
void sli_zigbee_af_remote_set_binding(
      // Return: The contents of the binding entry.
      EmberBindingTableEntry *entry)
{
(void)entry;

}



// Remote Delete Binding
// Return: ZDO response status
void sli_zigbee_af_remote_delete_binding(
      // The index of the binding whose deletion was requested.
      uint8_t index)
{
(void)index;

}



// Poll Complete
void sli_zigbee_af_poll_complete(
      // An EmberStatus value: EMBER_SUCCESS - Data was received in response to
      // the poll. EMBER_MAC_NO_DATA - No data was pending.
      // EMBER_DELIVERY_FAILED - The poll message could not be sent.
      // EMBER_MAC_NO_ACK_RECEIVED - The poll message was sent but not
      // acknowledged by the parent.
      EmberStatus status)
{
(void)status;

sli_zigbee_af_poll_complete_callback(status);
}



// Poll
void sli_zigbee_af_poll(
      // The node ID of the child that is requesting data.
      EmberNodeId childId,
      // True if transmit expected, false otherwise.
      bool transmitExpected)
{
(void)childId;
(void)transmitExpected;

sli_zigbee_af_poll_callback(childId, transmitExpected);
}



#ifdef EZSP_HOST
// Debug
void sli_zigbee_af_debug(
      // debug message length
      uint8_t messageLength,
      // debug message
      uint8_t *messageContents)
{
(void)messageLength;
(void)messageContents;

}
#else // !EZSP_HOST
// Debug
void sli_zigbee_af_debug(
      // debug message
      EmberMessageBuffer message)
{
(void)message;

}
#endif // EZSP_HOST



// Incoming Many To One Route Request
void sli_zigbee_af_incoming_many_to_one_route_request(
      // The short id of the concentrator.
      EmberNodeId source,
      // The EUI64 of the concentrator.
      EmberEUI64 longId,
      // The path cost to the concentrator. The cost may decrease as additional
      // route request packets for this discovery arrive, but the callback is
      // made only once.
      uint8_t cost)
{
(void)source;
(void)longId;
(void)cost;

sli_zigbee_af_incoming_many_to_one_route_requestCallback(source, longId, cost);
}



// Incoming Route Error
void sli_zigbee_af_incoming_route_error(
      // EMBER_SOURCE_ROUTE_FAILURE or EMBER_MANY_TO_ONE_ROUTE_FAILURE.
      EmberStatus status,
      // The short id of the remote node.
      EmberNodeId target)
{
(void)status;
(void)target;

sli_zigbee_af_incoming_route_error_callback(status, target);
}



// Incoming Network Status
void sli_zigbee_af_incoming_network_status(
      // One byte over-the-air error code from network status message
      uint8_t errorCode,
      // The short ID of the remote node
      EmberNodeId target)
{
(void)errorCode;
(void)target;

sli_zigbee_af_incoming_network_status_callback(errorCode, target);
}



// Incoming Route Record
void sli_zigbee_af_incoming_route_record(
      // The source of the route record.
      EmberNodeId source,
      // The EUI64 of the source.
      EmberEUI64 sourceEui,
      // The number of relays in relayList.
      uint8_t relayCount,
      // Header.
      EmberMessageBuffer header,
      // The relay List Index.
      uint8_t relayListIndex)
{
(void)source;
(void)sourceEui;
(void)relayCount;
(void)header;
(void)relayListIndex;

sli_zigbee_af_incoming_route_record_callback(source, sourceEui, relayCount, header, relayListIndex);
}



// Id Conflict
void sli_zigbee_af_id_conflict(
      // The short id for which a conflict was detected
      EmberNodeId conflictingId)
{
(void)conflictingId;

sli_zigbee_af_id_conflict_callback(conflictingId);
}



#ifdef EZSP_HOST
// Mac Passthrough Message
void sli_zigbee_af_mac_passthrough_message(
      // The type of MAC passthrough message received.
      EmberMacPassthroughType messageType,
      // last hop lqi.
      uint8_t lastHopLqi,
      // last hop rssi.
      int8_t lastHopRssi,
      // message length.
      uint8_t messageLength,
      // The raw message that was received.
      uint8_t *messageContents)
{
(void)messageType;
(void)lastHopLqi;
(void)lastHopRssi;
(void)messageLength;
(void)messageContents;

sli_zigbee_af_mac_passthrough_message_callback(messageType, lastHopLqi, lastHopRssi, messageLength, messageContents);
}
#else // !EZSP_HOST
// Mac Passthrough Message
void sli_zigbee_af_mac_passthrough_message(
      // The type of MAC passthrough message received.
      EmberMacPassthroughType messageType,
      // The raw message that was received.
      EmberMessageBuffer message)
{
(void)messageType;
(void)message;

sli_zigbee_af_mac_passthrough_message_callback(messageType, message);
}
#endif // EZSP_HOST



// Stack Token Changed
void sli_zigbee_af_stack_token_changed(
      // The address of the stack token that has changed.
      uint16_t tokenAddress)
{
(void)tokenAddress;

sli_zigbee_af_stack_token_changed_callback(tokenAddress);
}



// Timer
void sli_zigbee_af_timer(
      // Which timer generated the callback (0 or 1).
      uint8_t timerId)
{
(void)timerId;

}



// Counter Rollover
void sli_zigbee_af_counterRollover(
      // Type of Counter
      EmberCounterType type)
{
(void)type;

sli_zigbee_af_counter_rollover_callback(type);
}



// Raw Transmit Complete
void sli_zigbee_af_raw_transmit_complete(
      // message
      EmberMessageBuffer message,
      // EMBER_SUCCESS if the transmission was successful, or
      // EMBER_DELIVERY_FAILED if not
      EmberStatus status)
{
(void)message;
(void)status;

sli_zigbee_af_raw_transmit_complete_callback(message, status);
}



// Switch Network Key
void sli_zigbee_af_switch_network_key(
      // The sequence number of the new network key.
      uint8_t sequenceNumber)
{
(void)sequenceNumber;

sli_zigbee_af_switch_network_key_callback(sequenceNumber);
}



// Zigbee Key Establishment
void sli_zigbee_af_zigbee_key_establishment(
      // This is the IEEE address of the partner that the device successfully
      // established a key with. This value is all zeros on a failure.
      EmberEUI64 partner,
      // This is the status indicating what was established or why the key
      // establishment failed.
      EmberKeyStatus status)
{
(void)partner;
(void)status;

sli_zigbee_af_zigbee_key_establishment_callback(partner, status);
}



// Generate Cbke Keys
void sli_zigbee_af_generate_cbke_keys(
      // The result of the CBKE operation.
      EmberStatus status,
      // Return: The generated ephemeral public key.
      EmberPublicKeyData *ephemeralPublicKey)
{
(void)status;
(void)ephemeralPublicKey;

}



// Calculate Smacs
void sli_zigbee_af_calculate_smacs(
      // The Result of the CBKE operation.
      EmberStatus status,
      // Return: The calculated value of the initiator's SMAC
      EmberSmacData *initiatorSmac,
      // Return: The calculated value of the responder's SMAC
      EmberSmacData *responderSmac)
{
(void)status;
(void)initiatorSmac;
(void)responderSmac;

}



#ifdef EZSP_HOST
// Dsa Sign
void sli_zigbee_af_dsa_sign(
      // The result of the DSA signing operation.
      EmberStatus status,
      // Message length
      uint8_t messageLength,
      // The message that includes the original message and the
      // appended signature.
      uint8_t *message)
{
(void)status;
(void)messageLength;
(void)message;

}
#else // !EZSP_HOST
// Dsa Sign
void sli_zigbee_af_dsa_sign(
      // The result of the DSA signing operation.
      EmberStatus status,
      // The message and attached which includes the original message and the
      // appended signature.
      EmberMessageBuffer signedMessage)
{
(void)status;
(void)signedMessage;

}
#endif // EZSP_HOST



// Dsa Verify
void sli_zigbee_af_dsa_verify(
      // The result of the DSA verification operation.
      EmberStatus status)
{
(void)status;

}



#ifdef EZSP_HOST
// Incoming Bootload Message
void sli_zigbee_af_incoming_bootload_message(
      // The EUI64 of the sending node.
      EmberEUI64 longId,
      // last hop lqi.
      uint8_t lastHopLqi,
      // last hop rssi.
      int8_t lastHopRssi,
      // message length.
      uint8_t messageLength,
      // The bootload message that was sent.
      uint8_t *messageContents)
{
(void)longId;
(void)lastHopLqi;
(void)lastHopRssi;
(void)messageLength;
(void)messageContents;

sli_zigbee_af_incoming_bootload_message_callback(longId, lastHopLqi, lastHopRssi, messageLength, messageContents);
}
#else // !EZSP_HOST
// Incoming Bootload Message
void sli_zigbee_af_incoming_bootload_message(
      // The EUI64 of the sending node.
      EmberEUI64 longId,
      // The bootload message that was sent.
      EmberMessageBuffer message)
{
(void)longId;
(void)message;

sli_zigbee_af_incoming_bootload_message_callback(longId, message);
}
#endif // EZSP_HOST



#ifdef EZSP_HOST
// Bootload Transmit Complete
void sli_zigbee_af_bootload_transmit_complete(
      // An EmberStatus value of EMBER_SUCCESS if an ACK was received from the
      // destination or EMBER_DELIVERY_FAILED if no ACK was received.
      EmberStatus status,
      // message length.
      uint8_t messageLength,
      // The bootload message that was sent.
      uint8_t *messageContents)
{
(void)status;
(void)messageLength;
(void)messageContents;

sli_zigbee_af_bootload_transmit_complete_callback(status, messageLength, messageContents);
}
#else // !EZSP_HOST
// Bootload Transmit Complete
void sli_zigbee_af_bootload_transmit_complete(
      // The bootload message that was sent.
      EmberMessageBuffer message,
      // An EmberStatus value of EMBER_SUCCESS if an ACK was received from the
      // destination or EMBER_DELIVERY_FAILED if no ACK was received.
      EmberStatus status)
{
(void)message;
(void)status;

sli_zigbee_af_bootload_transmit_complete_callback(message, status);
}
#endif // EZSP_HOST



#ifdef EZSP_HOST
void sli_zigbee_af_zll_networkFound(
      // Information about the network.
      EmberZllNetwork *networkInfo,
      // Used to interpret deviceInfo field.
      bool isDeviceInfoNull,
      // Device specific information.
      EmberZllDeviceInfoRecord *deviceInfo,
      // The link quality from the node that last relayed the message.
      uint8_t lastHopLqi,
      // The energy level (in units of dBm) observed during reception.
      int8_t lastHopRssi)
{
(void)networkInfo;
(void)isDeviceInfoNull;
(void)deviceInfo;
(void)lastHopLqi;
(void)lastHopRssi;

sli_zigbee_af_zll_network_found_callback(networkInfo, isDeviceInfoNull, deviceInfo, lastHopLqi, lastHopRssi);
}
#else // !EZSP_HOST
// Zll Network Found
void sli_zigbee_af_zll_networkFound(
      // Return: Information about the network.
      const EmberZllNetwork *networkInfo,
      // Return: Device specific information.
      const EmberZllDeviceInfoRecord *deviceInfo)
{
(void)networkInfo;
(void)deviceInfo;

sli_zigbee_af_zll_network_found_callback(networkInfo, deviceInfo);
}
#endif // EZSP_HOST



// Zll Scan Complete
void sli_zigbee_af_zll_scan_complete(
      // Status of the operation.
      EmberStatus status)
{
(void)status;

sli_zigbee_af_zll_scan_complete_callback(status);
}



#ifdef EZSP_HOST
// Zll Address Assignment
void sli_zb_af_zll_address_assignment(
      // Address assignment information.
      EmberZllAddressAssignment *addressInfo,
      // The link quality from the node that last relayed the message.
      uint8_t lastHopLqi,
      // The energy level (in units of dBm) observed during reception.
      int8_t lastHopRssi)
{
(void)addressInfo;
(void)lastHopLqi;
(void)lastHopRssi;

sli_zigbee_af_zll_address_assignment_callback(addressInfo, lastHopLqi, lastHopRssi);
}
#else // !EZSP_HOST
// Zll Address Assignment
void sli_zb_af_zll_address_assignment(
      // Return: Address assignment information.
      const EmberZllAddressAssignment *addressInfo)
{
(void)addressInfo;

sli_zigbee_af_zll_address_assignment_callback(addressInfo);
}
#endif // EZSP_HOST



// Zll Touch Link Target
void sli_zigbee_af_zll_touch_link_target(
      // Return: Information about the network.
      const EmberZllNetwork *networkInfo)
{
(void)networkInfo;

sli_zigbee_af_zll_touch_link_target_callback(networkInfo);
}



#ifdef EZSP_HOST
// Mac Filter Match Message
void sli_zigbee_af_mac_filter_match_message(
      // filter index match.
      uint8_t filterIndexMatch,
      // message type.
      EmberMacPassthroughType messageType,
      // last hop lqi.
      uint8_t lastHopLqi,
      // last hop rssi.
      int8_t lastHopRssi,
      // message length.
      uint8_t messageLength,
      // message contents.
      uint8_t *messageContents)
{
(void)filterIndexMatch;
(void)messageType;
(void)lastHopLqi;
(void)lastHopRssi;
(void)messageLength;
(void)messageContents;

sli_zigbee_af_mac_filter_match_message_callback (filterIndexMatch, messageType, lastHopLqi, lastHopRssi, messageLength, messageContents);
}
#else // !EZSP_HOST
// Mac Filter Match Message
void sli_zigbee_af_mac_filter_match_message(
      // Return: macFilterMatchStruct.
      const EmberMacFilterMatchStruct *macFilterMatchStruct)
{
(void)macFilterMatchStruct;

sli_zigbee_af_mac_filter_match_message_callback(macFilterMatchStruct);
}
#endif // EZSP_HOST



// D Gp Sent
void sli_zigbee_af_d_gp_sent(
      // An EmberStatus value indicating success or the reason for failure.
      EmberStatus status,
      // The handle of the GPDF.
      uint8_t gpepHandle)
{
(void)status;
(void)gpepHandle;

sli_zigbee_af_d_gp_sent_callback(status, gpepHandle);
}



// Pan Id Conflict
// Return: An EmberStatus value indicating success or the reason for failure.
void sli_zigbee_af_pan_id_conflict(
      // Number of conflict reports
      int8_t conflictCount)
{
(void)conflictCount;

}



// Orphan Notification
void sli_zigbee_af_orphan_notification(
      // The 8 byte EUI64 of the sender.
      EmberEUI64 longId)
{
(void)longId;

}



// Counter
void sli_zigbee_af_counter(
      // Type of Counter
      EmberCounterType type,
      // Counter Info and value
      EmberCounterInfo Info)
{
(void)type;
(void)Info;

sli_zigbee_af_counter_callback(type, Info);
}



// Mac Passthrough Filter
// Return: True if mac passthrough filter was matched. False otherwise.
void sli_zigbee_af_mac_passthrough_filter(
      // Return: Mac Header of the matched messgae
      uint8_t *macHeader)
{
(void)macHeader;

}



// Generate Cbke Keys Handler283k1
void sli_zigbee_af_generate_cbke_keys_handler283k1(
      // The result of the CBKE operation.
      EmberStatus status,
      // Return: The generated ephemeral public key.
      EmberPublicKey283k1Data *ephemeralPublicKey)
{
(void)status;
(void)ephemeralPublicKey;

}



// Calculate Smacs Handler283k1
void sli_zigbee_af_calculate_smacsHandler283k1(
      // The Result of the CBKE operation.
      EmberStatus status,
      // Return: The calculated value of the initiator's SMAC
      EmberSmacData *initiatorSmac,
      // Return: The calculated value of the responder's SMAC
      EmberSmacData *responderSmac)
{
(void)status;
(void)initiatorSmac;
(void)responderSmac;

}



// Gpep Incoming Message
void sli_zigbee_af_gpep_incoming_message(
      // The status of the GPDF receive.
      EmberStatus status,
      // The gpdLink value of the received GPDF.
      uint8_t gpdLink,
      // The GPDF sequence number.
      uint8_t sequenceNumber,
      // The address of the source GPD.
      EmberGpAddress *addr,
      // The security level of the received GPDF.
      EmberGpSecurityLevel gpdfSecurityLevel,
      // The securityKeyType used to decrypt/authenticate the incoming GPDF.
      EmberGpKeyType gpdfSecurityKeyType,
      // Whether the incoming GPDF had the auto-commissioning bit set.
      bool autoCommissioning,
      // Bidirectional information represented in bitfields, where bit0 holds
      // the rxAfterTx of incoming gpdf and bit1 holds if tx queue is available
      // for outgoing gpdf.
      uint8_t bidirectionalInfo,
      // The security frame counter of the incoming GDPF.
      uint32_t gpdSecurityFrameCounter,
      // The gpdCommandId of the incoming GPDF.
      uint8_t gpdCommandId,
      // The received MIC of the GPDF.
      uint32_t mic,
      // The proxy table index of the corresponding proxy table entry to the
      // incoming GPDF.
      uint8_t proxyTableIndex,
      // The length of the GPD command payload.
      uint8_t gpdCommandPayloadLength,
      // The GPD command payload.
      uint8_t *gpdCommandPayload)
{
(void)status;
(void)gpdLink;
(void)sequenceNumber;
(void)addr;
(void)gpdfSecurityLevel;
(void)gpdfSecurityKeyType;
(void)autoCommissioning;
(void)bidirectionalInfo;
(void)gpdSecurityFrameCounter;
(void)gpdCommandId;
(void)mic;
(void)proxyTableIndex;
(void)gpdCommandPayloadLength;
(void)gpdCommandPayload;

sli_zigbee_af_gpep_incoming_message_callback(status, gpdLink, sequenceNumber, addr, gpdfSecurityLevel, gpdfSecurityKeyType, autoCommissioning, bidirectionalInfo, gpdSecurityFrameCounter, gpdCommandId, mic, proxyTableIndex, gpdCommandPayloadLength, gpdCommandPayload);
}



// Rtos Idle
// Return: True or False.
void sli_zigbee_af_rtos_idle(
      // Return: Idle time duration
      uint32_t *idleTimeMs)
{
(void)idleTimeMs;

}



// Rtos Stack Wakeup Isr
void sli_zb_af_rtos_stack_wakeup_isr(void)
{

}



// Radio Needs Calibrating
void sli_zigbee_af_radio_needs_calibrating(void)
{

}



// Scan Error
void sli_zigbee_af_scan_error(
      // The error status of a scan
      EmberStatus status)
{
(void)status;

}

