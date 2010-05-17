/*
 *  This file is part of Dismemberd.
 *
 *  Dismemberd is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Dismemberd is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dismemberd.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>
#include <corosync/swab.h>

#include "dismemberd.h"

GList	*groups = NULL;

static void init_cpg_group (gpointer n, gpointer data) {
	int result;
	struct group *grp = (struct group *)n;

	result = cpg_initialize (&(grp->handle),
			no_callbacks ? NULL : &callbacks);
	if (result != CS_OK) {
		g_critical("group %s: could not initialize CPG API: error %d", grp->name, result);
		exit(1);
	} else {
		g_message("initialized group %s.", grp->name);
	}
	cpg_fd_get(grp->handle, &(grp->select_fd));
}

void init_cpg_groups() {
	g_list_foreach(groups, init_cpg_group, NULL);
}

static void join_cpg_group (gpointer n, gpointer data) {
	int result;
	struct cpg_name group_name;
	struct group *grp = (struct group *)n;

	g_message("joining group: %s", grp->name);

	strcpy(group_name.value, grp->name);
	group_name.length = strlen(grp->name);
	result = cpg_join(grp->handle, &group_name);

	if (result != CS_OK) {
		g_critical ("could not join process group: error %d", result);
		exit (1);
	}
}

void join_cpg_groups() {
	g_list_foreach(groups, join_cpg_group, NULL);
}

void dispatch_group(gpointer n, gpointer data) {
	struct group *grp = (struct group *)n;
	fd_set *read_fds = (fd_set *)data;

	if (FD_ISSET (grp->select_fd, read_fds))
		if (cpg_dispatch (grp->handle, CS_DISPATCH_ALL) != CS_OK)
			g_critical("dispatch failed for %s", grp->name);
}

void set_group_fd (gpointer n, gpointer data) {
	struct group *grp = (struct group *)n;
	struct max_fd_set *read_fds = (struct max_fd_set *)data;

	FD_SET (grp->select_fd, &(read_fds->fds));
	if (grp->select_fd > read_fds->max_fd)
		read_fds->max_fd = grp->select_fd;
}

static void finalize_cpg_group (gpointer n, gpointer data) {
	int result;
	struct cpg_name group_name;
	struct group *grp = (struct group *)n;

	result = cpg_finalize (grp->handle);
	if (result != 1)
		g_warning("finalize %s: result is %d, should be 1", grp->name, result);
}

void finalize_cpg_groups () {
	int result;
	GList *n;
	struct group *grp;

	g_message("finalizing groups.");
	g_list_foreach(groups, finalize_cpg_group, NULL);
}

void add_cpg_group (char *name) {
	struct group *grp;
	int result;

	grp = (struct group *)malloc(sizeof(struct group));
	grp->name = name;
	groups = g_list_append(groups, grp);
}

