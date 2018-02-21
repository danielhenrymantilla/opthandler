#include "opthandler.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef print_fail
 #define print_fail(...) do { \
  fprintf(stderr, __VA_ARGS__); \
  exit(EXIT_FAILURE); \
 } while (0)
#endif

char * opthandler_argsname = "args";

static size_t options_count;
static struct opthandler_option * options;
static char * usage_intro_msg;
static char * progname;

static struct opthandler_option * getopt_by_char_name (char char_name)
{
  if (char_name == 'h')
    opthandler_usage(EXIT_SUCCESS);
  for (size_t i = 0; i < options_count; ++i) {
    if (options[i].char_name == char_name)
      return &options[i];
  }
  return NULL;
}

static struct opthandler_option * getopt_by_long_name (char * long_name)
{
  if (!long_name) return NULL;
  if (strcmp(long_name, "help") == 0)
    opthandler_usage(EXIT_SUCCESS);
  for (size_t i = 0; i < options_count; ++i) {
    char * option_name = options[i].long_name;
    if (option_name && strcmp(option_name, long_name) == 0)
      return &options[i];
  }
  return NULL;
}

#define opthandler_fail(fmt, ...) do { \
  fprintf(stderr, "Error: " fmt ".\n", ##__VA_ARGS__); \
  opthandler_usage(EXIT_FAILURE); \
} while (0)

char * const * opthandler_handle_opts (char * const * argv)
{
  if (!options)
    print_fail("opthandler_handle_opts error: opthandler not initialised.\n");
  progname = strdup(*(argv++));
  if (!progname) perror("strdup");
  for (char * arg; (arg = *argv); ++argv) {
    if (arg[0] != '-')				/* Not an option */
      return argv;
    struct opthandler_option * option;
    if (arg[1] != '\0' && arg[2] == '\0') {	/* short (char) name */
      option = getopt_by_char_name(arg[1]);
    } else {					/* long name */
      if (arg[1] != '-')			/* syntax error */
        opthandler_fail("syntax error at '%s'", arg);
      char * argptr = strchr(&arg[2], '=');
      if (argptr) {				/* argument provided */
        *argptr = '\0';
        option = getopt_by_long_name(&arg[2]);
        if (!option)
          opthandler_fail("unrecognised option '%s'", arg);
        if (option->arg_name) {			/* handle option's argument */
          if (option->value.string)
            free(option->value.string);
          /* Use strdup to achieve a persistent/global scope for the args */
          if (!(option->value.string = strdup(argptr + 1)))
            perror("strdup");
          continue;
        } else {				/* argument not required */
          opthandler_fail("option '%s' does not require an argument (got %s)",
                          arg, argptr + 1);
        }
      } else
        option = getopt_by_long_name(&arg[2]);
    }
    if (!option)
      opthandler_fail("unrecognised option '%s'", arg);
    /* option should point to the corresponding option struct */
    if (!option->arg_name) {		/* no args_name => boolean flag */
      option->value.flag = 1;
    } else {				/* argument required => ++argv */
      if (!(++argv)) {
        opthandler_fail("missing argument '%s' for option %s",
                        option->arg_name, arg);
      }
      if (option->value.string) free(option->value.string);
      /* Use strdup to achieve a persistent/global scope for the args */
      if (!(option->value.string = strdup(*argv))) perror("strdup");
    }

  }
  return argv;
}

void opthandler_init (size_t _options_count,
                     struct opthandler_option * _options,
                     char * _usage_intro_msg)
{
  if (!_options) print_fail("opthandler_init: error, got NULL options.\n");
  options = _options;
  options_count = _options_count;
  usage_intro_msg = _usage_intro_msg;
  for (size_t i = 0; i < options_count; ++i) {
    struct opthandler_option * option = &options[i];
    if (!option) print_fail("opthandler_init: error, got NULL option.\n");
    if (!option->arg_name) {
      if (option->value.flag) fprintf(stderr,
        "opthandler_init: warning, got non 0 starting flag.\n");
    } else if (option->value.string) {
      option->value.string = strdup(option->value.string);
      if (!option->value.string) perror("strdup");
    }
  }
}

void opthandler_free (void)
{
  if (progname) free(progname);
  if (!options)
    print_fail("opthandler_free error: opthandler not initialised.\n");
  for (size_t i = 0; i < options_count; ++i) {
    struct opthandler_option * option = &options[i];
    if (option->arg_name && option->value.string)
      free(option->value.string);
  }
}

__attribute__((noreturn)) void opthandler_usage (int exit_code)
{
  if (!options)
    print_fail("opthandler_usage error: opthandler not initialised.\n");
  if (exit_code == EXIT_SUCCESS)
    fprintf(stderr, "%s\n", usage_intro_msg);
  fprintf(stderr, "\nUsage: %s [options] %s\n",
    progname ? progname : "program",
    opthandler_argsname);
  fprintf(stderr, "Options:\n");
  for (size_t i = 0; i < options_count; ++i) {
    struct opthandler_option * option = &options[i];
    if (option->long_name)
      fprintf(stderr, " --%s\t%s\t%s\n",
        option->long_name,
        option->arg_name ? option->arg_name : "\t",
        option->usage_description);
    if (option->char_name != '\0')
      fprintf(stderr, " -%c\t\t%s\t%s\n",
        option->char_name,
        option->arg_name ? option->arg_name : "\t",
        option->usage_description);
  }
  fprintf(stderr, "\n");
  exit(exit_code);
}
