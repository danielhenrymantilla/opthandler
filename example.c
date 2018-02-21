#include <stdlib.h>
#include <stdio.h>
#include "opthandler.h"

enum {
  verbose,
  outputfile,
  no_colors,
};

struct opthandler_option options[] = {
  [verbose] = {"enable verbose output",
    'v', "verbose",	NULL,		arg_flag},
  [outputfile] = {"set the output file",
    'o', "output-file",	"filename",	arg_default("-")},
  [no_colors] = {"disable colored output in console",
    '\0', "no-colors",	NULL,		arg_flag},
};

#define array_count(a) ( sizeof(a) / sizeof(*(a)) )

int main (int argc, char * const * argv)
{
  opthandler_init(array_count(options), options, "Example program");
  argv = opthandler_handle_opts(argv);
  printf("Remaining args: '");
  for (size_t i = 0; argv[i]; ++i)
    printf("%s%s", (i ? " " : ""), argv[i]);
  printf ("'\n"
          "output-file: '%s'\n"
          "verbose = %s\n"
          "colors = %s\n",
          options[outputfile].value.string,
          options[verbose].value.flag ? "yes" : "no",
          options[no_colors].value.flag ? "no" : "yes");
  opthandler_free();
  return 0;
}
