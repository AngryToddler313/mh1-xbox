/* em_cmd_r31 - monster command 0x0055EF40-0x0055F114: em_cmd_myemtype_sel (CMD_SEL_FUNC macro of include/em_cmd.h). Whole file in em_cmd_nm.c. */
#include "g_alias.h"
#include "em_cmd.h"

CMD_SEL_FUNC(em_cmd_myemtype_sel, GV(0x2C, 0x70), u8, u8, em->kind)
