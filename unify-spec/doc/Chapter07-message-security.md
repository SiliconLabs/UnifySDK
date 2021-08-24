# Message Security

This chapter describes how to manage message security crossing from one MQTT client (PHY Protocol Controller or Cloud Service) to another.

## Security Scale for Messages

PHY translators MUST annotate MQTT publish messages triggered by incoming PAN frame. The annotation must indicate the incoming security level of the PAN frame.
The annotation must use a scale that enumerates all the security modes in all the translated PHYs, ranked by security level. For example, the Z-Wave S2 Authenticated and BLE Diffie-Hellman inclusion methods must be placed appropriately on the same scale.
The security scale must be extensible to add more PHYs of both high and low security levels.
The security level MUST NOT be used for outgoing frames towards a PHY. The security field is for incoming security level only.

## Requirements

The ACL engine must admit only MQTT publishes from a given MQTT Client (PHY Translator or IoT Service alike) with security levels that PHY Translator is trusted to manage. For example, a completely non-secure X10 PHY Translator should never be allowed to publish a Security 2 Authenticated level message. The highest security level allowed for an MQTT client may be hard coded in the ACL configuration or derived from a certificate presented by the MQTT client.
A PHY translator must never upgrade security on PAN Frames. If necessary, it must publish the frame on the PAN side with a less secure security level. The PHY translator may drop an outgoing frame to the PAN, if it can guarantee that the frame will not be processed by the node due to the security level in use.
The Security Level must be published per-node by a PHY Translator to support PHYs where individual nodes may have different security levels.
The ACL engine will monitor the node status topic to detect the security level supported by a node. Likewise, a rules engine must check the affected nodes security status fields before accepting a rule.

## Automatic Selection of PAN Security Level

Security levels of messages egressing the broker into a PAN MUST be chosen by the PHY Translator.  The PHY Translator must choose the appropriate security level based on knowledge on the specific PHY.
