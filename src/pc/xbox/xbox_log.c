/* Xbox log: stderr goes to a file next to the game's data. When the game exits,
 * the last lines are shown on screen for 10 s (for the capture card), then the
 * Xbox goes back to the dashboard; the next start of the network loader
 * (E:\Games\MH1\default.xbe) sends the log to the laptop and fetches the newest
 * build (docs/xbox.md "Network loader"). */
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XBOX_LOG "E:\\Games\\MH1\\mh1log.txt"
#define TAIL 24

static void xbox_log_show(void)
{
    static char lines[TAIL][96];
    char buf[96];
    int n = 0, i;
    FILE *f;

    fflush(stdout);
    fflush(stderr);
    if ((f = fopen(XBOX_LOG, "r")) != NULL) {
        while (fgets(buf, sizeof buf, f))
            strcpy(lines[n++ % TAIL], buf);
        fclose(f);
    }
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    debugClearScreen();
    debugPrint("MH1 stopped. Last lines of %s:\n\n", XBOX_LOG);
    for (i = n > TAIL ? n - TAIL : 0; i < n; i++)
        debugPrint("%s", lines[i % TAIL]);
    debugPrint("\nBack to the dashboard in 10 s.\n");
    Sleep(10000);
    XLaunchXBE(NULL);
}

/* SDL's own checks: log them and go on (its default asks Abort/Retry/Ignore on
 * a console nobody can answer, then aborts) */
static SDL_AssertState xbox_sdl_assert(const SDL_AssertData *d, void *u)
{
    (void)u;
    fprintf(stderr, "SDL assert %s:%d %s: %s\n", d->filename, d->linenum, d->function, d->condition);
    return SDL_ASSERTION_ALWAYS_IGNORE;
}

/* after E: is mounted */
void xbox_log_start(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    debugPrint("MH1 starting\n");
    if (freopen(XBOX_LOG, "w", stderr))
        setvbuf(stderr, NULL, _IONBF, 0);
    else
        debugPrint("cannot write %s\n", XBOX_LOG);
    SDL_SetAssertionHandler(xbox_sdl_assert, NULL);
    atexit(xbox_log_show);
}

void xbox_log_note(const char *msg)
{
    debugPrint("%s\n", msg);
    fprintf(stderr, "%s\n", msg);
}

/* liveness (the null-graphics build draws nothing else): once a second on the
 * text screen, every 30 s in the log */
void xbox_log_frame(int frame, unsigned ms)
{
    static unsigned shown, logged;
    if (ms / 1000 != shown) {
        shown = ms / 1000;
        debugPrint("frame %d, %u s\n", frame, shown);
    }
    if (ms / 30000 != logged) {
        logged = ms / 30000;
        fprintf(stderr, "alive: frame %d, %u s\n", frame, ms / 1000);
    }
}
