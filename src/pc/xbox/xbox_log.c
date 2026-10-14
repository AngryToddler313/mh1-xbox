/* Xbox log: stderr goes to a file next to the game's data. When the game exits,
 * the last lines are shown on screen for 10 s (for the capture card), then the
 * Xbox goes back to the dashboard; the next start of the network loader
 * (E:\Games\MH1\default.xbe) sends the log to the laptop and fetches the newest
 * build (docs/xbox.md "Network loader"). */
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XBOX_LOG "E:\\Games\\MH1\\mh1log.txt"
#define TAIL 24

/* xemu: the game's log lines also go out of the first serial port (COM1,
 * 0x3F8, the LPC47M157 Super I/O xemu emulates; tools/run_xemu.sh starts xemu
 * with -serial file:xbox_files/xemu_debug.txt), because xemu's blank HDD
 * image has no E: for the log file. Retail Xbox hardware has no UART there:
 * the line status register reads 0xFF, so this stays off on hardware (only a
 * read is made to find out). */
#define COM1 0x3F8
static int serial = -1;

static unsigned char port_in(unsigned short p)
{
    unsigned char v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(p));
    return v;
}

void xbox_serial_write(const char *s, int n)
{
    if (serial < 0) {
        unsigned char lsr = port_in(COM1 + 5);
        serial = !(lsr & 0x80) && (lsr & 0x60) == 0x60;    /* idle UART: transmitter empty */
    }
    if (!serial)
        return;
    while (n-- > 0) {
        int spin = 100000;
        while (!(port_in(COM1 + 5) & 0x20) && --spin)
            ;
        __asm__ volatile("outb %0, %1" : : "a"((unsigned char)*s++), "Nd"((unsigned short)COM1));
    }
}

static void serial_line(const char *s)
{
    xbox_serial_write(s, (int)strlen(s));
    xbox_serial_write("\n", 1);
}

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

/* Back + Start: straight to the log screen and the dashboard. The normal end of main
 * (SDL audio close, gfx shutdown) hung on the real Xbox on 12 Oct 2026 (nothing after
 * "Back + Start held" in the log; most likely closing the audio device, whose callback
 * thread waits on an AC97 interrupt that no longer comes), and nothing there matters
 * when the console reboots anyway. Does not return. */
void xbox_ac97_stop(void);
void xbox_log_quit(void)
{
    xbox_ac97_stop();           /* no audio DMA left running into the next program (xbox_ac97.c) */
    xbox_log_show();
}

/* after E: is mounted */
void xbox_log_start(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    debugPrint("MH1 starting\n");
    CreateDirectoryA("E:\\Games", NULL);          /* the network loader's folder; a freshly formatted */
    CreateDirectoryA("E:\\Games\\MH1", NULL);     /* disk (xemu, tools/xbox_fatx.py) has none */
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
    serial_line(msg);
}

/* liveness (the null-graphics build draws nothing else): once a second on the
 * text screen, every 30 s in the log */
void xbox_log_frame(int frame, unsigned ms)
{
    static unsigned shown, logged, dlogged;
    if (ms / 1000 != shown) {
        shown = ms / 1000;
        debugPrint("frame %d, %u s\n", frame, shown);
    }
    if (ms / 30000 != logged) {
        logged = ms / 30000;
        fprintf(stderr, "alive: frame %d, %u s\n", frame, ms / 1000);
    }
    if (ms / 5000 != dlogged) {
        char b[64];
        MM_STATISTICS st;           /* the memory left (tools/xemu_soak.sh follows it) */
        memset(&st, 0, sizeof st);
        st.Length = sizeof st;
        dlogged = ms / 5000;
        if (NT_SUCCESS(MmQueryStatistics(&st)))
            snprintf(b, sizeof b, "alive: frame %d, %u s, %lu KB free", frame, ms / 1000, (unsigned long)st.AvailablePages * 4);
        else
            snprintf(b, sizeof b, "alive: frame %d, %u s", frame, ms / 1000);
        serial_line(b);
    }
}
