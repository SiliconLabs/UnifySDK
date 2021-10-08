/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup datastore_attributes Attributes storage.
 * @ingroup datastore
 * @brief Datastore API for Attributes (from the @ref attribute_store)
 *
 * Persistent datastore, that allows for storing our @ref attribute_store.
 * It uses a tailored table allowing to capture the @ref attribute_store
 * tree.
 *
 * DESCRIBE TABLE; gives the following:
 *
 *
 * @{
 */

#ifndef DATASTORE_ATTRIBUTES_H
#define DATASTORE_ATTRIBUTES_H

#include <inttypes.h>
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t datastore_attribute_id_t;

/**
 * @brief Store an attribute in the persistent datastore.
 *
 * It will insert it if no ID already exists, else it will update the data
 * in the data store.
 *
 * @param id                  Unique ID for the Attribute Store node to be
 *                            inserted/updated. It has to be >0, as 0 is
 *                            reserved for "no parents".
 * @param type                Node type from the Attribute Store.
 * @param parent_id           Unique ID of the Attribute Store parent node for
 *                            the node to be inserted/updated. Set this to 0x00
 *                            for a node without parent (root node)
 * @param reported_value      Pointer to the value to write the the attribute
 *                            (REPORTED value)
 * @param reported_value_size Size of the data contained in the reported_value
 *                            pointer.
 * @param desired_value       Pointer to the value to write the the attribute
 *                            (DESIRED value)
 * @param desired_value_size  Size of the data contained in the desired_value
 *                            pointer.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error happened
 */
sl_status_t datastore_store_attribute(datastore_attribute_id_t id,
                                      uint32_t type,
                                      datastore_attribute_id_t parent_id,
                                      const uint8_t *reported_value,
                                      uint8_t reported_value_size,
                                      const uint8_t *desired_value,
                                      uint8_t desired_value_size);

/**
 * @brief Fetch an attribute from the persistent datastore.
 *
 * @param id                  Unique ID for the Attribute Store node to be
 *                            inserted/updated.
 * @param type                Pointer to copy the node type from the
 *                            Attribute Store.
 * @param parent_id           Pointer where the Unique ID of the parent will
 *                            be written
 * @param reported_value      Pointer to the stored reported value of the
 *                            attribute will be copied. The value buffer must
 *                            be able to contain
 *                            ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH bytes.
 * @param reported_value_size Pointer to a variable where the size
 *                            of the reported value/payload written to the
 *                            reported value pointer will be written.
 * @param desired_value       Pointer to the stored desired value of the
 *                            attribute will be copied. The value buffer must
 *                            be able to contain
 *                            ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH bytes.
 * @param desired_value_size  Pointer to a variable where the size
 *                            of the desired value/payload written to the
 *                            desired value pointer will be written.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error happened
 * @returns SL_STATUS_NOT_FOUND the id was not found in the database.
 */
sl_status_t datastore_fetch_attribute(const datastore_attribute_id_t id,
                                      uint32_t *type,
                                      datastore_attribute_id_t *parent_id,
                                      uint8_t *reported_value,
                                      uint8_t *reported_value_size,
                                      uint8_t *desired_value,
                                      uint8_t *desired_value_size);

/**
 * @brief Fetch the child of an attribute from the persistent datastore.
 *
 * @param parent_id           Unique ID for the Attribute Store node of which the
 *                            nth child will be returned.
 * @param child_index         The index of the child to find. 0 for the first child,
 *                            1 for the second child, etc.
 * @param child_id            Pointer where the Unique ID of the found child will be copied
 * @param type                Pointer to copy the type of the found child
 *                            from the Attribute Store.
 * @param reported_value      Pointer to the stored reported value of the
 *                            attribute will be copied. The value buffer must
 *                            be able to contain
 *                            ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH bytes.
 * @param reported_value_size Pointer to a variable where the size
 *                            of the reported value/payload written to the
 *                            reported value pointer will be written.
 * @param desired_value       Pointer to the stored desired value of the
 *                            attribute will be copied. The value buffer must
 *                            be able to contain
 *                            ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH bytes.
 * @param desired_value_size  Pointer to a variable where the size
 *                            of the desired value/payload written to the
 *                            desired value pointer will be written.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error happened
 * @returns SL_STATUS_NOT_FOUND nth child or parent does not exist.
 */
sl_status_t datastore_fetch_attribute_child(datastore_attribute_id_t parent_id,
                                            uint32_t child_index,
                                            datastore_attribute_id_t *child_id,
                                            uint32_t *type,
                                            uint8_t *reported_value,
                                            uint8_t *reported_value_size,
                                            uint8_t *desired_value,
                                            uint8_t *desired_value_size);

/**
 * @brief Check if the datastore contains an attribute for given key.
 *
 * @param id Unique ID for the Attribute Store node to search for
 *
 * @returns true if the attribute datastore_id is present in the datatore
 * @returns false if the key is not present in the datatore
 */
bool datastore_contains_attribute(const datastore_attribute_id_t id);

/**
 * @brief Delete an attribute from the persistent datastore.
 *
 * @param id              Unique ID for the Attribute Store node to be
 *                        deleted
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error happened
 */
sl_status_t datastore_delete_attribute(const datastore_attribute_id_t id);

/**
 * @brief Delete an attribute and all its children from the persistent datastore.
 *
 * @param id              Unique ID for the Attribute Store node to be
 *                        deleted (with all its children)
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error happened
 */
sl_status_t
  datastore_delete_attribute_with_children(const datastore_attribute_id_t id);

/**
 * @brief Delete the whole attribute table in the persistent datastore.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error happened
 */
sl_status_t datastore_delete_all_attributes();

#ifdef __cplusplus
}
#endif

#endif  //DATASTORE_ATTRIBUTES_H
/** @} end datastore_attributes */
