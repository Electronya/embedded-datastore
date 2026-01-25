/**
 * Copyright (C) 2026 by Electronya
 *
 * @file      main.c
 * @author    jbacon
 * @date      2026-01-24
 * @brief     Datastore Command Tests
 *
 *            Unit tests for datastore shell command functions.
 */

#include <zephyr/ztest.h>
#include <zephyr/fff.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

DEFINE_FFF_GLOBALS;

/* Prevent headers from being included */
#define DATASTORE_SRV
#define DATASTORE_META
#define SHELL_H__

/* Provide needed declarations */
struct shell;
struct k_msgq;

enum shell_vt100_color {
  SHELL_NORMAL = 0,
  SHELL_INFO,
  SHELL_ERROR,
  SHELL_WARNING
};

/* Button state enum (from datastoreMeta.h) */
typedef enum __attribute__((mode(SI)))
{
  BUTTON_UNPRESSED = 0,
  BUTTON_SHORT_PRESSED,
  BUTTON_LONG_PRESSED,
  BUTTON_STATE_COUNT
} ButtonState_t;

/* Datapoint counts - minimal for testing */
#define BINARY_DATAPOINT_COUNT 4
#define BUTTON_DATAPOINT_COUNT 2
#define FLOAT_DATAPOINT_COUNT 2
#define INT_DATAPOINT_COUNT 2
#define MULTI_STATE_DATAPOINT_COUNT 2
#define UINT_DATAPOINT_COUNT 2

/* Message queue count */
#define DATASTORE_MSG_COUNT 10

/* Captured shell output */
#define MAX_SHELL_OUTPUT_COUNT 16
#define MAX_SHELL_OUTPUT_LEN 256
static char captured_shell_output[MAX_SHELL_OUTPUT_COUNT][MAX_SHELL_OUTPUT_LEN];
static int shell_info_call_count = 0;
static int shell_error_call_count = 0;
static int shell_output_index = 0;

/* Mock for shell_fprintf (what shell_info and shell_error expand to) */
void shell_fprintf(const struct shell *shell, enum shell_vt100_color color,
                   const char *fmt, ...)
{
  va_list args;

  ARG_UNUSED(shell);

  if(shell_output_index < MAX_SHELL_OUTPUT_COUNT)
  {
    va_start(args, fmt);
    vsnprintf(captured_shell_output[shell_output_index], MAX_SHELL_OUTPUT_LEN, fmt, args);
    va_end(args);
    shell_output_index++;
  }

  if(color == SHELL_ERROR)
    shell_error_call_count++;
  else if(color == SHELL_INFO)
    shell_info_call_count++;
}

/* Mock shell functions */
FAKE_VALUE_FUNC(unsigned long, shell_strtoul, const char *, int, int *);
FAKE_VALUE_FUNC(long, shell_strtol, const char *, int, int *);
FAKE_VALUE_FUNC(bool, shell_strtobool, const char *, int, int *);
FAKE_VOID_FUNC(shell_help, const struct shell *);

/* Mock datastore API functions */
FAKE_VALUE_FUNC(int, datastoreReadBinary, uint32_t, size_t, struct k_msgq *, bool *);
FAKE_VALUE_FUNC(int, datastoreWriteBinary, uint32_t, bool *, size_t, struct k_msgq *);
FAKE_VALUE_FUNC(int, datastoreReadButton, uint32_t, size_t, struct k_msgq *, ButtonState_t *);
FAKE_VALUE_FUNC(int, datastoreWriteButton, uint32_t, ButtonState_t *, size_t, struct k_msgq *);
FAKE_VALUE_FUNC(int, datastoreReadFloat, uint32_t, size_t, struct k_msgq *, float *);
FAKE_VALUE_FUNC(int, datastoreWriteFloat, uint32_t, float *, size_t, struct k_msgq *);
FAKE_VALUE_FUNC(int, datastoreReadInt, uint32_t, size_t, struct k_msgq *, int32_t *);
FAKE_VALUE_FUNC(int, datastoreWriteInt, uint32_t, int32_t *, size_t, struct k_msgq *);
FAKE_VALUE_FUNC(int, datastoreReadMultiState, uint32_t, size_t, struct k_msgq *, uint32_t *);
FAKE_VALUE_FUNC(int, datastoreWriteMultiState, uint32_t, uint32_t *, size_t, struct k_msgq *);
FAKE_VALUE_FUNC(int, datastoreReadUint, uint32_t, size_t, struct k_msgq *, uint32_t *);
FAKE_VALUE_FUNC(int, datastoreWriteUint, uint32_t, uint32_t *, size_t, struct k_msgq *);

#define FFF_FAKES_LIST(FAKE) \
  FAKE(shell_strtoul) \
  FAKE(shell_strtol) \
  FAKE(shell_strtobool) \
  FAKE(shell_help) \
  FAKE(datastoreReadBinary) \
  FAKE(datastoreWriteBinary) \
  FAKE(datastoreReadButton) \
  FAKE(datastoreWriteButton) \
  FAKE(datastoreReadFloat) \
  FAKE(datastoreWriteFloat) \
  FAKE(datastoreReadInt) \
  FAKE(datastoreWriteInt) \
  FAKE(datastoreReadMultiState) \
  FAKE(datastoreWriteMultiState) \
  FAKE(datastoreReadUint) \
  FAKE(datastoreWriteUint)

/* Define shell macros for testing */
#define shell_info(shell, fmt, ...) shell_fprintf(shell, SHELL_INFO, fmt, ##__VA_ARGS__)
#define shell_error(shell, fmt, ...) shell_fprintf(shell, SHELL_ERROR, fmt, ##__VA_ARGS__)
#define SHELL_CMD(...)
#define SHELL_CMD_ARG(...)
#define SHELL_SUBCMD_SET_END
#define SHELL_STATIC_SUBCMD_SET_CREATE(...)
#define SHELL_CMD_REGISTER(...)

#undef STRINGIFY
#define STRINGIFY(x) #x

/* Suppress K_MSGQ_DEFINE from the source file - we mock it */
#undef K_MSGQ_DEFINE
#define K_MSGQ_DEFINE(name, ...) /* mocked */

/* Define datapoint X-macros for testing */
#define DATASTORE_BINARY_DATAPOINTS \
  X(BINARY_FIRST_DATAPOINT, 0, true) \
  X(BINARY_SECOND_DATAPOINT, 0, false) \
  X(BINARY_THIRD_DATAPOINT, 0, true) \
  X(BINARY_FOURTH_DATAPOINT, 0, false)

#define DATASTORE_BUTTON_DATAPOINTS \
  X(BUTTON_FIRST_DATAPOINT, 0, BUTTON_UNPRESSED) \
  X(BUTTON_SECOND_DATAPOINT, 0, BUTTON_UNPRESSED)

#define DATASTORE_FLOAT_DATAPOINTS \
  X(FLOAT_FIRST_DATAPOINT, 0, 0.0f) \
  X(FLOAT_SECOND_DATAPOINT, 0, 0.0f)

#define DATASTORE_INT_DATAPOINTS \
  X(INT_FIRST_DATAPOINT, 0, 0) \
  X(INT_SECOND_DATAPOINT, 0, 0)

#define DATASTORE_MULTI_STATE_DATAPOINTS \
  X(MULTI_STATE_FIRST_DATAPOINT, 0, 0) \
  X(MULTI_STATE_SECOND_DATAPOINT, 0, 0)

#define DATASTORE_UINT_DATAPOINTS \
  X(UINT_FIRST_DATAPOINT, 0, 0) \
  X(UINT_SECOND_DATAPOINT, 0, 0)

/* Mock message queue */
static struct k_msgq datastoreCmdResQueue;

/* Include command implementation */
#include "datastoreCmd.c"

/* Test suite setup and teardown */
static void *cmd_tests_setup(void)
{
  return NULL;
}

static void cmd_tests_before(void *f)
{
  ARG_UNUSED(f);
  FFF_FAKES_LIST(RESET_FAKE);
  FFF_RESET_HISTORY();

  memset(captured_shell_output, 0, sizeof(captured_shell_output));
  shell_info_call_count = 0;
  shell_error_call_count = 0;
  shell_output_index = 0;
}

/**
 * @test  The getStringIndex function must return -ESRCH when the string
 *        is not found in the list.
 */
ZTEST(datastore_cmd_tests, test_get_string_index_not_found)
{
  uint32_t index;
  int result;

  result = getStringIndex("UNKNOWN_DATAPOINT", binaryNames, BINARY_DATAPOINT_COUNT, &index);

  zassert_equal(result, -ESRCH, "getStringIndex should return -ESRCH when string is not found");
}

/**
 * @test  The getStringIndex function must return 0 and set the index
 *        when the string is found in the list.
 */
ZTEST(datastore_cmd_tests, test_get_string_index_found)
{
  uint32_t index;
  int result;

  result = getStringIndex("BINARY_SECOND_DATAPOINT", binaryNames, BINARY_DATAPOINT_COUNT, &index);

  zassert_equal(result, 0, "getStringIndex should return 0 when string is found");
  zassert_equal(index, 1, "Index should be 1 for BINARY_SECOND_DATAPOINT");
}

/**
 * @test  The toUpper function must convert all characters in the string
 *        to uppercase.
 */
ZTEST(datastore_cmd_tests, test_to_upper)
{
  char str[] = "binary_first_datapoint";

  toUpper(str);

  zassert_str_equal(str, "BINARY_FIRST_DATAPOINT", "toUpper should convert to uppercase");
}

/**
 * @test  The execListBinary function must return 0 and print the header
 *        followed by all binary datapoint names.
 */
ZTEST(datastore_cmd_tests, test_exec_list_binary)
{
  const struct shell *shell = (const struct shell *)0x1234;
  char *argv[] = {"ls"};
  int result;

  result = execListBinary(shell, 1, argv);

  zassert_equal(result, 0, "execListBinary should return 0");
  zassert_equal(shell_info_call_count, BINARY_DATAPOINT_COUNT + 1,
                "shell_info should be called for header + each datapoint");
  zassert_str_equal(captured_shell_output[0], "List of binary datapoint:",
                    "first shell_info output should be the header");
  zassert_str_equal(captured_shell_output[1], "BINARY_FIRST_DATAPOINT",
                    "second shell_info output should be BINARY_FIRST_DATAPOINT");
  zassert_str_equal(captured_shell_output[2], "BINARY_SECOND_DATAPOINT",
                    "third shell_info output should be BINARY_SECOND_DATAPOINT");
  zassert_str_equal(captured_shell_output[3], "BINARY_THIRD_DATAPOINT",
                    "fourth shell_info output should be BINARY_THIRD_DATAPOINT");
  zassert_str_equal(captured_shell_output[4], "BINARY_FOURTH_DATAPOINT",
                    "fifth shell_info output should be BINARY_FOURTH_DATAPOINT");
}

/**
 * @test  The execReadBinary function must return -ESRCH and print an error
 *        when the datapoint name is not found.
 */
ZTEST(datastore_cmd_tests, test_exec_read_binary_unknown_datapoint)
{
  const struct shell *shell = (const struct shell *)0x1234;
  char arg0[] = "read";
  char arg1[] = "unknown_datapoint";
  char *argv[] = {arg0, arg1};
  int result;

  result = execReadBinary(shell, 2, argv);

  zassert_equal(result, -ESRCH, "execReadBinary should return -ESRCH for unknown datapoint");
  zassert_equal(shell_error_call_count, 1,
                "shell_error should be called once");
  zassert_equal(shell_help_fake.call_count, 1,
                "shell_help should be called once");
  zassert_true(strstr(captured_shell_output[0], "FAIL") == captured_shell_output[0],
               "shell_error output should start with FAIL");
  zassert_true(strstr(captured_shell_output[0], "UNKNOWN_DATAPOINT") != NULL,
               "shell_error output should contain the datapoint name");
}

/* Custom fake for shell_strtoul that sets error parameter */
static unsigned long shell_strtoul_with_error(const char *str, int base, int *err)
{
  ARG_UNUSED(str);
  ARG_UNUSED(base);

  *err = -EINVAL;
  return 0;
}

/**
 * @test  The execReadBinary function must return -EINVAL and print an error
 *        when the value count argument is invalid.
 */
ZTEST(datastore_cmd_tests, test_exec_read_binary_invalid_value_count)
{
  const struct shell *shell = (const struct shell *)0x1234;
  char arg0[] = "read";
  char arg1[] = "binary_first_datapoint";
  char arg2[] = "invalid";
  char *argv[] = {arg0, arg1, arg2};
  int result;

  shell_strtoul_fake.custom_fake = shell_strtoul_with_error;

  result = execReadBinary(shell, 3, argv);

  zassert_equal(result, -EINVAL, "execReadBinary should return -EINVAL for invalid value count");
  zassert_equal(shell_strtoul_fake.call_count, 1,
                "shell_strtoul should be called once");
  zassert_equal(shell_error_call_count, 1,
                "shell_error should be called once");
  zassert_equal(shell_help_fake.call_count, 1,
                "shell_help should be called once");
  zassert_true(strstr(captured_shell_output[0], "FAIL") == captured_shell_output[0],
               "shell_error output should start with FAIL");
  zassert_true(strstr(captured_shell_output[0], "invalid") != NULL,
               "shell_error output should contain the invalid argument");
}

/**
 * @test  The execReadBinary function must return the error code and print an error
 *        when datastoreReadBinary fails.
 */
ZTEST(datastore_cmd_tests, test_exec_read_binary_datastore_read_fails)
{
  const struct shell *shell = (const struct shell *)0x1234;
  char arg0[] = "read";
  char arg1[] = "binary_first_datapoint";
  char *argv[] = {arg0, arg1};
  int result;

  datastoreReadBinary_fake.return_val = -EIO;

  result = execReadBinary(shell, 2, argv);

  zassert_equal(result, -EIO, "execReadBinary should return -EIO when datastore read fails");
  zassert_equal(datastoreReadBinary_fake.call_count, 1,
                "datastoreReadBinary should be called once");
  zassert_equal(datastoreReadBinary_fake.arg0_val, 0,
                "datastoreReadBinary should be called with datapoint ID 0");
  zassert_equal(datastoreReadBinary_fake.arg1_val, 1,
                "datastoreReadBinary should be called with value count 1");
  zassert_equal(shell_error_call_count, 1,
                "shell_error should be called once");
  zassert_equal(shell_help_fake.call_count, 1,
                "shell_help should be called once");
  zassert_true(strstr(captured_shell_output[0], "FAIL") == captured_shell_output[0],
               "shell_error output should start with FAIL");
}

/* Custom fake for datastoreReadBinary that sets output values */
static int datastoreReadBinary_success(uint32_t datapointId, size_t valCount,
                                       struct k_msgq *resQueue, bool *values)
{
  ARG_UNUSED(datapointId);
  ARG_UNUSED(resQueue);

  for(size_t i = 0; i < valCount; ++i)
    values[i] = (i % 2 == 0);

  return 0;
}

/**
 * @test  The execReadBinary function must return 0 and print the values
 *        when datastoreReadBinary succeeds with multiple values.
 */
ZTEST(datastore_cmd_tests, test_exec_read_binary_success)
{
  const struct shell *shell = (const struct shell *)0x1234;
  char arg0[] = "read";
  char arg1[] = "binary_first_datapoint";
  char arg2[] = "3";
  char *argv[] = {arg0, arg1, arg2};
  int result;

  shell_strtoul_fake.return_val = 3;
  datastoreReadBinary_fake.custom_fake = datastoreReadBinary_success;

  result = execReadBinary(shell, 3, argv);

  zassert_equal(result, 0, "execReadBinary should return 0 on success");
  zassert_equal(datastoreReadBinary_fake.call_count, 1,
                "datastoreReadBinary should be called once");
  zassert_equal(datastoreReadBinary_fake.arg0_val, 0,
                "datastoreReadBinary should be called with datapoint ID 0");
  zassert_equal(datastoreReadBinary_fake.arg1_val, 3,
                "datastoreReadBinary should be called with value count 3");
  zassert_equal(shell_info_call_count, 4,
                "shell_info should be called 4 times (header + 3 values)");
  zassert_str_equal(captured_shell_output[0], "SUCCESS: here are the values read",
                    "first shell_info output should be the success header");
  zassert_true(strstr(captured_shell_output[1], "BINARY_FIRST_DATAPOINT") != NULL,
               "second shell_info output should contain the first datapoint name");
  zassert_true(strstr(captured_shell_output[1], "true") != NULL,
               "second shell_info output should contain true");
  zassert_true(strstr(captured_shell_output[2], "BINARY_SECOND_DATAPOINT") != NULL,
               "third shell_info output should contain the second datapoint name");
  zassert_true(strstr(captured_shell_output[2], "false") != NULL,
               "third shell_info output should contain false");
  zassert_true(strstr(captured_shell_output[3], "BINARY_THIRD_DATAPOINT") != NULL,
               "fourth shell_info output should contain the third datapoint name");
  zassert_true(strstr(captured_shell_output[3], "true") != NULL,
               "fourth shell_info output should contain true");
}

ZTEST_SUITE(datastore_cmd_tests, NULL, cmd_tests_setup, cmd_tests_before, NULL, NULL);
