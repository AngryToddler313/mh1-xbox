/* mkmapTexture: loads the map picture for stage a1 (0x83..0x8A: tutorial maps share the
 * filedef_sys_map_tuto table) and registers it as mem_tex[idx]. SLPM_654.95 0x0011EC20. */
#include "g_alias.h"
#include "types.h"

extern void *data_load_ptr;
extern int f_type[2];
extern u32 mem_tex[];
extern s16 filedef_sys_map[8];
extern s16 filedef_sys_map_tuto[5];

int load_file_mdl(void *, int);
void *flCreateTextureFromApx_mem(void *, int);

void mkmapTexture(int a, int stage, int idx, int type) {
    void *p = data_load_ptr;
    int file;

    switch (stage) {
    case GV(0x83, 10001):
    case GV(0x84, 10002):
        file = filedef_sys_map_tuto[0];
        break;
    case GV(0x85, 10003):
        file = filedef_sys_map_tuto[1];
        break;
    case GV(0x86, 10004):
        file = filedef_sys_map_tuto[2];
        break;
    case GV(0x87, 10005):
    case GV(0x88, 10006):
        file = filedef_sys_map_tuto[3];
        break;
    case GV(0x8A, 10010):
        file = filedef_sys_map_tuto[4];
        break;
    default:
        file = filedef_sys_map[a];
        break;
    }
    load_file_mdl(p, file);
    mem_tex[idx] = (u32)flCreateTextureFromApx_mem(p, f_type[type]);
}
