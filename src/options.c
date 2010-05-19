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
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "dismemberd.h"

int	no_callbacks	=	0;
char	*group_list_dir	=	".";
char	*logfile	=	NULL;
int	log_syslog	=	0;
int	execute_script	=	0;
char	*execute_script_path
			=	NULL;

static void usage (FILE *out) {
	fprintf(out, "dismemberd: usage: dismemberd [ -n ] [ -d <dir> ] [ -g <group> [ -g <group> ... ]\n");
}

void process_options(int argc, char *argv[]) {
	int c;

	while ((c = getopt(argc, argv, OPTSTRING)) != EOF) {
		switch (c) {
			case OPT_LOG_SYSLOG:
				log_syslog = 1;
				break;
			case OPT_LOGFILE:
				logfile = strdup(optarg);
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
			case OPT_EXECUTE_SCRIPT:
				execute_script = 1;
				execute_script_path = strdup(optarg);
				break;

			case '?':
				usage(stderr);
				exit(2);
		}
	}
}

