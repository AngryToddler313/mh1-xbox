/* lb_m02 - browser small helpers 0x005E2DD0-0x005E2DD8: BsBgImgTrans. Whole file in lb_m.c. */
#include "g_alias.h"
#include "lobby_f.h"
#ifdef MHG_PS2
void _cnet_Return_CallBack();
#endif
void BsWorkTrans();
void To_RetVal();
void flSetRenderState();
int strncmp();















void BsBgImgTrans(void) {
    GV(BsWorkTrans, _cnet_Return_CallBack)(0);
}
