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

#endif    /* DATASTORE_SRV_UTIL */

/** @} */
