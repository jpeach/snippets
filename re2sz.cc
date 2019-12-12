// Test code to show program sized for different RE2 regexes.

#include <stdio.h>
#include <re2/re2.h>

int main(int argc, const char ** argv)
{
    for (int i = 1; i < argc; ++i) {
        RE2 regex(argv[i]);

        if (!regex.ok()) {
            printf("failed to compile regex: %s\n", regex.error().c_str());
            continue;
        }

        printf("program size is %d\n", regex.ProgramSize());
    }

    return 0;
}
