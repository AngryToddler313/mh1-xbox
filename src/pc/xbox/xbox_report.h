/* xbox_report.h - bug reports from the Xbox to the laptop (xbox_report.c): the game saves a report folder under
 * <game folder>\reports\ (pick.c); the network loader sends the folders not sent yet at its next start, the online
 * game build right away when its network is up. One POST /report?game=mh1|mhg to tools/xbox_serve.py carries them
 * all: "MH1REPORTS <n>\n", then per report "R <folder name>\n", per file "F <size> <name>\n" + the bytes, and "E\n".
 * A folder that went over gets a sent.txt (it stays on E:). docs/xbox.md "Bug reporter on the Xbox". */
#ifndef XBOX_REPORT_H
#define XBOX_REPORT_H
/* the loader: send what is pending in DIR (E:\Games\MH1\reports) for GAME ("mh1" / "mhg"); returns the number of
 * reports sent, 0 none pending, -1 the laptop did not take them */
int xbox_report_send_pending(const char *dir, const char *game);
/* the game, after saving a report under DIR: the online build sends at once when its network is up (the laptop from
 * the game folder's loader.txt); 1 sent, 0 not tried (no network in this build or not up), -1 failed */
int xbox_report_send_now(const char *dir);
#endif
