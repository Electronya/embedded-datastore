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
#include "datastoreUtil.h"

/* Setting module logging */
LOG_MODULE_REGISTER(DATASTORE_LOGGER_NAME, CONFIG_ENYA_DATASTORE_LOG_LEVEL);

/**
 * @brief   The datastore service stack size.
 */
#define DATASTORE_STACK_SIZE                                    (256)

/**
 * @brief   The datastore response timeout [ms].
 */
#define DATASTORE_RESPONSE_TIMEOUT                              (5)

/**
 * @brief   The datastore buffer pool allocation timeout.
 */
#define DATASTORE_BUFFER_ALLOC_TIMEOUT                          (4)

/**
 * @brief   The datastore buffer count.
 */
#define DATASTORE_BUFFER_COUNT                                  (10)

/**
 * @brief The thread stack.
*/
K_THREAD_STACK_DEFINE(datastoreStack, DATASTORE_STACK_SIZE);

/**
 * @brief   The datastore message type.
 */
typedef enum
{
  DATASTORE_READ = 0,
  DATASTORE_WRITE,
  DATASTORE_MSG_TYPE_COUNT,
} DatastoreMsgtype_t;

/**
 * @brief   The datastore message.
 */
typedef struct
{
  DatastoreMsgtype_t msgType;
  DatapointType_t datapointType;
  uint32_t datapointId;
  DatapointValue_t *values;
  size_t valCount;
  struct k_msgq *response;
} DatastoreMsg_t;

/**
 * @brief   The service thread.
 */
static struct k_thread thread;

/**
 * @brief   The datastore buffer pool.
 */
static osMemoryPoolId_t bufferPool = NULL;

/**
 * @brief   The datastore message queue.
 */
K_MSGQ_DEFINE(datastoreQueue, sizeof(DatastoreMsg_t), DATASTORE_MSG_COUNT, 4);

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
  int errOp;
  bool needToNotify = false;
  DatastoreMsg_t msg;

  // TODO: Initialize the datapoints from the NVM.

  // TODO: Do initial notifications.

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
        // errOp = datastoreUtilRead(msg.datapointType, msg.datapointId, msg.valCount, msg.values);
      break;
      case DATASTORE_WRITE:
        // errOp = datastoreUtilWrite(msg.datapointType, msg.datapointId, msg.values, msg.valCount, &needToNotify);

        // if(errOp == 0 && needToNotify)
        // {
        //   err = datastoreUtilNotify(msg.datapointType, msg.datapointId);
        //   if(err)
        //     LOG_ERR("ERROR %d: unable to notify", err);
        // }
      break;
      default:
        LOG_WRN("unsupported message type %d", msg.msgType);
      break;
    }

    // if(msg.response)
    //   k_msgq_put(msg.response, &errOp, K_NO_WAIT);
  }
}

int datastoreInit(size_t maxSubs[DATAPOINT_TYPE_COUNT], uint32_t priority, k_tid_t *threadId)
{
  int err;
  size_t datapointCounts[DATAPOINT_TYPE_COUNT] = {BINARY_DATAPOINT_COUNT, BUTTON_DATAPOINT_COUNT, FLOAT_DATAPOINT_COUNT,
                                                  INT_DATAPOINT_COUNT, MULTI_STATE_DATAPOINT_COUNT, UINT_DATAPOINT_COUNT};

  err = datastoreUtilAllocateBinarySubs(maxSubs[DATAPOINT_BINARY]);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateButtonSubs(maxSubs[DATAPOINT_BUTTON]);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateFloatSubs(maxSubs[DATAPOINT_FLOAT]);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateIntSubs(maxSubs[DATAPOINT_INT]);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateMultiStateSubs(maxSubs[DATAPOINT_MULTI_STATE]);
  if(err < 0)
    return err;

  err = datastoreUtilAllocateUintSubs(maxSubs[DATAPOINT_UINT]);
  if(err < 0)
    return err;

  bufferPool = osMemoryPoolNew(DATASTORE_BUFFER_COUNT, datastoreUtilCalculateBufferSize(datapointCounts), NULL);

  *threadId = k_thread_create(&thread, datastoreStack, DATASTORE_STACK_SIZE, run,
                              NULL, NULL, NULL, K_PRIO_PREEMPT(priority), 0, K_FOREVER);

  err = k_thread_name_set(*threadId, STRINGIFY(DATASTORE_LOGGER_NAME));
  if(err< 0)
    LOG_ERR("ERROR %d: unable to set datastore thread name", err);

  return err;
}

int datastoreRead(DatapointType_t datapointType, uint32_t datapointId, size_t valCount,
                  struct k_msgq *response, DatapointValue_t values[])
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_READ, .datapointType = datapointType, .datapointId = datapointId,
                        .values = values, .valCount = valCount, .response = response };

  err = k_msgq_put(&datastoreQueue, &msg, K_NO_WAIT);
  if(err < 0)
    return err;

  err = k_msgq_get(response, &resStatus, K_MSEC(DATASTORE_RESPONSE_TIMEOUT));
  if(err < 0)
    return err;

  return resStatus;
}

int datastoreWrite(DatapointType_t datapointType, uint32_t datapointId,
                   DatapointValue_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  int resStatus = 0;
  DatastoreMsg_t msg = {.msgType = DATASTORE_WRITE, .datapointType = datapointType, .datapointId = datapointId,
                        .values = values, .valCount = valCount, .response = response };

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

inline int datastoreSubscribeBinary(DatastoreBinarySub_t *sub)
{
  return datastoreUtilAddBinarySub(sub);
}

int datastorePauseSubBinary(DatastoreBinarySubCb_t subCallback)
{
  return datastoreUtilSetBinarySubPauseState(subCallback, true);
}

int datastoreUnpauseSubBinary(DatastoreBinarySubCb_t subCallback)
{
  return datastoreUtilSetBinarySubPauseState(subCallback, false);
}

int datastoreReadBinary(uint32_t datapointId, size_t valCount, struct k_msgq *response, bool values[])
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0 || !response)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  err = datastoreRead(DATAPOINT_BINARY, datapointId, valCount, response, buffer);
  if(err < 0)
  {
    LOG_ERR("ERROR %d: unable to read binary datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    values[i] = (bool)buffer[i].uintVal;

  osMemoryPoolFree(bufferPool, buffer);

  return err;
}

int datastoreWriteBinary(uint32_t datapointId, bool values[], size_t valCount, struct k_msgq *response)
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    buffer[i].uintVal = (uint32_t)values[i];

  err = datastoreWrite(DATAPOINT_BINARY, datapointId, buffer, valCount, response);
  if(err < 0)
    LOG_ERR("ERROR %d: unable to write binary datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);

  return err;
}

int datastoreSubscribeButton(DatastoreButtonSub_t *sub)
{
  return datastoreUtilAddButtonSub(sub);
}

int datastorePauseSubButton(DatastoreButtonSubCb_t subCallback)
{
  return datastoreUtilSetButtonSubPauseState(subCallback, true);
}

int datastoreUnpauseSubButton(DatastoreButtonSubCb_t subCallback)
{
  return datastoreUtilSetButtonSubPauseState(subCallback, false);
}

int datastoreReadButton(uint32_t datapointId, size_t valCount, struct k_msgq *response, uint32_t values[])
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0 || !response)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  err = datastoreRead(DATAPOINT_BUTTON, datapointId, valCount, response, buffer);
  if(err < 0)
  {
    LOG_ERR("ERROR %d: unable to read button datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    values[i] = buffer[i].uintVal;

  osMemoryPoolFree(bufferPool, buffer);

  return err;
}

int datastoreWriteButton(uint32_t datapointId, uint32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    buffer[i].uintVal = values[i];

  err = datastoreWrite(DATAPOINT_BUTTON, datapointId, buffer, valCount, response);
  if(err < 0)
    LOG_ERR("ERROR %d: unable to write button datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);

  return err;
}

int datastoreSubscribeFloat(DatastoreFloatSub_t *sub)
{
  return datastoreUtilAddFloatSub(sub);
}

int datastorePauseSubFloat(DatastoreFloatSubCb_t subCallback)
{
  return datastoreUtilSetFloatSubPauseState(subCallback, true);
}

int datastoreUnpauseSubFloat(DatastoreFloatSubCb_t subCallback)
{
  return datastoreUtilSetFloatSubPauseState(subCallback, false);
}

int datastoreReadFloat(uint32_t datapointId, size_t valCount, struct k_msgq *response, float values[])
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0 || !response)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  err = datastoreRead(DATAPOINT_FLOAT, datapointId, valCount, response, buffer);
  if(err < 0)
  {
    LOG_ERR("ERROR %d: unable to read float datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    values[i] = buffer[i].floatVal;

  osMemoryPoolFree(bufferPool, buffer);

  return err;
}

int datastoreWriteFloat(uint32_t datapointId, float values[], size_t valCount, struct k_msgq *response)
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    buffer[i].floatVal = values[i];

  err = datastoreWrite(DATAPOINT_FLOAT, datapointId, buffer, valCount, response);
  if(err < 0)
    LOG_ERR("ERROR %d: unable to write float datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);

  return err;
}

int datastoreSubscribeInt(DatastoreIntSub_t *sub)
{
  return datastoreUtilAddIntSub(sub);
}

int datastorePauseSubInt(DatastoreIntSubCb_t subCallback)
{
  return datastoreUtilSetIntSubPauseState(subCallback, true);
}

int datastoreUnpauseSubInt(DatastoreIntSubCb_t subCallback)
{
  return datastoreUtilSetIntSubPauseState(subCallback, false);
}

int datastoreReadInt(uint32_t datapointId, size_t valCount, struct k_msgq *response, int32_t values[])
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0 || !response)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  err = datastoreRead(DATAPOINT_INT, datapointId, valCount, response, buffer);
  if(err < 0)
  {
    LOG_ERR("ERROR %d: unable to read signed integer datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    values[i] = buffer[i].intVal;

  osMemoryPoolFree(bufferPool, buffer);

  return err;
}

int datastoreWriteInt(uint32_t datapointId, int32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    buffer[i].intVal = values[i];

  err = datastoreWrite(DATAPOINT_INT, datapointId, buffer, valCount, response);
  if(err < 0)
    LOG_ERR("ERROR %d: unable to write signed integer datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);

  return err;
}

int datastoreSubscribeMultiState(DatastoreMultiStateSub_t *sub)
{
  return datastoreUtilAddMultiStateSub(sub);
}

int datastorePauseSubMultiState(DatastoreMultiStateSubCb_t subCallback)
{
  return datastoreUtilSetMultiStateSubPauseState(subCallback, true);
}

int datastoreUnpauseSubMultiState(DatastoreMultiStateSubCb_t subCallback)
{
  return datastoreUtilSetMultiStateSubPauseState(subCallback, false);
}

int datastoreReadMultiState(uint32_t datapointId, size_t valCount, struct k_msgq *response, uint32_t values[])
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0 || !response)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  err = datastoreRead(DATAPOINT_MULTI_STATE, datapointId, valCount, response, buffer);
  if(err < 0)
  {
    LOG_ERR("ERROR %d: unable to read multi-state datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    values[i] = buffer[i].uintVal;

  osMemoryPoolFree(bufferPool, buffer);

  return err;
}

int datastoreWriteMultiState(uint32_t datapointId, uint32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    buffer[i].uintVal = values[i];

  err = datastoreWrite(DATAPOINT_MULTI_STATE, datapointId, buffer, valCount, response);
  if(err < 0)
    LOG_ERR("ERROR %d: unable to write multi-state datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);

  return err;
}

int datastoreSubscribeUint(DatastoreUintSub_t *sub)
{
  return datastoreUtilAddUintSub(sub);
}

int datastorePauseSubUint(DatastoreUintSubCb_t subCallback)
{
  return datastoreUtilSetUintSubPauseState(subCallback, true);
}

int datastoreUnpauseSubUint(DatastoreUintSubCb_t subCallback)
{
  return datastoreUtilSetUintSubPauseState(subCallback, false);
}

int datastoreReadUint(uint32_t datapointId, size_t valCount, struct k_msgq *response, uint32_t values[])
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0 || !response)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  err = datastoreRead(DATAPOINT_UINT, datapointId, valCount, response, buffer);
  if(err < 0)
  {
    LOG_ERR("ERROR %d: unable to read unsigned integer datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    values[i] = buffer[i].uintVal;

  osMemoryPoolFree(bufferPool, buffer);

  return err;
}

int datastoreWriteUint(uint32_t datapointId, uint32_t values[], size_t valCount, struct k_msgq *response)
{
  int err;
  DatapointValue_t *buffer;

  if(!values || valCount == 0)
  {
    err = -EINVAL;
    LOG_ERR("ERROR %d: invalid operation parameters", err);
    return err;
  }

  buffer = osMemoryPoolAlloc(bufferPool, DATASTORE_BUFFER_ALLOC_TIMEOUT);
  if(!buffer)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate a buffer for operation", err);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
    buffer[i].uintVal = values[i];

  err = datastoreWrite(DATAPOINT_UINT, datapointId, buffer, valCount, response);
  if(err < 0)
    LOG_ERR("ERROR %d: unable to write unsigned integer datapoint %d up to datapoint %d", err, datapointId, datapointId + valCount);

  return err;
}

/** @} */
