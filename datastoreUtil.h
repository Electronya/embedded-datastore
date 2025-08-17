/**
 * Copyright (C) 2025 by Electronya
 *
 * @file      datastoreUtils.h
 * @author    jbacon
 * @date      2025-08-17
 * @brief     Datastore Utilities
 *
 *            Datastore utility functions.
 *
 * @ingroup   datastore
 *
 * @{
 */

#ifndef DATASTORE_SRV_UTIL
#define DATASTORE_SRV_UTIL

#include "datastore.h"
#include "datastoreBufferPool.h"

/**
 * @brief   The generic notifier callback.
 */
typedef int (*NotifyCallback_t)(DatapointData_t values[], size_t valCount);

typedef struct
{
  uint32_t datapointId;                 /**< The datapoint ID */
  size_t valCount;                      /**< The datapoint count */
  bool isPaused;                        /**< The paused subscription flag */
  NotifyCallback_t callback;            /**< The subscription callback */
} GenericSubscription_t

/**
 * @brief   Allocate the array for the float subscriptions.
 *
 * @param[in]   datapointType: The datapoint type.
 * @param[in]   maxSubCount: The maximum number of subscriptions.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAllocateSubs(DatapointType_t datapointType, size_t maxSubCount);

/**
 * @brief   Initialize the datastore buffer pool.
 *
 * @param[in]   maxSubs: The maximum subscription count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilInitBufferPool(DatastoreMaxSubs_t *maxSubs);

/**
 * @brief   Do the initial notifications.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilDoInitNotifications(void);

/**
 * @brief   Add a subscription.
 *
 * @param[in]   datapointType: The datapoint type.
 * @param[in]   sub: The subscription.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddSubscription(DatapointType_t datapointType, GenericSubscription_t *sub);

/**
 * @brief   Pause a subscription.
 *
 * @param[in]   datapointType: The datapoint type.
 * @param[in]   callback: The subscription callback.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilPauseSubscription(DatapointType_t datapointType, NotifyCallback_t callback);

/**
 * @brief   Unpause a subscription.
 *
 * @param[in]   datapointType: The datapoint type.
 * @param[in]   callback: The subscription callback;
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilUnpauseSubscription(DatapointType_t datapointType, NotifyCallback_t callback);

/**
 * @brief   Notify for a specific datapoint.
 *
 * @param[in]   datapointType: The datapoint type.
 * @param[in]   datapointId: The datapoint id.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilNotify(DatapointType_t datapointType, uint32_t datapointId);

/**
 * @brief   Read values.
 *
 * @param[in]   datapointType: The datapoint type.
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   valCount: The value count to read.
 * @param[out]  values: The output buffer.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilReadData(DatapointType_t datapointType, uint32_t datapointId, size_t valCount, DatapointData_t values[]);

/**
 * @brief   Write values.
 *
 * @param[in]   datapointType: The datapoint type.
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   values: The values.
 * @param[in]   valCount: The values count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilWriteData(DatapointType_t datapointType, uint32_t datapointId, DatapointData_t values[], size_t valCount);

#endif    /* DATASTORE_SRV_UTIL */

/** @} */
