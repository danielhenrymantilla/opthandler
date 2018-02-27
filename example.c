#include <stdlib.h>
#include <stdio.h>
#include "opthandler.h"

enum options_order {
  o_OUTPUT_FILE,
  o_VERBOSE,
  o_NO_COLORS,
};

struct opthandler_option options[] = {
  [o_VERBOSE] = {
    "enable verbose output",
    'v',	"verbose",	NULL,		arg_flag},
  [o_OUTPUT_FILE] = {
    "set the output file",
    'o',	"output-file",	"filename",	arg_default("-")},
  [o_NO_COLORS] = {
    "disable colored output in console",
    '\0',	"no-colors",	NULL,		arg_flag},
};

#define array_count(a) ( sizeof(a) / sizeof(*(a)) )

int main (int argc, char * argv[])
{
  opthandler_init(array_count(options), options, "Example program");
  opthandler_handle_opts(&argc, &argv);
  printf("Remaining %d args: '", argc);
  for (size_t i = 0; argv[i]; ++i)
    printf("%s%s", (i ? " " : ""), argv[i]);
  printf ("'\n"
          "output-file: '%s'\n"
          "verbose = %s\n"
          "colors = %s\n",
          options[o_OUTPUT_FILE].value.string,
          options[o_VERBOSE].value.flag ? "yes" : "no",
          options[o_NO_COLORS].value.flag ? "no" : "yes");
  opthandler_free();
  return 0;
}
