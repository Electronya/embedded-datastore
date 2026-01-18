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

/**
 * @brief   The datastore buffer pool allocation timeout.
 */
#define DATASTORE_BUFFER_ALLOC_TIMEOUT                          (4)

/**
 * @brief   Allocate the array for the binary subscriptions.
 *
 * @param[in]   maxSubCount: The maximum subscription count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAllocateBinarySubs(size_t maxSubCount);

/**
 * @brief   Allocate the array for the button subscription.
 *
 * @param[in]   maxSubCount: The maximum subscription count.
 *
 * @return  0 if successful, the error code.
 */
int datastoreUtilAllocateButtonSubs(size_t maxSubCount);

/**
 * @brief   Allocate the array for the float subscription.
 *
 * @param[in]   maxSubCount: The maximum subscription count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAllocateFloatSubs(size_t maxSubCount);

/**
 * @brief   Allocate the array for the signed integer subscription.
 *
 * @param[in]   maxSubCount: The maximum subscription count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAllocateIntSubs(size_t maxSubCount);

/**
 * @brief   Allocate the array for the multi-state subscription.
 *
 * @param[in]   maxSubCount: The maximum subscription count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAllocateMultiStateSubs(size_t maxSubCount);

/**
 * @brief   Allocate the array for the unsigned integer subscription.
 *
 * @param[in]   maxSubCount: The maximum subscription count.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAllocateUintSubs(size_t maxSubCount);

/**
 * @brief   Calculate the maximum buffer size.
 *
 * @param[in]   datapointCounts: The datapoint count for each types.
 *
 * @return  The calculated maximum buffer size.
 */
size_t datastoreUtilCalculateBufferSize(size_t datapointCounts[DATAPOINT_TYPE_COUNT]);

/**
 * @brief   Add a binary subscription.
 *
 * @param[in]   sub: The subscription.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddBinarySub(DatastoreBinarySub_t *sub, osMemoryPoolId_t pool);

/**
 * @brief   Remove a binary subscription.
 *
 * @param[in]   callback: The subscription callback.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilRemoveBinarySub(DatastoreBinarySubCb_t callback);

/**
 * @brief   Set a binary subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetBinarySubPauseState(DatastoreBinarySubCb_t subCallback, bool isPaused, osMemoryPoolId_t pool);

/**
 * @brief   Add a button subscription.
 *
 * @param[in]   sub: The subscription.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddButtonSub(DatastoreButtonSub_t *sub, osMemoryPoolId_t pool);

/**
 * @brief   Remove a button subscription.
 *
 * @param[in]   callback: The subscription callback.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilRemoveButtonSub(DatastoreButtonSubCb_t callback);

/**
 * @brief   Set a button subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetButtonSubPauseState(DatastoreButtonSubCb_t subCallback, bool isPaused, osMemoryPoolId_t pool);

/**
 * @brief   Add a float subscription.
 *
 * @param[in]   sub: The subscription.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddFloatSub(DatastoreFloatSub_t *sub, osMemoryPoolId_t pool);

/**
 * @brief   Remove a float subscription.
 *
 * @param[in]   callback: The subscription callback.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilRemoveFloatSub(DatastoreFloatSubCb_t callback);

/**
 * @brief   Set a float subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetFloatSubPauseState(DatastoreFloatSubCb_t subCallback, bool isPaused, osMemoryPoolId_t pool);

/**
 * @brief   Add a signed integer subscription.
 *
 * @param[in]   sub: The subscription.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddIntSub(DatastoreIntSub_t *sub, osMemoryPoolId_t pool);

/**
 * @brief   Remove a signed integer subscription.
 *
 * @param[in]   callback: The subscription callback.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilRemoveIntSub(DatastoreIntSubCb_t callback);

/**
 * @brief   Set a signed integer subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetIntSubPauseState(DatastoreIntSubCb_t subCallback, bool isPaused, osMemoryPoolId_t pool);

/**
 * @brief   Add a multi-state subscription.
 *
 * @param[in]   sub: The subscription.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddMultiStateSub(DatastoreMultiStateSub_t *sub, osMemoryPoolId_t pool);

/**
 * @brief   Remove a multi-state subscription.
 *
 * @param[in]   callback: The subscription callback.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilRemoveMultiStateSub(DatastoreMultiStateSubCb_t callback);

/**
 * @brief   Set a multi-state subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetMultiStateSubPauseState(DatastoreMultiStateSubCb_t subCallback, bool isPaused, osMemoryPoolId_t pool);

/**
 * @brief   Add a unsigned integer subscription.
 *
 * @param[in]   sub: The subscription.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddUintSub(DatastoreUintSub_t *sub, osMemoryPoolId_t pool);

/**
 * @brief   Remove a unsigned integer subscription.
 *
 * @param[in]   callback: The subscription callback.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilRemoveUintSub(DatastoreUintSubCb_t callback);

/**
 * @brief   Set a unsigned integer subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetUintSubPauseState(DatastoreUintSubCb_t subCallback, bool isPaused, osMemoryPoolId_t pool);

/**
 * @brief   Read from the datastore.
 *
 * @param[in]   type: The datapoint type.
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   valCount: The value count.
 * @param[out]  values: The output buffer.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilRead(DatapointType_t type, uint32_t datapointId, size_t valCount, DatapointValue_t values[]);

/**
 * @brief   Write to the datastore.
 *
 * @param[in]   type: The datapoint type.
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   values: The input buffer.
 * @param[in]   valCount: The value count.
 * @param[in]   pool: The buffer pool
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilWrite(DatapointType_t type, uint32_t datapointId, DatapointValue_t values[],
                       size_t valCount, osMemoryPoolId_t pool);

/**
 * @brief   Notify subscribers.
 *
 * @param[in]   type: The datapoint type.
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilNotify(DatapointType_t type, uint32_t datapointId, osMemoryPoolId_t pool);

#endif    /* DATASTORE_SRV_UTIL */

/** @} */
