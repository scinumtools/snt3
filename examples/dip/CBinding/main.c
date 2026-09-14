#include <stdio.h>
#include <snt/c.h>

int main(void) {
    const char* source = "answer int = 42\nmessage str = \"hello\"\n";
    snt_error error = {0, NULL};
    snt_dip* dip = NULL;
    char value[128];

    if (snt_dip_create(&dip, &error) ||
        snt_dip_add_string(dip, source, &error) ||
        snt_dip_parse(dip, &error)) {
        fprintf(stderr, "DIPL error: %s\n", error.message);
        snt_dip_free(dip);
        return 1;
    }
    if (snt_dip_get(dip, "answer", value, sizeof(value), &error)) {
        fprintf(stderr, "DIPL lookup error: %s\n", error.message);
        snt_dip_free(dip);
        return 1;
    }
    printf("answer = %s\n", value);
    snt_dip_free(dip);
    return 0;
}
