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
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddBinarySub(DatastoreBinarySub_t *sub);

/**
 * @brief   Set a binary subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetBinarySubPauseState(DatastoreBinarySubCb_t subCallback, bool isPaused);

/**
 * @brief   Add a button subscription.
 *
 * @param[in]   sub: The subscription.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddButtonSub(DatastoreButtonSub_t *sub);

/**
 * @brief   Set a button subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetButtonSubPauseState(DatastoreButtonSubCb_t subCallback, bool isPaused);

/**
 * @brief   Add a float subscription.
 *
 * @param[in]   sub: The subscription.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddFloatSub(DatastoreFloatSub_t *sub);

/**
 * @brief   Set a float subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetFloatSubPauseState(DatastoreFloatSubCb_t subCallback, bool isPaused);

/**
 * @brief   Add a signed integer subscription.
 *
 * @param[in]   sub: The subscription.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddIntSub(DatastoreIntSub_t *sub);

/**
 * @brief   Set a signed integer subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetIntSubPauseState(DatastoreIntSubCb_t subCallback, bool isPaused);

/**
 * @brief   Add a multi-state subscription.
 *
 * @param[in]   sub: The subscription.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddMultiStateSub(DatastoreMultiStateSub_t *sub);

/**
 * @brief   Set a multi-state subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetMultiStateSubPauseState(DatastoreMultiStateSubCb_t subCallback, bool isPaused);

/**
 * @brief   Add a unsigned integer subscription.
 *
 * @param[in]   sub: The subscription.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilAddUintSub(DatastoreUintSub_t *sub);

/**
 * @brief   Set a unsigned integer subscription pause state.
 *
 * @param[in]   subCallback: The subscription callback.
 * @param[in]   isPaused: The pause flag.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreUtilSetUintSubPauseState(DatastoreUintSubCb_t subCallback, bool isPaused);

#endif    /* DATASTORE_SRV_UTIL */

/** @} */
