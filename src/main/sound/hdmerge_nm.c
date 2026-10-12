/* NEAR-MATCH (not linked): HdMerge, same structure, ~250 of 1070 instructions differ (register allocation; per-section variables and declhill2 brought it from 532 to 494 align lines). */
/* hdmerge - HdMerge (SLPM_654.95 main 0x0021E310-0x0021F3C8): merges n sound bank headers (Sony .hd files) into one at dst.
 * Each .hd is a Vers chunk, a Head chunk (offsets of the other chunks, -1 = none) and the Vagi (wave info), Smpl (samples),
 * Sset (sample sets) and Prog (programs) chunks; every chunk is a 0x10 byte header (max = highest index) followed by an
 * offset table and the records. The merged bank renumbers what refers to the previous banks: wave offsets (+ body sizes),
 * sample -> wave index, set -> sample index, split -> set index. Programs keep their numbers (a program number used by two
 * banks returns -4). Every chunk is padded with 0xFF to 16 bytes. Returns the merged header size.
 * Layout from the Sony SDK's .hd format (sdhd); field names are guesses. */
#include "types.h"

typedef struct HDVERS {
    s32 creator;
    s32 type;
    u32 size;
    u16 reserved;
    u8 major;
    u8 minor;
} HDVERS;

typedef struct HDHEAD {
    s32 creator;
    s32 type;
    u32 size;
    u32 header_size;            /* 0x0C */
    u32 body_size;              /* 0x10 */
    s32 prog;                   /* 0x14 */
    s32 sset;                   /* 0x18 */
    s32 smpl;                   /* 0x1C */
    s32 vagi;                   /* 0x20 */
    s32 setb;                   /* 0x24 */
} HDHEAD;

typedef struct HDCHUNK {
    s32 creator;
    s32 type;
    u32 size;
    u32 max;
} HDCHUNK;

typedef struct HDLIST {
    s32 creator;
    s32 type;
    u32 size;
    u32 max;                    /* 0x0C highest index */
    s32 offset[1];              /* 0x10 [max + 1], -1 = unused */
} HDLIST;

typedef struct HDVAGI {
    u32 offset;
    u16 rate;
    u8 loop;
    u8 reserved;
} HDVAGI;

typedef struct HDSMPL {
    u16 vag;
    u8 vel_low;
    u8 vel_fade;
    u8 vel_high;
    s8 vel_pitch;
    u8 vel_pitch_center;
    u8 vel_pitch_curve;
    s8 vel_amp;
    u8 vel_amp_center;
    u8 vel_amp_curve;
    u8 base_note;
    s8 detune;
    s8 panpot;
    u8 group;
    u8 priority;
    u8 volume;
    u8 reserved;
    u16 adsr1;
    u16 adsr2;
    s8 key_ar;
    u8 key_ar_center;
    s8 key_dr;
    u8 key_dr_center;
    s8 key_sr;
    u8 key_sr_center;
    s8 key_rr;
    u8 key_rr_center;
    s8 key_sl;
    u8 key_sl_center;
    u16 pitch_lfo_delay;
    u16 pitch_lfo_fade;
    u16 amp_lfo_delay;
    u16 amp_lfo_fade;
    u8 pitch_lfo_depth;
    u8 amp_lfo_depth;
} HDSMPL;

typedef struct HDSSET {
    u8 vel_curve;
    u8 vel_low;
    u8 vel_high;
    u8 n;
    u16 smpl[1];
} HDSSET;

typedef struct HDPROG {
    u32 split;
    u8 n;
    u8 split_size;
    u8 volume;
    s8 panpot;
    s8 transpose;
    s8 detune;
    s8 key_pan;
    u8 key_pan_center;
    u8 attr;
    u8 reserved;
    u8 lfo_wave;
    u8 lfo_wave2;
    u8 lfo_phase;
    u8 lfo_phase2;
    u8 lfo_random;
    u8 lfo_random2;
    u16 lfo_freq;
    u16 lfo_freq2;
    s16 lfo_pitch;
    s16 lfo_pitch2;
    s16 lfo_amp;
    s16 lfo_amp2;
    s8 x20;
    s8 x21;
    s8 x22;
    s8 x23;
} HDPROG;

typedef struct HDSPLIT {
    u16 sset;
    u8 range_low;
    u8 fade;
    u8 range_high;
    u8 number;
    u16 bend_low;
    u16 bend_high;
    s8 key_pitch;
    u8 key_pitch_center;
    s8 key_amp;
    u8 key_amp_center;
    s8 key_pan;
    u8 key_pan_center;
    u8 volume;
    s8 panpot;
    s8 transpose;
    s8 detune;
} HDSPLIT;

typedef struct HDINFO {
    u8 *top;                    /* 0x00 */
    HDVERS *vers;               /* 0x04 */
    HDHEAD *head;               /* 0x08 */
    s32 setb;                   /* 0x0C */
    HDLIST *prog;               /* 0x10 */
    HDLIST *sset;               /* 0x14 */
    HDLIST *smpl;               /* 0x18 */
    HDLIST *vagi;               /* 0x1C */
} HDINFO;

extern HDINFO DstHd;
extern HDINFO SrcHd[16];

void flMemset(void *, int, int);

int HdMerge(u32 n, u8 **src, u8 *dst) {
    u16 vcnt[16];
    u16 scnt[16];
    u16 sscnt[16];
    u32 i;
    u32 j;
    u32 j_3;
    u32 pad;
    u32 pad_1;
    u32 k;
    u32 k_2;
    s32 *tbl;
    s32 *tbl_3;
    u8 *q0;
    u8 *q1;
    u8 *q2;
    u8 *q3;
    u8 *q4;
    u32 body;
    u16 base0;
    u16 base1;
    u16 base2;
    u16 base3;
    u16 base4;

    flMemset(vcnt, 0, 0x20);
    flMemset(scnt, 0, 0x20);
    flMemset(sscnt, 0, 0x20);
    flMemset(&DstHd, 0, 0x20);
    flMemset(SrcHd, 0, 0x200);
    for (i = 0; i < n; i++) {
        SrcHd[i].top = src[i];
        SrcHd[i].vers = (HDVERS *)SrcHd[i].top;
        SrcHd[i].head = (HDHEAD *)(SrcHd[i].top + 0x10);
        SrcHd[i].setb = (SrcHd[i].head->setb == -1) ? -1 : (s32)(SrcHd[i].top + SrcHd[i].head->setb);
        SrcHd[i].prog = (HDLIST *)((SrcHd[i].head->prog == -1) ? -1 : (s32)(SrcHd[i].top + SrcHd[i].head->prog));
        SrcHd[i].sset = (HDLIST *)((SrcHd[i].head->sset == -1) ? -1 : (s32)(SrcHd[i].top + SrcHd[i].head->sset));
        SrcHd[i].smpl = (HDLIST *)((SrcHd[i].head->smpl == -1) ? -1 : (s32)(SrcHd[i].top + SrcHd[i].head->smpl));
        SrcHd[i].vagi = (HDLIST *)((SrcHd[i].head->vagi == -1) ? -1 : (s32)(SrcHd[i].top + SrcHd[i].head->vagi));
    }

    DstHd.top = dst;
    DstHd.vers = (HDVERS *)DstHd.top;
    {
        HDVERS *d = DstHd.vers;
        HDVERS *s = SrcHd[0].vers;

        d->creator = s->creator;
        d->type = s->type;
        d->size = s->size;
        d->reserved = s->reserved;
        d->major = s->major;
        d->minor = s->minor;
    }
    DstHd.vers->size = 0x10;
    DstHd.head = (HDHEAD *)(DstHd.top + 0x10);
    {
        HDHEAD *d = DstHd.head;
        HDHEAD *s = SrcHd[0].head;

        d->creator = s->creator;
        d->type = s->type;
        d->size = s->size;
        d->header_size = s->header_size;
        d->body_size = s->body_size;
        d->prog = s->prog;
        d->sset = s->sset;
        d->smpl = s->smpl;
        d->vagi = s->vagi;
        d->setb = s->setb;
    }
    DstHd.head->header_size = -1;
    DstHd.head->body_size = -1;
    DstHd.head->prog = -1;
    DstHd.head->sset = -1;
    DstHd.head->smpl = -1;
    DstHd.head->vagi = -1;
    DstHd.head->setb = -1;
    q0 = (u8 *)DstHd.head + 0x28;
    for (i = 0; i < 0x18; i++) {
        *q0++ = 0xFF;
    }

    /* wave info */
    DstHd.vagi = (HDLIST *)q0;
    {
        HDLIST *d;
        HDLIST *s;

        s = SrcHd[0].vagi;
        d = DstHd.vagi;

        d->creator = s->creator;
        d->type = s->type;
        d->size = s->size;
        d->max = s->max;
    }
    DstHd.vagi->size = -1;
    DstHd.vagi->max = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].vagi != -1) {
            vcnt[i] = SrcHd[i].vagi->max + 1;
            DstHd.vagi->max += vcnt[i];
        }
    }
    DstHd.vagi->max--;
    tbl = DstHd.vagi->offset;
    q1 = (u8 *)(tbl + (DstHd.vagi->max + 1));
    body = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].vagi != -1) {
            for (j = 0; j < vcnt[i]; j++) {
                s32 o = SrcHd[i].vagi->offset[j];
                if (o != -1) {
                    HDVAGI *v = (HDVAGI *)(o + (s32)SrcHd[i].vagi);
                    *tbl++ = q1 - (u8 *)DstHd.vagi;
                    ((HDVAGI *)q1)->offset = v->offset;
                    ((HDVAGI *)q1)->rate = v->rate;
                    ((HDVAGI *)q1)->loop = v->loop;
                    ((HDVAGI *)q1)->reserved = v->reserved;
                    ((HDVAGI *)q1)->offset += body;
                    q1 += 8;
                } else {
                    *tbl++ = o;
                }
            }
            body += SrcHd[i].head->body_size;
        }
    }
    pad_1 = 0x10 - ((q1 - (u8 *)DstHd.vagi) & 0xF);
    if (pad_1 != 0x10) {
        for (k = 0; k < pad_1; k++) {
            *q1++ = 0xFF;
        }
    }
    DstHd.vagi->size = q1 - (u8 *)DstHd.vagi;

    /* samples */
    DstHd.smpl = (HDLIST *)q1;
    {
        HDLIST *d;
        HDLIST *s;

        s = SrcHd[0].smpl;
        d = DstHd.smpl;

        d->creator = s->creator;
        d->type = s->type;
        d->size = s->size;
        d->max = s->max;
    }
    DstHd.smpl->size = -1;
    DstHd.smpl->max = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].smpl != -1) {
            scnt[i] = SrcHd[i].smpl->max + 1;
            DstHd.smpl->max += scnt[i];
        }
    }
    DstHd.smpl->max--;
    tbl = DstHd.smpl->offset;
    q2 = (u8 *)(tbl + (DstHd.smpl->max + 1));
    base2 = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].smpl != -1) {
            for (j = 0; j < scnt[i]; j++) {
                s32 o = SrcHd[i].smpl->offset[j];
                if (o != -1) {
                    HDSMPL *v = (HDSMPL *)(o + (s32)SrcHd[i].smpl);
                    *tbl++ = q2 - (u8 *)DstHd.smpl;
                    ((HDSMPL *)q2)->vag = v->vag;
                    ((HDSMPL *)q2)->vel_low = v->vel_low;
                    ((HDSMPL *)q2)->vel_fade = v->vel_fade;
                    ((HDSMPL *)q2)->vel_high = v->vel_high;
                    ((HDSMPL *)q2)->vel_pitch = v->vel_pitch;
                    ((HDSMPL *)q2)->vel_pitch_center = v->vel_pitch_center;
                    ((HDSMPL *)q2)->vel_pitch_curve = v->vel_pitch_curve;
                    ((HDSMPL *)q2)->vel_amp = v->vel_amp;
                    ((HDSMPL *)q2)->vel_amp_center = v->vel_amp_center;
                    ((HDSMPL *)q2)->vel_amp_curve = v->vel_amp_curve;
                    ((HDSMPL *)q2)->base_note = v->base_note;
                    ((HDSMPL *)q2)->detune = v->detune;
                    ((HDSMPL *)q2)->panpot = v->panpot;
                    ((HDSMPL *)q2)->group = v->group;
                    ((HDSMPL *)q2)->priority = v->priority;
                    ((HDSMPL *)q2)->volume = v->volume;
                    ((HDSMPL *)q2)->reserved = v->reserved;
                    ((HDSMPL *)q2)->adsr1 = v->adsr1;
                    ((HDSMPL *)q2)->adsr2 = v->adsr2;
                    ((HDSMPL *)q2)->key_ar = v->key_ar;
                    ((HDSMPL *)q2)->key_ar_center = v->key_ar_center;
                    ((HDSMPL *)q2)->key_dr = v->key_dr;
                    ((HDSMPL *)q2)->key_dr_center = v->key_dr_center;
                    ((HDSMPL *)q2)->key_sr = v->key_sr;
                    ((HDSMPL *)q2)->key_sr_center = v->key_sr_center;
                    ((HDSMPL *)q2)->key_rr = v->key_rr;
                    ((HDSMPL *)q2)->key_rr_center = v->key_rr_center;
                    ((HDSMPL *)q2)->key_sl = v->key_sl;
                    ((HDSMPL *)q2)->key_sl_center = v->key_sl_center;
                    ((HDSMPL *)q2)->pitch_lfo_delay = v->pitch_lfo_delay;
                    ((HDSMPL *)q2)->pitch_lfo_fade = v->pitch_lfo_fade;
                    ((HDSMPL *)q2)->amp_lfo_delay = v->amp_lfo_delay;
                    ((HDSMPL *)q2)->amp_lfo_fade = v->amp_lfo_fade;
                    ((HDSMPL *)q2)->pitch_lfo_depth = v->pitch_lfo_depth;
                    ((HDSMPL *)q2)->amp_lfo_depth = v->amp_lfo_depth;
                    ((HDSMPL *)q2)->vag += base2;
                    q2 += 0x2A;
                } else {
                    *tbl++ = o;
                }
            }
            base2 += vcnt[i];
        }
    }
    pad = 0x10 - ((q2 - (u8 *)DstHd.smpl) & 0xF);
    if (pad != 0x10) {
        for (k_2 = 0; k_2 < pad; k_2++) {
            *q2++ = 0xFF;
        }
    }
    DstHd.smpl->size = q2 - (u8 *)DstHd.smpl;

    /* sample sets */
    DstHd.sset = (HDLIST *)q2;
    {
        HDLIST *d;
        HDLIST *s;

        s = SrcHd[0].sset;
        d = DstHd.sset;

        d->creator = s->creator;
        d->type = s->type;
        d->size = s->size;
        d->max = s->max;
    }
    DstHd.sset->size = -1;
    DstHd.sset->max = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].sset != -1) {
            sscnt[i] = SrcHd[i].sset->max + 1;
            DstHd.sset->max += sscnt[i];
        }
    }
    DstHd.sset->max--;
    tbl_3 = DstHd.sset->offset;
    q3 = (u8 *)(tbl_3 + (DstHd.sset->max + 1));
    base3 = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].sset != -1) {
            for (j_3 = 0; j_3 < sscnt[i]; j_3++) {
                s32 o = SrcHd[i].sset->offset[j_3];
                if (o != -1) {
                    HDSSET *v = (HDSSET *)(o + (s32)SrcHd[i].sset);
                    u16 *a = v->smpl;
                    *tbl_3 = q3 - (u8 *)DstHd.sset;
                    ((HDSSET *)q3)->vel_curve = v->vel_curve;
                    ((HDSSET *)q3)->vel_low = v->vel_low;
                    ((HDSSET *)q3)->vel_high = v->vel_high;
                    ((HDSSET *)q3)->n = v->n;
                    q3 += 4;
                    for (k = 0; k < v->n; k++) {
                        *(u16 *)q3 = *a++ + base3;
                        q3 += 2;
                    }
                    tbl_3++;
                } else {
                    *tbl_3++ = o;
                }
            }
            base3 += scnt[i];
        }
    }
    pad = 0x10 - ((q3 - (u8 *)DstHd.sset) & 0xF);
    if (pad != 0x10) {
        for (k = 0; k < pad; k++) {
            *q3++ = 0xFF;
        }
    }
    DstHd.sset->size = q3 - (u8 *)DstHd.sset;

    /* programs */
    DstHd.prog = (HDLIST *)q3;
    {
        HDLIST *d;
        HDLIST *s;

        s = SrcHd[0].prog;
        d = DstHd.prog;

        d->creator = s->creator;
        d->type = s->type;
        d->size = s->size;
        d->max = s->max;
    }
    DstHd.prog->size = -1;
    DstHd.prog->max = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].prog != -1) {
            if (DstHd.prog->max < SrcHd[i].prog->max) {
                DstHd.prog->max = SrcHd[i].prog->max;
            }
        }
    }
    for (i = 0; i <= DstHd.prog->max; i++) {
        DstHd.prog->offset[i] = -1;
    }
    q4 = (u8 *)DstHd.prog->offset;
    q4 += (DstHd.prog->max + 1) * 4;
    base4 = 0;
    for (i = 0; i < n; i++) {
        if ((s32)SrcHd[i].prog != -1) {
            for (j = 0; j <= SrcHd[i].prog->max; j++) {
                s32 *so = &SrcHd[i].prog->offset[j];
                s32 *d = &DstHd.prog->offset[j];
                if (*so != -1) {
                    HDPROG *p;
                    HDSPLIT *sp;
                    if (*d != -1) {
                        return -4;
                    }
                    *d = q4 - (u8 *)DstHd.prog;
                    p = (HDPROG *)(*so + (s32)SrcHd[i].prog);
                    sp = (HDSPLIT *)(p + 1);
                    ((HDPROG *)q4)->split = p->split;
                    ((HDPROG *)q4)->n = p->n;
                    ((HDPROG *)q4)->split_size = p->split_size;
                    ((HDPROG *)q4)->volume = p->volume;
                    ((HDPROG *)q4)->panpot = p->panpot;
                    ((HDPROG *)q4)->transpose = p->transpose;
                    ((HDPROG *)q4)->detune = p->detune;
                    ((HDPROG *)q4)->key_pan = p->key_pan;
                    ((HDPROG *)q4)->key_pan_center = p->key_pan_center;
                    ((HDPROG *)q4)->attr = p->attr;
                    ((HDPROG *)q4)->reserved = p->reserved;
                    ((HDPROG *)q4)->lfo_wave = p->lfo_wave;
                    ((HDPROG *)q4)->lfo_wave2 = p->lfo_wave2;
                    ((HDPROG *)q4)->lfo_phase = p->lfo_phase;
                    ((HDPROG *)q4)->lfo_phase2 = p->lfo_phase2;
                    ((HDPROG *)q4)->lfo_random = p->lfo_random;
                    ((HDPROG *)q4)->lfo_random2 = p->lfo_random2;
                    ((HDPROG *)q4)->lfo_freq = p->lfo_freq;
                    ((HDPROG *)q4)->lfo_freq2 = p->lfo_freq2;
                    ((HDPROG *)q4)->lfo_pitch = p->lfo_pitch;
                    ((HDPROG *)q4)->lfo_pitch2 = p->lfo_pitch2;
                    ((HDPROG *)q4)->lfo_amp = p->lfo_amp;
                    ((HDPROG *)q4)->lfo_amp2 = p->lfo_amp2;
                    ((HDPROG *)q4)->x20 = p->x20;
                    ((HDPROG *)q4)->x21 = p->x21;
                    ((HDPROG *)q4)->x22 = p->x22;
                    ((HDPROG *)q4)->x23 = p->x23;
                    q4 += 0x24;
                    for (k = 0; k < p->n; k++) {
                        ((HDSPLIT *)q4)->sset = sp->sset;
                        ((HDSPLIT *)q4)->range_low = sp->range_low;
                        ((HDSPLIT *)q4)->fade = sp->fade;
                        ((HDSPLIT *)q4)->range_high = sp->range_high;
                        ((HDSPLIT *)q4)->number = sp->number;
                        ((HDSPLIT *)q4)->bend_low = sp->bend_low;
                        ((HDSPLIT *)q4)->bend_high = sp->bend_high;
                        ((HDSPLIT *)q4)->key_pitch = sp->key_pitch;
                        ((HDSPLIT *)q4)->key_pitch_center = sp->key_pitch_center;
                        ((HDSPLIT *)q4)->key_amp = sp->key_amp;
                        ((HDSPLIT *)q4)->key_amp_center = sp->key_amp_center;
                        ((HDSPLIT *)q4)->key_pan = sp->key_pan;
                        ((HDSPLIT *)q4)->key_pan_center = sp->key_pan_center;
                        ((HDSPLIT *)q4)->volume = sp->volume;
                        ((HDSPLIT *)q4)->panpot = sp->panpot;
                        ((HDSPLIT *)q4)->transpose = sp->transpose;
                        ((HDSPLIT *)q4)->detune = sp->detune;
                        sp++;
                        ((HDSPLIT *)q4)->sset += base4;
                        q4 += 0x14;
                    }
                }
            }
            base4 += sscnt[i];
        }
    }
    pad = 0x10 - ((q4 - (u8 *)DstHd.prog) & 0xF);
    if (pad != 0x10) {
        for (k = 0; k < pad; k++) {
            *q4++ = 0xFF;
        }
    }
    DstHd.prog->size = q4 - (u8 *)DstHd.prog;

    DstHd.head->header_size = q4 - DstHd.top;
    DstHd.head->body_size = body;
    DstHd.head->prog = (u8 *)DstHd.prog - DstHd.top;
    DstHd.head->sset = (u8 *)DstHd.sset - DstHd.top;
    DstHd.head->smpl = (u8 *)DstHd.smpl - DstHd.top;
    DstHd.head->vagi = (u8 *)DstHd.vagi - DstHd.top;
    DstHd.head->setb = -1;
    return DstHd.head->header_size;
}
