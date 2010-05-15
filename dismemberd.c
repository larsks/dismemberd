#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#include <glib.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>
#include <corosync/swab.h>

#include "version.h"

#define OPT_GROUP	'g'
#define OPT_GROUPDIR	'd'
#define OPT_NOCB	'n'
#define OPT_FOREGROUND	'f'
#define OPT_LOGFILE	'l'
#define OPTSTRING	":g:d:nfl:"

static int quit = 0;
static int foreground = 0;
static int show_ip = 1;
static int no_callbacks = 0;
static char *group_list_dir = ".";
static char *logfile = NULL;

#include <sys/queue.h>

GList	*groups = NULL;

struct group {
	char		*name;
	cpg_handle_t	handle;
	int		select_fd;
};

static void write_entry(gpointer k, gpointer v, gpointer data) {
	struct in_addr saddr;
	unsigned int nodeid = *(int *)k;
	FILE *fd = (FILE *)data;

#if __BYTE_ORDER == __BIG_ENDIAN
	saddr.s_addr = swab32(nodeid);
#else
	saddr.s_addr = nodeid;
#endif

	fprintf(fd, "%s\n", inet_ntoa(saddr));
}

static void write_entries(const struct cpg_name *group_name,
		const struct cpg_address *member_list, size_t member_list_entries) {
	char *group_name_s;
	int i;
	GHashTable *hosts_table;
	GHashTableIter iter;
	gpointer k,v;
	FILE *fd;
	char *fn;
	int fnlen;

	hosts_table = g_hash_table_new(g_int_hash, g_int_equal);

	// Get a list of unique hosts.
	for (i=0; i<member_list_entries; i++)
		g_hash_table_insert(hosts_table, (gpointer)&(member_list[i].nodeid), GINT_TO_POINTER(1));

	// Put the group name in a useful string.
	group_name_s = (char *)malloc(group_name->length + 1);
	snprintf(group_name_s, group_name->length + 1, group_name->value);

	// Build filename from group_list_dir and group name.
	fnlen = (strlen(group_name_s) + strlen(group_list_dir) + 2);
	fn = (char *)malloc(fnlen);
	snprintf(fn, fnlen, "%s/%s", group_list_dir, group_name_s);

	fd = fopen(fn, "w");
	g_hash_table_foreach(hosts_table, write_entry, (gpointer)fd);
	fclose(fd);
}

static void confchg_cb (
	cpg_handle_t handle,
	const struct cpg_name *groupName,
	const struct cpg_address *member_list, size_t member_list_entries,
	const struct cpg_address *left_list, size_t left_list_entries,
	const struct cpg_address *joined_list, size_t joined_list_entries)
{
	int i;

	g_message("config change: total: %d, joined: %d, left: %d", 
			member_list_entries, joined_list_entries, left_list_entries);
	write_entries(groupName, member_list, member_list_entries);

	/*
	if (left_list_entries && left_list[0].pid == getpid()) {
		printf("We have left the building\n");
		quit = 1;
	}
	*/
}

static cpg_callbacks_t callbacks = {
	.cpg_confchg_fn =            confchg_cb,
};

static void usage (FILE *out) {
	fprintf(out, "dismemberd: usage: dismemberd [ -n ] [ -d <dir> ] [ -g <group> [ -g <group> ... ]\n");
}

static void join_cpg_group (gpointer p_grp, gpointer data) {
	int result;
	struct cpg_name group_name;
	struct group *grp = (struct group *)p_grp;

	g_message("joining group: %s", grp->name);

	strcpy(group_name.value, grp->name);
	group_name.length = strlen(grp->name);
	result = cpg_join(grp->handle, &group_name);

	if (result != CS_OK) {
		g_critical ("could not join process group: error %d", result);
		exit (1);
	}
}

static void join_cpg_groups() {
	g_list_foreach(groups, join_cpg_group, NULL);
}

static void dispatch_group(gpointer n, gpointer data) {
	struct group *grp = (struct group *)n;
	fd_set *read_fds = (fd_set *)data;

	if (FD_ISSET (grp->select_fd, read_fds))
		if (cpg_dispatch (grp->handle, CS_DISPATCH_ALL) != CS_OK)
			g_critical("dispatch failed for %s", grp->name);
}

static void loop () {
	int max_fd;
	int result;
	GList *n;
	struct group *grp;
	fd_set read_fds;

	FD_ZERO (&read_fds);

	do {
		for (n = g_list_first(groups); n != NULL; n = g_list_next(groups)) {
			grp = (struct group *)n->data;
			FD_SET (grp->select_fd, &read_fds);
			if (grp->select_fd > max_fd)
				max_fd = grp->select_fd;
		}

		result = select (max_fd + 1, &read_fds, 0, 0, 0);
		if (result == -1) {
			if (errno == EINTR) return;

			g_critical("select failed: %s", strerror(errno));
			exit(1);
		}

		g_list_foreach(groups, dispatch_group, &read_fds);

	} while (!quit);
}

static void sigint_handler (int signum) {
	quit = 1;
}

static void finalize_cpg_groups () {
	int result;
	GList *n;
	struct group *grp;

	g_message("finalizing groups.");

	for (n=g_list_first(groups); n != NULL; n = g_list_next(groups)) {
		grp = (struct group *)n->data;

		result = cpg_finalize (grp->handle);
		if (result != 1)
			g_warning("finalize %s: result is %d, should be 1", grp->name, result);
	}
}

static void add_cpg_group (char *name) {
	struct group *grp;
	int result;

	grp = (struct group *)malloc(sizeof(struct group));
	grp->name = name;
	result = cpg_initialize (&(grp->handle), no_callbacks ? NULL : &callbacks);
	if (result != CS_OK) {
		g_critical("could not initialize Cluster Process Group API instance: error %d", result);
		exit(1);
	} else {
		g_message("initialized group %s.", name);
	}
	cpg_fd_get(grp->handle, &(grp->select_fd));
	groups = g_list_append(groups, grp);
}

static int test_list_dir () {
	int res = 0;
	int fnlen;
	char *fn;
	FILE *fd;

	fnlen = (strlen(group_list_dir) + strlen(".test") + 2);
	fn = (char *)malloc(fnlen);
	snprintf(fn, fnlen, "%s/%s", group_list_dir, ".test");

	fd = fopen(fn, "w");
	if (fd == NULL)
		res = 1;
	else if (fclose(fd) != 0)
		res = 1;
	else if (unlink(fn) != 0)
		res = 1;

	return res;
}

int main (int argc, char *argv[]) {
	int c;

	g_message("dismemberd v%s by Lars Kellogg-Stedman", VERSION);
	g_log_set_fatal_mask(NULL, G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_ERROR);

	while ((c = getopt(argc, argv, OPTSTRING)) != EOF) {
		switch (c) {
			case OPT_LOGFILE:
				logfile = strdup(optarg);
				break;
			case OPT_FOREGROUND:
				foreground = 1;
				break;
			case OPT_NOCB:
				no_callbacks = 1;
				break;
			case OPT_GROUPDIR:
				group_list_dir = strdup(optarg);
				break;
			case OPT_GROUP:
				add_cpg_group(strdup(optarg));
				break;

			case '?':
				usage(stderr);
				exit(2);
		}
	}

	if (test_list_dir() != 0) {
		g_critical("cannot create group lists in %s.", group_list_dir);
		exit(1);
	}

	join_cpg_groups();
	signal(SIGINT, sigint_handler);
	loop();
	finalize_cpg_groups();
}

