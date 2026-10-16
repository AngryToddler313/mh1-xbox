/* tools/ps2launch/syms.ld - the game's addresses the launcher's in-game blob (src/ps2patch/launch_hook.c + online.c)
 * links against: Monster Hunter (Japan) SLPM-65495, main (SLPM_654.95) and lobby.bin. From the byte-matching rebuild's
 * symbol lists (config/*_undefined_*_auto.txt, build/ps2patch/*_syms.ld). Addresses only, no game bytes. */
PatchExecCS = 0x28C7D0;
memset = 0x19DD28;
strncpy = 0x1A1858;
bsCsvWork = 0x3A3B60;
D_3A3B71 = 0x3A3B71;
ConnectLbsId = 0x3A38A0;
BsLbsInfo = 0x39F720;
BsLbsCount = 0x38A12C;
COM_R_No_2 = 0x38A870;
Net_Err_No = 0x38A128;
MMBB_LOGIN = 0x38A150;
Net_fade_kill = 0x26C040;
cnLbc_LoadNetModel = 0x5B2C20;
cnLbc_LoadModelWait = 0x5B2C30;
