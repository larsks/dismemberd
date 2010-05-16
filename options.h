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

#define OPT_GROUP	'g'
#define OPT_GROUPDIR	'd'
#define OPT_NOCB	'n'
#define OPT_LOGFILE	'l'
#define OPT_LOG_SYSLOG	's'
#define OPTSTRING	":g:d:nsl:"

/* options.c */
extern int show_ip;
extern int no_callbacks;
extern char *group_list_dir;
extern char *logfile;
extern int log_syslog;

