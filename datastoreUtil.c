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
LOG_MODULE_REGISTER(DATASTORE_LOGGER_NAME);

/**
 * @brief   Float datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t floats[] = {
#define X(name, flags, defaultVal) {.data.floatVal = defaultVal, .flags = flags}
  DATASTORE_FLOAT_DATAPOINTS
#undef
};

/**
 * @brief   Unsigned integer datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t uints[] = {
#define X(name, flags, defaultVal) {.data.uintVal = defaultVal, .flags = flags}
  DATASTORE_UINT_DATAPOINTS
#undef
};

/**
 * @brief   Singed integer datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t ints[] = {
#define X(name, flags, defaultVal) {.data.intVal = defaultVal, .flags = flags}
  DATASTORE_FLOAT_DATAPOINTS
#undef
};

/**
 * @brief   Multi-state datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t multiStates[] = {
#define X(name, flags, defaultVal) {.data.uintVal = defaultVal, .flags = flags}
  DATASTORE_MULTI_STATE_DATAPOINTS
#undef
};

/**
 * @brief   Button datapoints.
 * @note    Data is coming from X-macros in datastoreMeta.h
 */
static Datapoint_t buttons[] = {
#define X(name, flags, defaultVal) {.data.uintVal = defaultVal, .flags = flags}
  DATASTORE_FLOAT_DATAPOINTS
#undef
};

/**
 * @brief   The list of datapoint for each data type.
 */
static Datapoint_t **datapoints[DATAPOINT_TYPE_COUNT] = {floats, uints, ints, multiStates, buttons};

/**
 * @brief   The datapoint count of each data type.
 */
static size_t datapointCounts[DATAPOINT_TYPE_COUNT] = {FLOAT_DATAPOINT_COUNT, UINT_DATAPOINT_COUNT, INT_DATAPOINT_COUNT,
                                                       MULTI_STATE_DATAPOINT_COUNT, BUTTON_DATAPOINT_COUNT};

/**
 * @brief   The float subscriptions.
 */
static GenericSubscription_t *floatSubs = NULL;

/**
 * @brief   The unsigned integer subscriptions.
 */
static GenericSubscription_t *uintSubs = NULL;

/**
 * @brief   The signed integer subscriptions.
 */
static GenericSubscription_t *intSubs = NULL;

/**
 * @brief   The multi-state subscriptions.
 */
static GenericSubscription_t *multiStateSubs = NULL;

/**
 * @brief   The button subscriptions.
 */
static GenericSubscription_t *buttonSubs = NULL;

/**
 * @brief   The list of subscription for each data type.
 */
static GenericSubscription_t *subscriptions[DATAPOINT_TYPE_COUNT] = {floatSubs, uintSubs, intSubs, multiStateSubs, buttonSubs};

/**
 * @brief   The maximum count of subscriptions for each data type.
 */
static size_t subMaxCounts[DATAPOINT_TYPE_COUNT] = {0};

/**
 * @brief   The count of subscriptions for each data type.
 */
static size_t subCounts[DATAPOINT_TYPE_COUNT] = {0};

/**
 * @brief   The datastore buffer pool.
 */
static DatastoreBufferPool_t *bufPool;

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

int datastoreUtilAllocateSubs(DatapointType_t datapointType, size_t maxSubCount)
{
  int err;
  GenericSubscription_t *subs;

  if(datapointType >= DATAPOINT_TYPE_COUNT)
  {
    err = -ENOTSUP;
    LOG_ERR("ERROR %d: unsupported data type %d", err, datapointType);
    return err;
  }

  subs = subscriptions[datapointType];
  subMaxCounts[datapointType] = maxSubCount;

  subs = k_malloc(maxSubCount * sizeof(GenericSubscription_t));
  if(!floatSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for float subscription", err);
    return err;
  }

  return 0;
}

int datastoreUtilInitBufferPool(DatastoreMaxSubs_t *maxSubs)
{
  size_t poolSize = maxSubs->maxFloatSubs;
  size_t bufSize = FLOAT_DATAPOINT_COUNT;

  poolSize = maxSubs->maxUintSubs > poolSize ? maxSubs->maxUintSubs : poolSize;
  poolSize = maxSubs->maxIntSubs > poolSize ? maxSubs->maxIntSubs : poolSize;
  poolSize = maxSubs->maxMultiStateSubs > poolSize ? maxSubs->maxMultiStateSubs : poolSize;
  poolSize = maxSubs->maxButtonSubs > poolSize ? maxSubs->maxButtonSubs : poolSize;

  bufSize = UINT_DATAPOINT_COUNT > bufSize ? UINT_DATAPOINT_COUNT : bufSize;
  bufSize = INT_DATAPOINT_COUNT > bufSize ? INT_DATAPOINT_COUNT : bufSize;
  bufSize = MULTI_STATE_DATAPOINT_COUNT > bufSize ? MULTI_STATE_DATAPOINT_COUNT : bufSize;
  bufSize = BUTTON_DATAPOINT_COUNT > bufSize ? BUTTON_DATAPOINT_COUNT : bufSize;

  bufPool = datastoreBufPoolInit(bufSize + DATASTORE_MSG_COUNT, poolSize + DATASTORE_MSG_COUNT);
  if(!bufPool)
    return -ENOSPC;

  return 0;
}

int datastoreUtilDoInitNotifications(void)
{
  int err;
  GenericSubscription_t *subs;
  size_t subCount;
  DatapointData_t *buffer;

  for(uint32_t i = 0; i < DATAPOINT_TYPE_COUNT; i++)
  {
    subs = subscriptions[i];
    subCount = subCounts[i];

    for(uint32_t i = 0; i < subCount; ++i)
    {
      if(!subs[i].isPaused)
      {
        buffer = datastoreBufPoolGet(bufPool);
        if(!buffer)
          return -ENOSPC;

        for(uint32_t j = subs[i].datapointId; j < subs[i].datapointId + subs[i].valCount; ++j)
          buffer[j - subs[i].datapointId] = floats[j];

        err = subs[i].callback(buffer, subs[i].valCount);
        if(err < 0)
          return err;
      }
    }
  }

  return 0;
}

int datastoreUtilAddSubscription(DatapointType_t datapointType, GenericSubscription_t *sub)
{
  int err;

  if(datapointType >= DATAPOINT_TYPE_COUNT)
  {
    err = -ENOTSUP;
    LOG_ERR("ERROR %d: unsupported data type %d", err, datapointType);
    return err;
  }

  if(!subscriptions[datapointType])
  {
    err = -EACCES;
    LOG_ERR("ERROR %d: subscription records not initialized", err);
    return err;
  }

  if(subCounts[datapointType] + 1 >= subMaxCounts[datapointType])
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: no more free float subscription record", err);
    return err;
  }

  if(!sub)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid new float subscription record", err);
    return err;
  }

  memcpy(subscriptions[datapointType] + subCounts[datapointType], sub, sizeof(GenericSubscription_t));
  ++subCounts[datapointType];

  return 0;
}

int datastoreUtilPauseSubscription(DatapointType_t datapointType, GenericCallback_t callback)
{
  int err = -ESRCH;
  GenericSubscription_t *subs;

  if(datapointType >= DATAPOINT_TYPE_COUNT)
  {
    err = -ENOTSUP;
    LOG_ERR("ERROR %d: unsupported data type %d", err, datapointType);
    return err;
  }

  if(!callback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  subs = subscriptions[datapointType];

  for(uint32_t i = 0; i < subCounts[datapointType]; ++i)
  {
    if(subs[i].callback == callback)
    {
      err = 0;
      subs[i].isPaused = true;
    }
  }

  return err;
}

int datastoreUtilUnpauseSubscription(DatapointType_t datapointType, GenericCallback_t callback)
{
  int err = -ESRCH;
  GenericSubscription_t *subs;

  if(datapointType >= DATAPOINT_TYPE_COUNT)
  {
    err = -ENOTSUP;
    LOG_ERR("ERROR %d: unsupported data type %d", err, datapointType);
    return err;
  }

  if(!callback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid subscription callback", err);
    return err;
  }

  subs = subscriptions[datapointType];

  for(uint32_t i = 0; i < subCounts[datapointType]; ++i)
  {
    if(subs[i].callback == callback)
    {
      err = 0;
      subs[i].isPaused = false;
    }
  }

  return err;
}

int datastoreUtilNotify(DatapointType_t datapointType, uint32_t datapointId)
{
  int err;
  GenericSubscription_t *subs;
  DatapointData_t *buffer = NULL;

  if(datapointType >= DATAPOINT_TYPE_COUNT)
  {
    err = -ENOTSUP;
    LOG_ERR("ERROR %d: unsupported data type %d", err, datapointType);
    return err;
  }

  subs = subscriptions[datapointType];

  for(size_t i = 0; i < subCounts[datapointType]; ++i)
  {
    if(isFloatDatapointInSubRange(datapointId, subscriptions[datapointType] + i) && !subscriptions[datapointType][i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = subs[i].datapointId; j < subs[i].datapointId + subs[i].valCount; ++j)
        buffer[j - subs[i].datapointId] = floats[j];

      err = subs[i].callback(buffer, subs[i].valCount);
      if(err < 0)
        return err;
    }
  }
}

int datastoreUtilReadData(DatapointType_t datapointType, uint32_t datapointId, size_t valCount, DatapointData_t values[])
{
  int err;

  if(datapointType >= DATAPOINT_TYPE_COUNT)
  {
    err = -ENOTSUP;
    LOG_ERR("ERROR %d: unsupported data type %d", err, datapointType);
    return err;
  }

  if(isDatapointIdAndValCountValid(datapointId, valCount, datapointCounts[datapointType]))
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: reading more data than available", err);
    return err;
  }

  for(uint32_t i = datapointId; i < datapointId + valCount; ++i)
    values[i] = datapoints[datapointType][i];

  return 0;
}

int datastoreUtilWriteData(DatapointType_t datapointType, uint32_t datapointId,
                           DatapointData_t values[], size_t valCount, bool *needToNotify)
{
  int err;

  if(datapointType >= DATAPOINT_TYPE_COUNT)
  {
    err = -ENOTSUP;
    LOG_ERR("ERROR %d: unsupported data type %d", err, datapointType);
    return err;
  }

  if(isDatapointIdAndValCountValid(datapointId, valCount, datapointCounts[datapointType]))
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: writing more data than available", err);
    return err;
  }

  *needToNotify = false;

  for(uint32_t i = datapointId; i < datapointId + valCount; ++i)
  {
    if(datapoints[datapointType][i] != values[i])
    {
      datapoints[datapointType][i] = values[i];
      *needToNotify = true;
    }
  }

  return 0;
}

/** @} */
