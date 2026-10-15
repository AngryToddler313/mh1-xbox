/* getenv for the Xbox build. nxdk's C library (pdclib) has no environment
 * (its getenv always returns NULL), so the RT_* test aids and traces the PC
 * build reads from the environment (docs/pc.md) could not be used on the
 * Xbox. For test runs, env.txt in the game's data folder (D:\data\env.txt or
 * E:\Games\MH1\data\env.txt; tools/run_xemu.sh --env FILE puts it on the
 * xemu disc) holds one NAME=VALUE per line, '#' starts a comment line.
 * Without the file every getenv returns NULL, as before. This object's
 * getenv replaces pdclib's (the linker takes the library member only for
 * undefined symbols).
 * viewer.c loads it first thing in main: several switches are read once, at
 * their first use (rt_memstat.c's RT_MEM at the first malloc). No malloc
 * here for the same reason. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xbox_title.h"

#define ENV_MAX 64
static char buf[8192];
static char *env[ENV_MAX];
static int nenv;

void xbox_log_note(const char *msg);

/* the first env.txt found in the data folders; E: must be mounted */
void xbox_env_load(void)
{
    static const char *const paths[] = { "D:\\data\\env.txt", XBOX_GAME_DIR "\\data\\env.txt" };
    FILE *f = NULL;
    size_t n;
    char *s;
    int i;
    for (i = 0; i < 2 && !f; i++)
        f = fopen(paths[i], "rb");
    if (!f)
        return;
    n = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[n] = 0;
    for (s = buf; *s && nenv < ENV_MAX; ) {
        char *e = s + strcspn(s, "\r\n");
        char *next = *e ? e + 1 : e;
        *e = 0;
        if (s[0] != '#' && s[0] != '=' && strchr(s, '='))
            env[nenv++] = s;
        s = next;
    }
}

/* the loaded lines into the log (after the log is open) */
void xbox_env_note(void)
{
    char note[160];
    int i;
    for (i = 0; i < nenv; i++) {
        snprintf(note, sizeof note, "env.txt: %.140s", env[i]);
        xbox_log_note(note);
    }
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
