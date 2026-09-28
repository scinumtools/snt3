#ifndef APPS_SNT_MAIN_H
#define APPS_SNT_MAIN_H

class ArgParser;

int module_server(int argc, char* argv[]);
int module_view(int argc, char* argv[]);
int module_dmap(int argc, char* argv[]);

void module_dip(ArgParser& argpar);
void module_report(ArgParser& argpar);
void module_puq(ArgParser& argpar);

#endif // APPS_SNT_MAIN_H
