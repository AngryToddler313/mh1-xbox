/*
 * savecard.c - PC card folder <-> PCSX2 memory card image, for tools/run_pcsx2.sh (built on the fly with gcc).
 *   savecard ROOT --export-save CARD.ps2   ROOT/BISLPM-65495MH (a PC port save, byte for byte the PS2's files)
 *                                          -> a fresh formatted 8 MB card holding just that save
 *   savecard ROOT --import-save CARD.ps2   the save on a card image -> ROOT/BISLPM-65495MH (to read it back)
 * The work is src/pc/rt/rt_save.c + src/pc/fmt/ps2save.c (the PC port's own import / export).
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "../../src/pc/rt/rt_save.h"

void rt_mkdirs(const char *path)
{
    char tmp[1024], *s;
    snprintf(tmp, sizeof tmp, "%s", path);
    for (s = tmp + 1; *s; s++)
        if (*s == '/') {
            *s = 0;
            mkdir(tmp, 0755);
            *s = '/';
        }
    mkdir(tmp, 0755);
}

const char *rt_mc_root(void) { return "."; }

int main(int argc, char **argv)
{
    int r;
    if (argc != 4) {
        fprintf(stderr, "usage: savecard ROOT --export-save|--import-save CARD.ps2\n");
        return 2;
    }
    r = rt_save_cli(argv[1], argc - 1, argv + 1);
    return r < 0 ? 2 : r;
}
