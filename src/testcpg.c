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

#include <corosync/corotypes.h>
#include <corosync/cpg.h>
#include <corosync/swab.h>

static int quit = 0;
static int show_ip = 0;

static void print_cpgname (const struct cpg_name *name)
{
	int i;

	for (i = 0; i < name->length; i++) {
		printf ("%c", name->value[i]);
	}
}

static char * node_pid_format(unsigned int nodeid,int pid) {
	static char buffer[100];
	if (show_ip) {
		struct in_addr saddr;
#if __BYTE_ORDER == __BIG_ENDIAN
		saddr.s_addr = swab32(nodeid);
#else
		saddr.s_addr = nodeid;
#endif
		sprintf(buffer, "node/pid %s/%d", inet_ntoa(saddr),pid);
	} 
	else {
		sprintf(buffer, "node/pid %d/%d", nodeid, pid);
	} 
	return buffer;
}


static void DeliverCallback (
	cpg_handle_t handle,
	const struct cpg_name *groupName,
	uint32_t nodeid,
	uint32_t pid,
	void *msg,
	size_t msg_len)
{
	printf("DeliverCallback: message (len=%lu)from %s: '%s'\n",
		       (unsigned long int) msg_len, node_pid_format(nodeid, pid),
		       (const char *)msg);
}

static void ConfchgCallback (
	cpg_handle_t handle,
	const struct cpg_name *groupName,
	const struct cpg_address *member_list, size_t member_list_entries,
	const struct cpg_address *left_list, size_t left_list_entries,
	const struct cpg_address *joined_list, size_t joined_list_entries)
{
	int i;

	printf("\nConfchgCallback: group '");
	print_cpgname(groupName);
	printf("'\n");
	for (i=0; i<joined_list_entries; i++) {
		printf("joined %s reason: %d\n",
				node_pid_format(joined_list[i].nodeid, joined_list[i].pid),
				joined_list[i].reason);
	}

	for (i=0; i<left_list_entries; i++) {
		printf("left %s reason: %d\n",
				node_pid_format(left_list[i].nodeid, left_list[i].pid),
				left_list[i].reason);
	}

	printf("nodes in group now %lu\n",
	       (unsigned long int) member_list_entries);
	for (i=0; i<member_list_entries; i++) {
		printf("%s\n",
				node_pid_format(member_list[i].nodeid, member_list[i].pid));
	}

	/* Is it us??
	   NOTE: in reality we should also check the nodeid */
	if (left_list_entries && left_list[0].pid == getpid()) {
		printf("We have left the building\n");
		quit = 1;
	}
}

static cpg_callbacks_t callbacks = {
	.cpg_deliver_fn =            DeliverCallback,
	.cpg_confchg_fn =            ConfchgCallback,
};

static void sigintr_handler (int signum) __attribute__((__noreturn__));
static void sigintr_handler (int signum) {
	exit (0);
}
static struct cpg_name group_name;

int main (int argc, char *argv[]) {
	cpg_handle_t handle;
	fd_set read_fds;
	int select_fd;
	int result;
	const char *options = "i";
	int opt;
	unsigned int nodeid;
	char *fgets_res;

	while ( (opt = getopt(argc, argv, options)) != -1 ) {
		switch (opt) {
		case 'i':
			show_ip = 1;
			break;
		}
	}

	if (argc > optind) {
		strcpy(group_name.value, argv[optind]);
		group_name.length = strlen(argv[optind]);
	}
	else {
		strcpy(group_name.value, "GROUP");
		group_name.length = 6;
	}

	result = cpg_initialize (&handle, &callbacks);
	if (result != CS_OK) {
		printf ("Could not initialize Cluster Process Group API instance error %d\n", result);
		exit (1);
	}
	result = cpg_local_get (handle, &nodeid);
	if (result != CS_OK) {
		printf ("Could not get local node id\n");
		exit (1);
	}

	printf ("Local node id is %x\n", nodeid);
	result = cpg_join(handle, &group_name);
	if (result != CS_OK) {
		printf ("Could not join process group, error %d\n", result);
		exit (1);
	}

	FD_ZERO (&read_fds);
	cpg_fd_get(handle, &select_fd);
	printf ("Type EXIT to finish\n");
	do {
		FD_SET (select_fd, &read_fds);
		FD_SET (STDIN_FILENO, &read_fds);
		result = select (select_fd + 1, &read_fds, 0, 0, 0);
		if (result == -1) {
			perror ("select\n");
		}
		if (FD_ISSET (STDIN_FILENO, &read_fds)) {
			char inbuf[132];
			struct iovec iov;

			fgets_res = fgets(inbuf, sizeof(inbuf), stdin);
			if (fgets_res == NULL) {
				cpg_leave(handle, &group_name);
			}
			if (strncmp(inbuf, "EXIT", 4) == 0) {
				cpg_leave(handle, &group_name);
			}
			else {
				iov.iov_base = inbuf;
				iov.iov_len = strlen(inbuf)+1;
				cpg_mcast_joined(handle, CPG_TYPE_AGREED, &iov, 1);
			}
		}
		if (FD_ISSET (select_fd, &read_fds)) {
			if (cpg_dispatch (handle, CS_DISPATCH_ALL) != CS_OK)
				exit(1);
		}
	} while (result && !quit);


	result = cpg_finalize (handle);
	printf ("Finalize  result is %d (should be 1)\n", result);
	return (0);
}
