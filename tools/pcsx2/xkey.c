/* xkey - press keys in a hidden X display (Xvfb) for scripted PCSX2 input (tools/run_pcsx2.sh, tools/ps2_online_run.sh).
 * The keys go to PCSX2's game window (the largest mapped top-level window gets the input focus first), through XTEST.
 * usage: DISPLAY=:N xkey KEY[:HOLD_MS[:WAIT_MS]] ...    KEY = an X keysym name (Return, Up, x, ...); default hold 120 ms,
 *        wait 250 ms after the release. "sleep:MS" waits. Built by run_pcsx2.sh: gcc xkey.c -lX11 (+ libXtst.so.6).
 * XTest.h may be missing (no dev package): the two calls used are declared here. */
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int XTestFakeKeyEvent(Display *, unsigned int, Bool, unsigned long);
extern int XTestFakeMotionEvent(Display *, int, int, int, unsigned long);

static Window best; static long best_area;
static void find(Display *d, Window w, int depth) {
    Window root, parent, *kids = NULL; unsigned int n = 0, i;
    XWindowAttributes a;
    if (XGetWindowAttributes(d, w, &a) && a.map_state == IsViewable && depth > 0) {
        long area = (long)a.width * a.height;
        if (area > best_area) { best_area = area; best = w; }
    }
    if (depth > 3 || !XQueryTree(d, w, &root, &parent, &kids, &n)) return;
    for (i = 0; i < n; i++) find(d, kids[i], depth + 1);
    if (kids) XFree(kids);
}

int main(int argc, char **argv) {
    Display *d = XOpenDisplay(NULL);
    int i;
    if (!d) { fprintf(stderr, "xkey: no display\n"); return 1; }
    find(d, DefaultRootWindow(d), 0);
    if (best) {
        XWindowAttributes a; int x, y; Window c;
        XGetWindowAttributes(d, best, &a);
        XTranslateCoordinates(d, best, DefaultRootWindow(d), a.width / 2, a.height / 2, &x, &y, &c);
        XTestFakeMotionEvent(d, -1, x, y, 0);
        XSetInputFocus(d, best, RevertToPointerRoot, CurrentTime);
        XSync(d, False);
        usleep(50000);
    }
    for (i = 1; i < argc; i++) {
        char name[64]; int hold = 120, wait = 250; KeySym ks; KeyCode kc;
        char *c = strchr(argv[i], ':');
        size_t len = c ? (size_t)(c - argv[i]) : strlen(argv[i]);
        if (len >= sizeof name) len = sizeof name - 1;
        memcpy(name, argv[i], len); name[len] = 0;
        if (c) { hold = atoi(c + 1); c = strchr(c + 1, ':'); if (c) wait = atoi(c + 1); }
        if (!strcmp(name, "sleep")) { usleep(hold * 1000); continue; }
        ks = XStringToKeysym(name);
        if (ks == NoSymbol || !(kc = XKeysymToKeycode(d, ks))) { fprintf(stderr, "xkey: unknown key %s\n", name); return 2; }
        XTestFakeKeyEvent(d, kc, True, 0); XSync(d, False); usleep(hold * 1000);
        XTestFakeKeyEvent(d, kc, False, 0); XSync(d, False); usleep(wait * 1000);
    }
    XCloseDisplay(d);
    return 0;
}
