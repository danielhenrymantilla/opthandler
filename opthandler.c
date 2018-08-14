#include "opthandler.h"
#include <ctype.h>
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

#define perror_fatal(s) do { \
  perror(s); \
  exit(EXIT_FAILURE); \
} while (0)


static char * strdup_toupper (char * s)
{
  char * buf = malloc(strlen(s) + 1);
  if (!buf) perror_fatal("malloc");
  size_t i;
  for (i = 0; s[i]; ++i)
    buf[i] = toupper(s[i]);
  buf[i] = '\0';
  return buf;
}

char * opthandler_argsname = "args";

char opthandler_help_char = 'h';

static size_t options_count;
static struct opthandler_option * options;
static char * usage_intro_msg;
static char * progname;

#define opthandler_fail(fmt, ...) do { \
  fprintf(stderr, "Error: " fmt ".\n", ##__VA_ARGS__); \
  opthandler_usage(EXIT_FAILURE); \
} while (0)

static struct opthandler_option * getopt_by_char_name (char char_name)
{
  if (char_name == opthandler_help_char)
    opthandler_usage(EXIT_SUCCESS);
  for (size_t i = 0; i < options_count; ++i) {
    if (options[i].char_name == char_name)
      return &options[i];
  }
  opthandler_fail("unrecognised option '-%c'", char_name);
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
  opthandler_fail("unrecognised option '--%s'", long_name);
}

void opthandler_handle_opts (int * at_argc, char *** at_argv)
{
#define argc (*at_argc)
#define argv (*at_argv)
  if (!options)
    print_fail("opthandler_handle_opts error: opthandler not initialised.\n");
  progname = strdup(*argv);
  --argc;
  if (!progname) perror_fatal("strdup");
  for (char * arg; (arg = *(++argv)); --argc) {
    if (arg[0] != '-')				/* > Not an option */
      break;
    if (arg[1] == '\0')		/* syntax error: got '-' */
      opthandler_fail("syntax error at '%s'", arg);
    struct opthandler_option * option;
    if (arg[1] != '-') {	/* > short (char) name */
      option = getopt_by_char_name(arg[1]);
      if (!option->arg_name) {		/* no arg_name => boolean flag */
        option->value.flag = 1;
        if (arg[2] != '\0')
          opthandler_fail("extraneous argument '%s' for option '-%c'",
                           &arg[2], arg[1]);
      } else {				/* argument required */
        if (option->value.string)
          free(option->value.string);
        if (arg[2] == '\0') {			/* arg is next argv */
          if (!(++argv)) {
            opthandler_fail("missing argument '%s' for option '%s'",
                            option->arg_name, arg);
          }
          --argc;
          /* Use strdup to achieve a persistent/global scope for the args */
          if (!(option->value.string = strdup(*argv))) perror_fatal("strdup");
        } else {				/* arg is adjacent */
          if (!(option->value.string = strdup(&arg[2]))) perror_fatal("strdup");
        }
      }
    } else {					/* > long name */
      char * argptr = strchr(&arg[2], '=');
      if (argptr) {			/* argument provided */
        *argptr = '\0';
        option = getopt_by_long_name(&arg[2]);
        if (!option->arg_name)		/* yet argument not required */
          opthandler_fail("extraneous argument '%s' for option '%s'",
                          argptr + 1, arg);
        *argptr = '='; /* Put back the '=' sign to leave argv args untouched */
        if (option->value.string) free(option->value.string);
        /* Use strdup to achieve a persistent/global scope for the args */
        if (!(option->value.string = strdup(argptr + 1)))
          perror_fatal("strdup");
      } else {				/* argument not provided */
        option = getopt_by_long_name(&arg[2]);
        if (option->arg_name) {		/* yet argument required */
          opthandler_fail("please specify a %s for option '%s'",
                          option->arg_name, arg);
        }
        option->value.flag = 1;
      }
    }
  }
#undef argv
#undef argc
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
      if (!option->value.string) perror_fatal("strdup");
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

static void display_option (char cn, char * ln, char * an, char * ud)
{
  char * buf = malloc(20 + (ln ? strlen(ln) : 0) + (an ? strlen(an) : 0));
  if (!buf) perror_fatal("malloc");
  size_t n = 0;
  if (cn != '\0')
    n += sprintf(buf + n, "  -%c", cn);
  else
    n += sprintf(buf + n, "    ");
  n += sprintf(buf + n, "%c", (cn != '\0' && ln) ? ',' : ' ');
  if (ln)
    n += sprintf(buf + n, " --%s%s", ln, an ? "=" : "");
  if (an) {
    char * an_upper = strdup_toupper(an);
    n += sprintf(buf + n, "%s", an_upper);
    free(an_upper);
  }
  if (n < 30)
    fprintf(stderr, "%-30s%s\n", buf, ud);
  else
    fprintf(stderr, "%s\n%-30s%s\n", buf, "", ud);
  free(buf);
}

__attribute__((noreturn)) void opthandler_usage (int exit_code)
{
  if (!options)
    print_fail("opthandler_usage error: opthandler not initialised.\n");
  fprintf(stderr, "\nUsage: %s [options] %s\n",
    progname ? progname : "program",
    opthandler_argsname);
  fprintf(stderr, "%s\nOptions:\n", usage_intro_msg);
  display_option(opthandler_help_char, "help", NULL, "display this help");
  for (size_t i = 0; i < options_count; ++i) {
    struct opthandler_option * option = &options[i];
    display_option(option->char_name, option->long_name, option->arg_name,
      option->usage_description);
  }
  fprintf(stderr, "\n");
  exit(exit_code);
}
