#include "parameters.h"

#include <string.h>

int main(void) {
    const Parameters* values = &snt_parameters;
    if (strcmp(values->experiment.title[0], "Flow \"study\" \\ trial") != 0) return 1;
    if (values->experiment.steps[0] != 4) return 2;
    if (values->experiment.gains[0] != 1.25 || values->experiment.gains[2] != 3.75) return 3;
    if (strcmp(values->experiment.paths[0], "alpha") != 0 ||
        strcmp(values->experiment.paths[1], "beta") != 0) return 4;
    if (values->sensors[0].id[0] != 7 || !values->sensors[0].enabled[0]) return 5;
    if (values->sensors[1].id[0] != 9 || values->sensors[1].enabled[0]) return 6;
    return 0;
}
