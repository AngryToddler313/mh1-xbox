/* f_rewardd - quest reward screen, item grid (SLPM_654.95 0x00293690-0x00293B68): reward_itembox draws the 16-slot
 * reward item list as icons on a background grid, the cursor, and the name / count of the selected item.
 * Byte-matches (agent B, 12 Oct 2026): the background rectangle is an s16 array (the original keeps the
 * addresses of its elements in registers), the alpha is written out at each use, the selected slot is read
 * through a pointer, `mode == 0` first. Meanings are guesses. */
#include "reward.h"

typedef struct RWD_RECT2 {
    s16 p[4];           /* 0x0 x0, y0, x1, y1 (an array: the original keeps &p[1..3] in registers) */
    u32 col;            /* 0x8 */
} RWD_RECT2;

typedef struct RWD_SPR {
    s16 pos[2];
    s16 size[2];
    u32 col;
    s16 uv[4];
} RWD_SPR;

extern u8 reward_item_base[];
extern u32 item_col_tbl[];
extern char *item_str[];
extern char lit_1001_00386A58[];
extern char lit_1002_00386A60[];
extern char lit_1003_00386A70[];
void DispFrameMessageA();
void flps0004();
void flps0008();
void SetFilterMode();
void reload_tex();
void SetTextureStage();
void font_print_uf();
int sprintf(char *, const char *, ...);

void reward_itembox(cur, mode, xb)
s16 cur;
u8 mode;
s8 xb;
{
    RWD_RECT2 rect;
    RWD_SPR spr;
    char buf[0x20];
    s16 i;
    u8 col;
    PL_ITEM *ip;
    int id;
    u8 icon;

    col = (mode == 0) ? 0xFF : 0x60;
    DispFrameMessageA(reward_item_base, 0, col);
    rect.col = 0xFF200000;
    for (i = 0; i < 16; i++) {
        rect.p[0] = (s16)(0.8f * (313.0f + 36.0f * (i & 7))) + 4;
        rect.p[2] = rect.p[0] + 0x19;
        rect.p[1] = (i >> 3) * 32 + 0xD4;
        rect.p[3] = rect.p[1] + 0x1C;
        flps0004(&rect);
    }
    SetFilterMode(0);
    reload_tex(1, 0x118);
    SetTextureStage(0x118);
    spr.size[0] = 0x20;
    spr.size[1] = 0x20;
    for (ip = game_w.reward_item, i = 0; i < 16; i++, ip++) {
        spr.pos[0] = (s16)(0.8f * (313.0f + 36.0f * (i & 7)));
        spr.pos[1] = (i >> 3) * 32 + 0xD2;
        id = ip->id;
        if (id != 0) {
            icon = Item_data[id][5];
            if (icon != 0xFF) {
                spr.uv[0] = ((icon + 1) & 7) * 32 + 1;
                spr.uv[1] = ((icon + 1) >> 3) * 32 + 1;
                spr.uv[2] = ((icon + 1) & 7) * 32 + 0x1F;
                spr.uv[3] = ((icon + 1) >> 3) * 32 + 0x1F;
                spr.col = item_col_tbl[Item_data[id][6]];
                spr.col = (spr.col & 0xFFFFFF) | ((u32)col << 24);
                flps0008(&spr);
            }
        }
    }
    if (cur >= 0) {
        spr.pos[0] = (s16)(0.8f * (313.0f + 36.0f * (cur & 7)));
        spr.pos[1] = (cur >> 3) * 32 + 0xD2;
        *(u32 *)&spr.uv[0] = 0;
        *(u32 *)&spr.uv[2] = 0x200020;
        spr.col = ((u32)col << 24) | 0xFFFFFF;
        flps0008(&spr);
        flfntSetSize(0x12, 0x12);
        if (mode == 0) {
            font_set_palette(0);
        } else {
            font_set_palette(0xA);
        }
        ip = &game_w.reward_item[cur];
        id = ip->id;
        if (id == 0) {
            sprintf(buf, item_str[0]);
        } else {
            switch (Item_data[id][3]) {
            case 1:
                sprintf(buf, item_str[id]);
                break;
            case 0xFF:
                sprintf(buf, lit_1001_00386A58, item_str[id]);
                break;
            default:
                sprintf(buf, lit_1002_00386A60, item_str[id], ip->num);
                break;
            }
        }
        flfntLocate((s16)(0x1CB - (unsigned)strlen(buf) * 9 / 2), 0x13B);
        font_print_uf(buf);
        if (xb >= 0) {
            font_set_palette(2);
            flfntLocate(0x13B, 0x125);
            font_print_uf(lit_1003_00386A70);
        }
    }
}
