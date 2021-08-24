

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

#include "sl_log.h"

#include "zigpc_common_unid.h"
#include "zigpc_common_ucl.h"

static const char LOG_TAG[]                   = "zigpc_common_ucl";
static const unsigned int unid_padding_length = 11;

sl_status_t ucl_extract_unid_string(const char *topic_string,
                                    char *unid_string,
                                    const size_t unid_size)
{
  sl_status_t status = SL_STATUS_FAIL;

  const char unid_regex_str[]
    = ZIGPC_UNID_MQTT_TOPIC ZIGPC_UNID_PREFIX "[0-9|a-f|A-F]*";

  bool valid_topic = (topic_string != NULL);
  bool valid_dest  = (unid_string != NULL);

  regex_t id_expr;
  regmatch_t id_match[1];
  (void)regcomp(&id_expr, unid_regex_str, REG_EXTENDED);

  int id_success = -1;

  if (valid_topic) {
    id_success = regexec(&id_expr, topic_string, 1, id_match, 0);
  }

  if ((id_success == 0) && valid_dest) {
    unsigned int id_length
      = id_match[0].rm_eo - id_match[0].rm_so - unid_padding_length;
    if (id_length < unid_size) {
      unsigned int start_index = id_match[0].rm_so + unid_padding_length;
      strncpy(unid_string, topic_string + start_index, id_length);
      unid_string[id_length] = '\0';

      status = SL_STATUS_OK;
    } else {
      status = SL_STATUS_FAIL;
      sl_log_warning(LOG_TAG,
                     "Destination string is too short for UNID result");
    }
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG,
                   "Unable to find UNID in topic string or invalid input");
  }

  regfree(&id_expr);
  return status;
}

sl_status_t ucl_extract_endpoint_string(const char *topic_string,
                                        char *endpoint_str,
                                        const size_t endpoint_size)
{
  sl_status_t status     = SL_STATUS_FAIL;
  const size_t unid_size = UCL_STRING_MAX_LENGTH;
  char unid_string[unid_size];
  char endpoint_regex_str[unid_size + 11];

  bool valid_topic = (topic_string != NULL);
  bool valid_dest  = (endpoint_str != NULL);

  int endpoint_success = -1;
  regex_t endpoint_expr;
  regmatch_t endpoint_match[1];

  if (valid_topic) {
    status = ucl_extract_unid_string(topic_string, unid_string, unid_size);

    if (status == SL_STATUS_OK) {
      strncpy(endpoint_regex_str, unid_string, unid_size);
      strncat(endpoint_regex_str, "/ep[0-9]+", 10);
      endpoint_regex_str[unid_size + 10] = '\0';

      (void)regcomp(&endpoint_expr, endpoint_regex_str, REG_EXTENDED);

      endpoint_success
        = regexec(&endpoint_expr, topic_string, 1, endpoint_match, 0);
    }
  }

  if ((endpoint_success == 0) && valid_dest) {
    unsigned int endpoint_length
      = endpoint_match[0].rm_eo - endpoint_match[0].rm_so - strlen(unid_string);
    if (endpoint_length <= endpoint_size) {
      /* start_index incremented to remove preceeding '/' in regex match */
      unsigned int start_index
        = endpoint_match[0].rm_so + strlen(unid_string) + 1;
      strncpy(endpoint_str, topic_string + start_index, (int)endpoint_length);
      endpoint_str[endpoint_length - 1] = '\0';

      status = SL_STATUS_OK;
    } else {
      status = SL_STATUS_FAIL;
      sl_log_warning(LOG_TAG,
                     "Destination string is too short for endpoint result");
    }
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG,
                   "Unable to find endpoint in topic string or invalid input");
  }

  regfree(&endpoint_expr);
  return status;
}

sl_status_t ucl_extract_cluster_string(const char *topic_string,
                                       char *cluster_string,
                                       const size_t cluster_size)
{
  sl_status_t status         = SL_STATUS_FAIL;
  const size_t endpoint_size = UCL_STRING_MAX_LENGTH;
  char endpoint_string[endpoint_size];
  char cluster_regex_str[endpoint_size + 6];

  bool valid_topic = (topic_string != NULL);
  bool valid_dest  = (cluster_string != NULL);

  int cluster_success = -1;
  regex_t cluster_expr;
  regmatch_t cluster_match[1];

  if (valid_topic) {
    status = ucl_extract_endpoint_string(topic_string,
                                         endpoint_string,
                                         endpoint_size);

    if (status == SL_STATUS_OK) {
      strncpy(cluster_regex_str, endpoint_string, endpoint_size);
      strncat(cluster_regex_str, "/\\w+", 5);
      cluster_regex_str[endpoint_size + 5] = '\0';

      (void)regcomp(&cluster_expr, cluster_regex_str, REG_EXTENDED);

      cluster_success
        = regexec(&cluster_expr, topic_string, 1, cluster_match, 0);
    }
  }

  if ((cluster_success == 0) && valid_dest) {
    unsigned int cluster_length = cluster_match[0].rm_eo
                                  - cluster_match[0].rm_so
                                  - strlen(endpoint_string);
    if (cluster_length <= cluster_size) {
      /* start_index incremented to remove preceeding '/' in regex match */
      unsigned int start_index
        = cluster_match[0].rm_so + strlen(endpoint_string) + 1;
      strncpy(cluster_string, topic_string + start_index, (int)cluster_length);
      cluster_string[cluster_length - 1] = '\0';

      status = SL_STATUS_OK;
    } else {
      status = SL_STATUS_FAIL;
      sl_log_warning(LOG_TAG,
                     "Destination string is too short for cluster result");
    }
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG,
                   "Unable to find cluster in topic string or invalid input");
  }

  regfree(&cluster_expr);
  return status;
}

sl_status_t ucl_extract_cmd_string(const char *topic_string,
                                   char *cmd_string,
                                   const size_t cmd_size)
{
  sl_status_t status        = SL_STATUS_FAIL;
  const size_t cluster_size = UCL_STRING_MAX_LENGTH;
  char cluster_string[cluster_size];
  char cmd_regex_str[cluster_size + 6];

  bool valid_topic = (topic_string != NULL);
  bool valid_dest  = (cmd_string != NULL);

  int cmd_success = -1;
  regex_t cmd_expr;
  regmatch_t cmd_match[1];

  if (valid_topic) {
    status
      = ucl_extract_cluster_string(topic_string, cluster_string, cluster_size);

    if (status == SL_STATUS_OK) {
      strncpy(cmd_regex_str, cluster_string, cluster_size);
      strncat(cmd_regex_str, "/\\w+", 5);
      cmd_regex_str[cluster_size + 5] = '\0';

      (void)regcomp(&cmd_expr, cmd_regex_str, REG_EXTENDED);

      cmd_success = regexec(&cmd_expr, topic_string, 1, cmd_match, 0);
    }
  }

  if ((cmd_success == 0) && valid_dest) {
    unsigned int cmd_length
      = cmd_match[0].rm_eo - cmd_match[0].rm_so - strlen(cluster_string);
    if (cmd_length <= cmd_size) {
      /* start_index incremented to remove preceeding '/' in regex match */
      unsigned int start_index
        = cmd_match[0].rm_so + strlen(cluster_string) + 1;
      strncpy(cmd_string, topic_string + start_index, (int)cmd_length);
      cmd_string[cmd_length - 1] = '\0';

      status = SL_STATUS_OK;
    } else {
      status = SL_STATUS_FAIL;
      sl_log_warning(LOG_TAG,
                     "Destination string too short for command result");
    }
  } else {
    status = SL_STATUS_FAIL;
  }

  regfree(&cmd_expr);
  return status;
}

sl_status_t ucl_endpoint_str_to_int(const char *endpoint_str,
                                    int *endpoint_dest)
{
  sl_status_t status;

  if ((endpoint_str != NULL) && (endpoint_dest != NULL)) {
    int format_success = strncmp(endpoint_str, "ep", 2);

    if (format_success == 0) {
      *endpoint_dest = strtoul(endpoint_str + 2, NULL, 0);
      status         = SL_STATUS_OK;
    } else {
      status = SL_STATUS_FAIL;
      sl_log_warning(LOG_TAG,
                     "Invalid format for endpoint string to int conversion");
    }
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG,
                   "Invalid input to endpoint string to int conversion");
  }
  return status;
}
