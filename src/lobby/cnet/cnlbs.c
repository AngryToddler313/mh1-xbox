/* G: several cnlbs wrappers call the G function whose map name is a neighbour's (the names of the cnLBS_Read_* / Piece*
 * requests in config/mhg/symbols/lobby.txt look shifted); GV() names the G symbol at the address G calls. */
/* cnlbs, run 1: _cnet_RecvFromLbs_NoticeMailMessage .. _cnet_CallBack_Result_Plaza_PlazaStatus_005A6E60 (lobby.bin 0x005A2A20-0x005A6EC4): the matching functions of cnlbs_nm.c. */
#include "lbnet_proto.h"
#pragma readonly_strings on


typedef struct { s16 a, b, c; } CPLACE3;

typedef struct { s8 val; u8 pad[6]; } R7;

#if !defined(G_PART)
void _cnet_RecvFromLbs_NoticeMailMessage(void) {
    __cnet_Recv_MailMessage();
    _cnetEvent_JumpCallBack(3, 0);
}
#endif

#if !defined(G_PART) || G_PART == 94
int cnLBS_SendMessage(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_SendMail, __cnet_SendReq_RoomSetExplain)(arg0, arg1);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 35
void cnLBS_Get_RecvMessage(char *a, char *b, char *c) {
    if (a != 0) {
        strcpy(a, CNWP(0x308E4));
    }
    if (b != 0) {
        strcpy(b, CNWP(0x308EC));
    }
    if (c != 0) {
        strcpy(c, CNWP(0x30900));
    }
}
#endif

#if !defined(G_PART) || G_PART == 35
void _cnet_RecvFromLbs_AnswerSendMail(void) {
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 59
int __cnet_SendReq_SendMail(int arg0, int arg1) {
    int cmd = SetSendCommand(&send_work, GV(0xF1, 0x11A)) & 0xFFFF;
    SetSendStringData2(&send_work, arg0, 6);
    SetSendStringData2(&send_work, arg1, strlen(arg1) & 0xFFFF);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 36
void __cnet_Recv_MailMessage(void) {
    GetRecvDataString(CNWP(0x30900), GetRecvDataString(CNWP(0x308EC), GetRecvDataString(CNWP(0x308E4), &recv_work)));
}
#endif

#if !defined(G_PART) || G_PART == 36
int cnLBS_SerchUserPlace(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_SearchUser(arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 39
void cnLBS_Get_SerchUserPlace(u16 *arg0, u16 *arg1, u16 *arg2, u8 *arg3) {
    *arg0 = CNW(u16, 0x30980);
    *arg1 = CNW(u16, 0x30982);
    *arg2 = CNW(u16, 0x30984);
    *arg3 = CNW(u8, 0x30987);
}
#endif

#if !defined(G_PART) || G_PART == 36
void cnLBS_Get_SerchUserPlaceMessage(char *d) {
    strcpy(d, CNWP(0x30988));
}
#endif

#if !defined(G_PART) || G_PART == 36
void _cnet_RecvFromLbs_AnswerSearchUser(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_SearchUser();
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 60
int __cnet_SendReq_SearchUser(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0xEA, 0x109)) & 0xFFFF;
    SetSendStringData2(&send_work, arg0, 6);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 37
void __cnet_Recv_SearchUser(void) {
    char sp10[9];

    GetRecvDataString(CNWP(0x30988), GetRecvData8(CNWP(0x30987), GetRecvData8(CNWP(0x30986), GetRecvData16(CNWP(0x30984), GetRecvData16(CNWP(0x30982), GetRecvData16(CNWP(0x30980), GetRecvDataString(&sp10, recv_work)))))));
}
#endif

#if !defined(G_PART) || G_PART == 88
void _cnet_RecvFromLbs_RequestAdminMessage(void) {
    GetRecvDataString(CNWP(0x30900), GetRecvDataString(CNWP(0x308EC), &recv_work));
    CNEVT(4, 4);
}
#endif

#if !defined(G_PART) || G_PART == 61
void cnLBS_AnswerAdminMessage(void) {
    SetSendCommand(&send_work, GV(0xF5, 0x11E));
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
}
#endif

#if !defined(G_PART) || G_PART == 62
int cnLBS_ConditionSearchUser(cond, cb)
CNET_COND *cond;
int cb;
{
    CNET_COND c;
    int slot;

    c = *cond;
    memset(&CnetSys_w.csearch, 0, GV(0x1CC4, 0x23F4));
    slot = __cnetSub_Set_BgProcess(1, 0, cb);
    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_ConditionSearchUser(c);
        CnetSys_w.cs_slot = slot;
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 38
int cnLBS_Get_ConditionSearchUser(void **arg0) {
    *arg0 = CNWP(0x39D8C);
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 63
int __cnet_SendReq_ConditionSearchUser(CNET_COND c) {
    int cmd;
    int n;
    int i;
    u8 *e;

    cmd = SetSendCommand(&send_work, GV(0xEC, 0x10B)) & 0xFFFF;
    SetSendData8(&send_work, c.b[0]);
    n = c.b[1];
    SetSendData8(&send_work, n);
    n &= 0xFF;
    if (0 < n) {
        i = 0;
        e = c.b;
        do {
        int t = e[4];
        SetSendData8(&send_work, t);
        switch (t & 0xFF) {
        case 1:
            SetSendStringData2(&send_work, e + 8, 6);
            break;
        case 2:
            SetSendStringData2(&send_work, e + 8, e[5]);
            break;
        case 3:
            SetSendData8(&send_work, e[8]);
            break;
        case 4:
            SetSendData8(&send_work, e[8]);
            break;
        case 5:
            SetSendData8(&send_work, e[8]);
            break;
        case 6:
            SetSendData8(&send_work, e[8]);
            SetSendData8(&send_work, e[9]);
            break;
        }
            i++;
            e += 0x44;
        } while (i < n);
    }
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 64
int __cnet_Send_ConditionSearchUserCertify(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0xEE, 0x10D)) & 0xFFFF;
    SetSendData8(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 65
void _cnet_RecvFromLbs_AnswerConditionSearchUser(void) {
    u8 sp3F;
    u8 sp3E;
    u8 sp3D;
    u8 sp3C;
    int r;
    int i;
    CNET_B5C *p;

    if (CnetSys_w.rres == 0) {
        r = GetRecvData8(&sp3C, GetRecvData8(&sp3D, GetRecvData8(&sp3E, GetRecvData8(&sp3F, recv_work))));
        p = &CnetSys_w.csearch.rec[sp3E];
        for (i = 0; i < sp3D; i++) {
            r = GetRecvDataOption3(p->b + 0x1C, 0x40, GetRecvDataOption3(p->b + 8, 0x10, GetRecvDataOption3(p->b, 8, r)));
            p++;
        }
        if (sp3C == 0) {
            CnetSys_w.bg[CnetSys_w.cs_slot].cmd = __cnet_Send_ConditionSearchUserCertify((sp3D + sp3E) & 0xFF);
            return;
        }
        CnetSys_w.csearch.n = sp3F;
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 40
int cnLBS_RegistPersonalData(pd, cb)
CNET_PDATA *pd;
int cb;
{
    CNET_PDATA tmp;

    tmp = *pd;
    if (CnetSys_w.burst[GV(9, 11)].state == 0) {
        CnetSys_w.pdata = tmp;
        CnetSys_w.burst[GV(9, 11)].cb = (void *)cb;
        CnetSys_w.burst[GV(9, 11)].state = 1;
        CnetSys_w.burst[GV(9, 11)].run = __cnet_bgProg_RegistPersonalData;
        CnetSys_w.burst[GV(9, 11)].x21 = 0;
        return 0;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 41
int cnLBS_RequestPersonalDataChange(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PersonalDataChange();
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 66
int __cnet_SendReq_PersonalDataChange(void) {
    int cmd = SetSendCommand(&send_work, GV(0xB2, 0xD0)) & 0xFFFF;
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 42
void _cnet_RecvFromLbs_AnswerPersonalDataChange(void) {
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 42
int cnLBS_Send_PersonalData(cb)
int cb;
{
    int slot = __cnetSub_Set_BgProcess(1, 0, cb);

    if (slot != -1) {
        __cnet_SendSet_PersonalDataName();
        __cnet_SendSet_PersonalDataZip();
        __cnet_SendSet_PersonalDataAddress();
        __cnet_SendSet_PersonalDataTelephone();
        __cnet_SendSet_PersonalDataAge();
        __cnet_SendSet_PersonalDataMailAddress();
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PersonalDataRegisted();
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 67
void __cnet_SendSet_PersonalDataName(void) {
    char *s0 = CnetSys_w.pdata.name;

    SetSendCommand(&send_work, GV(0xB4, 0xD2));
    SetSendEncodeStringData(&send_work, s0, strlen(s0) & 0xFFFF);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
}
#endif

#if !defined(G_PART) || G_PART == 68
void __cnet_SendSet_PersonalDataZip(void) {
    char *s0 = CnetSys_w.pdata.name;

    SetSendCommand(&send_work, GV(0xB5, 0xD3));
    SetSendEncodeStringData(&send_work, s0 + 0x41, strlen(s0 + 0x41) & 0xFFFF);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
}
#endif

#if !defined(G_PART) || G_PART == 69
void __cnet_SendSet_PersonalDataAddress(void) {
    char *s0 = CnetSys_w.pdata.name;

    SetSendCommand(&send_work, GV(0xB6, 0xD4));
    SetSendEncodeStringData(&send_work, s0 + 0x4C, strlen(s0 + 0x4C) & 0xFFFF);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
}
#endif

#if !defined(G_PART) || G_PART == 70
void __cnet_SendSet_PersonalDataTelephone(void) {
    char *s0 = CnetSys_w.pdata.name;

    SetSendCommand(&send_work, GV(0xB7, 0xD5));
    SetSendEncodeStringData(&send_work, s0 + 0xCD, strlen(s0 + 0xCD) & 0xFFFF);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
}
#endif

#if !defined(G_PART) || G_PART == 71
void __cnet_SendSet_PersonalDataAge(void) {
    SetSendCommand(&send_work, GV(0xB8, 0xD6));
    SetSendData8(&send_work, CnetSys_w.pdata.age);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
}
#endif

#if !defined(G_PART) || G_PART == 72
void __cnet_SendSet_PersonalDataMailAddress(void) {
    char *s0 = CnetSys_w.pdata.name;

    SetSendCommand(&send_work, GV(0xB9, 0xD7));
    SetSendEncodeStringData(&send_work, s0 + 0x14F, strlen(s0 + 0x14F) & 0xFFFF);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
}
#endif

#if !defined(G_PART) || G_PART == 73
int __cnet_SendReq_PersonalDataRegisted(void) {
    int cmd = SetSendCommand(&send_work, GV(0xBA, 0xD8)) & 0xFFFF;
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerPersonalDataRegisted(void) {
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 32
void _cnet_CallBack_Result_PersonalDataChange(CNET_RES res) {
    if (res.val == 0) {
        CNW(s8, 0xF7E) = 1;
        return;
    }
    CNW(s8, 0xF7E) = 2;
}
#endif

#if !defined(G_PART) || G_PART == 107
void __cnet_bgProg_RegistPersonalData(void) {
    CNET_BURST *b = &CnetSys_w.burst[GV(9, 11)];
    CNET_RES res;

    if (b->state != 0) {
        switch (b->x21) {
        case 0:
            b->x21++;
            b->res = 0;
            cnLBS_RequestPersonalDataChange((int)_cnet_CallBack_Result_PersonalDataChange);
            break;
        case 1:
            if (b->res == 1) {
                b->x21++;
            } else if (b->res == 2) {
                b->x21 = 5;
            }
            break;
        case 2:
            b->x21++;
            b->res = 0;
            cnLBS_Send_PersonalData((int)_cnet_CallBack_Result_PersonalDataChange);
            break;
        case 3:
            if (b->res == 1) {
                b->x21++;
            } else if (b->res == 2) {
                b->x21 = 5;
            }
            break;
        case 4:
            res.val = 0;
            b->state = 0;
            b->x21 = 0;
            b->cb(res, &res);
            break;
        case 5:
            res.val = -1;
            b->state = 0;
            b->x21 = 0;
            b->cb(res, &res);
            break;
        }
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
static void __cnet_KeepEntryFloorInfo(kind, val)
int kind;
s8 val;
{
    switch (kind & 0xFF) {
    case 0:
        CNW(s8, 0x4054) = val;
        break;
    case 1:
        CNW(s8, 0x4055) = val;
        break;
    case 2:
        CNW(s8, 0x4056) = val;
        break;
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
void __cnet_ClearEntryFloorInfo(int kind) {
    switch (kind & 0xFF) {
    case 0:
        CNW(s8, 0x405E) = 0;
        break;
    case 1:
        CNW(s8, 0x405F) = 0;
        break;
    case 2:
        CNW(s8, 0x4060) = 0;
        break;
    }
}
#endif

#if !defined(G_PART) || G_PART == 1
void __cnet_SetEntryFloorInfo(int kind) {
    switch (kind & 0xFF) {
    case 0:
        CNW(u8, 0x405E) = CNW(u8, 0x4054);
        break;
    case 1:
        CNW(u8, 0x405F) = CNW(u8, 0x4055);
        break;
    case 2:
        CNW(u8, 0x4060) = CNW(u8, 0x4056);
        break;
    }
}
#endif

#if !defined(G_PART)
int cnLBS_Read_PlazaAllocation(int unused, int val, int cb) {
    if (CnetSys_w.burst[2].state == 0) {
        CnetSys_w.burst[2].cb = (void *)cb;
        CnetSys_w.burst[2].val = val & 0xF;
        CnetSys_w.burst[2].state = 1;
        CnetSys_w.burst[2].run = __cnet_bgProg_ReadPlazaAllocation;
        CnetSys_w.burst[2].x21 = 0;
        return 0;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 79
int cnLBS_Read_PlazaCount(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceCount(GV(0, 1));
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 2
int cnLBS_Get_PlazaCount(u16 *arg0) {
    *arg0 = CNW(u16, 0x404E);
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 2
void _cnet_RecvFromLbs_AnswerPlazaNumOfPlaza(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_NumOfPiece(CNWP(0x404E));
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 80
int cnLBS_Read_PlazaName(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceName(GV(0, 1), arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 74
int cnLBS_Get_PlazaName(idx, d)
int idx;
char *d;
{
    strcpy(d, (u8 *)&CnetSys_w + (((u16)idx - 1) * 0x164) + CNOE(0x4082));
    return 0;
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerPlazaName(void) {
    char buf[0x4E];
    u16 id;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_PieceName(&id, buf);
        CnetSys_w.plaza[id - 1].id = id;
        strcpy((u8 *)&CnetSys_w + ((id - 1) * 0x164) + CNOE(0x4082), buf);
        CnetSys_w.last_id = id;
        strcpy(CnetSys_w.last_name, buf);
        CnetSys_w.plaza[id - 1].flags |= 4;
    }
    _cnet_Return_CallBack(0xE);
}
#endif

#if !defined(G_PART) || G_PART == 81
int cnLBS_Read_PlazaJoinUser(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceJoinUser(GV(0, 1), arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 3
int cnLBS_Get_mhPlazaJoinUser(idx, a, b)
int idx;
u16 *a;
u16 *b;
{
    *a = CnetSys_w.plaza[(u16)idx - 1].ja;
    *b = CnetSys_w.plaza[(u16)idx - 1].jb;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 43
void _cnet_RecvFromLbs_BothPlazaJoinUser(void) {
    if (CNW(s8, 0xFEC) == 0) {
        _sub_ReceiveJoinUser(CNWP(0x4064));
    }
    _cnet_Return_CallBack(GV(0xF, 0x15));
}
#endif

#if !defined(G_PART) || G_PART == 82
int cnLBS_Read_PlazaStatus(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceStatus(GV(0, 1), arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 4
int cnLBS_Get_PlazaStatus(int idx, u8 *d) {
    *d = CnetSys_w.plaza[(u16)idx - 1].status;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 108
void _cnet_RecvFromLbs_BothPlazaStatus(void) {
    u8 st;
    u16 id;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_PieceStatus(&id, &st);
        CnetSys_w.plaza[id - 1].id = id;
        CnetSys_w.plaza[id - 1].status = st;
        CnetSys_w.last_id = id;
#ifndef MHG_PS2
        CnetSys_w.last_status = st;   /* G keeps no last status */
#endif
        CnetSys_w.plaza[id - 1].flags |= 2;
    }
    _cnet_Return_CallBack(GV(0x10, 0x16));
}
#endif

#if !defined(G_PART) || G_PART == 83
int cnLBS_Read_PlazaExplain(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceExplain(GV(0, 1), arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 84
void _cnet_RecvFromLbs_BothPlazaExplain(void) {
    char buf[0x108];
    u16 id;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_PieceExplain(&id, buf);
        CnetSys_w.plaza[id - 1].id = id;
        strcpy((u8 *)&CnetSys_w + ((id - 1) * 0x164) + CNOE(0x40C4), buf);
        CnetSys_w.plaza[id - 1].flags |= 8;
    }
    _cnet_Return_CallBack(GV(0x11, 0x17));
}
#endif

#if !defined(G_PART)
int cnLBS_PlazaEntry(arg0, arg1)
int arg0;
int arg1;
{
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);
    u16 cmd;

    if (slot != -1) {
        cmd = __cnet_SendReq_PieceEntry(0, arg0);
        __cnet_KeepEntryFloorInfo(0, arg0);
        CnetSys_w.bg[slot].cmd = cmd;
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 85
void _cnet_RecvFromLbs_AnswerPlazaEntry(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_SetEntryFloorInfo(GV(0, 1));
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 95
int cnLBS_PlazaExit(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_PieceExit(0), __cnet_SendReq_PieceExit());
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerPlazaExit(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_ClearEntryFloorInfo(0);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART)
int cnLBS_Read_LobbyAllocation(int unused, int val, int cb) {
    if (CnetSys_w.burst[3].state == 0) {
        CnetSys_w.burst[3].cb = (void *)cb;
        CnetSys_w.burst[3].val = val & 0xF;
        CnetSys_w.burst[3].state = 1;
        CnetSys_w.burst[3].run = __cnet_bgProg_ReadLobbyAllocation;
        CnetSys_w.burst[3].x21 = 0;
        return 0;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 86
int cnLBS_Read_LobbyCount(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceCount(GV(1, 2));
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 11
int cnLBS_Get_LobbyCount(u16 *arg0) {
    *arg0 = CNW(u16, 0x4050);
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 90
void _cnet_RecvFromLbs_AnswerLobbyNumOfLobby(void) {
    if (CNW(s8, 0xFEC) == 0) {
        GV(__cnet_Recv_NumOfPiece, lobby_func_005BF880)(CNWP(0x4050));
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 102
int cnLBS_Read_LobbyName(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_PieceName(1, arg0), __cnet_SendReq_PieceJoinUser(2, arg0));
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 78
int cnLBS_Get_LobbyName(idx, d)
int idx;
char *d;
{
    strcpy(d, (u8 *)&CnetSys_w + (((u16)idx - 1) * 0x164) + CNOE(0x4E6A));
    return 0;
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerLobbyName(void) {
    char buf[0x4E];
    u16 id;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_PieceName(&id, buf);
        CnetSys_w.lobby[id - 1].id = id;
        strcpy((u8 *)&CnetSys_w + ((id - 1) * 0x164) + CNOE(0x4E6A), buf);
        CnetSys_w.last_id = id;
        strcpy(CnetSys_w.last_name, buf);
        CnetSys_w.lobby[id - 1].flags |= 4;
    }
    _cnet_Return_CallBack(0x13);
}
#endif

#if !defined(G_PART)
int cnLBS_Read_LobbyJoinUser(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceJoinUser(1, arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 12
int cnLBS_Get_mhLobbyJoinUser(idx, a, b)
int idx;
u16 *a;
u16 *b;
{
    *a = CnetSys_w.lobby[(u16)idx - 1].ja;
    *b = CnetSys_w.lobby[(u16)idx - 1].jb;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 45
void _cnet_RecvFromLbs_BothLobbyJoinUser(void) {
    if (CNW(s8, 0xFEC) == 0) {
        _sub_ReceiveJoinUser(CNWP(0x4E4C));
    }
    _cnet_Return_CallBack(GV(0x14, 0x1E));
}
#endif

#if !defined(G_PART) || G_PART == 87
int cnLBS_Read_LobbyStatus(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceStatus(GV(1, 2), arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 13
int cnLBS_Get_LobbyStatus(int idx, u8 *d) {
    *d = CnetSys_w.lobby[(u16)idx - 1].status;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 109
void _cnet_RecvFromLbs_BothLobbyStatus(void) {
    u8 st;
    u16 id;

    if (CnetSys_w.rres == 0) {
        GV(__cnet_Recv_PieceStatus, lobby_func_005BF8D0)(&id, &st);
        CnetSys_w.lobby[id - 1].id = id;
        CnetSys_w.lobby[id - 1].status = st;
        CnetSys_w.last_id = id;
#ifndef MHG_PS2
        CnetSys_w.last_status = st;   /* G keeps no last status */
#endif
        CnetSys_w.lobby[id - 1].flags |= 2;
    }
    _cnet_Return_CallBack(GV(0x15, 0x1F));
}
#endif

#if !defined(G_PART) || G_PART == 103
int cnLBS_Read_LobbyExplain(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_PieceExplain(1, arg0), __cnet_SendReq_PieceName(2, arg0));
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 106
void _cnet_RecvFromLbs_AnswerLobbyExplain(void) {
    char buf[0x108];
    u16 id;

    if (CnetSys_w.rres == 0) {
        GV(__cnet_Recv_PieceExplain, lobby_func_005E3BE0)(&id, buf);
        CnetSys_w.lobby[id - 1].id = id;
        strcpy((u8 *)&CnetSys_w + ((id - 1) * 0x164) + CNOE(0x4EAC), buf);
        CnetSys_w.lobby[id - 1].flags |= 8;
    }
    _cnet_Return_CallBack(GV(0x16, 0x13));
}
#endif

#if !defined(G_PART)
int cnLBS_LobbyEntry(arg0, arg1)
int arg0;
int arg1;
{
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);
    u16 cmd;

    if (slot != -1) {
        cmd = __cnet_SendReq_PieceEntry(1, arg0);
        __cnet_KeepEntryFloorInfo(1, arg0);
        CnetSys_w.bg[slot].cmd = cmd;
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 5
void _cnet_RecvFromLbs_AnswerLobbyEntry(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_SetEntryFloorInfo(1);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 93
int cnLBS_LobbyExit(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_PieceExit, lobby_func_005AD450)(1);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 6
void _cnet_RecvFromLbs_AnswerLobbyExit(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_ClearEntryFloorInfo(1);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART)
int cnLBS_Read_RoomAllocation(int unused, int val, int cb) {
    if (CnetSys_w.burst[4].state == 0) {
        CnetSys_w.burst[4].cb = (void *)cb;
        CnetSys_w.burst[4].val = val & 0xFF;
        CnetSys_w.burst[4].state = 1;
        CnetSys_w.burst[4].run = __cnet_bgProg_ReadRoomAllocation;
        CnetSys_w.burst[4].x21 = 0;
        return 0;
    }
    return -1;
}
#endif

#if !defined(G_PART)
int cnLBS_Read_RoomCount(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceCount(2);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART)
int cnLBS_Get_RoomCount(u16 *arg0) {
    *arg0 = CNW(u16, 0x4052);
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 89
void _cnet_RecvFromLbs_AnswerRoomNumOfRoom(void) {
    if (CNW(s8, 0xFEC) == 0) {
        GV(__cnet_Recv_NumOfPiece, __cnet_Recv_Byte)(CNWP(0x4052));
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 101
int cnLBS_Read_RoomJoinUser(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_PieceJoinUser(2, arg0), lobby_func_005AE360(arg0));
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 33
int cnLBS_Get_mhRoomJoinUser(idx, a, b)
int idx;
u16 *a;
u16 *b;
{
    *a = CnetSys_w.room[(u16)idx - 1].ja;
    *b = CnetSys_w.room[(u16)idx - 1].jb;
    return 0;
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_BothRoomJoinUser(void) {
    if (CNW(s8, 0xFEC) == 0) {
        _sub_ReceiveJoinUser(CNWP(0x61C4));
    }
    _cnet_Return_CallBack(0x19);
}
#endif

#if !defined(G_PART) || G_PART == 105
int cnLBS_Read_RoomStatus(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_PieceStatus(2, arg0), __cnet_SendReq_RoomProperty(arg0));
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 34
int cnLBS_Get_RoomStatus(int idx, u8 *d) {
    *d = CnetSys_w.room[(u16)idx - 1].status;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 113
void _cnet_RecvFromLbs_BothRoomStatus(void) {
    u8 st;
    u16 id;

    if (CnetSys_w.rres == 0) {
        GV(__cnet_Recv_PieceStatus, __cnet_Recv_WordByte)(&id, &st);
        CnetSys_w.room[id - 1].id = id;
        CnetSys_w.room[id - 1].status = st;
        CnetSys_w.last_id = id;
#ifndef MHG_PS2
        CnetSys_w.last_status = st;   /* G keeps no last status */
#endif
        CnetSys_w.room[id - 1].flags |= 2;
    }
    _cnet_Return_CallBack(GV(0x1A, 0x12));
}
#endif

#if !defined(G_PART)
int cnLBS_Read_RoomName(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceName(2, arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 75
int cnLBS_Get_RoomName(idx, d)
int idx;
char *d;
{
    strcpy(d, (u8 *)&CnetSys_w + (((u16)idx - 1) * 0x164) + CNOE(0x61E2));
    return 0;
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_BothRoomName(void) {
    char buf[0x108];
    u16 id;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_PieceName(&id, buf);
        CnetSys_w.room[id - 1].id = id;
        strcpy((u8 *)&CnetSys_w + ((id - 1) * 0x164) + CNOE(0x61E2), buf);
        CnetSys_w.last_id = id;
        strcpy(CnetSys_w.last_name, buf);
        CnetSys_w.room[id - 1].flags |= 4;
    }
    _cnet_Return_CallBack(0x18);
}
#endif

#if !defined(G_PART) || G_PART == 14
int cnLBS_Read_RoomExplain(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_PieceExplain(2, arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 76
int cnLBS_Get_RoomExplain(idx, d)
int idx;
char *d;
{
    strcpy(d, (u8 *)&CnetSys_w + (((u16)idx - 1) * 0x164) + CNOE(0x6224));
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 104
void _cnet_RecvFromLbs_BothRoomExplain(void) {
    char buf[0x108];
    u16 id;

    if (CnetSys_w.rres == 0) {
        GV(__cnet_Recv_PieceExplain, lobby_func_005BF910)(&id, buf);
        CnetSys_w.room[id - 1].id = id;
        strcpy((u8 *)&CnetSys_w + ((id - 1) * 0x164) + CNOE(0x6224), buf);
        CnetSys_w.room[id - 1].flags |= 8;
    }
    _cnet_Return_CallBack(GV(0x1B, 0x20));
}
#endif

#if !defined(G_PART) || G_PART == 15
int cnLBS_Read_RoomJoinInfo(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RoomJoinInfo(arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 15
int cnLBS_Get_RoomJoinInfo(idx, a, b, c, d, e)
int idx;
u16 *a;
u16 *b;
u16 *c;
u16 *d;
u16 *e;
{
    *a = CnetSys_w.room[(u16)idx - 1].ri[0];
    *b = CnetSys_w.room[(u16)idx - 1].ri[1];
    *c = CnetSys_w.room[(u16)idx - 1].ri[2];
    *d = CnetSys_w.room[(u16)idx - 1].ri[3];
    *e = CnetSys_w.room[(u16)idx - 1].ri[4];
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 46
int __cnet_SendReq_RoomJoinInfo(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0x8D, 0x8C)) & 0xFFFF;
    SetSendData16(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 110
void _cnet_RecvFromLbs_BothRoomJoinInfo(void) {
    u16 id;
    CNET_PIECE *p;

    if (CnetSys_w.rres == 0) {
        int r = GetRecvData16(&id, recv_work);
        p = &CnetSys_w.room[id - 1];
        /* G stores the last four words one slot on (one more word after ri[0]) */
        GetRecvData16(&p->ri[0] + GV(4, 5), GetRecvData16(&p->ri[0] + GV(2, 3), GetRecvData16(&p->ri[0] + GV(3, 4), GetRecvData16(&p->ri[0] + GV(1, 2), GetRecvData16(&p->ri[0], r)))));
        p->flags |= 0x20;
    }
    _cnet_Return_CallBack(GV(0x1D, 0x21));
}
#endif

#if !defined(G_PART) || G_PART == 116
#ifdef MHG_PS2
u16 lobby_func_005BC190();
#endif
int cnLBS_RoomCreate(arg0, arg1)
int arg0;
int arg1;
{
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);
    u16 cmd;

    if (slot != -1) {
#ifdef MHG_PS2
        __cnet_KeepEntryFloorInfo(2, arg0);
        cmd = lobby_func_005BC190(arg0);    /* G's own room-create request */
#else
        cmd = __cnet_SendReq_RoomCreate(arg0);
        __cnet_KeepEntryFloorInfo(2, arg0);
#endif
        CnetSys_w.bg[slot].cmd = cmd;
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 91
void _cnet_RecvFromLbs_AnswerRoomCreate(void) {
    if (CNW(s8, 0xFEC) == 0) {
        GV(__cnet_SetEntryFloorInfo, __cnet_ClearEntryFloorInfo)(2);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART)
int cnLBS_Set_RoomRule(rule, cb)
CNET_RULE *rule;
int cb;
{
    CnetSys_w.rule = *rule;
    if (CnetSys_w.burst[6].state == 0) {
        CnetSys_w.burst[6].cb = (void *)cb;
        CnetSys_w.burst[6].state = 1;
        CnetSys_w.burst[6].run = __cnet_bgProg_RoomSetRule;
        CnetSys_w.burst[6].x21 = 0;
        return 0;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 98
int cnLBS_Set_RoomRuleFinish(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_RoomSetFinish, g_SendReq_005BCE20)();
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 115
#ifdef MHG_PS2
u16 lobby_func_005BBF60();
#endif
int cnLBS_RoomEntry(arg0, arg1)
int arg0;
int arg1;
{
    int slot = __cnetSub_Set_BgProcess(1, 0);
    u16 cmd;

    if (slot != -1) {
#ifdef MHG_PS2
        __cnet_KeepEntryFloorInfo(2, arg0);
        cmd = lobby_func_005BBF60(arg0, arg1);    /* G's own room-entry request */
#else
        cmd = __cnet_SendReq_RoomEntry(arg0, arg1);
        __cnet_KeepEntryFloorInfo(2, arg0);
#endif
        CnetSys_w.bg[slot].cmd = cmd;
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 118
#ifdef MHG_PS2
/* G 0x5BC5B0 (the map calls it cnLBS_Read_RoomRuleAllocation): a menu step on a net window */
void g_step_005BC5B0(u8 *a) {
    u8 st = a[0x23];

    if (st == 1) {
        a[0x22]++;
        a[0x23] = 0;
    } else if (st == 2) {
        a[0x21] = 11;
        a[0x22] = 0;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 19
void _cnet_RecvFromLbs_AnswerRoomEntry(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_SetEntryFloorInfo(2);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 96
int cnLBS_RoomExit(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_PieceExit, lobby_func_005AD450)(2);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 92
int cnLBS_Read_RoomProperty(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_RoomProperty, __cnet_SendReq_RoomCreate)(arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 16
int cnLBS_Get_RoomProperty(int idx, int *d) {
    *d = CnetSys_w.room[(u16)idx - 1].prop;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 47
int __cnet_SendReq_RoomProperty(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0x98, 0x84)) & 0xFFFF;
    SetSendData16(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 48
void _cnet_RecvFromLbs_BothRoomProperty(void) {
    u16 id;
    s32 prop;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_WordLong(&id, &prop);
        CnetSys_w.room[id - 1].id = id;
        CnetSys_w.room[id - 1].prop = prop;
        CnetSys_w.last_id = id;
        CnetSys_w.room[id - 1].flags |= 0x80;
    }
    _cnet_Return_CallBack(GV(0x29, 0x23));
}
#endif

#if !defined(G_PART) || G_PART == 17
int cnLBS_Set_RoomProperty(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_SetRoomProperty(arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 49
int __cnet_SendReq_SetRoomProperty(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0x96, 0xB7)) & 0xFFFF;
    SetSendData32(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 18
void _cnet_RecvFromLbs_AnswerSetRoomProperty(void) {
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 22
int cnLBS_Get_RoomLeaveUser(CNET_B5C *d) {
    *d = CnetSys_w.leave_user;
    return 0;
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_NoticeRoomCommer(void) {
    _sub_InOutRoomMember(2);
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_NoticeRoomLeaver(void) {
    _sub_InOutRoomMember(3);
}
#endif

#if !defined(G_PART) || G_PART == 114
void _sub_InOutRoomMember(kind)
int kind;
{
    int r;

    if (CnetSys_w.rres == 0) {
        memset(&CnetSys_w.leave_user, 0, 0x5C);
        r = GetRecvDataOption3(&CnetSys_w.leave_user, 8, recv_work);
        switch (kind & 0xFF) {
        case 0:
        case 2:
        case 4:
            r = GetRecvDataOption3(&CnetSys_w.leave_user.b[8], 0x10, r);
            GetRecvDataOption3(&CnetSys_w.leave_user.b[0x1C], 0x40, r);
            break;
        }
    }
    if (CnetSys_w.rcat == 0x10) {
        switch (kind & 0xFF) {
        case 0:
            _cnet_Return_CallBack(GV(0x26, 0x1A));
            break;
        case 1:
            _cnet_Return_CallBack(GV(0x27, 0x1B));
            break;
        case 2:
            _cnet_Return_CallBack(GV(0x1F, 0x24));
            break;
        case 3:
            _cnet_Return_CallBack(GV(0x20, 0x25));
            break;
        case 4:
            _cnet_Return_CallBack(GV(0x24, 0x29));
            break;
        case 5:
            _cnet_Return_CallBack(GV(0x25, 0x2A));
            break;
        }
    }
}
#endif

#if !defined(G_PART)
void _sub_ReceiveJoinUser(tbl)
CNET_PIECE *tbl;
{
    u16 id;
    u16 ja;
    u16 jb;
    CNET_PIECE *p;

    if (CnetSys_w.rres == 0) {
        p = tbl;
        if (CNW(u8, 0x10D2) != 4) {
            __cnet_Recv_PieceJoinUser(&id, &ja);
            p += id - 1;
            p->id = id;
            p->ja = ja;
            p->flags |= 1;
        } else if (CNW(u8, 0x10D2) == 4) {
            __cnet_Recv_PieceJoinUserMH(&id, &ja, &jb);
            p += id - 1;
            p->id = id;
            p->ja = ja;
            p->jb = jb;
            p->flags |= 1;
        }
        CnetSys_w.last_id = id;
        CnetSys_w.last_ja = ja;
    }
}
#endif

#if !defined(G_PART)
int cnLBS_Read_RoomRuleAllocation(int val, int cb) {
    if (CnetSys_w.burst[5].state == 0) {
        CnetSys_w.burst[5].cb = (void *)cb;
        CNW(s32, 0xEE0) = val & 0xFFFF;
        CnetSys_w.burst[5].state = 1;
        CnetSys_w.burst[5].run = __cnet_bgProg_ReadRoomRule;
        CnetSys_w.burst[5].x21 = 0;
        return 0;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 20
int cnLBS_Read_RoomRuleCount(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_NumOfRule(arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 97
int cnLBS_Read_RoomNamePermission(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = GV(__cnet_SendReq_RoomNamePermission, __cnet_SendReq_RoomSetName)(arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 117
#ifdef MHG_PS2
/* G 0x5BCE20 (the map calls it _cnet_RecvFromLbs_AnswerRoomNamePermission): a bare request, command 175 */
int g_SendReq_005BCE20(void) {
    int cmd = SetSendCommand(&send_work, 175) & 0xFFFF;
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerRoomNamePermission(void) {
    u8 v;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Byte(&v);
        CnetSys_w.ruletbl.name_perm = v;
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 50
int __cnet_SendReq_RoomNamePermission(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0x55, 0x99)) & 0xFFFF;
    SetSendData16(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 23
int cnLBS_Read_RoomPasswordPermission(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RoomPasswordPermission(arg0 & 0xFFFF);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 111
void _cnet_RecvFromLbs_AnswerRoomPasswordPermission(void) {
#ifdef MHG_PS2
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Byte((u8 *)&CnetSys_w + 0xA3BD);   /* G receives into the table (G address 0x704BC5; LB CNET_SYS places ruletbl 0xA8 later, unverified) */
    }
#else
    u8 v;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Byte(&v);
        CnetSys_w.ruletbl.pw_perm = v;
    }
#endif
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 51
int __cnet_SendReq_RoomPasswordPermission(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0x57, 0x95)) & 0xFFFF;
    SetSendData16(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 24
int cnLBS_Read_RoomPasswordInfo(int arg0, int arg1) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg1);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RoomPasswordInfo(arg0);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 24
int cnLBS_Get_RoomPasswordInfo(int idx, u8 *d) {
    *d = CnetSys_w.room[(u16)idx - 1].pwinfo;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 52
int __cnet_SendReq_RoomPasswordInfo(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0x86, 0x87)) & 0xFFFF;
    SetSendData16(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 112
void _cnet_RecvFromLbs_BothRoomPasswordInfo(void) {
    u8 pw;
    u16 id;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_WordByte(&id, &pw);
        CnetSys_w.room[id - 1].pwinfo = pw;
        CnetSys_w.room[id - 1].flags |= 0x10;
        CnetSys_w.last_id = id;
#ifndef MHG_PS2
        CnetSys_w.last_pwinfo = pw;   /* G keeps no last password info */
#endif
    }
    _cnet_Return_CallBack(GV(0x1E, 0x22));
}
#endif

#if !defined(G_PART) || G_PART == 53
int cnLBS_Read_RoomMemberList(arg, cb)
int arg;
int cb;
{
    int slot;

    memset(CnetSys_w.room_member, 0, GV(0x300, 0x3C0));
    slot = __cnetSub_Set_BgProcess(1, 0, cb);
    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RoomMember(arg);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 77
int cnLBS_Get_RoomMemberList(idx, d1, d2, d3)
int idx;
char *d1;
char *d2;
void *d3;
{
    u8 *t = (u8 *)&CnetSys_w + (u16)idx * 0x60;

    strcpy(d1, t + CNO(0x39EE));
    strcpy(d2, t + CNO(0x39F6));
    memcpy(d3, t + CNO(0x3A0A), 0x40);
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 54
int __cnet_SendReq_RoomMember(int arg0) {
    int cmd = SetSendCommand(&send_work, GV(0x8B, 0x8A)) & 0xFFFF;
    SetSendData16(&send_work, arg0);
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 25
void _cnet_RecvFromLbs_AnswerRoomMember(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_RoomMember();
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 55
void __cnet_Recv_RoomMember(void) {
    memset(CnetSys_w.room_member, 0, GV(0x300, 0x3C0));
    __cnet_Recv_MemberSub(&CnetSys_w.n_room_member, CnetSys_w.room_member);
}
#endif

#if !defined(G_PART) || G_PART == 7
void _cnet_RecvFromLbs_AnswerAppointJump(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_SetEntryFloorInfo(0);
        __cnet_SetEntryFloorInfo(1);
        __cnet_SetEntryFloorInfo(2);
        __cnet_Recv_ServerMessage();
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 7
int cnLBS_TopPageJump(int arg0) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg0);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_TopPageJump();
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 7
void _cnet_RecvFromLbs_AnswerTopPageJump(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_ClearEntryFloorInfo(0);
        __cnet_ClearEntryFloorInfo(1);
        __cnet_ClearEntryFloorInfo(2);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 7
int __cnet_SendReq_TopPageJump(void) {
    int cmd = SetSendCommand(&send_work, 0x2C) & 0xFFFF;
    SetSendCommandLen(&send_work);
    Write_Socket(&send_work);
    return cmd;
}
#endif

#if !defined(G_PART) || G_PART == 9
int cnLBS_Read_RoomRuleCaption(int arg0, int arg1, int arg2) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg2);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RuleListHeadWord(arg0, arg1);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 10
int cnLBS_Read_RoomRuleChoiceCount(int arg0, int arg1, int arg2) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg2);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RuleNumOfChoice(arg0, arg1);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 21
int cnLBS_Read_RoomRuleNow(int arg0, int arg1, int arg2) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg2);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RuleListNow(arg0, arg1);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 26
int cnLBS_Read_RoomRuleChoicePermission(int arg0, int arg1, int arg2) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg2);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RuleListPermission(arg0, arg1);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 28
int cnLBS_Read_RoomRuleChoiceName(int arg0, int arg1, int arg2, int arg3) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg3);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RuleListName(arg0, arg1, arg2);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 29
int cnLBS_Read_RoomRuleChoiceControl(int arg0, int arg1, int arg2, int arg3) {
    int slot = __cnetSub_Set_BgProcess(1, 0, arg3);

    if (slot != -1) {
        CnetSys_w.bg[slot].cmd = __cnet_SendReq_RuleControl(arg0, arg1, arg2);
        return slot;
    }
    return -1;
}
#endif

#if !defined(G_PART) || G_PART == 7
int cnetGet_Room_LastRecvNumber(u16 *arg0) {
    *arg0 = CNW(u16, 0x6CEC);
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 31
int cnLBS_Get_RoomRuleAllocation(unused, d)
int unused;
CNET_RULETBL *d;
{
    *d = CnetSys_w.ruletbl;
    return 0;
}
#endif

#if !defined(G_PART) || G_PART == 44
void _cnet_RecvFromLbs_BothGameJoin(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_GameJoin();
    }
    _cnet_Return_CallBack(GV(0xD, 0xF));
}
#endif

#if !defined(G_PART) || G_PART == 8
void _cnet_RecvFromLbs_AnswerRoomNumOfRule(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_Byte(CNWP(0x6E4B));
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 56
void _cnet_RecvFromLbs_AnswerRuleListHeadWord(void) {
    u8 n;
    char buf[0x4E];

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ByteString(&n, buf);
        strcpy(CnetSys_w.ruletbl.e[n].head, buf);
        CnetSys_w.ruletbl.e[n].flags |= 1;
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 99
void _cnet_RecvFromLbs_AnswerRuleListPermission(void) {
    u8 n;
    u8 v;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ByteByte(&n, &v);
        CnetSys_w.ruletbl.e[n].perm = v;
        CnetSys_w.ruletbl.e[n].flags |= GV(8, 4);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 27
void _cnet_RecvFromLbs_AnswerRuleListNow(void) {
    u8 n;
    u8 v;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ByteByte(&n, &v);
        CnetSys_w.ruletbl.e[n].now = v;
        CnetSys_w.ruletbl.e[n].flags |= 2;
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 100
void _cnet_RecvFromLbs_AnswerRuleListNumOf(void) {
    u8 n;
    u8 v;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ByteByte(&n, &v);
        CnetSys_w.ruletbl.e[n].numof = v;
        CnetSys_w.ruletbl.e[n].flags |= GV(4, 8);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 57
void _cnet_RecvFromLbs_AnswerRuleListName(void) {
    u8 n;
    u8 c;
    char buf[0x4E];

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ByteByteString(&n, &c, buf);
        strcpy(CnetSys_w.ruletbl.e[n].names + c * 0x41, buf);
        CnetSys_w.ruletbl.e[n].cflag[c] |= 1;
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART) || G_PART == 58
void _cnet_RecvFromLbs_AnswerRuleControl(void) {
    u8 rule;
    u8 ch;
    u8 *src;
    int i;
    int n;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_RuleControl(&rule, &ch, &src);
        n = *src;
        src++;
        for (i = 0; i < n; i++) {
            CnetSys_w.ruletbl.e[rule].tri[ch][i].a = src[0];
            CnetSys_w.ruletbl.e[rule].tri[ch][i].b = src[1];
            CnetSys_w.ruletbl.e[rule].tri[ch][i].c = src[2];
            src += 3;
        }
        CnetSys_w.ruletbl.e[rule].tcnt[ch] = n;
        CnetSys_w.ruletbl.e[rule].cflag[ch] |= 2;
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerRoomSetName(void) {

}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerRoomSetRule(void) {

}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_AnswerRoomSetFinish(void) {
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_BothRoomExit(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_ClearEntryFloorInfo(2);
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_NoticePlazaRemove(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_ServerMessage();
    }
    _cnetEvent_JumpCallBack(9, 0);
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_NoticeLobbyRemove(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_ServerMessage();
    }
    _cnetEvent_JumpCallBack(0xA, 0);
}
#endif

#if !defined(G_PART)
void _cnet_RecvFromLbs_NoticeRoomRemove(void) {
    if (CNW(s8, 0xFEC) == 0) {
        __cnet_Recv_ServerMessage();
    }
    _cnetEvent_JumpCallBack(0xB, 0);
}
#endif

#if !defined(G_PART) || G_PART == 30
void _cnet_RecvFromLbs_AnswerRoomRestTime(void) {
    u16 t;

    if (CnetSys_w.rres == 0) {
        __cnet_Recv_WordWord(&t, CNWP(0x302FE));
    }
    _cnet_Return_CallBack(0);
}
#endif

#if !defined(G_PART)
void __cnet_CallBack_Result_Plaza_NumOfPlaza_005A6E20(CNET_RES res) {
    if (res.val == 0) {
        CnetSys_w.burst[2].res = 1;
        return;
    }
    CnetSys_w.burst[2].res = 2;
}
#endif

#if !defined(G_PART)
void _cnet_CallBack_Result_Plaza_PlazaStatus_005A6E60(CNET_RES res) {
    CNET_RES r;

    if (res.val == 0) {
        r.val = 2;
        r.id = 0xB;
        CnetSys_w.burst[2].cb(r, &r);
        CnetSys_w.burst[2].res = 1;
        return;
    }
    CnetSys_w.burst[2].res = 2;
}
#endif

#if !defined(G_PART) || G_PART == 119
#ifdef MHG_PS2
/* G only */
void lobby_func_005BC510(u8 *a) {
    a[0x22]++;
    a[0x23] = 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 120
#ifdef MHG_PS2
/* G only */
void lobby_func_005BD370(u8 *a) {
    a[0x22]++;
    *(int *)(a + 8) = 0;
    *(int *)(a + 0xC) = 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 124
#ifdef MHG_PS2
/* G only */
void lobby_func_005BD8A0(u8 *a) {
    a[0x22]++;
    *(int *)(a + 8) = 0;
    *(int *)(a + 0xC) = 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 125
#ifdef MHG_PS2
/* G only */
void lobby_func_005BDA40(u8 *a) {
    a[0x22]++;
    *(int *)(a + 0xC) = 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 126
#ifdef MHG_PS2
/* G only */
void lobby_func_005C02B0();
void _cnet_CallBack_Result_PersonalDataChange();
void lobby_func_005BFBA0(u8 *a) {
    a[0x22]++;
    a[0x23] = 0;
    lobby_func_005C02B0(_cnet_CallBack_Result_PersonalDataChange);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 127
#ifdef MHG_PS2
/* G only */
int lobby_func_005E3420(u16 *p) {
    *p = *(u16 *)((u8 *)&CnetSys_w + 0x43B4);
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 128
#ifdef MHG_PS2
/* G only */
void lobby_func_005E3CD0(f32);
void lobby_func_005E3C20() {
    lobby_func_005E3CD0(1.0f);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 129
#ifdef MHG_PS2
/* G only */
int lobby_func_005E6680(u8 **p) {
    *p = (u8 *)&CnetSys_w + 0x38E5E;
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 130
#ifdef MHG_PS2
/* G only */
int lobby_func_005E6EC0(u8 **p) {
    *p = (u8 *)&CnetSys_w + 0x39632;
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 121
#ifdef MHG_PS2
/* G only */
void lobby_func_005ABA60(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 122
#ifdef MHG_PS2
/* G only */
void lobby_func_005ABC40(u8 *a) {
    a[0x22]++;
    a[0x23] = 0;
    if (*(u16 *)((u8 *)&CnetSys_w + 0x43B6) == 0) {
        a[0x21] = 1;
        a[0x22] = 0;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 123
#ifdef MHG_PS2
/* G only */
extern u8 g_cn_00700EA2[];
int lobby_func_005AC230(u16 i, u16 *out) {
    *out = *(u16 *)(g_cn_00700EA2 + i * 356);
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 132
#ifdef MHG_PS2
/* G only */
void lobby_func_005ADF20(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 133
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE6F0(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 136
#ifdef MHG_PS2
/* G only */
void lobby_func_005BC4A0(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 137
#ifdef MHG_PS2
/* G only */
void lobby_func_005BD210(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 138
#ifdef MHG_PS2
/* G only */
void lobby_func_005BFB30(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 139
#ifdef MHG_PS2
/* G only */
void lobby_func_005BFF10(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 142
#ifdef MHG_PS2
/* G only */
void lobby_func_005E2F10(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 135
#ifdef MHG_PS2
/* G only */
void lobby_func_005BB0C0(u8 *a) {
    a[0x22]++;
    a[0x23] = 0;
    if (*(u16 *)((u8 *)&CnetSys_w + 0x43B8) == 0) {
        a[0x21] = 1;
        a[0x22] = 0;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 143
#ifdef MHG_PS2
/* G only */
void lobby_func_005E30F0(u8 *a) {
    a[0x22]++;
    a[0x23] = 0;
    if (*(u16 *)((u8 *)&CnetSys_w + 0x43B4) == 0) {
        a[0x21] = 1;
        a[0x22] = 0;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 131
#ifdef MHG_PS2
/* G only */
extern u8 g_cn_00700E98[];
int lobby_func_005AC580(u16 i, u32 *out) {
    *out = *(u32 *)(g_cn_00700E98 + i * 356);
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 134
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE760(u8 *a) {
    a[0x22]++;
    *(u16 *)((u8 *)&CnetSys_w + 0x1032) = 0;
    *(int *)(a + 8) = 1;
    *(int *)(a + 0xC) = 1;
    a[0x23] = 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 140
#ifdef MHG_PS2
/* G only */
void lobby_func_005C0580() {
    __cnet_Recv_ServerMessage();
    _cnet_Return_CallBack(45);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 141
#ifdef MHG_PS2
/* G only */
int lobby_func_005C0620(u8 *a, u8 **b) {
    if (a != 0) {
        *a = *(u8 *)((u8 *)&CnetSys_w + 0x3A5EC);
    }
    if (b != 0) {
        *b = (u8 *)&CnetSys_w + 0x3A5F4;
    }
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 145
#ifdef MHG_PS2
/* G only */
void lobby_func_005ADF50(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 146
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE0B0(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xE38;

    if (res.val == 0) {
        b[0x23] |= 0x1;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 147
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE0F0(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xE38;

    if (res.val == 0) {
        b[0x23] |= 0x2;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 148
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE130(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xE38;

    if (res.val == 0) {
        b[0x23] |= 0x4;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 150
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE720(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 151
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE9D0(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xEB0;

    if (res.val == 0) {
        b[0x23] |= 0x1;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 152
#ifdef MHG_PS2
/* G only */
void lobby_func_005AEA10(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xEB0;

    if (res.val == 0) {
        b[0x23] |= 0x2;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 153
#ifdef MHG_PS2
/* G only */
void lobby_func_005AEA50(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xEB0;

    if (res.val == 0) {
        b[0x23] |= 0x4;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 154
#ifdef MHG_PS2
/* G only */
void lobby_func_005AEA90(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xEB0;

    if (res.val == 0) {
        b[0x23] |= 0x4;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 155
#ifdef MHG_PS2
/* G only */
void lobby_func_005BC4D0(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 156
#ifdef MHG_PS2
/* G only */
void lobby_func_005BD240(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 157
#ifdef MHG_PS2
/* G only */
void lobby_func_005BFB60(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 158
#ifdef MHG_PS2
/* G only */
void lobby_func_005BFF40(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 159
#ifdef MHG_PS2
/* G only */
void lobby_func_005C00A0(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xFA0;

    if (res.val == 0) {
        b[0x23] |= 0x1;
    } else {
        b[0x23] |= 0x80;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 160
#ifdef MHG_PS2
/* G only */
void lobby_func_005E2F40(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 161
#ifdef MHG_PS2
/* G only */
extern u8 g_cn_006FEA7A[];
int lobby_func_005E36A0(i, out)
u16 i;
u16 *out;
{
    *out = *(u16 *)(g_cn_006FEA7A + i * 356);
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 162
#ifdef MHG_PS2
/* G only */
extern u8 g_cn_006FEA7C[];
int lobby_func_005E36D0(i, out)
u16 i;
u16 *out;
{
    *out = *(u16 *)(g_cn_006FEA7C + i * 356);
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 163
#ifdef MHG_PS2
/* G only */
void __cnet_Recv_MailMessage();
void lobby_func_005E5C20() {
    __cnet_Recv_MailMessage();
    _cnet_Return_CallBack(3);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 144
#ifdef MHG_PS2
/* G only */
void lobby_func_005ACE90() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ServerMessage();
    }
    _cnet_Return_CallBack(9);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 149
#ifdef MHG_PS2
/* G only */
void lobby_func_005AE450() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Byte((u8 *)&CnetSys_w + 0x33862);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 164
#ifdef MHG_PS2
/* G only */
void lobby_func_005AEFC0() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 165
#ifdef MHG_PS2
/* G only */
void lobby_func_005B07F0() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 166
#ifdef MHG_PS2
/* G only */
void lobby_func_005B0BA0() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 169
#ifdef MHG_PS2
/* G only */
void lobby_func_005BCB80() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 170
#ifdef MHG_PS2
/* G only */
void lobby_func_005BCC90() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 171
#ifdef MHG_PS2
/* G only */
void lobby_func_005BCDA0() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 172
#ifdef MHG_PS2
/* G only */
void lobby_func_005BCE70() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 175
#ifdef MHG_PS2
/* G only */
void lobby_func_005BF830() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 181
#ifdef MHG_PS2
/* G only */
void lobby_func_005E6C00() {
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 177
#ifdef MHG_PS2
/* G only */
void lobby_func_005E1CE0() {
    _cnet_Return_CallBack(5);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 167
#ifdef MHG_PS2
/* G only */
void lobby_func_005BC770(u8 *a) {
    a[0x22]++;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 168
#ifdef MHG_PS2
/* G only */
void lobby_func_005BCA70() {
}
#endif
#endif

#if !defined(G_PART) || G_PART == 178
#ifdef MHG_PS2
/* G only */
void lobby_func_005E27B0() {
}
#endif
#endif

#if !defined(G_PART) || G_PART == 179
#ifdef MHG_PS2
/* G only */
void lobby_func_005E27C0() {
}
#endif
#endif

#if !defined(G_PART) || G_PART == 180
#ifdef MHG_PS2
/* G only */
void lobby_func_005E27D0() {
}
#endif
#endif

#if !defined(G_PART) || G_PART == 173
#ifdef MHG_PS2
/* G only */
void lobby_func_005BCEC0() {
    _sub_InOutRoomMember(2);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 174
#ifdef MHG_PS2
/* G only */
void lobby_func_005BCED0() {
    _sub_InOutRoomMember(3);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 176
#ifdef MHG_PS2
/* G only */
void lobby_func_005BF880(void *a) {
    GetRecvData16(a, recv_work);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 182
#ifdef MHG_PS2
/* G only */
void lobby_func_005BAED0(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    a[0x22] = 0;
    res.val = 0;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 183
#ifdef MHG_PS2
/* G only */
void lobby_func_005BAF10(u8 *a) {
    R7 res;

    a[0x20] = 0;
    a[0x21] = 0;
    a[0x22] = 0;
    res.val = -1;
    (*(void (**)())(a + 4))(res, &res);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 189
#ifdef MHG_PS2
/* G only */
void lobby_func_005BDDC0() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Byte((u8 *)&CnetSys_w + 0xA3BF);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 190
#ifdef MHG_PS2
/* G only */
void lobby_func_005BDEF0() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Byte((u8 *)&CnetSys_w + 0xA3BC);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 191
#ifdef MHG_PS2
/* G only */
void lobby_func_005BF6B0() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Byte((u8 *)&CnetSys_w + 0xA3BE);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 184
#ifdef MHG_PS2
/* G only */
void lobby_func_005BC200() {
    if (CnetSys_w.rres == 0) {
        __cnet_SetEntryFloorInfo(2);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 186
#ifdef MHG_PS2
/* G only */
void lobby_func_005BC9A0(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xE88;

    if (res.val == 0) {
        b[0x23] = 1;
    } else {
        b[0x23] = 2;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 188
#ifdef MHG_PS2
/* G only */
void lobby_func_005BDC20(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xE60;

    if (res.val == 0) {
        b[0x23] = 1;
    } else {
        b[0x23] = 2;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 185
#ifdef MHG_PS2
/* G only */
void lobby_func_005BC960(u8 *a) {
    if (a[0x23] == 1) {
        a[0x21] = 10;
        a[0x22] = 0;
    } else if (a[0x23] == 2) {
        a[0x21] = 11;
        a[0x22] = 0;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 187
#ifdef MHG_PS2
/* G only */
void lobby_func_005BD280(u8 *a) {
    a[0x22]++;
    *(int *)(a + 8) = 1;
    *(int *)(a + 12) = 1;
    a[0x23] = 0;
    *(s16 *)((u8 *)&CnetSys_w + 0x1032) = 0;
    memset((u8 *)&CnetSys_w + 0xA3BC, 0, 0x294A4);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 192
#ifdef MHG_PS2
/* G only */
void lobby_func_005BF840() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ServerMessage();
    }
    _cnet_Return_CallBack(10);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 204
#ifdef MHG_PS2
/* G only */
void lobby_func_005E3B60() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_ServerMessage();
        _cnet_Return_CallBack(8);
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 193
#ifdef MHG_PS2
/* G only */
void lobby_func_005BF8D0(void *a, void *b) {
    GetRecvData8(b, GetRecvData16(a, recv_work));
}
#endif
#endif

#if !defined(G_PART) || G_PART == 194
#ifdef MHG_PS2
/* G only */
void lobby_func_005BF910(void *a, void *b) {
    GetRecvDataString(b, GetRecvData16(a, recv_work));
}
#endif
#endif

#if !defined(G_PART) || G_PART == 205
#ifdef MHG_PS2
/* G only */
int lobby_func_005E3BA0(void *a, void *b) {
    return GetRecvDataString(b, GetRecvData16(a, recv_work));
}
#endif
#endif

#if !defined(G_PART) || G_PART == 206
#ifdef MHG_PS2
/* G only */
void lobby_func_005E3BE0(void *a, void *b) {
    GetRecvDataString(b, GetRecvData16(a, recv_work));
}
#endif
#endif

#if !defined(G_PART) || G_PART == 195
#ifdef MHG_PS2
/* G only */
void lobby_func_005BFF80(u8 *a) {
    if (*(int *)0x731168 != 0) {
        a[0x22]++;
        a[0x23] = 0;
        *(int *)(a + 12) = 0;
    } else {
        a[0x21] = 2;
        a[0x22] = 0;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 196
#ifdef MHG_PS2
/* G only */
void lobby_func_005C0870() {
    if (CnetSys_w.rres == 0) {
        GetRecvDataString(*(int *)0x734DF8, recv_work);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 197
#ifdef MHG_PS2
/* G only */
int lobby_func_005C14D0(s8 *a, u8 n) {
    int c = 0;
    int i;

    for (i = 0; i < n; i++, a += 24) {
        if (*a == 0) {
            break;
        }
        c++;
    }
    return c;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 198
#ifdef MHG_PS2
/* G only */
void To_RetVal();
extern u8 MMBB_LOGIN;
extern s8 bsMainRetVal;
void lobby_func_005C90B0() {
    To_RetVal(1);
    if (MMBB_LOGIN == 3) {
        bsMainRetVal = -100;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 199
#ifdef MHG_PS2
/* G only */
void lobby_func_005E3300(CNET_RES res) {
    u8 *b = (u8 *)&CnetSys_w + 0xDC0;

    if (res.val == 0) {
        b[0x23] = 1;
    } else {
        b[0x23] = 2;
    }
}
#endif
#endif

#if !defined(G_PART) || G_PART == 200
#ifdef MHG_PS2
/* G only */
void lobby_func_005E3440() {
    if (CnetSys_w.rres == 0) {
        __cnet_Recv_Word((u8 *)&CnetSys_w + 0x43B4);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 201
#ifdef MHG_PS2
/* G only */
int lobby_func_005E3900(u16 i, char *dst) {
    strcpy(dst, (u8 *)&CnetSys_g + (i - 1) * 356 + 17618);
    return 0;
}
#endif
#endif

#if !defined(G_PART) || G_PART == 202
#ifdef MHG_PS2
/* G only */
void lobby_func_005E3A70() {
    if (CnetSys_w.rres == 0) {
        __cnet_SetEntryFloorInfo(0);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 203
#ifdef MHG_PS2
/* G only */
void lobby_func_005E3B20() {
    if (CnetSys_w.rres == 0) {
        __cnet_ClearEntryFloorInfo(0);
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 207
#ifdef MHG_PS2
/* G only */
void lobby_func_005E6B20();
void lobby_func_005E6A70() {
    if (CnetSys_w.rres == 0) {
        lobby_func_005E6B20();
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 209
#ifdef MHG_PS2
/* G only */
void lobby_func_005E2920();
void lobby_func_005E6DF0() {
    if (CnetSys_w.rres == 0) {
        lobby_func_005E2920();
    }
    _cnet_Return_CallBack(0);
}
#endif
#endif

#if !defined(G_PART) || G_PART == 208
#ifdef MHG_PS2
/* G only */
typedef struct { s16 a[200]; } G_S16A;
extern G_S16A D_00733CAA;
void lobby_func_005E6D20(G_S16A *d) {
    *d = D_00733CAA;
}
#endif
#endif
