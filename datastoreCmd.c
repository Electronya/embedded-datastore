/**
 * Copyright (C) 2025 by Electronya
 *
 * @file      datastoreCmd.c
 * @author    jbacon
 * @date      2025-08-10
 * @brief     Datastore Service Command
 *
 *            Datastore service command set.
 *
 * @ingroup   datastore
 * @{
 */

#include <zephyr/shell/shell.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "datastore.h"
#include "datastoreMeta.h"

/**
 * @brief   The datastore command value string max length.
 */
#define DATASTORE_CMD_VALUE_STR_LENGTH                              (10)

/**
 * @brief   The list of datapoint type names.
 */
static char *typeNames[DATAPOINT_TYPE_COUNT] = {"float", "uint", "int", "multi-state", "button"};

/**
 * @brief   The list of float datapoint names.
 */
static char *floatNames[FLOAT_DATAPOINT_COUNT] = {
#define X(name, flags, defaultVal) STRINGIFY(name),
  DATASTORE_FLOAT_DATAPOINTS
#undef
};

/**
 * @brief   The list of unsigned integer datapoint names.
 */
static char *uintNames[UINT_DATAPOINT_COUNT] = {
#define X(name, flags, defaultVal) STRINGIFY(name),
  DATASTORE_UINT_DATAPOINTS
#undef
};

/**
 * @brief   The list of signed integer datapoint names.
 */
static char *intNames[INT_DATAPOINT_COUNT] = {
#define X(name, flags, defaultVal) STRINGIFY(name),
  DATASTORE_INT_DATAPOINTS
#undef
};

/**
 * @brief   The list of multi-state datapoint names.
 */
static char *multiStateNames[MULTI_STATE_DATAPOINT_COUNT] = {
#define X(name, flags, defaultVal) STRINGIFY(name),
  DATASTORE_MULTI_STATE_DATAPOINTS
#undef
};

/**
 * @brief   The list of button datapoint names.
 */
static char *buttonNames[BUTTON_DATAPOINT_COUNT] = {
#define X(name, flags, defaultVal) STRINGIFY(name),
  DATASTORE_BUTTON_DATAPOINTS
#undef
};

/**
 * @brief   The list of all datapoint name by their type.
 */
static char **datapointNames[DATAPOINT_TYPE_COUNT] = {floatNames, uintNames, intNames, multiStateNames, buttonNames};

/**
 * @brief   The list of datapoint count by their type.
 */
static size_t datapointCounts[DATAPOINT_TYPE_COUNT] = {FLOAT_DATAPOINT_COUNT, UINT_DATAPOINT_COUNT, INT_DATAPOINT_COUNT,
                                                       MULTI_STATE_DATAPOINT_COUNT, BUTTON_DATAPOINT_COUNT};

static char *valFormats[DATAPOINT_TYPE_COUNT] = {"%f", "%u", "%d", "%u", "%u"}

/**
 * @brief   Datastore command response queue.
 */
K_MSGQ_DEFINE(datastoreCmdResQueue, sizeof(int), DATASTORE_MSG_COUNT, 4);

/**
 * @brief   Get the index of the string.
 *
 * @param[in]   str: The string to look for.
 * @param[in]   strList: The list of string to look in.
 * @param[in]   listSize: The string list size.
 * @param[out]  index: The found index.
 *
 * @return  0 if successful, the error code otherwise.
 */
static int getStringIndex(char *str, char **strList, size_t listSize, uint32_t *index)
{
  char *listedStr;
  bool indexFound = false;

  *index = 0;
  listedStr = strList[*index];

  while(!indexfound && *index < listSize)
  {
    if(strcmp(str, listedStr) != 0)
      indexFound = true;

    ++(*index);
  }

  if(indexFound)
    return 0;

  return -ESRCH;
}

/**
 * @brief   Convert a string to upper case.
 *
 * @param[in,out] str: The string to convert.
 */
static void toUpper(char *str)
{
  size_t strLength = strlen(str);

  for(size_t i = 0; i < strlen; ++i)
    str[i] = toupper(str[i]);
}

/**
 * @brief   Execute the list datapoint types command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execListTypes(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  for(uint32_t i = 0; i < DATAPOINT_TYPE_COUNT; ++i)
    shell_info(shell, typeNames[i]);

  return 0;
}

/**
 * @brief   Execute the list datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execListDatapoint(const struct shell *shell, size_t argc, char **argv)
{
  int err;
  size_t datapointCount;
  DatapointType_t type;

  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  err = getStringIndex(argv[1], typeNames, DATAPOINT_TYPE_COUNT, (uint32_t *)&type);
  if(err < 0)
  {
    shell_error(shell, "FAIL: unknown datapoint type (%s)", argv[1]);
    shell_help(shell);
    return err;
  }

  datapointCount = datapointCounts[type];

  for(uint32_t i = 0; i < datapointCount; ++i)
    shell_info(shell, datapointNames[type][i]);

  return 0;
}

/**
 * @brief   Execute the list datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execReadDatapoint(const struct shell *shell, size_t argc, char **argv)
{
  int err;
  size_t datapointCount;
  DatapointType_t type;
  uint32_t datapointId;
  Datapoint_t value;
  char valueStr[DATASTORE_CMD_VALUE_STR_LENGTH + 1] = {'\0'};

  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  err = getStringIndex(argv[1], typeNames, DATAPOINT_TYPE_COUNT, (uint32_t *)&type);
  if(err < 0)
  {
    shell_error(shell, "FAIL: unknown datapoint type (%s)", argv[1]);
    shell_help(shell);
    return err;
  }

  toUpper(argv[2]);

  err = getStringIndex(argv[2], datapointNames[type], datapointCounts[type], &datapointId);
  if(err < 0)
  {
    shell_error(shell, "FAIL: unknown datapoint name %s of type %s", argv[2], argv[1]);
    shell_help(shell);
    return err;
  }

  err = dataStoreRead(type, datapointId, 1, &datastoreCmdResQueue, &value);
  if(err < 0)
  {
    shell_error(shell, "FAIL: error %d reading datapoint %s of type %s", err, argv[2], argv[1]);
    return err;
  }

  sprint(valueStr, valFormats[type], value);

  shell_info(shell, "SUCCESS: %s = %s", argv[2], valueStr);

  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(datastore_sub,
	SHELL_CMD(ls_types, NULL, "List the datapoint types.\n\tUsage: datastore ls_types", execListTypes),
  SHELL_CMD_ARG(ls, NULL, "List the datapoints of a type.\n\tUsage: datastore ls <float|uint|int|multi-state|button>",
                execListDatapoint, 2, 0);
	SHELL_CMD_ARG(read, NULL, "Read a datapoint.\n\tUsage: datastore read <float|uint|int|multi-state|button> <datapoint_name>",
                execReadDatapoint, 2, 0),
	SHELL_CMD_ARG(write, NULL, "Write a datapoint.\n\tUsage: datastore read <float|uint|int|multi-state|button> <datapoint_name>",
                execWriteDatapoint, 2, 0),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(datastore, &datastore_sub, APP_CMD_USAGE,	NULL);

/** @} */
