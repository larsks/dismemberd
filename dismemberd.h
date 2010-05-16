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

#include <glib.h>

#include <corosync/corotypes.h>
#include <corosync/cpg.h>
#include <corosync/swab.h>

#include "version.h"
#include "logging.h"
#include "groups.h"
#include "options.h"

struct max_fd_set {
	fd_set	fds;
	int	max_fd;
};

extern cpg_callbacks_t callbacks;

