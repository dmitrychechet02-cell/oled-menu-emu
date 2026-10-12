#include <stdio.h>
#include <string.h>

#include "tt.h"

#define TT_MAX_TESTS 256

static struct {
    const char *name;
    tt_fn fn;
} tests[TT_MAX_TESTS];
static int test_count;
static int current_failed;

void tt_register(const char *name, tt_fn fn)
{
    if (test_count < TT_MAX_TESTS) {
        tests[test_count].name = name;
        tests[test_count].fn = fn;
        test_count++;
    }
}

void tt_fail(const char *file, int line, const char *expr)
{
    current_failed = 1;
    printf("    %s:%d: assertion failed: %s\n", file, line, expr);
}

int main(int argc, char **argv)
{
    const char *filter = argc > 1 ? argv[1] : NULL;
    int run = 0, failed = 0;

    for (int i = 0; i < test_count; i++) {
        if (filter != NULL && strstr(tests[i].name, filter) == NULL) {
            continue;
        }
        current_failed = 0;
        tests[i].fn();
        run++;
        if (current_failed) {
            failed++;
            printf("FAIL %s\n", tests[i].name);
        } else {
            printf("ok   %s\n", tests[i].name);
        }
    }

    printf("\n%d tests, %d passed, %d failed\n", run, run - failed, failed);
    if (run == 0) {
        printf("no tests matched filter \"%s\"\n", filter != NULL ? filter : "");
        return 1;
    }
    return failed == 0 ? 0 : 1;
}
