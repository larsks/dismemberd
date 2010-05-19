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

#include "dismemberd.h"

static int quit = 0;

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
	execute_confchange_script();
}

cpg_callbacks_t callbacks = {
	.cpg_confchg_fn =            confchg_cb,
};

static void sigint_handler (int signum) {
	quit = 1;
}

static void loop () {
	int result;
	GList *n;
	struct group *grp;
	struct max_fd_set read_fds;

	read_fds.max_fd = 0;
	FD_ZERO (&(read_fds.fds));

	signal(SIGINT, sigint_handler);

	do {
		g_list_foreach(groups, set_group_fd, &read_fds);

		result = select (read_fds.max_fd + 1, &(read_fds.fds), 0, 0, 0);
		if (result == -1) {
			if (errno == EINTR) break;

			g_critical("select failed: %s", strerror(errno));
			exit(1);
		}

		g_list_foreach(groups, dispatch_group, &read_fds);

	} while (!quit);
}

int main (int argc, char *argv[]) {
	process_options(argc, argv);

	init_logging(log_syslog);
	g_message("dismemberd v%s by Lars Kellogg-Stedman", VERSION);

	if (path_is_writeable_dir(group_list_dir) != 0) {
		g_critical("cannot create group lists in %s.", group_list_dir);
		exit(1);
	}

	if (execute_script && (path_is_executable(execute_script_path) != 0)) {
		g_critical("cannot execute script (%s).",
				execute_script_path);
		exit(1);
	}

	init_cpg_groups();
	join_cpg_groups();
	loop();
	finalize_cpg_groups();
}

