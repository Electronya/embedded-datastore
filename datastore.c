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

#define DATASTORE_LOGGER_NAME datastore

/* Setting module logging */
LOG_MODULE_REGISTER(DATASTORE_LOGGER_NAME);

#define DATASTORE_STACK_SIZE                                    (256)
#define DATASTORE_MSG_COUNT                                     (10)

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
  DatapointData_t WriteData;
  DatapointData_t *ReadData;
} DatastoreMsg_t;

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

static k_thread thread;
static DatastoreFloatSub_t *floatSubs = NULL;
static uint32_t floatSubCount = 0;
static DatastoreUintSub_t *uintSubs = NULL;
static uint32_t uintSubCount = 0;
static DatastoreIntSub_t *intSubs = NULL;
static uint32_t intSubCount = 0;
static DatastoreMultiStateSub_t multiStateSubs = NULL;
static uint32_t multiStateSubCount = 0;
static DatastoreButtonSub_t buttonSubs = NULL;
static uint32_t buttonSubCount = 0;

K_MSGQ_DEFINE(datastoreQueue, sizeof(DatastoreMsg_t), DATASTORE_MSG_COUNT, 4);

/**
 * @brief   Do the initial notifications.
 *
 * @return  0 if successful, the error code otherwise.
 */
static int doInitialNotifications(void)
{

}

/**
 * @brief   The datastore service thread function.
 *
 * @param p1  Thread first parameter.
 * @param p2  Thread second parameter.
 * @param p3  Thread third parameter.
 */
static void run(void *p1, void *p2, void *p3)
{
  int err;
  Datapoint_t *datapointTypes[DATAPOINT_TYPE_COUNT] = {floats, uints, ints,
                                                       multiStates, buttons};

  // TODO: Initialize the datapoints from the NVM.

  err = doInitialNotifications();
  if(err < 0)
    LOG_ERR("ERROR %d: unable to make initial notifications", err);

  for(;;)
  {

  }
}

int datastoreInit(DatastoreMaxSubs_t maxSubs, uint32_t priority,
                  k_tid_t *threadId)
{
  int err;

  floatSubs = k_malloc(maxSubs.maxFloatSubs * sizeof(DatastoreFloatSub_t));
  if(!floatSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for float subscription", err);
    goto cleanup;
  }

  uintSubs = k_malloc(maxSubs.maxUintSubs * sizeof(DatastoreUintSub_t));
  if(!uintSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for unsigned integer subscription", err);
    k_free(floatSubs);
    goto cleanup;
  }

  intSubs = k_malloc(maxSubs.maxIntSubs * sizeof(DatastoreIntSub_t));
  if(!intSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for signed integer subscription", err);
    goto cleanup;
  }

  multiStateSubs = k_malloc(maxSubs.maxMultiStateSubs * sizeof(DatastoreMultiStateSub_t));
  if(!multiStateSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for multi-state subscription", err);
    goto cleanup;
  }

  buttonSubs = k_malloc(maxSubs.maxButtonSubs * sizeof(DatastoreButtonSub_t));
  if(!buttonSubs)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate memory for button subscription", err);
    goto cleanup;
  }

  *threadId = k_thread_create(&thread, datastoreStack, DATASTORE_STACK_SIZE,
                              run, NULL, NULL, NULL, K_PRIO_PREEMPT(priority),
                              0, K_FOREVER);

  err = k_thread_name_set(*threadId, "datastore");
  if(err< 0)
  {
    LOG_ERR("ERROR %d: unable to set datastore thread name", err)
    goto cleanup;
  }

  goto exit;

cleanup:
  k_free(floatSubs);
  k_free(uintSubs);
  k_free(intSubs);
  k_free(multiStateSubs);
  k_free(buttonSubs);
exit:
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

int datastoreReadFloat(uint32_t datapointId, size_t valCount,
                       struct k_msgq *response, float values[])
{
  int err;
  DatastoreMsg_t msg;
  size_t valId = 0;

  return 0;
}

int datastoreWriteFloat(uint32_t datapointId, float values[],
                        size_t valCount, struct k_msgq *response)
{
  return 0;
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

int datastoreReadUint(uint32_t datapointId, size_t valCount,
                      struct k_msgq *response, uint32_t values[])
{
  return 0;
}

int datastoreWriteUint(uint32_t datapointId, uint32_t values[],
                       size_t valCount, struct k_msgq *response)
{
  return 0;
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

int datastoreReadInt(uint32_t datapointId, size_t valCount,
                     struct k_msgq *response, int32_t values[])
{
  return 0;
}

int datastoreWriteInt(uint32_t datapointId, int32_t values[],
                      size_t valCount, struct k_msgq *response)
{
  return 0;
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

int datastoreReadMultiState(uint32_t datapointId, size_t valCount,
                            struct k_msgq *response, uint32_t values[])
{
  return 0;
}

int datastoreWriteMultiState(uint32_t datapointId, uint32_t values[],
                             size_t valCount, struct k_msgq *response)
{
  return 0;
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

int datastoreReadButton(uint32_t datapointId, size_t valCount,
                        struct k_msgq *response, uint32_t values[])
{
  return 0;
}

int datastoreWriteButton(uint32_t datapointId, uint32_t values[],
                         size_t valCount, struct k_msgq *response)
{
  return 0;
}
/** @} */
