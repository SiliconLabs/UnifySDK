// CLEANUP: Stuff used outside the serialapi

// CLEANUP: The TRANSMIT_COMPLETE_xxx defines seems to only be used by zip
// gateway for node queue management (i.e. on a higher abstraction layer than
// serial api)

/**
 * Used to indicate that transmissions was not completed due to a SendData that
 * returned false. This is used in some of the higher level sendata calls where
 * lower layer senddata is called async.
 */
#define TRANSMIT_COMPLETE_ERROR 0xFF

/**
 * Tell upper queue to requeue this frame because ClassicZIPNode is busy sending
 * another frame.
 */
#define TRANSMIT_COMPLETE_REQUEUE 0xFE

/**
 * Tell upper queue to requeue this frame because ClassicZIPNode cannot send to
 * PAN side because NetworkManagament or Mailbox are busy, or because the
 * destination node is being probed. The frame should be re-queued to long-queue
 * or Mailbox.
 */
#define TRANSMIT_COMPLETE_REQUEUE_QUEUED 0xFD


// CLEANUP: chip_descriptor chip_desc belongs in serial_api_process component

/**
 * This chip descriptor contains the version of chip (e.g. 500 or 700).
 * It is populated when serialapi process starts up.
 * Until populated, it contains the value CHIP_DESCRIPTOR_UNINITIALIZED.
 */
struct chip_descriptor {
  uint8_t my_chip_type;
  uint8_t my_chip_version;
};
extern struct chip_descriptor chip_desc;
#define CHIP_DESCRIPTOR_UNINITIALIZED 0x42