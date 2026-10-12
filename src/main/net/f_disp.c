/* f_disp - SLPM_654.95 0x0026CD60-0x002702C8: the network screens' sprite and
 * message drawer. net_connect_draw runs every frame while a network menu is up:
 * it advances the blink / cursor animation counters, draws the sprite sets
 * requested in net_common_w.x2C (one bit each, disp_spr_sub) around the two
 * queued message lists (Ncm_* requests, drawn by kind) and the font layers.
 * disp_spr_sub draws one set: menu frames for the current menu (x35 / x37
 * select the layout, x08 / x0A / x29 the cursor), the cursor with its
 * zoom-in animation, the connection status, error messages by error code
 * (x80). Sprite records are 10 halfwords (x, y, w, h, u, v, u2, v2, colour
 * high / low) drawn by disp_spr_sub_sub with flps0008. Names are the original
 * symbols; meanings are guesses from the code. */
#include "types.h"
#include "netcw.h"

typedef struct SPR {
    s16 x, y, w, h;     /* 0x00 */
    u32 col;            /* 0x08 */
    s16 u, v, u2, v2;   /* 0x0C */
} SPR;

void flps0008(SPR *);
void InitRenderState(int);
void flSetRenderState(int, int);
void SetFilterMode(int);
void flfntSetZ(f32);
void flfntDraw(int);
void Net_Fade_Sub(void);
void Ncm_mssage_disp(u8);
void Ncm_menu_disp(u8);
void Net_disp_net_name(u8);
void Ncm_err_mssage_disp(u8);
void Ncm_mssage_disp_option(u8);
void Ncm_mssage_disp_req(u8);
void Net_disp_net_name_req(u8);
void *memcpy(void *, const void *, unsigned int);
void *memset(void *, int, unsigned int);

extern u32 mem_tex[];
extern s16 cur_pos_wid_tbl[][2];
extern s16 net_spr_data00[];
extern s16 net_spr_data01[];
extern u16 net_spr_data02[];
extern s16 net_spr_data030[];
extern s16 net_spr_data031[];
extern s16 net_spr_data032[];
extern s16 net_spr_data040[];
extern s16 net_spr_data041[];
extern s16 net_spr_data042[];
extern s16 net_spr_data05[];
extern s16 *net_spr_data06_tbl[];
extern u16 net_spr_data06_suu_tbl[3];
extern s16 net_spr_data07[];
extern s16 net_spr_data08[];
extern s16 net_spr_data09[];
extern s16 net_spr_data0a[];
extern s16 net_spr_data0b[];
extern s16 net_spr_data13[];

static void disp_spr_tex_set_sub(int n);
static void disp_spr_sub(int flag);
static void disp_spr_sub_sub(s16 *d, int n, int y);

void net_connect_draw(void) {
    int i;

    if (++net_common_w.x2A > 4) {
        net_common_w.x2A = 0;
        if (++net_common_w.x2B >= 0x10) net_common_w.x2B = 0;
    }
    InitRenderState(1);
    flSetRenderState(1, 0);
    flSetRenderState(0x15, 2);
    flSetRenderState(0xC, 0);
    flSetRenderState(0x62, 0);
    if (net_common_w.x2C & 0x1) disp_spr_sub(0x1);
    SetFilterMode(0);
    if (net_common_w.x2C & 0x2) disp_spr_sub(0x2);
    if (net_common_w.x2C & 0x4) disp_spr_sub(0x4);
    if (net_common_w.x2C & 0x40) disp_spr_sub(0x40);
    if (net_common_w.x2C & 0x80) disp_spr_sub(0x80);
    if (net_common_w.x2C & 0x100) disp_spr_sub(0x100);
    if (net_common_w.x2C & 0x400000) disp_spr_sub(0x400000);
    flfntSetZ(850.0f);
    for (i = 0; i < net_common_w.ia; i++) {
        switch (net_common_w.ka[i]) {
        case 0:
            Ncm_mssage_disp(net_common_w.qa[i]);
            break;
        case 1:
            Ncm_menu_disp(net_common_w.qa[i]);
            break;
        case 2:
            Net_disp_net_name(net_common_w.qa[i]);
            break;
        case 3:
            Ncm_err_mssage_disp(net_common_w.qa[i]);
            break;
        case 4:
            Ncm_mssage_disp_option(net_common_w.qa[i]);
            break;
        }
    }
    net_common_w.ia = 0;
    flSetRenderState(0x6C, 0);
    flfntDraw(4);
    flSetRenderState(0x6C, 0);
    if (net_common_w.x2C & 0x200) disp_spr_sub(0x200);
    SetFilterMode(1);
    if (net_common_w.x2C & 0x800) disp_spr_sub(0x800);
    SetFilterMode(0);
    if (net_common_w.x2C & 0x1000) disp_spr_sub(0x1000);
    SetFilterMode(1);
    if (net_common_w.x2C & 0x2000) disp_spr_sub(0x2000);
    if (net_common_w.x2C & 0x4000) disp_spr_sub(0x4000);
    if (net_common_w.x2C & 0x1000000) disp_spr_sub(0x1000000);
    if (net_common_w.x2C & 0x2000000) disp_spr_sub(0x2000000);
    if (net_common_w.x2C & 0x8000) disp_spr_sub(0x8000);
    SetFilterMode(0);
    if (net_common_w.x2C & 0x40000) disp_spr_sub(0x40000);
    if (net_common_w.x2C & 0x80000) disp_spr_sub(0x80000);
    if (net_common_w.x2C & 0x100000) disp_spr_sub(0x100000);
    if (net_common_w.x2C & 0x800000) disp_spr_sub(0x800000);
    flfntSetZ(650.0f);
    for (i = 0; i < net_common_w.ib; i++) {
        switch (net_common_w.kb[i]) {
        case 0:
            Ncm_mssage_disp(net_common_w.qb[i]);
            break;
        case 1:
            Ncm_menu_disp(net_common_w.qb[i]);
            break;
        case 2:
            Net_disp_net_name(net_common_w.qb[i]);
            break;
        case 3:
            Ncm_err_mssage_disp(net_common_w.qb[i]);
            break;
        case 4:
            Ncm_mssage_disp_option(net_common_w.qb[i]);
            break;
        }
    }
    net_common_w.ib = 0;
    flSetRenderState(0x6C, 0);
    flfntDraw(3);
    flSetRenderState(0x6C, 0);
    flfntDraw(2);
    flfntDraw(1);
    flfntDraw(0);
    Net_Fade_Sub();
    flSetRenderState(0x5F, 4);
    flSetRenderState(0x60, 0);
    flSetRenderState(0x5E, 0x32);
    flSetRenderState(0x6C, 1);
    SetFilterMode(1);
    InitRenderState(1);
}

static void disp_spr_tex_set_sub(int n) {
    flSetRenderState(4, mem_tex[0x14D + n]);
}

static void disp_spr_sub(int flag) {
    s16 buf[90];
    u16 cnt;
    s16 w;
    s16 h;
    s8 idx;
    s32 c;

    switch (flag) {
    case 0x1:
        disp_spr_tex_set_sub(0);
        disp_spr_sub_sub(net_spr_data00, 1, 0);
        break;
    case 0x2:
        disp_spr_tex_set_sub(2);
        disp_spr_sub_sub(net_spr_data01, 2, 0);
        break;
    case 0x4:
        disp_spr_tex_set_sub(2);
        disp_spr_sub_sub(net_spr_data05, 8, 0);
        break;
    case 0x40:
        disp_spr_tex_set_sub(2);
        switch (net_common_w.x34) {
        case 0:
            memcpy(buf, net_spr_data040, 0x8C);
            cnt = 7;
            break;
        case 1:
            memcpy(buf, net_spr_data041, 0xA0);
            cnt = 8;
            break;
        default:
            memcpy(buf, net_spr_data042, 0x64);
            cnt = 5;
            break;
        }
        switch (net_common_w.x35) {
        case 0:
            if (net_common_w.x29 != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x29 != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 1:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 2:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 3:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 4:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 5:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, -52);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 != 3) disp_spr_sub_sub(buf, cnt, 20);
            if (net_common_w.x08 != 4) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 6:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 7:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A != 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 8:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 9:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 10:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, -66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, -22);
            break;
        case 11:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        }
        break;
    case 0x80:
        disp_spr_tex_set_sub(2);
        memset(buf, 0, 0xB4);
        switch (net_common_w.x34) {
        case 0:
            memcpy(buf, net_spr_data030, 0x8C);
            cnt = 7;
            break;
        case 1:
            memcpy(buf, net_spr_data031, 0xA0);
            cnt = 8;
            break;
        default:
            memcpy(buf, net_spr_data032, 0x64);
            cnt = 5;
            break;
        }
        switch (net_common_w.x35) {
        case 0:
            if (net_common_w.x29 == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x29 == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 1:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 2:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 3:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 4:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 5:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, -52);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 == 3) disp_spr_sub_sub(buf, cnt, 20);
            if (net_common_w.x08 == 4) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 6:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 7:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A == 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 8:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 9:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 10:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, -66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, -22);
            break;
        case 11:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        }
        break;
    case 0x100:
        disp_spr_tex_set_sub(1);
        memset(buf, 0, 0xB4);
        memcpy(buf, net_spr_data02, 0x14);
        buf[0] = cur_pos_wid_tbl[net_common_w.x34][0];
        buf[2] = cur_pos_wid_tbl[net_common_w.x34][1];
        if (net_common_w.x2B < 8) {
            buf[4] = net_spr_data02[4];
            buf[6] = net_spr_data02[6];
            buf[5] = net_spr_data02[5] + net_common_w.x2B * 32;
            buf[7] = net_spr_data02[7] + net_common_w.x2B * 32;
        } else {
            buf[4] = net_spr_data02[4] + 0x80;
            buf[6] = net_spr_data02[6] + 0x80;
            buf[5] = net_spr_data02[5] + (net_common_w.x2B - 8) * 32;
            buf[7] = net_spr_data02[7] + (net_common_w.x2B - 8) * 32;
        }
        net_common_w.x60 = -0x8000;
        switch (net_common_w.x35) {
        case 0:
            if (net_common_w.x29 == 0) net_common_w.x60 = 66;
            if (net_common_w.x29 == 1) net_common_w.x60 = 110;
            break;
        case 1:
            if (net_common_w.x08 == 0) net_common_w.x60 = -88;
            if (net_common_w.x08 == 1) net_common_w.x60 = -16;
            if (net_common_w.x08 == 2) net_common_w.x60 = 56;
            break;
        case 2:
            if (net_common_w.x08 == 0) net_common_w.x60 = -88;
            if (net_common_w.x08 == 1) net_common_w.x60 = -16;
            if (net_common_w.x08 == 2) net_common_w.x60 = 56;
            break;
        case 3:
            if (net_common_w.x08 == 0) net_common_w.x60 = 110;
            if (net_common_w.x08 == 1) net_common_w.x60 = 154;
            break;
        case 4:
            if (net_common_w.x0A == 0) net_common_w.x60 = 66;
            if (net_common_w.x0A == 1) net_common_w.x60 = 110;
            break;
        case 5:
            if (net_common_w.x08 == 0) net_common_w.x60 = -88;
            if (net_common_w.x08 == 1) net_common_w.x60 = -52;
            if (net_common_w.x08 == 2) net_common_w.x60 = -16;
            if (net_common_w.x08 == 3) net_common_w.x60 = 20;
            if (net_common_w.x08 == 4) net_common_w.x60 = 56;
            break;
        case 6:
            if (net_common_w.x08 == 0) net_common_w.x60 = 66;
            if (net_common_w.x08 == 1) net_common_w.x60 = 110;
            if (net_common_w.x08 == 2) net_common_w.x60 = 154;
            break;
        case 7:
            if (net_common_w.x0A == 0) net_common_w.x60 = 66;
            if (net_common_w.x0A == 1) net_common_w.x60 = 110;
            if (net_common_w.x0A == 2) net_common_w.x60 = 154;
            break;
        case 8:
            if (net_common_w.x0A == 0) net_common_w.x60 = 110;
            if (net_common_w.x0A == 1) net_common_w.x60 = 154;
            break;
        case 9:
            if (net_common_w.x08 == 0) net_common_w.x60 = 66;
            if (net_common_w.x08 == 1) net_common_w.x60 = 110;
            break;
        case 10:
            if (net_common_w.x0A == 0) net_common_w.x60 = -66;
            if (net_common_w.x0A == 1) net_common_w.x60 = -22;
            break;
        case 11:
            if (net_common_w.x0A == 0) net_common_w.x60 = 66;
            if (net_common_w.x0A == 1) net_common_w.x60 = 110;
            break;
        }
        if (net_common_w.x60 != 0x8000) disp_spr_sub_sub(buf, 1, net_common_w.x60);
        break;
    case 0x400000:
        disp_spr_tex_set_sub(1);
        memset(buf, 0, 0xB4);
        memcpy(buf, net_spr_data02, 0x14);
        buf[0] = cur_pos_wid_tbl[net_common_w.x34][0];
        buf[2] = cur_pos_wid_tbl[net_common_w.x34][1];
        if (net_common_w.x2B < 8) {
            buf[4] = net_spr_data02[4];
            buf[6] = net_spr_data02[6];
            buf[5] = net_spr_data02[5] + net_common_w.x2B * 32;
            buf[7] = net_spr_data02[7] + net_common_w.x2B * 32;
        } else {
            buf[4] = net_spr_data02[4] + 0x80;
            buf[6] = net_spr_data02[6] + 0x80;
            buf[5] = net_spr_data02[5] + (net_common_w.x2B - 8) * 32;
            buf[7] = net_spr_data02[7] + (net_common_w.x2B - 8) * 32;
        }
        if (++net_common_w.x3E > 10) {
            net_common_w.x2C &= ~0x400000;
            break;
        }
        w = buf[2];
        w = (w + net_common_w.x3E * (0x230 - w) / 10) / 2;
        buf[0] = 0x100 - w;
        buf[2] = w * 2;
        if (++net_common_w.x3F > 6) net_common_w.x3F = 6;
        h = (0x20 - net_common_w.x3F * 30 / 6) / 2;
        if (h <= 0) h = 1;
        buf[1] = 0xE0 - h;
        buf[3] = h * 2;
        disp_spr_sub_sub(buf, 1, net_common_w.x60);
        break;
    case 0x200:
        disp_spr_tex_set_sub(3);
        disp_spr_sub_sub(net_spr_data06_tbl[net_common_w.x30], net_spr_data06_suu_tbl[net_common_w.x30], 0);
        break;
    case 0x800:
        memset(buf, 0, 0xB4);
        memcpy(buf, net_spr_data07, 0x14);
        disp_spr_tex_set_sub(4);
        if (++net_common_w.x38 > 10) {
            net_common_w.x38 = 0;
            if (net_common_w.x39 == 0) {
                net_common_w.x39 = 1;
            } else {
                net_common_w.x39 = 0;
            }
        }
        if (net_common_w.x39 != 0) {
            buf[5] = 0x80;
            buf[7] = 0xFF;
        }
        disp_spr_sub_sub(buf, 1, 0);
        if (++net_common_w.x3A > 0x5A) {
            net_common_w.x3A = 0;
            if (net_common_w.x3B == 0) {
                net_common_w.x3B = 1;
            } else {
                net_common_w.x3B = 0;
            }
        }
        if (++net_common_w.x3C > 0x78) {
            net_common_w.x3C = 0;
            if (++net_common_w.x3D >= 5) net_common_w.x3D = 0;
        }
        Ncm_mssage_disp_req(net_common_w.x3B + 0x20);
        Ncm_mssage_disp_req(net_common_w.x3D + 0x22);
        break;
    case 0x1000:
        disp_spr_tex_set_sub(3);
        memset(buf, 0, 0xB4);
        memcpy(buf, net_spr_data08, 0x50);
        if (net_common_w.timer > 100) net_common_w.timer = 100;
        buf[32] = (u32)(buf[32] * net_common_w.timer) / 100;
        disp_spr_sub_sub(buf, 4, 0);
        Net_disp_net_name_req(5);
        break;
    case 0x2000:
        disp_spr_tex_set_sub(6);
        disp_spr_sub_sub(net_spr_data09, 1, 0);
        if (net_common_w.x15 == 1) {
            if (++net_common_w.x3A >= 30) {
                net_common_w.x3A = 0;
                net_common_w.x3B++;
                net_common_w.x3B &= 1;
            }
        } else if (net_common_w.x15 != 0) {
            net_common_w.x3B = 0;
        }
        if (net_common_w.x3B == 0) {
            Ncm_mssage_disp_req(0x1B);
        } else {
            Ncm_mssage_disp_req(0x6D);
        }
        break;
    case 0x4000:
        disp_spr_tex_set_sub(6);
        memset(buf, 0, 0xB4);
        memcpy(buf, net_spr_data0a, 0x28);
        switch (net_common_w.x38) {
        case 0:
            if (net_common_w.x15 == 1) {
                net_common_w.x39 = 0;
                net_common_w.x38++;
            }
            buf[8] = 0xFF;
            buf[18] = 0xFF;
            break;
        case 1:
            if (++net_common_w.x39 > 0x18) {
                net_common_w.x38++;
                buf[8] = -1;
                buf[18] = 0xFF;
                break;
            }
            buf[8] = (buf[8] & 0xFF) | ((u8)(net_common_w.x39 * 255 / 24) << 8);
            buf[18] = 0xFF;
            break;
        case 2:
            switch (net_common_w.x15) {
            case 2:
                net_common_w.x39 = 0;
                net_common_w.x38 = 5;
                break;
            case 3:
                net_common_w.x38++;
                net_common_w.x39 = 0;
                break;
            }
            buf[8] = -1;
            buf[18] = 0xFF;
            break;
        case 3:
            if (++net_common_w.x39 > 0x18) {
                net_common_w.x39 = 0;
                net_common_w.x38++;
                buf[8] = 0xFF;
            } else {
                buf[8] = (buf[8] & 0xFF) | ((u8)((24 - net_common_w.x39) * 255 / 24) << 8);
            }
            buf[18] = 0xFF;
            break;
        case 4:
            buf[8] = 0xFF;
            buf[18] = 0xFF;
            break;
        case 5:
            if (++net_common_w.x39 > 8) {
                net_common_w.x39 = 0;
                net_common_w.x38++;
                buf[8] = 0xFF;
                buf[18] = -1;
                break;
            }
            buf[8] = (buf[8] & 0xFF) | ((u8)((8 - net_common_w.x39) * 255 / 8) << 8);
            buf[18] = (buf[18] & 0xFF) | ((u8)(net_common_w.x39 * 255 / 8) << 8);
            break;
        case 6:
            if (net_common_w.x15 != 2) {
                net_common_w.x39 = 0;
                net_common_w.x38++;
            }
            buf[8] = 0xFF;
            buf[18] = -1;
            break;
        case 7:
            if (++net_common_w.x39 > 0x18) {
                net_common_w.x39 = 0;
                net_common_w.x38++;
                buf[18] = 0xFF;
            } else {
                buf[18] = (buf[18] & 0xFF) | ((u8)((24 - net_common_w.x39) * 255 / 24) << 8);
            }
            buf[8] = 0xFF;
            break;
        case 8:
            buf[8] = 0xFF;
            buf[18] = 0xFF;
            break;
        }
        disp_spr_sub_sub(buf, 2, 0);
        break;
    case 0x1000000:
        disp_spr_tex_set_sub(6);
        disp_spr_sub_sub(net_spr_data13, 1, 0);
        Ncm_mssage_disp_req(0x5D);
        Ncm_mssage_disp_req(0x5E);
        break;
    case 0x2000000:
        c = net_common_w.x80;
        if (c == -101) idx = 0;
        else if (c == -102) idx = 1;
        else if (c == -103) idx = 2;
        else if (c == -104) idx = 3;
        else if (c == -105) idx = 4;
        else if (c == -106) idx = 5;
        else if (c == -107) idx = 6;
        else if (c == -108) idx = 7;
        else if (c < -200 && c >= -204) idx = 8;
        else if (c < -400 && c >= -404) idx = 9;
        else if (c == -601) idx = 10;
        else if (c == -602) idx = 11;
        else if (c == -603 || c == -617) idx = 12;
        else if (c == -610 || c == -616 || c == -620 || c == -619 || c == -618 || c == -626) idx = 13;
        else if (c == -611 || c == -613 || c == -614) idx = 14;
        else if (c == -612) idx = 15;
        else if (c == -615) idx = 16;
        else if (c < -620 && c >= -625) idx = 17;
        else if (c < -700 && c >= -703) idx = 18;
        else if (c < -805 && c >= -1055) idx = 19;
        else if (c <= -1 && c >= -99) idx = 19;
        else if (c == 127) idx = 21;
        else idx = 20;
        Ncm_mssage_disp_req(idx + 0x29);
        if (idx != 21) Net_disp_net_name_req(6);
        break;
    case 0x8000:
        disp_spr_tex_set_sub(3);
        disp_spr_sub_sub(net_spr_data0b, 2, 0);
        break;
    case 0x40000:
        disp_spr_tex_set_sub(2);
        memset(buf, 0, 0xB4);
        switch (net_common_w.x36) {
        case 0:
            memcpy(buf, net_spr_data040, 0x8C);
            cnt = 7;
            break;
        case 1:
            memcpy(buf, net_spr_data041, 0xA0);
            cnt = 8;
            break;
        default:
            memcpy(buf, net_spr_data042, 0x64);
            cnt = 5;
            break;
        }
        switch (net_common_w.x37) {
        case 0:
            if (net_common_w.x29 != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x29 != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 1:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 2:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 3:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 4:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 5:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, -52);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 != 3) disp_spr_sub_sub(buf, cnt, 20);
            if (net_common_w.x08 != 4) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 6:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 != 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 7:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A != 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 8:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 9:
            if (net_common_w.x08 != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 10:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, -66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, -22);
            break;
        case 11:
            if (net_common_w.x0A != 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A != 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        }
        break;
    case 0x80000:
        disp_spr_tex_set_sub(2);
        memset(buf, 0, 0xB4);
        switch (net_common_w.x36) {
        case 0:
            memcpy(buf, net_spr_data030, 0x8C);
            cnt = 7;
            break;
        case 1:
            memcpy(buf, net_spr_data031, 0xA0);
            cnt = 8;
            break;
        default:
            memcpy(buf, net_spr_data032, 0x64);
            cnt = 5;
            break;
        }
        switch (net_common_w.x37) {
        case 0:
            if (net_common_w.x29 == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x29 == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 1:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 2:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 3:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 4:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 5:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, -88);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, -52);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, -16);
            if (net_common_w.x08 == 3) disp_spr_sub_sub(buf, cnt, 20);
            if (net_common_w.x08 == 4) disp_spr_sub_sub(buf, cnt, 56);
            break;
        case 6:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x08 == 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 7:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A == 2) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 8:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 110);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 154);
            break;
        case 9:
            if (net_common_w.x08 == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x08 == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        case 10:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, -66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, -22);
            break;
        case 11:
            if (net_common_w.x0A == 0) disp_spr_sub_sub(buf, cnt, 66);
            if (net_common_w.x0A == 1) disp_spr_sub_sub(buf, cnt, 110);
            break;
        }
        break;
    case 0x100000:
        disp_spr_tex_set_sub(1);
        memset(buf, 0, 0xB4);
        memcpy(buf, net_spr_data02, 0x14);
        buf[0] = cur_pos_wid_tbl[net_common_w.x36][0];
        buf[2] = cur_pos_wid_tbl[net_common_w.x36][1];
        if (net_common_w.x2B < 8) {
            buf[4] = net_spr_data02[4];
            buf[6] = net_spr_data02[6];
            buf[5] = net_spr_data02[5] + net_common_w.x2B * 32;
            buf[7] = net_spr_data02[7] + net_common_w.x2B * 32;
        } else {
            buf[4] = net_spr_data02[4] + 0x80;
            buf[6] = net_spr_data02[6] + 0x80;
            buf[5] = net_spr_data02[5] + (net_common_w.x2B - 8) * 32;
            buf[7] = net_spr_data02[7] + (net_common_w.x2B - 8) * 32;
        }
        net_common_w.x62 = -0x8000;
        switch (net_common_w.x37) {
        case 0:
            if (net_common_w.x29 == 0) net_common_w.x62 = 66;
            if (net_common_w.x29 == 1) net_common_w.x62 = 110;
            break;
        case 1:
            if (net_common_w.x08 == 0) net_common_w.x62 = -88;
            if (net_common_w.x08 == 1) net_common_w.x62 = -16;
            if (net_common_w.x08 == 2) net_common_w.x62 = 56;
            break;
        case 2:
            if (net_common_w.x08 == 0) net_common_w.x62 = -88;
            if (net_common_w.x08 == 1) net_common_w.x62 = -16;
            if (net_common_w.x08 == 2) net_common_w.x62 = 56;
            break;
        case 3:
            if (net_common_w.x08 == 0) net_common_w.x62 = 110;
            if (net_common_w.x08 == 1) net_common_w.x62 = 154;
            break;
        case 4:
            if (net_common_w.x0A == 0) net_common_w.x62 = 66;
            if (net_common_w.x0A == 1) net_common_w.x62 = 110;
            break;
        case 5:
            if (net_common_w.x08 == 0) net_common_w.x62 = -88;
            if (net_common_w.x08 == 1) net_common_w.x62 = -52;
            if (net_common_w.x08 == 2) net_common_w.x62 = -16;
            if (net_common_w.x08 == 3) net_common_w.x62 = 20;
            if (net_common_w.x08 == 4) net_common_w.x62 = 56;
            break;
        case 6:
            if (net_common_w.x08 == 0) net_common_w.x62 = 66;
            if (net_common_w.x08 == 1) net_common_w.x62 = 110;
            if (net_common_w.x08 == 2) net_common_w.x62 = 154;
            break;
        case 7:
            if (net_common_w.x0A == 0) net_common_w.x62 = 66;
            if (net_common_w.x0A == 1) net_common_w.x62 = 110;
            if (net_common_w.x0A == 2) net_common_w.x62 = 154;
            break;
        case 8:
            if (net_common_w.x0A == 0) net_common_w.x62 = 110;
            if (net_common_w.x0A == 1) net_common_w.x62 = 154;
            break;
        case 9:
            if (net_common_w.x08 == 0) net_common_w.x62 = 66;
            if (net_common_w.x08 == 1) net_common_w.x62 = 110;
            break;
        case 10:
            if (net_common_w.x0A == 0) net_common_w.x62 = -66;
            if (net_common_w.x0A == 1) net_common_w.x62 = -22;
            break;
        case 11:
            if (net_common_w.x0A == 0) net_common_w.x62 = 66;
            if (net_common_w.x0A == 1) net_common_w.x62 = 110;
            break;
        }
        if (net_common_w.x62 != 0x8000) disp_spr_sub_sub(buf, 1, net_common_w.x62);
        break;
    case 0x800000:
        disp_spr_tex_set_sub(1);
        memset(buf, 0, 0xB4);
        memcpy(buf, net_spr_data02, 0x14);
        buf[0] = cur_pos_wid_tbl[net_common_w.x34][0];
        buf[2] = cur_pos_wid_tbl[net_common_w.x34][1];
        if (net_common_w.x2B < 8) {
            buf[4] = net_spr_data02[4];
            buf[6] = net_spr_data02[6];
            buf[5] = net_spr_data02[5] + net_common_w.x2B * 32;
            buf[7] = net_spr_data02[7] + net_common_w.x2B * 32;
        } else {
            buf[4] = net_spr_data02[4] + 0x80;
            buf[6] = net_spr_data02[6] + 0x80;
            buf[5] = net_spr_data02[5] + (net_common_w.x2B - 8) * 32;
            buf[7] = net_spr_data02[7] + (net_common_w.x2B - 8) * 32;
        }
        if (++net_common_w.x3E > 10) {
            net_common_w.x2C &= ~0x800000;
            break;
        }
        w = buf[2];
        w = (w + net_common_w.x3E * (0x230 - w) / 10) / 2;
        buf[0] = 0x100 - w;
        buf[2] = w * 2;
        if (++net_common_w.x3F > 6) net_common_w.x3F = 6;
        h = (0x20 - net_common_w.x3F * 30 / 6) / 2;
        if (h <= 0) h = 1;
        buf[1] = 0xE0 - h;
        buf[3] = h * 2;
        disp_spr_sub_sub(buf, 1, net_common_w.x62);
        break;
    }
}

static void disp_spr_sub_sub(s16 *d, int n, int y) {
    int c;
    SPR spr;
    int i;

    for (i = 0; i < n; i++) {
        spr.x = *d++;
        spr.y = *d++;
        spr.y += (s16)y;
        spr.w = *d++;
        spr.h = *d++;
        spr.u = *d++;
        spr.v = *d++;
        spr.u2 = *d++;
        spr.v2 = *d++;
        c = d[0];
        c = (c << 16) | (u16)d[1];
        spr.col = c;
        d += 2;
        flps0008(&spr);
    }
}
