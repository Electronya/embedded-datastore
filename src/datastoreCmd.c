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
#include <stdlib.h>
#include <string.h>

#include "datastore.h"
#include "datastoreMeta.h"

/**
 * @brief   The string for true binary values.
 */
#define TRUE_STR                                                        "true"

/**
 * @brief   The string for false binary values.
 */
#define FALSE_STR                                                       "false"

/**
 * @brief   The string for the unpressed button value.
 */
#define UNPRESSED_STR                                                   "unpressed"

/**
 * @brief   The string for the short pressed button value.
 */
#define SHORT_PRESSED_STR                                               "short pressed"

/**
 * @brief   The string for the long pressed button value.
 */
#define LONG_PRESSED_STR                                                "long pressed"

/**
 * @brief   The list of binary datapoint names.
 */
static char *binaryNames[BINARY_DATAPOINT_COUNT] = {
#define X(name, flags, defaultVal) STRINGIFY(name),
  DATASTORE_BINARY_DATAPOINTS
#undef X
};

/**
 * @brief   The list of button datapoint names.
 */
static char *buttonNames[BUTTON_DATAPOINT_COUNT] = {
#define X(name, flags, defaultVal) STRINGIFY(name),
  DATASTORE_BUTTON_DATAPOINTS
#undef X
};

// /**
//  * @brief   The list of float datapoint names.
//  */
// static char *floatNames[FLOAT_DATAPOINT_COUNT] = {
// #define X(name, flags, defaultVal) STRINGIFY(name),
//   DATASTORE_FLOAT_DATAPOINTS
// #undef
// };

// /**
//  * @brief   The list of unsigned integer datapoint names.
//  */
// static char *uintNames[UINT_DATAPOINT_COUNT] = {
// #define X(name, flags, defaultVal) STRINGIFY(name),
//   DATASTORE_UINT_DATAPOINTS
// #undef
// };

// /**
//  * @brief   The list of signed integer datapoint names.
//  */
// static char *intNames[INT_DATAPOINT_COUNT] = {
// #define X(name, flags, defaultVal) STRINGIFY(name),
//   DATASTORE_INT_DATAPOINTS
// #undef
// };

// /**
//  * @brief   The list of multi-state datapoint names.
//  */
// static char *multiStateNames[MULTI_STATE_DATAPOINT_COUNT] = {
// #define X(name, flags, defaultVal) STRINGIFY(name),
//   DATASTORE_MULTI_STATE_DATAPOINTS
// #undef
// };

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

  while(!indexFound && *index < listSize)
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

  for(size_t i = 0; i < strLength; ++i)
    str[i] = toupper(str[i]);
}

/**
 * @brief   Convert a bool string.
 *
 * @param[in]   str: The bool string.
 * @param[out]  value: The converted bool value.
 *
 * @return  0 if the conversion is successful, the error code otherwise.
 */
static int convertBinStr(char *str, bool *value)
{
  if(strcmp(str, TRUE_STR) == 0)
  {
    *value = true;
    return 0;
  }

  if(strcmp(str, FALSE_STR) == 0)
  {
    *value = false;
    return 0;
  }

  return -EINVAL;
}

/**
 * @brief   Execute the list binary datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execListBinary(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  shell_info(shell, "List of binary datapoint:");

  for(size_t i = 0; i < BINARY_DATAPOINT_COUNT; ++i)
    shell_info(shell, "%s", binaryNames[i]);

  return 0;
}

/**
 * @brief   Execute the read binary datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execReadBinary(const struct shell *shell, size_t argc, char **argv)
{
  int err;
  uint32_t datapointId;
  size_t valCount;
  bool values[BINARY_DATAPOINT_COUNT];
  char *valStr;

  toUpper(argv[1]);

  err = getStringIndex(argv[1], binaryNames, BINARY_DATAPOINT_COUNT, &datapointId);
  if(err < 0)
  {
    shell_error(shell, "FAIL: unkown datapoint %s", argv[1]);
    shell_help(shell);
    return err;
  }

  valCount = argc == 3 ? shell_strtoul(argv[2], 10, &err) : 0;
  if(err < 0)
  {
    shell_error(shell, "FAIL: invalid value count to read %s", argv[2]);
    shell_help(shell);
    return err;
  }

  err = datastoreReadBinary(datapointId, valCount, &datastoreCmdResQueue, values);
  if(err < 0)
  {
    shell_error(shell, "FAIL: read operation fail with %d error code", err);
    shell_help(shell);
    return err;
  }

  shell_info(shell, "SUCCESS: here are the values read");

  for(size_t i = 0; i < valCount; ++i)
  {
    valStr = values[i] ? TRUE_STR : FALSE_STR;
    shell_info(shell, "%s: %s", binaryNames[datapointId + i], valStr);
  }

  return 0;
}

/**
 * @brief   Execute the write binary datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execWriteBinary(const struct shell *shell, size_t argc, char **argv)
{
  int err;
  uint32_t datapointId;
  size_t valCount;
  bool values[BINARY_DATAPOINT_COUNT];

  toUpper(argv[1]);

  err = getStringIndex(argv[1], binaryNames, BINARY_DATAPOINT_COUNT, &datapointId);
  if(err < 0)
  {
    shell_error(shell, "FAIL: unkown datapoint %s", argv[1]);
    shell_help(shell);
    return err;
  }

  valCount = shell_strtoul(argv[2], 10, &err);
  if(err < 0)
  {
    shell_error(shell, "FAIL: invalid value count to write %s", argv[2]);
    shell_help(shell);
    return err;
  }

  if(valCount < argc - 3)
  {
    shell_error(shell, "FAIL: not enough value provided (%d) for the requested value to write (%d)", valCount, argc - 3);
    shell_help(shell);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
  {
    err = convertBinStr(argv[2 + i], values + i);
    if(err < 0)
    {
      shell_error(shell, "FAIL: bad binary value %s for value %i", argv[2 + i], i);
      shell_help(shell);
      return err;
    }
  }

  err = datastoreWriteBinary(datapointId, values, valCount, &datastoreCmdResQueue);
  if(err < 0)
  {
    shell_error(shell, "FAIL: read operation fail with %d error code", err);
    shell_help(shell);
    return err;
  }

  shell_info(shell, "SUCCESS: write operation of %s up to %s done", binaryNames[datapointId], binaryNames[datapointId + valCount - 1]);

  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_binary,
	SHELL_CMD(ls, NULL, "List binary objects.\n\tUsage datastore binary_data ls", execListBinary),
  SHELL_CMD_ARG(read, NULL, "Read a binary datapoint.\n\tUsage datastore binary_data read <datapoint ID> [value count]", execReadBinary, 2, 1),
	SHELL_CMD_ARG(write, NULL, "Write a binary datapoint.\n\tUsage datastore binary_data write <datapoint ID> <value count> <true|false> [true|false] ...",
                execWriteBinary, 3, SHELL_OPT_ARG_CHECK_SKIP),
	SHELL_SUBCMD_SET_END);

/**
 * @brief   Convert a string to button state.
 *
 * @param str
 * @param value
 * @return int
 */
int convertButtonStateStr(char *str, ButtonState_t *value)
{
  if(strcmp(str, UNPRESSED_STR) == 0)
  {
    *value = BUTTON_UNPRESSED;
    return 0;
  }

  if(strcmp(str, SHORT_PRESSED_STR) == 0)
  {
    *value = BUTTON_SHORT_PRESSED;
    return 0;
  }

  if(strcmp(str, LONG_PRESSED_STR) == 0)
  {
    *value = BUTTON_LONG_PRESSED;
    return 0;
  }

  return -EINVAL;
}

/**
 * @brief   Execute the list button datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execListButton(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

  shell_info(shell, "List of button datapoint:");

  for(size_t i = 0; i < BUTTON_DATAPOINT_COUNT; ++i)
    shell_info(shell, "%s", buttonNames[i]);

  return 0;
}

/**
 * @brief   Execute the read button datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execReadButton(const struct shell *shell, size_t argc, char **argv)
{
  int err;
  uint32_t datapointId;
  size_t valCount;
  ButtonState_t values[BUTTON_DATAPOINT_COUNT];
  char *valStr;

  toUpper(argv[1]);

  err = getStringIndex(argv[1], buttonNames, BUTTON_DATAPOINT_COUNT, &datapointId);
  if(err < 0)
  {
    shell_error(shell, "FAIL: unkown datapoint %s", argv[1]);
    shell_help(shell);
    return err;
  }

  valCount = argc == 3 ? shell_strtoul(argv[2], 10, &err) : 0;
  if(err < 0)
  {
    shell_error(shell, "FAIL: invalid value count to read %s", argv[2]);
    shell_help(shell);
    return err;
  }

  err = datastoreReadButton(datapointId, valCount, &datastoreCmdResQueue, values);
  if(err < 0)
  {
    shell_error(shell, "FAIL: read operation fail with %d error code", err);
    shell_help(shell);
    return err;
  }

  shell_info(shell, "SUCCESS: here are the values read");

  for(size_t i = 0; i < valCount; ++i)
  {
    valStr = values[i] == BUTTON_SHORT_PRESSED ? SHORT_PRESSED_STR : UNPRESSED_STR;
    valStr = values[i] == BUTTON_LONG_PRESSED ? LONG_PRESSED_STR : valStr;
    shell_info(shell, "%s: %s", buttonNames[datapointId + i], valStr);
  }

  return 0;
}

/**
 * @brief   Execute the write button datapoints command.
 *
 * @param[in]   shell: The shell handle.
 * @param[in]   argc: The count of argument.
 * @param[in]   argv: The vector of argument.
 *
 * @return  0 if successful the error code otherwise.
 */
static int execWriteButton(const struct shell *shell, size_t argc, char **argv)
{
  int err;
  uint32_t datapointId;
  size_t valCount;
  ButtonState_t values[BUTTON_DATAPOINT_COUNT];

  toUpper(argv[1]);

  err = getStringIndex(argv[1], binaryNames, BUTTON_DATAPOINT_COUNT, &datapointId);
  if(err < 0)
  {
    shell_error(shell, "FAIL: unkown datapoint %s", argv[1]);
    shell_help(shell);
    return err;
  }

  valCount = shell_strtoul(argv[2], 10, &err);
  if(err < 0)
  {
    shell_error(shell, "FAIL: invalid value count to write %s", argv[2]);
    shell_help(shell);
    return err;
  }

  if(valCount < argc - 3)
  {
    shell_error(shell, "FAIL: not enough value provided (%d) for the requested value to write (%d)", valCount, argc - 3);
    shell_help(shell);
    return err;
  }

  for(size_t i = 0; i < valCount; ++i)
  {
    err = convertButtonStateStr(argv[2 + i], values + i);
    if(err < 0)
    {
      shell_error(shell, "FAIL: bad button value %s for value %i", argv[2 + i], i);
      shell_help(shell);
      return err;
    }
  }

  err = datastoreWriteButton(datapointId, values, valCount, &datastoreCmdResQueue);
  if(err < 0)
  {
    shell_error(shell, "FAIL: read operation fail with %d error code", err);
    shell_help(shell);
    return err;
  }

  shell_info(shell, "SUCCESS: write operation of %s up to %s done", binaryNames[datapointId], binaryNames[datapointId + valCount - 1]);

  return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_button,
	SHELL_CMD(ls, NULL, "List button objects.\n\tUsage datastore button_data ls", execListButton),
  SHELL_CMD_ARG(read, NULL, "Read a button datapoint.\n\tUsage datastore button_data read <datapoint ID> [value count]", execReadButton, 2, 1),
	SHELL_CMD_ARG(write, NULL, "Write a button datapoint.\n\tUsage datastore button_data write <datapoint ID> <value count> <true|false> [true|false] ...",
                execWriteButton, 3, SHELL_OPT_ARG_CHECK_SKIP),
	SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(datastore_sub,
	SHELL_CMD(binary_data, &sub_binary, "Binaries datapoint commands.", NULL),
  SHELL_CMD(button_data, &sub_button, "Button datapoint commands.", NULL),
	// SHELL_CMD(float_data, &sub_float, "Float datapoint commands.", NULL),
	// SHELL_CMD(int_data, &sub_int, "Signed integer datapoint commands.", NULL),
  // SHELL_CMD(multi_state_data, &sub_multi_state, "Multi-state datapoint commands.", NULL),
  // SHELL_CMD(uint_data, &sub_uint, "Unsigned integer datapoint commands.", NULL),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(datastore, &datastore_sub, "Datastore commands.",	NULL);

/** @} */
