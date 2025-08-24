/**
 * Copyright (C) 2025 by Electronya
 *
 * @file      datastoreBufferPool.c
 * @author    jbacon
 * @date      2025-08-17
 * @brief     Datastore Buffer Pool Implementation
 *
 *            Implementation of the buffer pool for the datastore.
 *
 * @ingroup   datastore
 * @{
 */

#include <zephyr/logging/log.h>
#include <string.h>

#include "datastoreBufferPool.h"

/* Setting module logging */
LOG_MODULE_DECLARE(DATASTORE_LOGGER_NAME);

/**
 * @brief   Allocate the buffers.
 *
 * @param pool    The buffer pool.
 *
 * @return  0 if successful, the error code otherwise.
 */
static int allocateBuffers(DatastoreBufferPool_t*pool)
{
  int err = 0;

  for(size_t i = 0; i < pool->poolSize && err == 0; i++)
  {
    pool->buffers[i] = k_malloc(pool->bufferSize);
    if(!pool->buffers[i])
    {
      err = -ENOSPC;
      LOG_ERR("ERROR %d: unable to allocate buffer %d", err, i);
    }
  }

  return err;
}

/**
 * @brief   Free the buffers.
 *
 * @param pool    The buffer pool.
 */
void freeBuffers(DatastoreBufferPool_t *pool)
{
  if(pool->buffers)
  {
    for(size_t i = 0; i < pool->poolSize; i++)
      k_free(pool->buffers[i]);
  }
}

DatastoreBufferPool_t *datastoreBufPoolInit(size_t bufferSize, size_t poolSize)
{
  int err;
  DatastoreBufferPool_t *pool = NULL;

  pool = k_malloc(sizeof(DatastoreBufferPool_t));
  if(!pool)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate the buffer pool structure", err);
    return NULL;
  }

  pool->bufferSize = bufferSize;
  pool->bufferInPool = bufferSize;
  pool->poolSize = poolSize;

  pool->buffers = k_malloc(pool->poolSize * sizeof(DatapointData_t*));
  if(!pool->buffers)
  {
    err = -ENOSPC;
    LOG_ERR("ERROR %d: unable to allocate the buffer pool container", err);
    return NULL;
  }

  err = allocateBuffers(pool);
  if(err < 0)
    return NULL;

  return pool;
}

DatapointData_t *datastoreBufPoolGet(DatastoreBufferPool_t *pool)
{
  DatapointData_t *buffer = NULL;

  if(!pool)
  {
    LOG_ERR("ERROR %d: invalid pool", -EINVAL);
    return buffer;
  }

  if(pool->bufferInPool == 0)
  {
    LOG_ERR("ERROR %d: no more buffer in the pool", -ENOSPC);
    return buffer;
  }

  buffer = pool->buffers[pool->bufferInPool - 1];
  pool->buffers[pool->bufferInPool - 1] = NULL;
  pool->bufferInPool--;

  return buffer;
}

int datastoreBufPoolReturn(DatastoreBufferPool_t *pool, DatapointData_t *buffer)
{
  if(!pool)
    return -EINVAL;

  if(pool->bufferInPool >= pool->poolSize && pool->buffers[pool->bufferInPool])
    return -ENOSPC;

  pool->buffers[pool->bufferInPool] = buffer;
  pool->bufferInPool++;

  return 0;
}

/** @} */
