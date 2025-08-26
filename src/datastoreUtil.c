/**
 * Copyright (C) 2025 by Electronya
 *
 * @file      datastoreUtils.c
 * @author    jbacon
 * @date      2025-08-17
 * @brief     Datastore Utilities Implementation
 *
 *            Datastore utility functions implementation.
 *
 * @ingroup   datastore
 * @{
 */

#include <zephyr/logging/log.h>

#include "datastoreUtil.h"

/* Setting module logging */
LOG_MODULE_DECLARE(DATASTORE_LOGGER_NAME);

/**
 * @brief   Binary datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t binaries[] = {
#define X(id, optFlags, defaultVal) {.value.uintVal = defaultVal, .flags = optFlags},
  DATASTORE_BINARY_DATAPOINTS
#undef X
};

/**
 * @brief   Button datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t buttons[] = {
#define X(id, optFlags, defaultVal) {.value.uintVal = defaultVal, .flags = optFlags},
  DATASTORE_BUTTON_DATAPOINTS
#undef X
};

/**
 * @brief   Float datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t floats[] = {
#define X(id, optFlags, defaultVal) {.value.floatVal = defaultVal, .flags = optFlags},
  DATASTORE_FLOAT_DATAPOINTS
#undef X
};

/**
 * @brief   Singed integer datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t ints[] = {
#define X(id, optFlags, defaultVal) {.value.intVal = defaultVal, .flags = optFlags},
  DATASTORE_INT_DATAPOINTS
#undef X
};

/**
 * @brief   Multi-state datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t multiStates[] = {
#define X(id, optFlags, defaultVal) {.value.uintVal = defaultVal, .flags = optFlags},
  DATASTORE_MULTI_STATE_DATAPOINTS
#undef X
};

/**
 * @brief   Unsigned integer datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t uints[] = {
#define X(id, optFlags, defaultVal) {.value.uintVal = defaultVal, .flags = optFlags},
  DATASTORE_UINT_DATAPOINTS
#undef X
};

/**
 * @brief   The list of datapoint for each value type.
 */
static Datapoint_t *datapoints[DATAPOINT_TYPE_COUNT] = {binaries, buttons, floats, ints, multiStates, uints};

/**
 * @brief   The datapoint count of each value type.
 */
static size_t datapointCounts[DATAPOINT_TYPE_COUNT] = {BINARY_DATAPOINT_COUNT, BUTTON_DATAPOINT_COUNT,
                                                       FLOAT_DATAPOINT_COUNT, INT_DATAPOINT_COUNT,
                                                       MULTI_STATE_DATAPOINT_COUNT, UINT_DATAPOINT_COUNT};

/**
 * @brief   The binary subscription structure.
 */
struct DatastoreBinarySub
{
  DatastoreBinarySub_t *entries;
  size_t maxCount;
  size_t activeCount;
};

/**
 * @brief   The binary subscriptions.
 */
static struct DatastoreBinarySub binarySubs  = {.entries = NULL, .maxCount = 0, .activeCount = 0};

/**
 * @brief   The button subscription structure.
 */
struct DatastoreButtonSub
{
  DatastoreButtonSub_t *entries;
  size_t maxCount;
  size_t activeCount;
};

/**
 * @brief   The button subscriptions.
 */
static struct DatastoreButtonSub buttonSubs  = {.entries = NULL, .maxCount = 0, .activeCount = 0};

/**
 * @brief   The float subscription structure.
 */
struct DatastoreFloatSub
{
  DatastoreFloatSub_t *entries;
  size_t maxCount;
  size_t activeCount;
};

/**
 * @brief   The float subscriptions.
 */
static struct DatastoreFloatSub floatSubs  = {.entries = NULL, .maxCount = 0, .activeCount = 0};

/**
 * @brief   The signed integer subscription structure.
 */
struct DatastoreIntSub
{
  DatastoreIntSub_t *entries;
  size_t maxCount;
  size_t activeCount;
};

/**
 * @brief   The signed integer subscriptions.
 */
static struct DatastoreIntSub intSubs  = {.entries = NULL, .maxCount = 0, .activeCount = 0};

/**
 * @brief   The multi-state subscription structure.
 */
struct DatastoreMultiStateSub
{
  DatastoreIntSub_t *entries;
  size_t maxCount;
  size_t activeCount;
};

/**
 * @brief   The multi-state subscriptions.
 */
static struct DatastoreMultiStateSub multiStateSubs  = {.entries = NULL, .maxCount = 0, .activeCount = 0};

/**
 * @brief   The unsigned integer subscription structure.
 */
struct DatastoreUintSub
{
  DatastoreIntSub_t *entries;
  size_t maxCount;
  size_t activeCount;
};

/**
 * @brief   The unsigned integer subscriptions.
 */
static struct DatastoreUintSub uintSubs  = {.entries = NULL, .maxCount = 0, .activeCount = 0};

/**
 * @brief   Check if the float datapoint is in rage of the subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   sub: The subscription.
 *
 * @return  true if the datapoint is in range, false otherwise.
 */
static inline bool isFloatDatapointInSubRange(uint32_t datapointId, DatastoreFloatSub_t *sub)
{
  return datapointId >= sub->datapointId && datapointId < sub->valCount;
}

/**
 * @brief   Check if the unsigned integer datapoint is in rage of the subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   sub: The subscription.
 *
 * @return  true if the datapoint is in range, false otherwise.
 */
static inline bool isUintDatapointInSubRange(uint32_t datapointId, DatastoreUintSub_t *sub)
{
  return datapointId >= sub->datapointId && datapointId < sub->valCount;
}

/**
 * @brief   Check if the signed integer datapoint is in rage of the subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   sub: The subscription.
 *
 * @return  true if the datapoint is in range, false otherwise.
 */
static inline bool isIntDatapointInSubRange(uint32_t datapointId, DatastoreIntSub_t *sub)
{
  return datapointId >= sub->datapointId && datapointId < sub->valCount;
}

/**
 * @brief   Check if the multi-state datapoint is in rage of the subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   sub: The subscription.
 *
 * @return  true if the datapoint is in range, false otherwise.
 */
static inline bool isMultiStateDatapointInSubRange(uint32_t datapointId, DatastoreMultiStateSub_t *sub)
{
  return datapointId >= sub->datapointId && datapointId < sub->valCount;
}

/**
 * @brief   Check if the button datapoint is in rage of the subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   sub: The subscription.
 *
 * @return  true if the datapoint is in range, false otherwise.
 */
static inline bool isButtonDatapointInSubRange(uint32_t datapointId, DatastoreButtonSub_t *sub)
{
  return datapointId >= sub->datapointId && datapointId < sub->valCount;
}

/**
 * @brief   Check if the datapoint ID and the value count are valid.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   valCount: The value count.
 * @param[in]   datapointCount: The datapoint count.
 *
 * @return  true if the datapoint ID and value count are valid, false otherwise.
 */
static inline bool isDatapointIdAndValCountValid(uint32_t datapointId, size_t valCount, size_t datapointCount)
{
  return datapointId >= datapointCount && datapointId + valCount >= datapointCount;
}
/* ------------------------------------------------------------------------- */

int datastoreUtilAllocateBinarySubs(size_t maxSubCount)
{
  int err;

  binarySubs.entries = k_malloc(maxSubCount * sizeof(DatastoreBinarySub_t));
  if(!binarySubs.entries)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for binary subscriptions", err);
    return err;
  }

  binarySubs.maxCount = maxSubCount;

  return 0;
}

int datastoreUtilAllocateButtonSubs(size_t maxSubCount)
{
  int err;

  buttonSubs.entries = k_malloc(maxSubCount * sizeof(DatastoreButtonSub_t));
  if(!buttonSubs.entries)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for button subscriptions", err);
    return err;
  }

  buttonSubs.maxCount = maxSubCount;

  return 0;
}

int datastoreUtilAllocateFloatSubs(size_t maxSubCount)
{
  int err;

  floatSubs.entries = k_malloc(maxSubCount * sizeof(DatastoreFloatSub_t));
  if(!floatSubs.entries)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for float subscriptions", err);
    return err;
  }

  floatSubs.maxCount = maxSubCount;

  return 0;
}

int datastoreUtilAllocateIntSubs(size_t maxSubCount)
{
  int err;

  intSubs.entries = k_malloc(maxSubCount * sizeof(DatastoreIntSub_t));
  if(!intSubs.entries)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for signed integer subscriptions", err);
    return err;
  }

  intSubs.maxCount = maxSubCount;

  return 0;
}

int datastoreUtilAllocateMultiStateSubs(size_t maxSubCount)
{
  int err;

  multiStateSubs.entries = k_malloc(maxSubCount * sizeof(DatastoreMultiStateSub_t));
  if(!multiStateSubs.entries)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for multi-state subscriptions", err);
    return err;
  }

  multiStateSubs.maxCount = maxSubCount;

  return 0;
}

int datastoreUtilAllocateUintSubs(size_t maxSubCount)
{
  int err;

  uintSubs.entries = k_malloc(maxSubCount * sizeof(DatastoreUintSub_t));
  if(!uintSubs.entries)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for multi-state subscriptions", err);
    return err;
  }

  uintSubs.maxCount = maxSubCount;

  return 0;
}

/** @} */
