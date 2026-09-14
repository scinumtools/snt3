#include <stdio.h>
#include <snt/c.h>

int main(void) {
    snt_error error = {0, NULL};
    snt_quantity* quantity = NULL;
    snt_quantity* converted = NULL;
    char output[128];

    if (snt_quantity_eval("2.5 m", &quantity, &error) != 0) {
        fprintf(stderr, "PUQ evaluation failed: %s\n", error.message);
        return 1;
    }
    if (snt_quantity_convert(quantity, "cm", &converted, &error) != 0) {
        fprintf(stderr, "PUQ conversion failed: %s\n", error.message);
        snt_quantity_free(quantity);
        return 1;
    }
    if (snt_quantity_format(converted, output, sizeof(output), &error) != 0) {
        fprintf(stderr, "PUQ formatting failed: %s\n", error.message);
        snt_quantity_free(converted);
        snt_quantity_free(quantity);
        return 1;
    }

    puts(output);
    snt_quantity_free(converted);
    snt_quantity_free(quantity);
    return 0;
}
