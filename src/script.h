#pragma once

/**
 * Runs script on group configuration change.  If the execute_script option
 * is enabled, this function will fork a subprocess that sets up the 
 * script environment and runs the given script.  The parent process waits 
 * for the subprocess to complete and then returns.
 */
extern void execute_confchange_script(void);

