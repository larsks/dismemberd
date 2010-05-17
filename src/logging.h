#pragma once
/* logging.c */
extern void log_to_stderr(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);
extern void log_to_syslog(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);
extern void init_logging(int log_syslog);
