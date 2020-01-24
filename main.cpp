#include <stdio.h>
#include <cstring>

int main(int argc, char** argv)
{
    int minarg = 2;

    if (argc >= minarg) {
        if(strncmp(argv[1], "build", 5) == 0) {
            minarg = 3;
        }
        else if(strncmp(argv[1], "config", 6)) {
            if(strncmp(argv[2], "set", 3) == 0)
                minarg = 5;
            else if(strncmp(argv[2], "get", 3) == 0)
                minarg = 4;
            else
                minarg = 3;
        }
    }

    if (argc != minarg) {
        fprintf(stderr, "usage: \n");
        fprintf(stderr, "  Config:\n");
        fprintf(stderr, "    meta config list\n");
        fprintf(stderr, "    meta config get <param>\n");
        fprintf(stderr, "    meta config set <param> <value>\n");
        fprintf(stderr, "  Build:\n");
        fprintf(stderr, "    meta build <toolchain>\n");
        fprintf(stderr, "    meta build <application>\n");
        fprintf(stderr, "    meta build <image>\n");
        return 0;
    }

    return 0;
}
