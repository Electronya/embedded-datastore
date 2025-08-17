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

int datastoreUtilInitBufferPool(DatastoreMaxSubs_t *maxSubs, DatastoreBufferPool_t **pool)
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

  *pool = datastoreBufPoolInit(bufSize + DATASTORE_MSG_COUNT, poolSize + DATASTORE_MSG_COUNT);
  if(!pool)
    return -ENOSPC;

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
