#pragma once
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

extern GList *groups;

struct group {
	char		*name;
	cpg_handle_t	handle;
	int		select_fd;
};

/* groups.c */
extern GList *groups;
extern void init_cpg_groups(void);
extern void join_cpg_groups(void);
extern void dispatch_group(gpointer n, gpointer data);
extern void set_group_fd(gpointer n, gpointer data);
extern void finalize_cpg_groups(void);
extern void add_cpg_group(char *name);

