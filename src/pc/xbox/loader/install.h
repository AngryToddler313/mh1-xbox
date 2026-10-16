/* install.c: game data through the loader (tools/xbox_serve.py --data) */
#ifndef LOADER_INSTALL_H
#define LOADER_INSTALL_H
/* fetch the laptop's /manifest?game=GAME and every missing or incomplete file into DIR (this loader's
 * folder, with a trailing backslash). Returns 0 when the data is complete, -1 when something is not
 * finished or the list is for another game. */
int install_data(const char *dir, const char *game);
#endif
