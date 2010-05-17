#pragma once

/**
 * Initialize the glib logging environment.  If log_syslog is true, output
 * goes to syslog, otherwise output goes to stderr.
 */
extern void init_logging(int log_syslog);

