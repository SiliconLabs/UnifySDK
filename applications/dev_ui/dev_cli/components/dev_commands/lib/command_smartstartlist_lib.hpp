/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

#ifndef COMMAND_UI_LIB
#define COMMAND_UI_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief The function publishes the input arguments for updating the smart
 * start list.
 *
 * \ingroup DEV CLI
 * @returns int
 */
int update_smart_start_list(const char *dsk,
                            const char *include,
                            const char *protocolunid,
                            const char *unid);

/**
 * @brief The function assigns a cb to ucl/SmartStart/List to print the smart
 * start list when received.
 *
 * @param cache You can choose to ignore a cached SmartStart list
 * \ingroup DEV CLI
 *
 */
void print_smart_start_list();

/**
 * @brief Removing a SmartStart list entry by publishing a DSK key to
 * ucl/SmartStart/List
 *
 * @param dsk_key The DSK key to remove from the SmartStart entry list
 * \ingroup DEV CLI
 * @returns int
 */
int remove_entry_smart_start_list(const char *dsk_key);

/**
 * @brief Parsing a const char * as a SmartStart list and printing it in a
 * readable format
 *
 * @param ss_list A pointer to the SmartStart list in a char array format
 * \ingroup DEV CLI
 * @returns int
 */
int parse_json_smart_start_list(const char *ss_list);

#ifdef __cplusplus
}
#endif
#endif