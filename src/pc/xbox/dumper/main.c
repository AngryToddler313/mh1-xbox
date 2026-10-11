/* MH1 bench: dump this Xbox's BIOS flash for xemu (docs/xbox.md "Dumping the
 * BIOS for xemu"). Read-only: the only write is one file on E:.
 *
 * Served by tools/xbox_serve.py --xbe build/xbox/dumper/default.xbe in place
 * of the game; the network loader downloads and starts it like a game build.
 * It does not use the network itself: a program started by XLaunchXBE right
 * after the loader used the NIC hangs in nxNetInit (the loader installed on
 * 11 Oct does not call nxNetShutdown first). So the dump goes into the game
 * log file, after a "MH1DUMP <name>" line; the next loader start sends that
 * file and xbox_serve.py saves it as build/xbox/dump/<name>.
 *
 * Dumped: 1 MB of the flash window from physical 0xFFE00000. The flash repeats
 * through the top 16 MB (a 256 KB chip 4 times per MB); the very top MB was
 * tried first and reading froze in its last 256 KB, where the MCPX boot ROM
 * overlays the flash, so the mirror below it is read instead. Not the EEPROM: it holds this console's keys, and xemu
 * makes its own. The MCPX boot ROM is hidden after start-up.
 */
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <nxdk/mount.h>
#include <xboxkrnl/xboxkrnl.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define FLASH_BASE 0xFFE00000u   /* the mirror below the top MB (see above) */
#define FLASH_SIZE 0x100000u
#define DUMP_FILE        "E:\\Games\\MH1\\mh1log.txt"
#define HEAD       "MH1DUMP flash.bin\n"

int main(void)
{
    static unsigned char flash[FLASH_SIZE];
    volatile unsigned char *f;
    FILE *o;
    int i, ok = 0;

    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    debugPrint("MH1 bench: BIOS flash dump (read only)\n");
    /* each step on screen: the first run on 11 Oct froze with nothing after the
     * title line, so where it stops is the next thing to learn */
    nxMountDrive('E', "\\Device\\Harddisk0\\Partition1\\");
    debugPrint("E: mounted\n");
    f = MmMapIoSpace(FLASH_BASE, FLASH_SIZE, PAGE_READONLY | PAGE_NOCACHE);
    debugPrint("flash window mapped at %p\n", (void *)f);
    if (f) {
        for (i = 0; i < (int)FLASH_SIZE; i++) {
            flash[i] = f[i];
            if ((i & 0x3FFFF) == 0x3FFFF)
                debugPrint("read %d KB\n", (i + 1) >> 10);
        }
        MmUnmapIoSpace((PVOID)f, FLASH_SIZE);
        debugPrint("writing %s\n", DUMP_FILE);
        if ((o = fopen(DUMP_FILE, "wb")) != NULL) {
            ok = fwrite(HEAD, 1, strlen(HEAD), o) == strlen(HEAD) && fwrite(flash, 1, FLASH_SIZE, o) == FLASH_SIZE;
            ok = fclose(o) == 0 && ok;
        }
    }
    debugPrint(ok ? "Saved for the loader to send (start Monster Hunter again).\n" : "Dump failed.\n");
    debugPrint("Back to the dashboard in 5 s.\n");
    Sleep(5000);
    XLaunchXBE(NULL);
    return 0;
}
