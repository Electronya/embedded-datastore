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
 * @brief   The float subscriptions.
 */
static DatastoreFloatSub_t *floatSubs = NULL;

/**
 * @brief   The float subscriptions count.
 */
static uint32_t floatSubCount = 0;

/**
 * @brief   The unsigned integer subscriptions.
 */
static DatastoreUintSub_t *uintSubs = NULL;

/**
 * @brief   The unsigned subscriptions count.
 */
static uint32_t uintSubCount = 0;

/**
 * @brief   The signed integer subscriptions.
 */
static DatastoreIntSub_t *intSubs = NULL;

/**
 * @brief   The signed integer subscriptions count.
 */
static uint32_t intSubCount = 0;

/**
 * @brief   The multi-state subscriptions.
 */
static DatastoreMultiStateSub_t multiStateSubs = NULL;

/**
 * @brief   The multi-state subscriptions count.
 */
static uint32_t multiStateSubCount = 0;

/**
 * @brief   The button subscriptions.
 */
static DatastoreButtonSub_t buttonSubs = NULL;

/**
 * @brief   The buttons subscriptions count.
 */
static uint32_t buttonSubCount = 0;

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

int datastoreUtilAllocateFloatSubs(size_t maxSubCount)
{
  int err;

  floatSubs = k_malloc(maxSubs->maxFloatSubs * sizeof(DatastoreFloatSub_t));
  if(!floatSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for float subscription", err);
    return err;
  }

  return 0;
}

int datastoreUtilAllocateUintSubs(size_t maxSubCount)
{
  int err;

  uintSubs = k_malloc(maxSubs->maxUintSubs * sizeof(DatastoreUintSub_t));
  if(!uintSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for unsigned integer subscription", err);
    k_free(floatSubs);
    return err;
  }

  return 0;
}

int datastoreUtilAllocateIntSubs(size_t maxSubCount)
{
  int err;

  intSubs = k_malloc(maxSubs->maxIntSubs * sizeof(DatastoreIntSub_t));
  if(!intSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for signed integer subscription", err);
    return err;
  }

  return 0;
}

int datastoreUtilAllocateMultiStateSubs(size_t maxSubCount)
{
  int err;

  multiStateSubs = k_malloc(maxSubs->maxMultiStateSubs * sizeof(DatastoreMultiStateSub_t));
  if(!multiStateSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for multi-state subscription", err);
    return err;
  }

  return 0;
}

int datastoreUtilAllocateButtonSubs(size_t maxSubCount)
{
  int err;

  buttonSubs = k_malloc(maxSubs->maxButtonSubs * sizeof(DatastoreButtonSub_t));
  if(!buttonSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for button subscription", err);
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
  DatapointData_t *buffer;

  for(size_t i = 0; i < floatSubCount; i++)
  {
    if(!floatSubs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = floatSubs[i].datapointId; j < floatSubs[i].valCount; j++)
        buffer[j - floatSubs[i].datapointId] = floats[j];

      err = floatSubs[i].callback((float *)buffer, floatSubs[i].valCount);
      if(err < 0)
        return err;
    }
  }

  for(size_t i = 0; i < uintSubCount; i++)
  {
    if(!uintSubs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = uintSubs[i].datapointId; j < uintSubs[i].valCount; j++)
        buffer[j - uintSubs[i].datapointId] = uints[j];

      err = uintSubs[i].callback((float *)buffer, uintSubs[i].valCount);
      if(err < 0)
        return err;
    }
  }

  for(size_t i = 0; i < intSubCount; i++)
  {
    if(!intSubs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = intSubs[i].datapointId; j < intSubs[i].valCount; j++)
        buffer[j - intSubs[i].datapointId] = ints[j];

      err = intSubs[i].callback((float *)buffer, intSubs[i].valCount);
      if(err < 0)
        return err;
    }
  }

  for(size_t i = 0; i < multiStateSubCount; i++)
  {
    if(!multiStateSubs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = multiStateSubs[i].datapointId; j < multiStateSubs[i].valCount; j++)
        buffer[j - multiStateSubs[i].datapointId] = multiStates[j];

      err = multiStateSubs[i].callback((float *)buffer, multiStateSubs[i].valCount);
      if(err < 0)
        return err;
    }
  }

  for(size_t i = 0; i < buttonSubCount; i++)
  {
    if(!buttonSubs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = buttonSubs[i].datapointId; j < buttonSubs[i].valCount; j++)
        buffer[j - buttonSubs[i].datapointId] = buttons[j];

      err = buttonSubs[i].callback((float *)buffer, buttonSubs[i].valCount);
      if(err < 0)
        return err;
    }
  }

  return 0;
}

int datastoreUtilNotifyFloat(uint32_t datapointId, DatapointFloatSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool)
{
  int err;
  DatapointData_t *buffer = NULL;

  for(size_t i = 0; i < subCount; i++)
  {
    if(isFloatDatapointInSubRange(datapointId, subs + i) && !subs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = subs[i].datapointId; j < subs[i].valCount; j++)
        buffer[j - subs[i].datapointId].floatVal = floats[j].floatVal;

      err = subs[i].callback((float *)buffer, subs[i].valCount);
      if(err < 0)
        return err;
    }
  }
}

int datastoreUtilNotifyUint(uint32_t datapointId, DatapointUintSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool)
{
  int err;
  DatapointData_t *buffer = NULL;

  for(size_t i = 0; i < subCount; i++)
  {
    if(isFloatDatapointInSubRange(datapointId, subs + i) && !subs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = subs[i].datapointId; j < subs[i].valCount; j++)
        buffer[j - subs[i].datapointId].uintVal = floats[j].uintVal;

      err = subs[i].callback((float *)buffer, subs[i].valCount);
      if(err < 0)
        return err;
    }
  }
}

int datastoreUtilNotifyInt(uint32_t datapointId, DatapointIntSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool)
{
  int err;
  DatapointData_t *buffer = NULL;

  for(size_t i = 0; i < subCount; i++)
  {
    if(isFloatDatapointInSubRange(datapointId, subs + i) && !subs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = subs[i].datapointId; j < subs[i].valCount; j++)
        buffer[j - subs[i].datapointId].intVal = floats[j].intVal;

      err = subs[i].callback((float *)buffer, subs[i].valCount);
      if(err < 0)
        return err;
    }
  }
}

int datastoreUtilNotifyMultiState(uint32_t datapointId, DatapointMultiStateSub_t *subs,
                                  size_t subCount, DatastoreBufferPool_t *bufPool)
{
  int err;
  DatapointData_t *buffer = NULL;

  for(size_t i = 0; i < subCount; i++)
  {
    if(isFloatDatapointInSubRange(datapointId, subs + i) && !subs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = subs[i].datapointId; j < subs[i].valCount; j++)
        buffer[j - subs[i].datapointId].uintVal = floats[j].uintVal;

      err = subs[i].callback((float *)buffer, subs[i].valCount);
      if(err < 0)
        return err;
    }
  }
}

int datastoreUtilNotifyButton(uint32_t datapointId, DatapointButtonSub_t *subs, size_t subCount, DatastoreBufferPool_t *bufPool)
{
  int err;
  DatapointData_t *buffer = NULL;

  for(size_t i = 0; i < subCount; i++)
  {
    if(isFloatDatapointInSubRange(datapointId, subs + i) && !subs[i].isPaused)
    {
      buffer = datastoreBufPoolGet(bufPool);
      if(!buffer)
        return -ENOSPC;

      for(uint32_t j = subs[i].datapointId; j < subs[i].valCount; j++)
        buffer[j - subs[i].datapointId].uintVal = floats[j].uintVal;

      err = subs[i].callback((float *)buffer, subs[i].valCount);
      if(err < 0)
        return err;
    }
  }
}

/** @} */
