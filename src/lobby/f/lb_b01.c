/* lb_b01 - lobby quest money / guild end 0x005C5D50-0x005C5F30: Lb_check_money, get_quest_info, Lb_get_quest_str, Lb_get_quest_str2, Lb_back_money, lb_guild_end. Whole file in lb_b.c. */
#include "g_alias.h"
#include "lbsys_g.h"
#include "lobby_f.h"
#define USER_GOLD (*(s32 *)0x3C6FE0)







#if !defined(G_PART)
int Lb_check_money(u32 n) {
    LBQUEST *q;
    s32 fee;
    if (n >= 0xC8) {
        q = get_quest_info();
    } else {
        q = lb_quest_all[n];
    }
    fee = q->fee;
    if (USER_GOLD < fee) {
        return 1;
    }
    Gold_add(-fee);
    cnWrap_SoundRequest(8);
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 1
LBQUEST *get_quest_info(void) {
    s32 *p = (s32 *)mission_area;
    return (LBQUEST *)(*p + (int)p);
}
#endif

#if !defined(G_PART)
int Lb_get_quest_str(int n) {
    u8 *qi = (u8 *)get_quest_info();
    int base = mission_area;
    int v = *(s32 *)(*(s32 *)(qi + 0x18) + base + n * 4);
    return v + base;
}
#endif

#if !defined(G_PART)
int Lb_get_quest_str2(int n) {
    LBQUEST *q = lb_quest_all[mhRule.quest];
    if (mhRule.quest >= 0xC8) {
        return Lb_get_quest_str(n);
    }
    return *(s32 *)(*(s32 *)((u8 *)q + 0x18) + n * 4);
}
#endif

#if !defined(G_PART)
void Lb_back_money(void) {
    u16 n = *(u16 *)0x3F33DC;
    LBQUEST *q;
    if (n >= 0xC8) {
        q = get_quest_info();
    } else {
        q = lb_quest_all[n];
    }
    Gold_add(q->fee);
    cnWrap_SoundRequest(8);
}
#endif

#if !defined(G_PART)
void lb_guild_end(void) {
    CW8(CW_OFS(0x2C08)) = 1;
    ((u8 *)&lb_sys)[LBSYS_OFS(0x87)] = 0x14;
    lb_sys.x6C = 0;
    ((u8 *)&lb_sys)[LBSYS_OFS(6)] = 0;
    lb_sys.x68 = 0;
    Lbc_init_network_work(1);
    NPCZoomInCameraCancel();
}
#endif

#if !defined(G_PART) || G_PART == 2
#ifdef MHG_PS2
/* G only */
extern u8 g_tbl_00300AC0[];
u8 *lobby_func_00559D30(u8 i) {
    u8 *p = g_tbl_00300AC0 + i * 0x200;
    return (u8 *)(*(int *)p + (int)p);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 3
#ifdef MHG_PS2
/* G only */
extern s32 quest_price;
void lobby_func_0055A140(u8 a) {
    Gold_add(quest_price);
    if (a == 0 && quest_price != 0) {
        cnWrap_SoundRequest(8);
    }
    quest_price = 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 4
#ifdef MHG_PS2
/* G only */
u8 *lobby_func_00566380(u8 a) {
    u8 *p = em_work;
    u32 i;

    for (i = 0; i < 20; i++, p += 2560) {
        if (p[0] != 0 && p[1144] == a) {
            return p;
        }
    }
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 5
#ifdef MHG_PS2
/* G only */
void Lb_send_trade_start();
void lobby_func_005975E0(u8 *a) {
    a[2288] = 1;
    if (a[2312] != 0) {
        a[2315] = 1;
        Lb_Pl_act_set2(a, 0, 45, 16);
        Lb_send_trade_start(a);
    }
}
#endif
#endif
