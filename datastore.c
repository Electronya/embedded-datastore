/**
 * Copyright (C) 2025 by Electronya
 *
 * @file      datastore.c
 * @author    jbacon
 * @date      2025-08-10
 * @brief     Datastore Service Implementation
 *
 *            Datastore service implementation.
 *
 * @ingroup   datastore
 * @{
 */

#include <zephyr/logging/log.h>
#include <string.h>

#include "datastore.h"
#include "datastoreBufferPool.h"
#include "datastoreUtil.h"

/* Setting module logging */
LOG_MODULE_REGISTER(DATASTORE_LOGGER_NAME);

#define DATASTORE_STACK_SIZE                                    (256)
#define DATASTORE_RESPONSE_TIMEOUT                              (5)

/**
 * @brief The thread stack.
*/
K_THREAD_STACK_DEFINE(datastoreStack, DATASTORE_STACK_SIZE);

typedef enum
{
  DATASTORE_READ = 0,
  DATASTORE_WRITE,
  DATASTORE_MSG_TYPE_COUNT,
} datastoreMsgtype_t;

typedef struct
{
  datastoreMsgtype_t msgType;
  DatapointType_t dataType;
  uint32_t dataId;
  DatapointData_t *data;
  size_t valCount;
  struct K_msgq *response;
} DatastoreMsg_t;

/**
 * @brief   The service thread.
 */
static k_thread thread;

/**
 * @brief   The list of all the datapoint classified by type.
 */
static Datapoint_t **datapoints[] = {floats, uints, ints, multiStates, buttons};

K_MSGQ_DEFINE(datastoreQueue, sizeof(DatastoreMsg_t), DATASTORE_MSG_COUNT, 4);

/**
 * @brief   Do the initial notifications.
 *
 * @return  0 if successful, the error code otherwise.
 */
static int doInitialNotifications(void)
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

/**
 * @brief   The datastore service thread function.
 *
 * @param[in]   p1: Thread first parameter.
 * @param[in]   p2: Thread second parameter.
 * @param[in]   p3: Thread third parameter.
 */
static void run(void *p1, void *p2, void *p3)
{
  int err;
  DatastoreMsg_t msg;
  Datapoint_t *datapointTypes[DATAPOINT_TYPE_COUNT] = {floats, uints, ints, multiStates, buttons};

  // TODO: Initialize the datapoints from the NVM.

  err = doInitialNotifications();
  if(err < 0)
    LOG_ERR("ERROR %d: unable to make initial notifications", err);

  for(;;)
  {
    err = k_msgq_get(&datastoreQueue, &msg, K_FOREVER);
    if(err < 0)
    {
      LOG_ERR("ERROR %d: unable to get a message", err);
      continue;
    }

    switch(msg.msgType)
    {
      case DATASTORE_READ:
      break;
      case DATASTORE_WRITE:
      break;
      default:
        LOG_WRN("unsupported message type %d", msg.msgType);
      break;
    }
  }
}

int datastoreInit(DatastoreMaxSubs_t *maxSubs, size_t maxBufferSize, uint32_t priority, k_tid_t *threadId)
{
  int err;

  err = datastoreUtilAllocateFloatSubs(maxSubs->maxFloatSubs);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateUintSubs(maxSubs->maxUintSubs);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateIntSubs(maxSubs->maxIntSubs);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateMultiStateSubs(maxSubs->maxMultiStateSubs);
  if(err < 0)
    return 0;

  err = datastoreUtilAllocateButtonSubs(maxSubs->maxButtonSubs);
  if(err < 0)
    return err;

  err = datastoreUtilInitBufferPool(maxSubs);
  if(err < 0)
    return err;

  *threadId = k_thread_create(&thread, datastoreStack, DATASTORE_STACK_SIZE, run,
                              NULL, NULL, NULL, K_PRIO_PREEMPT(priority), 0, K_FOREVER);

  err = k_thread_name_set(*threadId, "datastore");
  if(err< 0)
    LOG_ERR("ERROR %d: unable to set datastore thread name", err)

  return err;
}

int datastoreSubscribeFloat(DatastoreFloatSub_t *sub)
{
  int err;

  if(!floatSubs)
  {
    err = -EACCES;
    LOG_ERR("ERROR %d: float subscription records not initialized", err);
    return err;
  }

  if(floatSubCount >= FLOAT_DATAPOINT_COUNT)
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

  memcpy(floatSubs + floatSubCount, sub, sizeof(DatastoreFloatSub_t));
  floatSubCount++;

  return 0;
}

int datastorePauseSubFloat(DatastoreFloatSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid float subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < floatSubCount; i++)
  {
    if(floatSubs[i].callback == subCallback)
    {
      err = 0;
      floatSubs[i].isPaused = true;
    }
  }

  return err;
}

int datastoreUnpauseSubFloat(DatastoreFloatSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid float subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < floatSubCount; i++)
  {
    if(floatSubs[i].callback == subCallback)
    {
      err = 0;
      floatSubs[i].isPaused = false;
    }
  }

  return err;
}

int datastoreReadFloat(uint32_t datapointId, size_t valCount, struct k_msgq *response, float values[])
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_READ, .dataType = DATAPOINT_FLOAT, .dataId = datapointId,
                        .data.floatVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
  if(err < 0)
    return err;

  return resStatus;
}

int datastoreWriteFloat(uint32_t datapointId, float values[],
                        size_t valCount, struct k_msgq *response)
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_WRITE, .dataType = DATAPOINT_FLOAT, .dataId = datapointId,
                        .data.floatVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  if(response)
  {
    err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
    if(err < 0)
      return err;
  }

  return resStatus;
}

int datastoreSubscribeUint(DatastoreUintSub_t *sub)
{
  int err;

  if(!uintSubs)
  {
    err = -EACCES;
    LOG_ERR("ERROR %d: unsigned integer subscription records not initialized", err);
    return err;
  }

  if(uintSubCount >= UINT_DATAPOINT_COUNT)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: no more free unsigned integer subscription record", err);
    return err;
  }

  if(!sub)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid new unsigned integer subscription record", err);
    return err;
  }

  memcpy(uintSubs + uintSubCount, sub, sizeof(DatastoreUintSub_t));
  uintSubCount++;

  return 0;
}

int datastorePauseSubUint(DatastoreUintSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid unsigned integer subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < uintSubCount; i++)
  {
    if(uintSubs[i].callback == subCallback)
    {
      err = 0;
      uintSubs[i].isPaused = true;
    }
  }

  return err;
}

int datastoreUnpauseSubUint(DatastoreUintSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid unsigned integer subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < uintSubCount; i++)
  {
    if(uintSubs[i].callback == subCallback)
    {
      err = 0;
      uintSubs[i].isPaused = false;
    }
  }

  return err;
}

int datastoreReadUint(uint32_t datapointId, size_t valCount, struct k_msgq *response, uint32_t values[])
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_READ, .dataType = DATAPOINT_UINT, .dataId = datapointId,
                        .data.uintVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
  if(err < 0)
    return err;

  return resStatus;
}

int datastoreWriteUint(uint32_t datapointId, uint32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_WRITE, .dataType = DATAPOINT_UINT, .dataId = datapointId,
                        .data.uintVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  if(response)
  {
    err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
    if(err < 0)
      return err;
  }

  return resStatus;
}

int datastoreSubscribeInt(DatastoreIntSub_t *sub)
{
  int err;

  if(!intSubs)
  {
    err = -EACCES;
    LOG_ERR("ERROR %d: signed integer subscription records not initialized", err);
    return err;
  }

  if(intSubCount >= INT_DATAPOINT_COUNT)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: no more free signed integer subscription record", err);
    return err;
  }

  if(!sub)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid new signed integer subscription record", err);
    return err;
  }

  memcpy(intSubs + intSubCount, sub, sizeof(DatastoreIntSub_t));
  intSubCount++;

  return 0;
}

int datastorePauseSubInt(DatastoreIntSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid signed integer subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < intSubCount; i++)
  {
    if(intSubs[i].callback == subCallback)
    {
      err = 0;
      intSubs[i].isPaused = true;
    }
  }

  return err;
}

int datastoreUnpauseSubInt(DatastoreIntSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid signed integer subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < intSubCount; i++)
  {
    if(intSubs[i].callback == subCallback)
    {
      err = 0;
      intSubs[i].isPaused = false;
    }
  }

  return err;
}

int datastoreReadInt(uint32_t datapointId, size_t valCount, struct k_msgq *response, int32_t values[])
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_READ, .dataType = DATAPOINT_INT, .dataId = datapointId,
                        .data.intVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
  if(err < 0)
    return err;

  return resStatus;
}

int datastoreWriteInt(uint32_t datapointId, int32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_WRITE, .dataType = DATAPOINT_INT, .dataId = datapointId,
                        .data.intVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  if(response)
  {
    err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
    if(err < 0)
      return err;
  }

  return resStatus;
}

int datastoreSubscribeMultiState(DatastoreMultiStateSub_t *sub)
{
  int err;

  if(!multiStateSubs)
  {
    err = -EACCES;
    LOG_ERR("ERROR %d: multi-state subscription records not initialized", err);
    return err;
  }

  if(multiStateSubCount >= MULTI_STATE_DATAPOINT_COUNT)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: no more free multi-state subscription record", err);
    return err;
  }

  if(!sub)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid new multi-state subscription record", err);
    return err;
  }

  memcpy(multiStateSubs + multiStateSubCount, sub, sizeof(DatastoreButtonSub_t));
  multiStateSubCount++;

  return 0;
}

int datastorePauseSubMultiState(DatastoreMultiStateSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid multi-state subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < multiStateSubCount; i++)
  {
    if(multiStateSubs[i].callback == subCallback)
    {
      err = 0;
      multiStateSubs[i].isPaused = true;
    }
  }

  return err;
}

int datastoreUnpauseSubMultiState(DatastoreMultiStateSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid multi-state subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < multiStateSubCount; i++)
  {
    if(multiStateSubs[i].callback == subCallback)
    {
      err = 0;
      multiStateSubs[i].isPaused = false;
    }
  }

  return err;
}

int datastoreReadMultiState(uint32_t datapointId, size_t valCount, struct k_msgq *response, uint32_t values[])
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_READ, .dataType = DATAPOINT_MULTI_STATE, .dataId = datapointId,
                        .data.uintVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
  if(err < 0)
    return err;

  return resStatus;
}

int datastoreWriteMultiState(uint32_t datapointId, uint32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_WRITE, .dataType = DATAPOINT_MULTI_STATE, .dataId = datapointId,
                        .data.uintVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  if(response)
  {
    err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
    if(err < 0)
      return err;
  }

  return resStatus;
}

int datastoreSubscribeButton(DatastoreButtonSub_t *sub)
{
  int err;

  if(!buttonSubs)
  {
    err = -EACCES;
    LOG_ERR("ERROR %d: button subscription records not initialized", err);
    return err;
  }

  if(buttonSubCount >= BUTTON_DATAPOINT_COUNT)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: no more free button subscription record", err);
    return err;
  }

  if(!sub)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid new button subscription record", err);
    return err;
  }

  memcpy(buttonSubs + buttonSubCount, sub, sizeof(DatastoreButtonSub_t));
  buttonSubCount++;

  return 0;
}

int datastorePauseSubButton(DatastoreButtonSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid button subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < buttonSubCount; i++)
  {
    if(buttonSubs[i].callback == subCallback)
    {
      err = 0;
      buttonSubs[i].isPaused = true;
    }
  }

  return err;
}

int datastoreUnpauseSubButton(DatastoreButtonSubCb_t subCallback)
{
  int err = -ESRCH;

  if(!subCallback)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid button subscription callback", err);
    return err;
  }

  for(uint32_t i = 0; i < buttonSubCount; i++)
  {
    if(buttonSubs[i].callback == subCallback)
    {
      err = 0;
      buttonSubs[i].isPaused = false;
    }
  }

  return err;
}

int datastoreReadButton(uint32_t datapointId, size_t valCount, struct k_msgq *response, uint32_t values[])
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_READ, .dataType = DATAPOINT_BUTTON, .dataId = datapointId,
                        .data.uintVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
  if(err < 0)
    return err;

  return resStatus;
}

int datastoreWriteButton(uint32_t datapointId, uint32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_WRITE, .dataType = DATAPOINT_BUTTON, .dataId = datapointId,
                        .data.uintVal = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  if(response)
  {
    err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
    if(err < 0)
      return err;
  }

  return resStatus;
}

int datastoreReturnBuffer(DatapointData_t *buffer)
{
  return datastoreBufPoolReturn(bufPool, (Datapoint_t *)buffer);
}

int datastoreReturnUintBuffer(Uint32_t *buffer)
{
  return datastoreBufPoolReturn(bufPool, (Datapoint_t *)buffer);
}

int datastoreReturnIntBuffer(int32_t *buffer)
{
  return datastoreBufPoolReturn(bufPool, (Datapoint_t *)buffer);
}

int datastoreReturnMultiStateBuffer(Uint32_t *buffer)
{
  return datastoreBufPoolReturn(bufPool, (Datapoint_t *)buffer);
}

int datastoreReturnButtonBuffer(Uint32_t *buffer)
{
  return datastoreBufPoolReturn(bufPool, (Datapoint_t *)buffer);
}
/** @} */
