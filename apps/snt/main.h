/** @file
 * @brief Entry points for native SNT CLI modules.
 */
#ifndef APPS_SNT_MAIN_H
#define APPS_SNT_MAIN_H

class ArgParser;

/** Dispatch the REST server command and return its process exit status. */
int module_server(int argc, char* argv[]);
/** Dispatch the desktop viewer command and return its process exit status. */
int module_view(int argc, char* argv[]);
/** Dispatch Hub workspace commands and return their process exit status. */
int module_hub(int argc, char* argv[]);
/** Run the native MCP service using the requested transport and published inputs. */
int module_mcp(int argc, char* argv[]);
/** Dispatch the data mapping command and return its process exit status. */
int module_dmap(int argc, char* argv[]);
/** Format DIPL source files or check whether they match the project style. */
int module_fmt(int argc, char* argv[]);
/** Validate DIPL files and DIPfile projects through the native parser. */
int module_validate(int argc, char* argv[]);
/** Check evaluated DIPL for advisory quality issues. */
int module_lint(int argc, char* argv[]);

/** Execute a DIP parse command using parsed CLI arguments. */
void module_dip(ArgParser& argpar);
/** Compare saved environments; return zero when equal, one when different, or two on error. */
int module_dip_compare(int argc, char* argv[]);
/** Dispatch semantic inspection commands and emit their structured JSON output. */
int module_dip_semantic(int argc, char* argv[]);
/** Dispatch DIP package operations and return their process exit status. */
int module_dip_package(int argc, char* argv[]);
/** Generate a report from the parsed CLI arguments. */
void module_report(ArgParser& argpar);
/** Execute quantity evaluation, conversion, inspection, or unit listing. */
void module_puq(ArgParser& argpar);

#endif // APPS_SNT_MAIN_H
