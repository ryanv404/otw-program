#include "data_utils.h"

#include <stdio.h>
#include <stdlib.h>

#include "validate.h"

//XXX
int
store_pw(char *pw, char *level_name)
{
	arg_t split_arg;
	level_t level;

	/* Split level_name into name and number and determine if it's valid. */
	sscanf(level_name, "%[a-zA-Z]%d", split_arg->level_name, &split_arg->level_num);
	if (!is_valid_level(&split_arg)) {
		show_usage();
		exit(EXIT_FAILURE);
	}

	get_level_info(&split_arg, &level);

	printf("The password (%s) for level (%s) was stored locally.\n", pw, level_name);
	exit(EXIT_SUCCESS);
}
