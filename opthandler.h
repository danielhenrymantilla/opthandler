#ifndef __OPTHANDLER_H__
#define __OPTHANDLER_H__

#include <stddef.h>

struct opthandler_option {
  /* Short description that will be printed in usage. You may supply "" */
  char *		usage_description;

  /* Short name for the option. You may supply '\0' to disable */
  char			char_name;		/* e.g. 'o' */

  /* Long name for the option. You may supply NULL to disable */
  char *		long_name;		/* e.g.	"output-file" */

  /* Name of the option's argument (if any). NULL if no argument required */
  char *		arg_name;		/* e.g.	"filename" */

  /* This is where the value is stored after parsing:		*
   * -> for boolean-flag options (i.e. without argument),	*
   * value.flag gets set to 1 if the option is supplied,	*
   * -> for options with argument,				*
   * value.string gets a copy (strdup) of the supplied argument */
  union {
    char * string;				/* default value: e.g. "-" */
    int    flag;
  }			value;
};

/*
 * Main function, see example.c to see it in action.
 * You MUST init the opthandler first with the options and its parameters.
 * Then you can handle the options in the arguments with something like
 *             argv = opthandler_handle_opts(argv);
 * (argv will then point to the first argument that is not an option)
 * (POSIX-style)
 */
char * const * opthandler_handle_opts (char * const * argv);


void opthandler_init (size_t options_count,
                     struct opthandler_option * options,
                     char * usage_intro_msg);

void opthandler_free (void);

/* opthandler_usage prints ("%s [options] %s", argv[0], opthandler_argsname) */
char * opthandler_argsname /* by default, it is set to
  = "args" */;

void opthandler_usage (int exit_code) __attribute__((noreturn));

#endif /* __OPTHANDLER_H__ */

