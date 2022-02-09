#define COMMAND_CLASS_TRANSPORT_SERVICE_V2 0x55
#define COMMAND_FIRST_FRAGMENT             0xC0
#define COMMAND_SUBSEQUENT_FRAGMENT        0xE0
#define COMMAND_SEGMENT_REQUEST_V2         0xC8
#define COMMAND_SEGMENT_COMPLETE_V2        0xE8
#define COMMAND_SEGMENT_WAIT_V2            0xF0
#define COMMAND_CLASS_INDEX 0
/************************************************************/
/* Command Segment Request V2 command class structs */
/************************************************************/
typedef struct _ZW_COMMAND_SEGMENT_REQUEST_V2_FRAME_ {
  uint8_t cmdClass;        /* The command class */
  uint8_t cmd_reserved;    /* The command + parameter reserved */
  uint8_t properties2;     /* masked byte */
  uint8_t datagramOffset2; /**/
} ZW_COMMAND_SEGMENT_REQUEST_V2_FRAME;

/************************************************************/
/* Command Segment Wait V2 command class structs */
/************************************************************/
typedef struct _ZW_COMMAND_SEGMENT_WAIT_V2_FRAME_ {
  uint8_t cmdClass;         /* The command class */
  uint8_t cmd_reserved;     /* The command + parameter reserved */
  uint8_t pendingFragments; /**/
} ZW_COMMAND_SEGMENT_WAIT_V2_FRAME;

/************************************************************/
/* Command Segment Complete V2 command class structs */
/************************************************************/
typedef struct _ZW_COMMAND_SEGMENT_COMPLETE_V2_FRAME_ {
  uint8_t cmdClass;     /* The command class */
  uint8_t cmd_reserved; /* The command + parameter reserved */
  uint8_t properties2;  /* masked byte */
} ZW_COMMAND_SEGMENT_COMPLETE_V2_FRAME;

/************************************************************/
/* Command First Fragment 1byte command class structs */
/************************************************************/
typedef struct _ZW_COMMAND_FIRST_FRAGMENT_1BYTE_FRAME_ {
  uint8_t cmdClass;          /* The command class */
  uint8_t cmd_datagramSize1; /* The command + parameter datagram_size_1 */
  uint8_t datagramSize2;     /**/
  uint8_t properties2;       /* masked byte */
  uint8_t payload1;
  uint8_t checksum1; /* MSB */
  uint8_t checksum2; /* LSB */
} ZW_COMMAND_FIRST_FRAGMENT_1BYTE_FRAME;

/************************************************************/
/* Command Subsequent Fragment 1byte command class structs */
/************************************************************/
typedef struct _ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME_ {
  uint8_t cmdClass;          /* The command class */
  uint8_t cmd_datagramSize1; /* The command + parameter datagram_size_1 */
  uint8_t datagramSize2;     /**/
  uint8_t properties2;       /* masked byte */
  uint8_t datagramOffset2;   /**/
  uint8_t payload1;
  uint8_t checksum1; /* MSB */
  uint8_t checksum2; /* LSB */
} ZW_COMMAND_SUBSEQUENT_FRAGMENT_1BYTE_FRAME;

