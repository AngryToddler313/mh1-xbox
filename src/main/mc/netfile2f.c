/* netfile2f - SLPM_654.95 0x00289060-0x0028911C (Net_Icon_Data_Load): loads the net save icon model once
 * (net_common_w.x03 step 0 starts the load of file 0x6D4 or, for arg 1, 0x6D3; step 1 -> 2 done). Returns 1 when done. */
#include "g_alias.h"
#include "types.h"
#include "netcw.h"

extern u8 *data_load_ptr;
int load_file_mdl();
#ifdef MHG_PS2
int McActAvailSet();
#endif

int Net_Icon_Data_Load(arg)
int arg;
{
    int r;

    r = 0;
    switch (net_common_w.x03) {
    case 0:
        net_common_w.x03++;
        if (arg == 0) {
            load_file_mdl((int)data_load_ptr + GV(0x12000, 0x20000), GV(0x6D4, 0x9B1));
#ifdef MHG_PS2
            McActAvailSet((int)data_load_ptr + GV(0x12000, 0x20000));   /* G (main 0x229E10 in G's map) */
#endif
        } else {
            load_file_mdl((int)data_load_ptr + GV(0x12000, 0x20000), GV(0x6D3, 0x9B0));
        }
        break;
    case 1:
        net_common_w.x03++;
    case 2:
        r = 1;
        break;
    }
    return r;
}
