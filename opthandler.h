#ifndef __OPTHANDLER_H__
#define __OPTHANDLER_H__

#include <stddef.h>

/* opthandler_usage prints ("%s [options] %s", argv[0], opthandler_argsname) */
char * opthandler_argsname /* by default, it is set to
  = "args" */;

/* If you want to free the "-h" option (with, for instance, "-?"), change it */
char opthandler_help_char /* by default, it is set to
  = 'h' */;

union value {
  char * string;
  int flag;
};

struct opthandler_option {
  /* Short description that will be printed in usage. You may supply "" */
  char *		usage_description;

  /* Short name for the option. You may supply '\0' to disable */
  char			char_name;		/* e.g. 'o' */

  /* Long name for the option. You may supply NULL to disable */
  char *		long_name;		/* e.g.	"output-file" */

  /* Name of the option's argument (if any). NULL if no argument required */
  char *		arg_name;		/* e.g.	"filename" */

  /*	This is where the value is stored after arg parsing	*
   *                           					*
   * + for boolean-flag options (i.e. arg_name == NULL),	*
   *   \-> when the option is supplied, value.flag is set to 1	*
   *   \-> you should initialise this with `arg_flag`,		*
   *                           					*
   * + for options with argument (i.e. arg_name != NULL),	*
   *   \-> when the option is supplied,	value.string is set to 	*
   * a copy (strdup) of the supplied argument			*
   *   \-> you should initiliase this with `arg_default(...)`	*
   *                           					*/
  union value		value;			/* e.g. arg_default("-") */
};

#define arg_flag	((union value) {(int) 0})
#define arg_default(x)	((union value) {(char *) (x)})

/*
 * Main function, see example.c to see it in action.
 * You MUST init the opthandler first with the options and its parameters.
 * Then you can handle the options in the arguments with something like
 *             argv = opthandler_handle_opts(argv);
 * (argv will then point to the first argument that is not an option)
 * (POSIX-style)
 */
void opthandler_handle_opts (int * at_argc, char *** at_argv);


void opthandler_init (size_t options_count,
                     struct opthandler_option * options,
                     char * usage_intro_msg);

void opthandler_free (void);

void opthandler_usage (int exit_code) __attribute__((noreturn));

#endif /* __OPTHANDLER_H__ */

