/* install.c: game data through the loader (tools/xbox_serve.py --data) */
#ifndef LOADER_INSTALL_H
#define LOADER_INSTALL_H
/* fetch the laptop's /manifest and every missing or incomplete file into E:\Games\<folder>.
 * Returns -1 when something is not finished, 0 when MH1's data is complete, 1 when another
 * game's folder (MHG) is complete; GAME then holds that folder's default.xbe. */
int install_data(char *game, int ngame);
#endif
