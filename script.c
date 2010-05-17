#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <glib.h>

#include "dismemberd.h"

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

