/**
 * Copyright (C) 2025 by Electronya
 *
 * @file      datastoreBufferPool.h
 * @author    jbacon
 * @date      2025-08-17
 * @brief     Datastore Buffer Pool
 *
 *            Datastore service buffer pool.
 *
 * @ingroup   datastore
 *
 * @{
 */

#ifndef DATASTORE_SRV_BUFFER_POOL
#define DATASTORE_SRV_BUFFER_POOL

#include <zephyr/kernel.h>

#include "datastoreMeta.h"

typedef struct
{
  size_t poolSize;
  size_t bufferSize;
  size_t bufferInPool;
  DatapointData_t **buffers;
} DatastoreBufferPool_t;

/**
 * @brief   Initialize the datastore buffer pool.
 *
 * @param bufferSize    The size of the buffers.
 * @param poolSize      The pool size, the number of buffer in the pool.
 *
 * @return  The handle to the created buffer pool if successful, NULL otherwise.
 */
DatastoreBufferPool_t *datastoreBufPoolInit(size_t bufferSize, size_t poolSize);

/**
 * @brief   Get a buffer from the pool.
 *
 * @param pool          The buffer pool.
 *
 * @return  The free buffer if successful, NULL otherwise.
 */
DatapointData_t *datastoreBufPoolGet(DatastoreBufferPool_t *pool);

/**
 * @brief   Return a buffer to the pool.
 *
 * @param pool          The buffer pool.
 * @param buffer        The buffer.
 *
 * @return  0 if successful, the error code otherwise.
 */
int datastoreBufPoolReturn(DatastoreBufferPool_t *pool, DatapointData_t *buffer);

#endif    /* DATASTORE_SRV_BUFFER_POOL */

/** @} */
