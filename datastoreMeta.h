/**
 * Copyright (C) 2025 by Electronya
 *
 * @file      datastoreMeta.h
 * @author    jbacon
 * @date      2025-08-10
 * @brief     Datastore Meta Data
 *
 *            Datastore service meta data definition.
 *
 * @ingroup   datastore
 *
 * @{
 */

#ifndef DATASTORE_META
#define DATASTORE_META

#include <zephyr/kernel.h>

/**
 * @brief   Datapoint in NVM flag mask.
 */
#define DATAPOINT_FLAG_NVM_MASK                                   (1 << 0)

typedef enum
{
  DATAPOINT_FLOAT = 0,
  DATAPOINT_UINT,
  DATAPOINT_INT,
  DATAPOINT_MULTI_STATE,
  DATAPOINT_BUTTON,
  DATAPOINT_TYPE_COUNT,
} DatapointType_t;

/**
 * @brief   Datapoint data union.
 */
typedef union
{
  float floatVal;                 /**< Float value. */
  uint32_t uintVal;               /**< unsigned integer/multi-state/button value. */
  int32_t intVal;                 /**< signed integer value. */
} DatapointData_t;

/**
 * @brief   Datastore datapoint.
 */
typedef struct
{
  DatapointData_t data;           /**< The data. */
  uint32_t flags;                 /**< The datapoint flags. */
} Datapoint_t;

/**
 * @brief   Float datapoint information X-macro.
 * @note    X(datapoint name, datapoint flag)
 */
#define DATASTORE_FLOAT_DATAPOINTS \
  /*datapoint name,          datapoint flags,         default value */
  X(FLOAT_FIRST_DATAPOINT,   DATAPOINT_FLAG_NVM_MASK, 0.0f) \
  X(FLOAT_SECOND_DATAPOINT,  DATAPOINT_FLAG_NVM_MASK, 1.0f) \
  X(FLOAT_THIRD_DATAPOINT,   DATAPOINT_FLAG_NVM_MASK, 2.0f) \
  X(FLOAT_FORTH_DATAPOINT,   DATAPOINT_FLAG_NVM_MASK, 3.0f)

/**
 * @brief   Unsigned integer datapoint information X-macro.
 * @note    X(datapoint name, datapoint flag)
 */
#define DATASTORE_UINT_DATAPOINTS \
  /*datapoint name,          datapoint flags,         default value */
  X(UINT_FIRST_DATAPOINT,    DATAPOINT_FLAG_NVM_MASK, 0) \
  X(UINT_SECOND_DATAPOINT,   DATAPOINT_FLAG_NVM_MASK, 1) \
  X(UINT_THIRD_DATAPOINT,    DATAPOINT_FLAG_NVM_MASK, 2) \
  X(UINT_FORTH_DATAPOINT,    DATAPOINT_FLAG_NVM_MASK, 3)

/**
 * @brief   signed integer datapoint information X-macro.
 * @note    X(datapoint name, datapoint flag)
 */
#define DATASTORE_INT_DATAPOINTS \
  /*datapoint name,          datapoint flags,         default value */
  X(INT_FIRST_DATAPOINT,     DATAPOINT_FLAG_NVM_MASK, -1) \
  X(INT_SECOND_DATAPOINT,    DATAPOINT_FLAG_NVM_MASK,  0) \
  X(INT_THIRD_DATAPOINT,     DATAPOINT_FLAG_NVM_MASK,  1) \
  X(INT_FORTH_DATAPOINT,     DATAPOINT_FLAG_NVM_MASK,  2)

/**
 * @brief   Multi-state datapoint information X-macro.
 * @note    X(datapoint name, datapoint flag)
 */
#define DATASTORE_MULTI_STATE_DATAPOINTS \
  /*datapoint name,               datapoint flags,         default value */
  X(MULTI_STATE_FIRST_DATAPOINT,  DATAPOINT_FLAG_NVM_MASK, 0) \
  X(MULTI_STATE_SECOND_DATAPOINT, DATAPOINT_FLAG_NVM_MASK, 1) \
  X(MULTI_STATE_THIRD_DATAPOINT,  DATAPOINT_FLAG_NVM_MASK, 2) \
  X(MULTI_STATE_FORTH_DATAPOINT,  DATAPOINT_FLAG_NVM_MASK, 3)

/**
 * @brief   Button datapoint information X-macro.
 * @note    X(datapoint name, datapoint flag)
 */
#define DATASTORE_BUTTON_DATAPOINTS \
  /*datapoint name,          datapoint flags,         default value */
  X(BUTTON_FIRST_DATAPOINT,  DATAPOINT_FLAG_NVM_MASK, 0) \
  X(BUTTON_SECOND_DATAPOINT, DATAPOINT_FLAG_NVM_MASK, 0) \
  X(BUTTON_THIRD_DATAPOINT,  DATAPOINT_FLAG_NVM_MASK, 0) \
  X(BUTTON_FORTH_DATAPOINT,  DATAPOINT_FLAG_NVM_MASK, 0)

#endif    /* DATASTORE_META */

/** @} */
