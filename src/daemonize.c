#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

/*
 * daemonize: daemonizes the process per Steven's book
 * while calculating the max_fds to be closed.
 */
void daemonize() {
	int max_fd;
	pid_t pid;
	int i;

	// Parent terminates, child continues
	if ( (pid = fork()) != 0 ) {
		exit(0);
	}

	// Child #1: become a session leader
	setsid();
	signal(SIGHUP, SIG_IGN);

	// fork, then terminate first child to guarantee no terminal
	if ( (pid = fork()) != 0 ) {
		exit(0);
	}

	// Child #2: change working-dir and clear umask
	chdir("/");
	umask(0);

	// Child #2: close all descriptors
	max_fd = (int) sysconf(_SC_OPEN_MAX);
	for(i=0; i < max_fd; i++) {
		close(i);
	}
}

