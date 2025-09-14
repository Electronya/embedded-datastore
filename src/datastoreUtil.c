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
  DatastoreMultiStateSub_t *entries;
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
  DatastoreUintSub_t *entries;
  size_t maxCount;
  size_t activeCount;
};

/**
 * @brief   The unsigned integer subscriptions.
 */
static struct DatastoreUintSub uintSubs  = {.entries = NULL, .maxCount = 0, .activeCount = 0};

/**
 * @brief   Check if the binary datapoint is in rage of the subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   sub: The subscription.
 *
 * @return  true if the datapoint is in range, false otherwise.
 */
static inline bool isBinaryDatapointInSubRange(uint32_t datapointId, DatastoreBinarySub_t *sub)
{
  return datapointId >= sub->datapointId && datapointId < sub->valCount;
}

/**
 * @brief   Notify binary subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
static inline int notifyBinarySubs(uint32_t datapointId, osMemoryPoolId_t pool)
{
  int err = 0;
  bool *buffer;

  for(size_t i = 0; i < binarySubs.activeCount && err == 0; ++i)
  {
    if(isBinaryDatapointInSubRange(datapointId, binarySubs.entries + i) && !binarySubs.entries[i].isPaused)
    {
      buffer = osMemoryPoolAlloc(pool, DATASTORE_BUFFER_ALLOC_TIMEOUT);

      for(size_t j = 0; j < binarySubs.entries[i].valCount; ++j)
        buffer[j] = (bool)binaries[binarySubs.entries[i].datapointId + j].value.uintVal;

      err = binarySubs.entries[i].callback(buffer, binarySubs.entries[i].valCount, pool);
      if(err < 0)
        LOG_ERR("ERROR %d: unable to notify for binary entry %d", err, i);
    }
  }

  return err;
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
 * @brief   Notify button subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
static inline int notifyButtonSubs(uint32_t datapointId, osMemoryPoolId_t pool)
{
  int err = 0;
  uint32_t *buffer;

  for(size_t i = 0; i < buttonSubs.activeCount && err == 0; ++i)
  {
    if(isButtonDatapointInSubRange(datapointId, buttonSubs.entries + i) && !buttonSubs.entries[i].isPaused)
    {
      buffer = osMemoryPoolAlloc(pool, DATASTORE_BUFFER_ALLOC_TIMEOUT);

      for(size_t j = 0; j < buttonSubs.entries[i].valCount; ++j)
        buffer[j] = buttons[buttonSubs.entries[i].datapointId + j].value.uintVal;

      err = buttonSubs.entries[i].callback(buffer, buttonSubs.entries[i].valCount, pool);
      if(err < 0)
        LOG_ERR("ERROR %d: unable to notify for button entry %d", err, i);
    }
  }

  return err;
}

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
 * @brief   Notify float subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
static inline int notifyFloatSubs(uint32_t datapointId, osMemoryPoolId_t pool)
{
  int err = 0;
  float *buffer;

  for(size_t i = 0; i < floatSubs.activeCount && err == 0; ++i)
  {
    if(isFloatDatapointInSubRange(datapointId, floatSubs.entries + i) && !floatSubs.entries[i].isPaused)
    {
      buffer = osMemoryPoolAlloc(pool, DATASTORE_BUFFER_ALLOC_TIMEOUT);

      for(size_t j = 0; j < floatSubs.entries[i].valCount; ++j)
        buffer[j] = floats[floatSubs.entries[i].datapointId + j].value.floatVal;

      err = floatSubs.entries[i].callback(buffer, floatSubs.entries[i].valCount, pool);
      if(err < 0)
        LOG_ERR("ERROR %d: unable to notify for float entry %d", err, i);
    }
  }

  return err;
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
 * @brief   Notify signed integer subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
static inline int notifyIntSubs(uint32_t datapointId, osMemoryPoolId_t pool)
{
  int err = 0;
  int32_t *buffer;

  for(size_t i = 0; i < intSubs.activeCount && err == 0; ++i)
  {
    if(isIntDatapointInSubRange(datapointId, intSubs.entries + i) && !intSubs.entries[i].isPaused)
    {
      buffer = osMemoryPoolAlloc(pool, DATASTORE_BUFFER_ALLOC_TIMEOUT);

      for(size_t j = 0; j < intSubs.entries[i].valCount; ++j)
        buffer[j] = ints[intSubs.entries[i].datapointId + j].value.intVal;

      err = intSubs.entries[i].callback(buffer, intSubs.entries[i].valCount, pool);
      if(err < 0)
        LOG_ERR("ERROR %d: unable to notify for signed integer entry %d", err, i);
    }
  }

  return err;
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
 * @brief   Notify multi-state subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
static inline int notifyMultiStateSubs(uint32_t datapointId, osMemoryPoolId_t pool)
{
  int err = 0;
  uint32_t *buffer;

  for(size_t i = 0; i < multiStateSubs.activeCount && err == 0; ++i)
  {
    if(isMultiStateDatapointInSubRange(datapointId, multiStateSubs.entries + i) && !multiStateSubs.entries[i].isPaused)
    {
      buffer = osMemoryPoolAlloc(pool, DATASTORE_BUFFER_ALLOC_TIMEOUT);

      for(size_t j = 0; j < multiStateSubs.entries[i].valCount; ++j)
        buffer[j] = multiStates[multiStateSubs.entries[i].datapointId + j].value.uintVal;

      err = multiStateSubs.entries[i].callback(buffer, multiStateSubs.entries[i].valCount, pool);
      if(err < 0)
        LOG_ERR("ERROR %d: unable to notify for multi-state entry %d", err, i);
    }
  }

  return err;
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
 * @brief   Notify unsigned integer subscription.
 *
 * @param[in]   datapointId: The datapoint ID.
 * @param[in]   pool: The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
static inline int notifyUintSubs(uint32_t datapointId, osMemoryPoolId_t pool)
{
  int err = 0;
  uint32_t *buffer;

  for(size_t i = 0; i < uintSubs.activeCount && err == 0; ++i)
  {
    if(isUintDatapointInSubRange(datapointId, uintSubs.entries + i) && !uintSubs.entries[i].isPaused)
    {
      buffer = osMemoryPoolAlloc(pool, DATASTORE_BUFFER_ALLOC_TIMEOUT);

      for(size_t j = 0; j < uintSubs.entries[i].valCount; ++j)
        buffer[j] = uints[uintSubs.entries[i].datapointId + j].value.uintVal;

      err = uintSubs.entries[i].callback(buffer, uintSubs.entries[i].valCount, pool);
      if(err < 0)
        LOG_ERR("ERROR %d: unable to notify for binary entry %d", err, i);
    }
  }

  return err;
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
  return datapointId < datapointCount && datapointId + valCount <= datapointCount;
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

size_t datastoreUtilCalculateBufferSize(size_t datapointCounts[DATAPOINT_TYPE_COUNT])
{
  size_t bufferSize = 0;

  for(uint32_t i = 0; i < DATAPOINT_TYPE_COUNT; ++i)
  {
    bufferSize = bufferSize < datapointCounts[i] ? datapointCounts[i] : bufferSize;
  }

  return bufferSize * sizeof(Datapoint_t);
}

int datastoreUtilAddBinarySub(DatastoreBinarySub_t *sub)
{
  int err;

  if(binarySubs.activeCount + 1 >= binarySubs.maxCount)
  {
    err = -ENOBUFS;
    LOG_ERR("ERROR %d: unable to add new binary subscription, entries full", err);
    return err;
  }

  ++binarySubs.activeCount;
  memcpy(binarySubs.entries + binarySubs.activeCount, sub, sizeof(DatastoreBinarySub_t));

  return 0;
}

int datastoreUtilSetBinarySubPauseState(DatastoreBinarySubCb_t subCallback, bool isPaused)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  for(size_t i = 0; i < binarySubs.activeCount && err < 0; ++i)
  {
    if(binarySubs.entries[i].callback == subCallback)
    {
      binarySubs.entries[i].isPaused = isPaused;

      if(isPaused)
        LOG_INF("binary subscription entry %d paused", i);
      else
        LOG_INF("binary subscription entry %d unpaused", i);

      err = 0;
    }
  }

  if(err < 0)
    LOG_WRN("ERROR %d: unable to find binary subscription %p", err, subCallback);

  return err;
}

int datastoreUtilAddButtonSub(DatastoreButtonSub_t *sub)
{
  int err;

  if(buttonSubs.activeCount + 1 >= buttonSubs.maxCount)
  {
    err = -ENOBUFS;
    LOG_ERR("ERROR %d: unable to add new button subscription, entries full", err);
    return err;
  }

  memcpy(buttonSubs.entries + buttonSubs.activeCount, sub, sizeof(DatastoreButtonSub_t));
  ++buttonSubs.activeCount;

  return 0;
}

int datastoreUtilSetButtonSubPauseState(DatastoreButtonSubCb_t subCallback, bool isPaused)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  for(size_t i = 0; i < buttonSubs.activeCount && err < 0; ++i)
  {
    if(buttonSubs.entries[i].callback == subCallback)
    {
      buttonSubs.entries[i].isPaused = isPaused;

      if(isPaused)
        LOG_INF("button subscription entry %d paused", i);
      else
        LOG_INF("button subscription entry %d unpaused", i);

      err = 0;
    }
  }

  if(err < 0)
    LOG_WRN("ERROR %d: unable to find button subscription %p", err, subCallback);

  return err;
}

int datastoreUtilAddFloatSub(DatastoreFloatSub_t *sub)
{
  int err;

  if(floatSubs.activeCount + 1 >= floatSubs.maxCount)
  {
    err = -ENOBUFS;
    LOG_ERR("ERROR %d: unable to add new button subscription, entries full", err);
    return err;
  }

  memcpy(floatSubs.entries + floatSubs.activeCount, sub, sizeof(DatastoreFloatSub_t));
  ++floatSubs.activeCount;

  return 0;
}

int datastoreUtilSetFloatSubPauseState(DatastoreFloatSubCb_t subCallback, bool isPaused)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  for(size_t i = 0; i < floatSubs.activeCount && err < 0; ++i)
  {
    if(floatSubs.entries[i].callback == subCallback)
    {
      floatSubs.entries[i].isPaused = isPaused;

      if(isPaused)
        LOG_INF("float subscription entry %d paused", i);
      else
        LOG_INF("float subscription entry %d unpaused", i);

      err = 0;
    }
  }

  if(err < 0)
    LOG_WRN("ERROR %d: unable to find float subscription %p", err, subCallback);

  return err;
}

int datastoreUtilAddIntSub(DatastoreIntSub_t *sub)
{
  int err;

  if(intSubs.activeCount + 1 >= intSubs.maxCount)
  {
    err = -ENOBUFS;
    LOG_ERR("ERROR %d: unable to add new signed integer subscription, entries full", err);
    return err;
  }

  memcpy(intSubs.entries + intSubs.activeCount, sub, sizeof(DatastoreIntSub_t));
  ++intSubs.activeCount;

  return 0;
}

int datastoreUtilSetIntSubPauseState(DatastoreIntSubCb_t subCallback, bool isPaused)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  for(size_t i = 0; i < intSubs.activeCount && err < 0; ++i)
  {
    if(intSubs.entries[i].callback == subCallback)
    {
      intSubs.entries[i].isPaused = isPaused;

      if(isPaused)
        LOG_INF("signed integer subscription entry %d paused", i);
      else
        LOG_INF("signed integer subscription entry %d unpaused", i);

      err = 0;
    }
  }

  if(err < 0)
    LOG_WRN("ERROR %d: unable to find signed integer subscription %p", err, subCallback);

  return err;
}

int datastoreUtilAddMultiStateSub(DatastoreMultiStateSub_t *sub)
{
  int err;

  if(multiStateSubs.activeCount + 1 >= multiStateSubs.maxCount)
  {
    err = -ENOBUFS;
    LOG_ERR("ERROR %d: unable to add new multi-state subscription, entries full", err);
    return err;
  }

  memcpy(multiStateSubs.entries + multiStateSubs.activeCount, sub, sizeof(DatastoreMultiStateSub_t));
  ++multiStateSubs.activeCount;

  return 0;
}

int datastoreUtilSetMultiStateSubPauseState(DatastoreMultiStateSubCb_t subCallback, bool isPaused)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  for(size_t i = 0; i < multiStateSubs.activeCount && err < 0; ++i)
  {
    if(multiStateSubs.entries[i].callback == subCallback)
    {
      multiStateSubs.entries[i].isPaused = isPaused;

      if(isPaused)
        LOG_INF("multi-state subscription entry %d paused", i);
      else
        LOG_INF("multi-state subscription entry %d unpaused", i);

      err = 0;
    }
  }

  if(err < 0)
    LOG_WRN("ERROR %d: unable to find multi-state subscription %p", err, subCallback);

  return err;
}

int datastoreUtilAddUintSub(DatastoreUintSub_t *sub)
{
  int err;

  if(uintSubs.activeCount + 1 >= uintSubs.maxCount)
  {
    err = -ENOBUFS;
    LOG_ERR("ERROR %d: unable to add new unsigned integer subscription, entries full", err);
    return err;
  }

  memcpy(uintSubs.entries + uintSubs.activeCount, sub, sizeof(DatastoreUintSub_t));
  ++uintSubs.activeCount;

  return 0;
}

int datastoreUtilSetUintSubPauseState(DatastoreUintSubCb_t subCallback, bool isPaused)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  for(size_t i = 0; i < uintSubs.activeCount && err < 0; ++i)
  {
    if(uintSubs.entries[i].callback == subCallback)
    {
      uintSubs.entries[i].isPaused = isPaused;

      if(isPaused)
        LOG_INF("unsigned integer subscription entry %d paused", i);
      else
        LOG_INF("unsigned integer subscription entry %d unpaused", i);

      err = 0;
    }
  }

  if(err < 0)
    LOG_WRN("ERROR %d: unable to find unsigned integer subscription %p", err, subCallback);

  return err;
}

int datastoreUtilRead(DatapointType_t type, uint32_t datapointId, size_t valCount, DatapointValue_t values[])
{
  int err;
  Datapoint_t *root = datapoints[type];

  if(!isDatapointIdAndValCountValid(datapointId, valCount, datapointCounts[type]))
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid datapoint ID %d or value count %d", err, datapointId, valCount);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    values[i] = root[datapointId + i].value;

  return 0;
}

int datastoreUtilWrite(DatapointType_t type, uint32_t datapointId, DatapointValue_t values[],
                       size_t valCount, osMemoryPoolId_t pool)
{
  int err = 0;
  bool needToNotify = false;
  Datapoint_t *root = datapoints[type];

  if(!isDatapointIdAndValCountValid(datapointId, valCount, datapointCounts[type]))
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid datapoint ID %d or value count %d", err, datapointId, valCount);
  }
  else
  {
    for(size_t i = 0; i < valCount; ++i)
    {
      needToNotify = !needToNotify && values[i].uintVal == root[i].value.uintVal ? true : needToNotify;
      root[i].value = values[i];
    }
  }

  osMemoryPoolFree(pool, values);

  if(needToNotify)
  {
    err = datastoreUtilNotify(type, datapointId, pool);
    if(err)
      LOG_ERR("ERROR %d: unable to notify", err);
  }

  return err;
}

int datastoreUtilNotify(DatapointType_t type, uint32_t datapointId, osMemoryPoolId_t pool)
{
  int err;

  switch(type)
  {
    case DATAPOINT_BINARY:
      err = notifyBinarySubs(datapointId, pool);
    break;
    case DATAPOINT_BUTTON:
      err = notifyButtonSubs(datapointId, pool);
    break;
    case DATAPOINT_FLOAT:
      err = notifyFloatSubs(datapointId, pool);
    break;
    case DATAPOINT_INT:
      err = notifyIntSubs(datapointId, pool);
    break;
    case DATAPOINT_MULTI_STATE:
      err = notifyMultiStateSubs(datapointId, pool);
    break;
    case DATAPOINT_UINT:
      err = notifyUintSubs(datapointId, pool);
    break;
    default:
      err = -ENOTSUP;
      LOG_ERR("ERROR %d: unsupported datapoint type %d", err, type);
    break;
  }

  return err;
}

/** @} */
