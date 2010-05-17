#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <glib.h>

#include "dismemberd.h"

/**
 * Adds DMD_GROUP_x variable to the environment for one group.  This 
 * function is called once for each group by setup_script_env().
 *
 * \param n Pointer to a struct group.
 * \param data Pointer to an integer.  This is the group counter.
 */
static void set_group_env(gpointer n, gpointer data) {
	struct group *grp = (struct group *)n;
	int *i = (int *)data;
	char *varname;
	int len;

	len = strlen("DMD_GROUP_")
		+ (*i == 0 ? 1 : log10(*i))
		+ 2;
	g_message("got len %d for group %d (%s)",
			len, *i, grp->name);
	varname = (char *)malloc(len);
	snprintf(varname, len, "DMD_GROUP_%d", *i);
	setenv(varname, grp->name, 1);
	free(varname);
}

/**
 * Sets up environment variables before calling script.  This function
 * populates the environment with DMD_GROUP_LIST_DIR (path to group
 * list directory) and DMD_NUM_GROUPS (number of groups of which we are 
 * a member), and then calls set_group_env() once for each group to
 * create the necessary DMD_GROUP_x variables.
 */
static void setup_script_env() {
	int i = 0;
	int len;
	char *numgroups;

	len = log10(g_list_length(groups)) + 2;
	numgroups = (char *)malloc(len);
	snprintf(numgroups, len, "%d", g_list_length(groups));

	setenv("DMD_GROUP_LIST_DIR", group_list_dir, 1);
	setenv("DMD_NUM_GROUPS", numgroups, 1);
	g_list_foreach(groups, set_group_env, &i);

	free(numgroups);
}

void execute_confchange_script() {
	int pid;
	int status;

	if (! execute_script) return;

	if ((pid = fork()) == -1) {
		g_critical("fork failed: %s", strerror(errno));
		exit(1);
	} else if (pid == 0) {
		/* child */
		g_message("running confchange script %s",
				execute_script_path);
		setup_script_env();
		execl(execute_script_path, execute_script_path, NULL);
		exit(111);
	}

	/* parent */
	wait(&status);
	g_message("confchange script returned with status = %d", status);
}

