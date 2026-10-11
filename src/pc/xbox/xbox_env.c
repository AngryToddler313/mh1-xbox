/* getenv for the Xbox build. nxdk's C library (pdclib) has no environment
 * (its getenv always returns NULL), so the RT_* test aids and traces the PC
 * build reads from the environment (docs/pc.md) could not be used on the
 * Xbox. For test runs, viewer.c loads env.txt from the game's data folder
 * (D:\data\env.txt; tools/run_xemu.sh --env FILE puts it there): one
 * NAME=VALUE per line, '#' starts a comment line. Without the file every
 * getenv returns NULL, as before. This object's getenv replaces pdclib's
 * (the linker takes the library member only for undefined symbols). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_MAX 64
static char *env[ENV_MAX];
static int nenv;

void xbox_log_note(const char *msg);

void xbox_env_load(const char *path)
{
    char line[2048], note[160];
    FILE *f = fopen(path, "r");
    if (!f)
        return;
    while (nenv < ENV_MAX && fgets(line, sizeof line, f)) {
        size_t n = strcspn(line, "\r\n");
        line[n] = 0;
        if (line[0] == '#' || !strchr(line, '=') || line[0] == '=')
            continue;
        env[nenv] = malloc(n + 1);
        memcpy(env[nenv], line, n + 1);
        snprintf(note, sizeof note, "env.txt: %.140s", line);
        xbox_log_note(note);
        nenv++;
    }
    fclose(f);
}

char *getenv(const char *name)
{
    size_t n = strlen(name);
    int i;
    for (i = 0; i < nenv; i++)
        if (!strncmp(env[i], name, n) && env[i][n] == '=')
            return env[i] + n + 1;
    return NULL;
}
