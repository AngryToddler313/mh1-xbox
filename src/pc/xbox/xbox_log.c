/* Xbox log: stderr goes to a file next to the game's data. When the game exits,
 * the last lines are shown on screen for 10 s (for the capture card), then the
 * network loader (E:\Games\MH1\default.xbe) starts again; it sends the log
 * to the laptop and waits for the next build (docs/xbox.md "Network loader"). */
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>
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
    debugPrint("\nBack to the loader in 10 s.\n");
    Sleep(10000);
    {
        static unsigned char ld[3072] = "MH1X";   /* LAUNCH_DATA_PAGE.LaunchData */
        XLaunchXBEEx("E:\\Games\\MH1\\default.xbe", ld);
    }
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
    atexit(xbox_log_show);
}

void xbox_log_note(const char *msg)
{
    debugPrint("%s\n", msg);
    fprintf(stderr, "%s\n", msg);
}
