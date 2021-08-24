cat << EOF
#include <sl_status.h>

const char* sl_status_string(int status) {
  switch (status) {
EOF

grep -E "^\s*\Sdefine\s+SL_STATUS" ../include/sl_status.h | grep -v SL_STATUS_H | awk '{print "    case " $2 ": return \"" $2 "\";"}'

cat << EOF
    default: return "UNKNOWN";
  }
}

const char* sl_status_string_verbose(int status) {
  switch (status) {
EOF

grep -E "^\s*\Sdefine\s+SL_STATUS" ../include/sl_status.h | grep -v SL_STATUS_H | tr -s ' ' | cut -d\  -f2,5- | sed 's/ /:/' | awk -F: '{print "    case " $1 ": return \"" $2 "\";"}'

cat << EOF
    default: return "Unknown error.";
  }
}
EOF
