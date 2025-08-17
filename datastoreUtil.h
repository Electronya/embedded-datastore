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
 * @brief   Initialize the datastore buffer pool.
 *
 * @param[in]   maxSubs: The maximum subscription count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilInitBufferPool(DatastoreMaxSubs_t *maxSubs, DatastoreBufferPool_t **pool);

/**
 * @brief   Notify for a specific float datapoint.
 *
 * @param[in]   datapointId: The datapoint id.
 * @param[in]   subs: The float subscriptions.
 * @param[in]   subCount: The count of subscription.
 * @param[in]   bufPool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilNotifyFloat(uint32_t datapointId, DatapointFloatSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool);

/**
 * @brief   Notify for a specific unsigned integer datapoint.
 *
 * @param[in]   datapointId: The datapoint id.
 * @param[in]   subs: The unsigned integer subscriptions.
 * @param[in]   subCount: The count of subscription.
 * @param[in]   bufPool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilNotifyUint(uint32_t datapointId, DatapointUintSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool);

/**
 * @brief   Notify for a specific signed integer datapoint.
 *
 * @param[in]   datapointId: The datapoint id.
 * @param[in]   subs: The signed integer subscriptions.
 * @param[in]   subCount: The count of subscription.
 * @param[in]   bufPool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilNotifyInt(uint32_t datapointId, DatapointIntSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool);

/**
 * @brief   Notify for a specific multi-state datapoint.
 *
 * @param[in]   datapointId: The datapoint id.
 * @param[in]   subs: The multi-state subscriptions.
 * @param[in]   subCount: The count of subscription.
 * @param[in]   bufPool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilNotifyMultiState(uint32_t datapointId, DatapointMultiStateSub_t *subs,
                                  size_t subCount, DatastoreBufferPool_t *bufPool);

/**
 * @brief   Notify for a specific button datapoint.
 *
 * @param[in]   datapointId: The datapoint id.
 * @param[in]   subs: The button subscriptions.
 * @param[in]   subCount: The count of subscription.
 * @param[in]   bufPool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilNotifyButton(uint32_t datapointId, DatapointButtonSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool);

#endif    /* DATASTORE_SRV_UTIL */

/** @} */
