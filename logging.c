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
#include <syslog.h>

#include "dismemberd.h"

void log_to_stderr (const gchar *log_domain,
		GLogLevelFlags log_level,
		const gchar *message,
		gpointer user_data) {

	char *log_level_name;
	
	if (log_domain == NULL)
		log_domain = "dismemberd";

	if (log_level & G_LOG_LEVEL_ERROR)
		log_level_name = "ERROR";
	else if (log_level & G_LOG_LEVEL_CRITICAL)
		log_level_name = "CRITICAL";
	else if (log_level & G_LOG_LEVEL_WARNING)
		log_level_name = "WARNING";
	else if (log_level & G_LOG_LEVEL_MESSAGE)
		log_level_name = "NOTICE";
	else if (log_level & G_LOG_LEVEL_INFO)
		log_level_name = "INFO";
	else if (log_level & G_LOG_LEVEL_DEBUG)
		log_level_name = "DEBUG";

	fprintf(stderr, "%s: %s: %s\n",
			log_domain,
			log_level_name,
			message);
}

void log_to_syslog (const gchar *log_domain,
		GLogLevelFlags log_level,
		const gchar *message,
		gpointer user_data) {

	int level = LOG_INFO;

	if (log_domain == NULL)
		log_domain = "dismemberd";

	if (log_level & G_LOG_LEVEL_ERROR)
		level = LOG_ERR;
	else if (log_level & G_LOG_LEVEL_CRITICAL)
		level = LOG_CRIT;
	else if (log_level & G_LOG_LEVEL_WARNING)
		level = LOG_WARNING;
	else if (log_level & G_LOG_LEVEL_MESSAGE)
		level = LOG_NOTICE;
	else if (log_level & G_LOG_LEVEL_INFO)
		level = LOG_INFO;
	else if (log_level & G_LOG_LEVEL_DEBUG)
		level = LOG_DEBUG;

	syslog(level, "%s: %s", log_domain, message);
}

void init_logging (int log_syslog) {
	if (log_syslog) {
		openlog("dismemberd", LOG_PID, LOG_DAEMON);
		g_log_set_default_handler(log_to_syslog, NULL);
	} else {
		g_log_set_default_handler(log_to_stderr, NULL);
	}

	g_log_set_fatal_mask(NULL, G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_ERROR);
}

