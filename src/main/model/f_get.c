/* f_get - model work (SLPM_654.95 main 0x00121B70-0x001246D4), the whole original source file as one translation unit:
 * model_work_init, render attribute decoding (Attribute_from_amo, clay_attr_set/reset), the model builders (Sethierarchy, mkModel,
 * mkModel3 (player), mkModel4 (monster, also fills the per-slot matrix lists), mkMaterial), model work set/free per kind, the slot heaps
 * (one used-byte per slot: material 0x400, hierarchy 0x800, clay 0x180, mdlw 0x80) and the player model creation (pl_create_model,
 * weapon_create_model, Pl_model_id_set, armor_create_model).
 * Replaces the runs gm01..gm05, mkm01, hp01..hp04, light03 (get_mdlw_ptr), clay/get_clay_ptr and crmdl03 (Pl_model_id_set).
 * Statics are the LOCAL symbols of the original. Field names are guesses. */
#include "g_alias.h"
#ifdef MHG_PS2 /* G calls another function here (tools/check.py VERSION=mhg) */
#define load_texlist func_0010EF50
#define load_texlist_pl func_0010F0B0
#endif
#include "types.h"
#include "mdlw.h"

typedef struct CLAY {
    s32 handle;                 /* 0x00 clay handle, -1 = none */
    s32 n;                      /* 0x04 materials used by this part */
    s32 list[32];               /* 0x08 material numbers */
    u32 attr;                   /* 0x88 packed render attributes (Attribute_from_amo) */
} CLAY;                         /* 0x8C */

extern u8 *mdlw_heap_area;
extern int aa_material[2], aa_fog[2], aa_scissor[2], aa_fadecol[2], aa_uvscroll[2], aa_filt[2];
extern int aa_light[9], aa_specular[4], aa_cull[4], aa_alpha_src[10], aa_alpha_ope[3], aa_addr[3];
extern s32 fog_type[2];
extern s32 shader_type[];
extern s32 clay_type[];
extern s16 mat_list[][16][33];
extern s32 mem_tex[];
extern s32 data_load_ptr;
extern char lit_219_003583E0[], lit_220_00358400[], lit_221_00358420[], lit_222_00358440[];
extern char lit_451_00358460[];

extern u8 material_heap[0x400];
extern u8 hierarchy_heap[0x800];
extern u8 clay_heap[0x180];
extern u8 mdlw_heap[0x80];
extern int material_heap_area;     /* gp-relative, holds an address */
extern int hierarchy_heap_area;
extern int clay_heap_area;

void *memset(void *, int, unsigned);
void flSetRenderState(int, int);
void SetTrnslMode(int, int);
void SetOpeMode(int);
void SetFilterMode(int);
void flReleaseClayHandle(int);
void flReleaseInitMotionSetHandle(int);
void release_texture(s16, s16);
int load_texlist(int, int, int);
void load_texlist_pl(int *, s16, int, u16);
void system_error(char *, s16, s16, int);
void flGetFrame(void *);
void flReleaseFrame(void *);
int flCreateClayHandle(void *, int);
int plAMOGetModelNum(int);
int plAMOGetModelAttribute(void *, int, int);
int plAMOGetMaterialListNum(int, int);
int plAMOGetMaterialList(int, int, void *);
int plAMOCreateClayFromImage(void *, int, int);
int plAMOGetModelMatrixlist(void *, int, int);
s32 plAMOGetMaterialNum(int);
void plAMOSetMaterialData(int, int, void *);
int plAHIGetModelNum(int);
int plAHIGetTreeNum(int);
int plAHIGetTreeModelNum(int, int);
void plCreateInitMotionSetFromAHI(void *, int, int, int);
int flCreateInitMotionSetHandle(void *);
void flGetHierarchySI(void *, int);

static void clr_used_material(int, int);
static void clr_used_hierarchy(int, int);
static void clr_used_clay(int, int);
void clr_used_mdlw(int, int);
static int get_start_material(int);
static u8 *get_material_ptr(int);
static void set_used_material(int, int);
int get_start_hierarchy(int);
u8 *get_hierarchy_ptr(int);
void set_used_hierarchy(int, int);
static int get_start_clay(int);
static CLAY *get_clay_ptr(int);
static void set_used_clay(int, int);
int get_start_mdlw(int);
void set_used_mdlw(int, int);
void model_work_free2(int idx);

void model_work_init(void) {
    memset(mdlw_heap_area, 0, 0x4000);
    clr_used_material(0, 0x400);
    clr_used_hierarchy(0, 0x800);
    clr_used_clay(0, 0x180);
    clr_used_mdlw(0, 0x80);
}

/* Decodes a material attribute block (AMO) into render states and a packed word at *out:
 * bit 0 set, 1 fade, 2-5 alpha source, 6-9 alpha destination, 10-11 op, 12 filter, 13-14 address. */
static void Attribute_from_amo(int *a, u32 *out) {
    flSetRenderState(0x15, aa_material[a[1]]);
    flSetRenderState(0x12, aa_fog[a[8]]);
    flSetRenderState(1, aa_light[a[5]]);
    flSetRenderState(2, aa_specular[a[2]]);
    flSetRenderState(0xC, aa_scissor[a[4]]);
    flSetRenderState(0x66, aa_fadecol[a[9]]);
    flSetRenderState(0x62, aa_uvscroll[a[7]]);
    flSetRenderState(0, aa_cull[a[3]]);
    if (aa_fadecol[a[9]] == 1) {
        flSetRenderState(0x67, -1);
    }
    switch (a[10]) {
    case 6:
        flSetRenderState(0x5D, 0x6000000);
        break;
    case 5:
        flSetRenderState(0x5D, 0x5000000);
        break;
    case 0:
    default:
        flSetRenderState(0x5D, 0);
        break;
    }
    *out = 1;
    *out |= (a[9] & 1) << 1;
    *out |= (a[11] & 0xF) << 2;
    *out |= (a[12] & 0xF) << 6;
    *out |= (a[13] & 3) << 10;
    *out |= (a[16] & 1) << 12;
    *out |= (a[17] & 3) << 13;
}

void clay_attr_set(u32 attr) {
    if (attr & 1) {
        SetTrnslMode(aa_alpha_src[(attr >> 2) & 0xF], aa_alpha_src[(attr >> 6) & 0xF]);
        SetOpeMode(aa_alpha_ope[(attr >> 10) & 3]);
        SetFilterMode(aa_filt[(attr >> 12) & 1]);
        flSetRenderState(0x64, aa_addr[(attr >> 13) & 3]);
    }
}

void clay_attr_reset(void) {
    SetTrnslMode(4, 5);
    SetOpeMode(0);
    SetFilterMode(1);
}

/* Allocates the bone hierarchy for a model (type 0 none, 1 one set, 2 two sets for the two halves) and builds one
   skeleton per tree of the AHI file at amo + 0x128000. */
static void Sethierarchy(int amo, MDLW *w, int type) {
    int h;
    int n;
    u8 *b0;
    u8 *b1;
    int k;
    MDLW *p;
    u8 *q;
    int i;
    u8 tmp[16];

    amo = (int)((u8 *)amo + 0x128000);
    w->type = type;
    switch (type) {
    case 0:
        w->hier_no = 0;
        w->hier_n = 0;
        w->hier0 = 0;
        w->hier1 = 0;
        w->num = 0;
        return;
    case 1:
        n = plAHIGetModelNum(amo);
        break;
    case 2:
        n = plAHIGetModelNum(amo) * 2;
        break;
    }
    h = get_start_hierarchy(n);
    if (h >= 0) {
        w->hier_no = h;
        w->hier_n = n;
        w->hier0 = get_hierarchy_ptr(h);
        if (type == 2) {
            w->hier1 = get_hierarchy_ptr(h + n / 2);
        } else {
            w->hier1 = 0;
        }
        set_used_hierarchy(h, n);
        w->num = plAHIGetTreeNum(amo);
        k = 0;
        b0 = w->hier0;
        b1 = w->hier1;
        i = 0;
        if (0 < w->num) {
            p = w;
            do {
                plCreateInitMotionSetFromAHI(tmp, data_load_ptr, amo, i);
                p->si[0] = flCreateInitMotionSetHandle(tmp);
                q = b0 + k * 0x190;
                flGetHierarchySI(q, p->si[0]);
                p->a[0] = q;
                if (type == 2) {
                    q = b1 + k * 0x190;
                    flGetHierarchySI(q, p->si[0]);
                    p->b[0] = q;
                }
                k += plAHIGetTreeModelNum(amo, i);
                i++;
                p = (MDLW *)((u8 *)p + 4);
            } while (i < w->num);
        }
    }
}

/* Builds the clay (mesh) entries of a model: per part the render state from its AMO attributes (or defaults chosen by the
   flag bits 0x200..0x2000), its material list and the clay handle. flags 0x100 picks the clay type. */
static void mkModel(w, flags, type)
MDLW *w;
u16 flags;
int type;
{
    int k;
    int amo;
    int mat;
    int i;
    int j;
    CLAY *c;
    u8 img[40];
    int attr[20];
    u8 frame[8];

    k = (flags & 0x100) / 256;
    amo = w->mat_set;
    w->clay_n = plAMOGetModelNum(amo);
    if (w->clay_n == 0) {
        system_error(lit_219_003583E0, 0, 0, 0);
    }
    i = get_start_clay(w->clay_n);
    if (i >= 0) {
        w->clay_start = i;
        w->clay = (u8 *)(c = get_clay_ptr(i));
        set_used_clay(i, w->clay_n);
        mat = (int)w->mat;
        for (i = 0; i < w->clay_n; i++) {
            if (plAMOGetModelAttribute(attr, amo, i) >= 0) {
                Attribute_from_amo(attr, &c->attr);
                if (flags & 0x800) {
                    flSetRenderState(0x12, fog_type[1]);
                }
                if (flags & 0x1000) {
                    flSetRenderState(0x66, 1);
                }
                if (flags & 0x2000) {
                    flSetRenderState(0, 0x20);
                }
            } else {
                c->attr = 0;
                flSetRenderState(0x66, 0);
                flSetRenderState(0xC, 0);
                flSetRenderState(0x62, 0);
                flSetRenderState(0, 0);
                flSetRenderState(0x15, shader_type[0]);
                if (flags & 0x800) {
                    flSetRenderState(0x12, fog_type[1]);
                } else {
                    flSetRenderState(0x12, fog_type[0]);
                }
                if (flags & 0x400) {
                    flSetRenderState(2, 4);
                } else {
                    flSetRenderState(2, 0);
                }
                if (flags & 0x200) {
                    flSetRenderState(1, shader_type[5]);
                } else {
                    flSetRenderState(1, shader_type[1]);
                }
                if (flags & 0x2000) {
                    flSetRenderState(0, 0x20);
                } else {
                    flSetRenderState(0, 0);
                }
            }
            c->n = plAMOGetMaterialListNum(amo, i);
            if (c->n == 0) {
                system_error(lit_220_00358400, i, w->clay_n, 0);
            }
            if (plAMOGetMaterialList(amo, i, c->list) == 0) {
                system_error(lit_221_00358420, i, w->clay_n, 0);
            }
            for (j = 0; j < c->n; j++) {
                flSetRenderState((u8)(j + 0x3A), mat + c->list[j] * 0x4C);
            }
            flGetFrame(frame);
            if (plAMOCreateClayFromImage(img, amo, i) != 0) {
                c->handle = flCreateClayHandle(img, clay_type[k]);
            } else {
                system_error(lit_222_00358440, i, w->clay_n, 0);
                c->handle = -1;
            }
            flReleaseFrame(frame);
            c++;
        }
        Sethierarchy(amo, w, type);
        flSetRenderState(0x12, 0);
        flSetRenderState(0x66, 0);
        flSetRenderState(0xC, 0);
    }
}

/* mkModel for the hunter's models (pl_model_work_set): always fogged, no scissor/fade states. */
static void mkModel3(w, unused, flags, type)
MDLW *w;
int unused;
u16 flags;
int type;
{
    int k;
    int amo;
    int mat;
    int i;
    int j;
    CLAY *c;
    u8 img[40];
    int attr[20];
    u8 frame[8];

    k = (flags & 0x100) / 256;
    amo = w->mat_set;
    w->clay_n = plAMOGetModelNum(amo);
    if (w->clay_n == 0) {
        system_error(lit_219_003583E0, 0, 0, 0);
    }
    i = get_start_clay(w->clay_n);
    if (i >= 0) {
        w->clay_start = i;
        w->clay = (u8 *)(c = get_clay_ptr(i));
        set_used_clay(i, w->clay_n);
        mat = (int)w->mat;
        for (i = 0; i < w->clay_n; i++) {
            if (plAMOGetModelAttribute(attr, amo, i) >= 0) {
                Attribute_from_amo(attr, &c->attr);
                if (flags & 0x2000) {
                    flSetRenderState(0, 0x20);
                } else {
                    flSetRenderState(0, 0);
                }
            } else {
                c->attr = 0;
                flSetRenderState(0x66, 0);
                flSetRenderState(0x62, 0);
                flSetRenderState(0, 0);
                flSetRenderState(0x15, shader_type[0]);
                flSetRenderState(0x12, fog_type[1]);
                if (flags & 0x200) {
                    flSetRenderState(1, shader_type[5]);
                } else {
                    flSetRenderState(1, shader_type[1]);
                }
                if (flags & 0x2000) {
                    flSetRenderState(0, 0x20);
                } else {
                    flSetRenderState(0, 0);
                }
                flSetRenderState(2, 0);
            }
            c->n = plAMOGetMaterialListNum(amo, i);
            if (c->n == 0) {
                system_error(lit_220_00358400, i, w->clay_n, 0);
            }
            if (plAMOGetMaterialList(amo, i, c->list) == 0) {
                system_error(lit_221_00358420, i, w->clay_n, 0);
            }
            for (j = 0; j < c->n; j++) {
                flSetRenderState((u8)(j + 0x3A), mat + c->list[j] * 0x4C);
            }
            flGetFrame(frame);
            if (plAMOCreateClayFromImage(img, amo, i) != 0) {
                c->handle = flCreateClayHandle(img, clay_type[k]);
            } else {
                system_error(lit_222_00358440, i, w->clay_n, 0);
                c->handle = -1;
            }
            flReleaseFrame(frame);
            c++;
        }
        Sethierarchy(amo, w, type);
        flSetRenderState(2, 0);
    }
}

/* mkModel for monsters (em_model_work_set): also clears and fills the per-part matrix lists of monster slot `slot`
   (mat_list[slot][part], 0x42 bytes each; the clear loop writes 0x40 entries per part, past the 33 of a row). */
static void mkModel4(w, flags, slot, type)
MDLW *w;
u16 flags;
u8 slot;
int type;
{
    int k;
    int mat;
    int i;
    int j;
    CLAY *c;
    int amo;
    u8 img[40];
    int attr[20];
    u8 frame[8];

    k = (flags & 0x100) / 256;
    amo = w->mat_set;
    w->clay_n = plAMOGetModelNum(amo);
    if (w->clay_n == 0) {
        system_error(lit_219_003583E0, 0, 0, 0);
    }
    i = get_start_clay(w->clay_n);
    if (i >= 0) {
        w->clay_start = i;
        w->clay = (u8 *)(c = get_clay_ptr(i));
        set_used_clay(i, w->clay_n);
        mat = (int)w->mat;
        flGetFrame(frame);
        for (i = 0; i < w->clay_n; i++) {
            if (plAMOGetModelAttribute(attr, amo, i) >= 0) {
                Attribute_from_amo(attr, &c->attr);
                if (flags & 0x800) {
                    flSetRenderState(0x12, fog_type[1]);
                } else {
                    flSetRenderState(0x12, fog_type[0]);
                }
            } else {
                c->attr = 0;
                flSetRenderState(0x62, 0);
                flSetRenderState(0, 0);
                flSetRenderState(0x15, shader_type[0]);
                flSetRenderState(0x66, 0);
                if (flags & 0x800) {
                    flSetRenderState(0x12, fog_type[1]);
                } else {
                    flSetRenderState(0x12, fog_type[0]);
                }
                if (flags & 0x400) {
                    flSetRenderState(2, 4);
                }
                if (flags & 0x200) {
                    flSetRenderState(1, shader_type[5]);
                } else {
                    flSetRenderState(1, shader_type[1]);
                }
            }
            c->n = plAMOGetMaterialListNum(amo, i);
            if (c->n == 0) {
                system_error(lit_220_00358400, i, w->clay_n, 0);
            }
            if (plAMOGetMaterialList(amo, i, c->list) == 0) {
                system_error(lit_221_00358420, i, w->clay_n, 0);
            }
            for (j = 0; j < c->n; j++) {
                flSetRenderState((u8)(j + 0x3A), mat + c->list[j] * 0x4C);
            }
            if (plAMOCreateClayFromImage(img, amo, i) != 0) {
                c->handle = flCreateClayHandle(img, clay_type[k]);
                if (c->handle == 0) {
                    system_error(lit_222_00358440, i, w->clay_n, 0);
                }
            } else {
                system_error(lit_222_00358440, i, w->clay_n, 0);
                c->handle = -1;
            }
            c++;
        }
        for (i = 0; i < w->clay_n; i++) {
            for (j = 0; j < 0x40; j++) {
                mat_list[slot][i][j] = -1;
            }
        }
        for (i = 0; i < w->clay_n; i++) {
            plAMOGetModelMatrixlist(mat_list[slot][i], amo, i);
        }
        flReleaseFrame(frame);
        Sethierarchy(amo, w, type);
        if (flags & 0x400) {
            flSetRenderState(2, 0);
        }
    }
}

/* Reserves the model's material records, fills them from the AMO and relocates each texture index through mem_tex. */
static int mkMaterial(MDLW *w, int amo) {
    int h;
    int n;
    int i;
    u8 *m;

    n = (s16)plAMOGetMaterialNum(amo);
    if (n == 0) {
        system_error(lit_451_00358460, 0, 0, 0);
    }
    h = get_start_material(n);
    if (h < 0) {
        return -1;
    }
    w->mat_start = h;
    w->mat_n = n;
    w->mat = get_material_ptr(h);
    set_used_material(h, n);
    m = w->mat;
    i = 0;
    if (0 < w->mat_n) {
        do {
            plAMOSetMaterialData(amo, i, m);
            i++;
            *(s32 *)(m + 0x44) = mem_tex[w->tex_start + *(s32 *)(m + 0x44)];
            m += 0x4C;
        } while (i < w->mat_n);
    }
    return 0;
}

MDLW *model_work_set(int idx, int mat, int tex, int texfile, int flags, int type) {
    MDLW *w = get_mdlw_ptr((s16)idx);

    w->flag = 1;
    w->x75 = 0;
    w->x01 = 0;
    w->mat_set = mat;
    w->tex_start = tex;
    w->tex_n = load_texlist(texfile, (s16)tex, 0);
    if (mkMaterial(w, mat) == -1) {
        return 0;
    }
    mkModel(w, flags, type);
    return w;
}

/* Like model_work_set with a player texture list (pl[0] = count; skin picks the skin colour variant). */
MDLW *model_work_set2(int idx, int mat, s16 tex, int *pl, int flags, int skin, int type) {
    MDLW *w = get_mdlw_ptr((s16)idx);

    w->flag = 1;
    w->x75 = 0;
    w->x01 = 0;
    w->mat_set = mat;
    w->tex_start = tex;
    if (pl != 0) {
        w->tex_n = pl[0];
        load_texlist_pl(pl, (int)tex, 0, skin);
    } else {
        w->tex_n = 0;
    }
    if (mkMaterial(w, mat) == -1) {
        return 0;
    }
    mkModel(w, flags, type);
    return w;
}

MDLW *st_model_work_set(int idx, int mat, int tex, int texfile, int flags) {
    MDLW *w = get_mdlw_ptr((s16)idx);

    w->flag = 1;
    w->x75 = 0;
    w->x01 = 0;
    w->mat_set = mat;
    w->tex_start = tex;
    w->tex_n = load_texlist(texfile, (s16)tex, 0);
    if (mkMaterial(w, mat) == -1) {
        return 0;
    }
    mkModel(w, flags, 0);
    return w;
}

MDLW *pl_model_work_set(int idx, int mat, int tex, int texfile, int p1, int p2) {
    MDLW *w = get_mdlw_ptr((s16)idx);

    w->flag = 1;
    w->x75 = 0;
    w->x01 = 0;
    w->mat_set = mat;
    w->tex_start = tex;
    w->tex_n = load_texlist(texfile, (s16)tex, 0);
    if (mkMaterial(w, mat) == -1) {
        return 0;
    }
    mkModel3(w, p1, p2, 2);
    return w;
}

MDLW *em_model_work_set(int idx, int mat, int tex, int texfile, int flags, int slot) {
    MDLW *w = get_mdlw_ptr((s16)idx);

    w->flag = 1;
    w->x75 = 0;
    w->x01 = 0;
    w->mat_set = mat;
    w->tex_start = tex;
    w->tex_n = load_texlist(texfile, (s16)tex, 0);
    if (mkMaterial(w, mat) == -1) {
        return 0;
    }
    mkModel4(w, flags, slot, 2);
    return w;
}

/* Releases the clay handles of n CLAY entries. */
static void release_model(CLAY *clay, int n) {
    int i;

    for (i = 0; i < n; i++) {
        if (clay[i].handle != -1) {
            flReleaseClayHandle(clay[i].handle);
        }
    }
}

static void release_motion(MDLW *w, int *handles) {
    int i;

    for (i = 0; i < w->num; i++) {
        if (*handles != 0) {
            flReleaseInitMotionSetHandle(*handles);
        }
        handles++;
    }
}

void model_work_free(int idx) {
    MDLW *w = get_mdlw_ptr(idx);

    if (w->x75 != 0) {
        model_work_free2(idx);
    } else if (w->flag != 0) {
        clr_used_material(w->mat_start, w->mat_n);
        clr_used_hierarchy(w->hier_no, w->hier_n);
        clr_used_clay(w->clay_start, w->clay_n);
        release_model((CLAY *)w->clay, w->clay_n);
        if (w->tex_n != 0) {
            release_texture(w->tex_start, (int)w->tex_n);
        }
        release_motion(w, (int *)w->si);
        memset(w, 0, 0x80);
    }
}

void model_work_free2(int idx) {
    MDLW *w = get_mdlw_ptr(idx);

    if (w->flag != 0) {
        clr_used_hierarchy(w->hier_no, w->hier_n);
        memset(w, 0, 0x80);
    }
}

void all_model_free(void) {
    int i;

    for (i = 0; i < 0x80; i++) {
        model_work_free(i);
    }
}

void all_initmotion_free(void) {
    /* empty in the retail build */
}

/* First fit of n free slots (0 = free) in a heap of SIZE used-bytes; -1 when full. */
#define GET_START(NAME, HEAP, SIZE)                                   \
int get_start_##NAME(int n) {                                         \
    int pos = 0;                                                      \
    int sum;                                                          \
    int j;                                                            \
    u8 *p;                                                            \
                                                                      \
    if (n == 0) {                                                     \
        return 0;                                                     \
    }                                                                 \
    for (;;) {                                                        \
        if (pos < SIZE) {                                             \
            p = &HEAP[pos];                                           \
            do {                                                      \
                if (*p == 0) break;                                   \
                pos++;                                                \
                p++;                                                  \
            } while (pos < SIZE);                                     \
        }                                                             \
        if (pos >= SIZE) {                                            \
            return -1;                                                \
        }                                                             \
        j = pos + n;                                                  \
        if (j > SIZE) {                                               \
            return -1;                                                \
        }                                                             \
        j = sum = 0;                                                  \
        for (; j < n; j++) {                                          \
            sum += HEAP[pos + j];                                     \
        }                                                             \
        if (sum == 0) {                                               \
            break;                                                    \
        }                                                             \
        pos += n;                                                     \
    }                                                                 \
    return pos;                                                       \
}

#define SET_CLR(NAME, HEAP)                                           \
void set_used_##NAME(int start, int n) {                              \
    int i;                                                            \
                                                                      \
    for (i = 0; i < n; i++) {                                         \
        HEAP[start + i] = 1;                                          \
    }                                                                 \
}                                                                     \
                                                                      \
void clr_used_##NAME(int start, int n) {                              \
    int i;                                                            \
                                                                      \
    for (i = 0; i < n; i++) {                                         \
        HEAP[start + i] = 0;                                          \
    }                                                                 \
}

/* near-match (register choice of j and the pointer: 7 instructions), original bytes linked; the C is the PC/reading copy */
#ifdef __MWERKS__
static asm int get_start_material(int n)
{
#include "get_start_material.inc"
}
#else
static GET_START(material, material_heap, 0x400)
#endif

typedef struct MATERIAL { char pad[0x50]; } MATERIAL;

static u8 *get_material_ptr(int n) {
    return (u8 *)(material_heap_area + n * sizeof(MATERIAL));
}

static void set_used_material(int start, int n) {
    int i;

    for (i = 0; i < n; i++) {
        material_heap[start + i] = 1;
    }
}

static void clr_used_material(int start, int n) {
    int i;

    for (i = 0; i < n; i++) {
        material_heap[start + i] = 0;
    }
}

/* near-match (register choice of j and the pointer: 7 instructions), original bytes linked; the C is the PC/reading copy */
#ifdef __MWERKS__
asm int get_start_hierarchy(int n)
{
#include "get_start_hierarchy.inc"
}
#else
GET_START(hierarchy, hierarchy_heap, 0x800)
#endif

typedef struct HIER { char pad[400]; } HIER;

u8 *get_hierarchy_ptr(int n) {
    return (u8 *)(hierarchy_heap_area + n * sizeof(HIER));
}

void set_used_hierarchy(int start, int n) {
    int i;

    if (n != 0) {
        for (i = 0; i < n; i++) {
            hierarchy_heap[start + i] = 1;
        }
    }
}

static void clr_used_hierarchy(int start, int n) {
    int i;

    if (n != 0) {
        for (i = 0; i < n; i++) {
            hierarchy_heap[start + i] = 0;
        }
    }
}

/* near-match (register choice of j and the pointer: 7 instructions), original bytes linked; the C is the PC/reading copy */
#ifdef __MWERKS__
static asm int get_start_clay(int n)
{
#include "get_start_clay.inc"
}
#else
static GET_START(clay, clay_heap, 0x180)
#endif

static CLAY *get_clay_ptr(int n) {
    return (CLAY *)(clay_heap_area + n * sizeof(CLAY));
}

static void set_used_clay(int start, int n) {
    int i;

    for (i = 0; i < n; i++) {
        clay_heap[start + i] = 1;
    }
}

static void clr_used_clay(int start, int n) {
    int i;

    for (i = 0; i < n; i++) {
        clay_heap[start + i] = 0;
    }
}

/* near-match (register choice of j and the pointer: 7 instructions), original bytes linked; the C is the PC/reading copy */
#ifdef __MWERKS__
asm int get_start_mdlw(int n)
{
#include "get_start_mdlw.inc"
}
#else
GET_START(mdlw, mdlw_heap, 0x80)
#endif

MDLW *get_mdlw_ptr(int n) {
    int a = (int)mdlw_heap_area;
    return (MDLW *)(a + (n << 7));
}

SET_CLR(mdlw, mdlw_heap)

typedef struct PLM {            /* PLW fields touched here (player_work[], 0xA00 each) */
    u8 _pad00[0xC];
    u16 id;                     /* 0x00C */
    u8 _pad0E[0x11 - 0xE];
    u8 sex;                     /* 0x011 0 male, 1 female */
    u8 _pad12[0x110 - 0x12];
    void *part[32];             /* 0x110 matrix block pointers (pairs: block, block + 0xB0) */
    u8 _pad190[0x4E4 - 0x190];
    s16 x4E4;                   /* 0x4E4 */
    u8 _pad4E6[0x508 - 0x4E6];
    s16 mdl_n;                  /* 0x508 slots used by the body model */
    s16 mdl_no;                 /* 0x50A first slot */
    s32 mdl_ptr;                /* 0x50C */
    u8 _pad510[4];
    s32 wpn_ptr[4];             /* 0x514 */
    s16 wpn_n[4];               /* 0x524 */
    s16 wpn_no[4];              /* 0x52C */
    s32 part_ptr[6];            /* 0x534 armor part model pointers */
    s16 part_n[6];              /* 0x54C slots used by the armor parts */
    s16 part_no[6];             /* 0x558 first slot */
    u8 _pad564[0xA00 - 0x564];
} PLM;

extern PLM player_work[];
extern u8 parts_work[];
extern s32 WEAPON_TEX[];
extern s32 pl_area_top;
extern s32 set_top;
extern s32 set_mdlw;
extern s32 stage_model;
extern s32 set_model_data[];
extern s32 SET_TEX[];
extern s32 EDIT_TEX[];
extern s32 edit_mdlw[2];
extern s32 edit_top[2];


void load_weapon_model(int);
void load_set_model(int);
void load_edit_model(int);

/* near-match (the original's loop is unrolled 4x by the compiler with a running block pointer, ours 8x; 16 lines), original bytes linked */
#ifdef __MWERKS__
asm void pl_create_model(int pl_no)
{
#include "pl_create_model.inc"
}
#else
void pl_create_model(int pl_no) {
    PLM *pl = &player_work[pl_no];
    int i;
    u8 *blk;

    blk = parts_work + pl->id * 0x1600;
    i = 0;
    do {
        pl->part[i] = blk;
        pl->part[i + 1] = blk + 0xB0;
        blk += 0x160;
        i += 2;
    } while (i < 0x20);
    pl->id = pl_no;
    pl->x4E4 = 0;
}
#endif

/* near-match (scheduling of the pl_area_top load against the slot sum: 10 instructions), original bytes linked */
#ifdef __MWERKS__
asm void weapon_create_model(int wpn, int pl_no, int slot)
{
#include "weapon_create_model.inc"
}
#else
void weapon_create_model(int wpn, int pl_no, int slot) {
    PLM *pl = &player_work[pl_no];
    int h;

    load_weapon_model(wpn);
    h = get_start_mdlw(1);
    if (h >= 0) {
        pl->wpn_no[slot] = h;
        pl->wpn_n[slot] = 1;
        pl->wpn_ptr[slot] = (s32)get_mdlw_ptr(h);
        set_used_mdlw(h, 1);
        model_work_set((s16)h, pl_area_top, (s16)(slot + (pl_no * 4 + 0x1A)), WEAPON_TEX[wpn], 0x900, 1);
    }
}
#endif

typedef struct PLI {
    u8 _pad00[0x11];
    u8 sex;
    u8 _pad12[0x34E - 0x12];
    u8 hair;
    u8 _pad34F[0x352 - 0x34F];
    u8 leg, face, head, body, arm, waist;
    u8 m_leg, m_face, m_head, m_body, m_arm, m_waist;
    u8 _pad35E[0x607 - 0x35E];
    u8 skin;
} PLI;
typedef struct ARM { u8 m, f, flag; u8 _pad[0x11]; } ARM;
extern ARM Armor_Leg_Data[], Armor_Body_Data[], Armor_Arm_Data[], Armor_Waist_Data[], Armor_Head_Data[];
extern u8 parts_max_tbl[];
extern u8 skin_col_tbl_m[], skin_col_tbl_f[];

void Pl_model_id_set(PLI *pl) {
    s16 i;
    ARM *a;
    u8 f;

    i = 0;
    do {
        switch (i) {
        case 1:
            f = pl->face;
            if (f <= 0 && parts_max_tbl[pl->sex * 6 + 1] < f) {
                f = 1;
            }
            pl->m_face = f - 1;
            if (pl->sex == 0) {
                pl->skin = skin_col_tbl_m[pl->m_face];
            } else {
                pl->skin = skin_col_tbl_f[pl->m_face];
            }
            break;
        case 0:
            if (pl->sex == 0) {
                a = &Armor_Leg_Data[pl->leg];
                if (a->flag & 1) {
                    pl->m_leg = a->m;
                } else {
                    pl->m_leg = 0;
                }
            } else {
                a = &Armor_Leg_Data[pl->leg];
                if (a->flag & 2) {
                    pl->m_leg = a->f;
                } else {
                    pl->m_leg = 0;
                }
            }
            break;
        case 3:
            if (pl->sex == 0) {
                a = &Armor_Body_Data[pl->body];
                if (a->flag & 1) {
                    pl->m_body = a->m;
                } else {
                    pl->m_body = 0;
                }
            } else {
                a = &Armor_Body_Data[pl->body];
                if (a->flag & 2) {
                    pl->m_body = a->f;
                } else {
                    pl->m_body = 0;
                }
            }
            break;
        case 4:
            if (pl->sex == 0) {
                a = &Armor_Arm_Data[pl->arm];
                if (a->flag & 1) {
                    pl->m_arm = a->m;
                } else {
                    pl->m_arm = 0;
                }
            } else {
                a = &Armor_Arm_Data[pl->arm];
                if (a->flag & 2) {
                    pl->m_arm = a->f;
                } else {
                    pl->m_arm = 0;
                }
            }
            break;
        case 5:
            if (pl->sex == 0) {
                a = &Armor_Waist_Data[pl->waist];
                if (a->flag & 1) {
                    pl->m_waist = a->m;
                } else {
                    pl->m_waist = 0;
                }
            } else {
                a = &Armor_Waist_Data[pl->waist];
                if (a->flag & 2) {
                    pl->m_waist = a->f;
                } else {
                    pl->m_waist = 0;
                }
            }
            break;
        case 2:
            if (pl->head == 0) {
                pl->m_head = pl->hair;
            } else if (pl->sex == 0) {
                a = &Armor_Head_Data[pl->head];
                if (a->flag & 1) {
                    pl->m_head = a->m;
                } else {
                    pl->m_head = pl->hair;
                }
            } else {
                a = &Armor_Head_Data[pl->head];
                if (a->flag & 2) {
                    pl->m_head = a->f;
                } else {
                    pl->m_head = pl->hair;
                }
            }
            break;
        }
        i++;
    } while (i < 6);
}

typedef struct PLA {
    u8 _pad00[0xC];
    u16 id;                     /* 0x00C */
    u8 _pad0E[0x11 - 0xE];
    u8 sex;                     /* 0x011 */
    u8 _pad12[0x358 - 0x12];
    u8 mdl[6];                  /* 0x358 model number of the six parts */
    u8 _pad35E[0x508 - 0x35E];
    s16 body_n;                 /* 0x508 */
    s16 body_no;                /* 0x50A */
    s32 body_ptr;               /* 0x50C */
    u8 _pad510[0x534 - 0x510];
    s32 part_ptr[6];            /* 0x534 */
    s16 part_n[6];              /* 0x54C */
    s16 part_no[6];             /* 0x558 */
    u8 _pad564[0x607 - 0x564];
    u8 skin;                    /* 0x607 */
} PLA;

extern s16 reg_nude_model[], body_nude_model[], arm_nude_model[];
extern s32 parts_tex[];
extern s32 pl_area_top;

void load_armor_model();

/* near-match: every instruction matches except the s-register numbering of i/p2/p4/off (orig i=s2 p2=s1 p4=s7 off=s6 h=s4; a
   declaration-order hill climb and all orders of the four initialisations found nothing). Original bytes linked. */
#ifdef __MWERKS__
asm void armor_create_model(PLA *pl)
{
#include "armor_create_model.inc"
}
#else
void armor_create_model(PLA *pl) {
    int tex;
    u8 *p2;
    s16 i;
    int id;
    s16 h;
    int off;
    u8 *p4;

    Pl_model_id_set((PLI *)pl);
    i = 0;
    p2 = (u8 *)pl;
    p4 = (u8 *)pl;
    off = 0;
    do {
        id = (s16)pl->mdl[i];
        switch (i) {
        case 1:
        case 2:
        case 5:
            break;
        case 0:
            if (pl->sex == 0) {
                if (id == 0 || (u32)(id - 13) <= 1 || id == 15) {
                    id = reg_nude_model[pl->skin];
                }
            } else if (id == 0 || (u32)(id - 11) <= 1 || id == 13) {
                id = reg_nude_model[pl->skin + 4];
            }
            break;
        case 3:
            if (pl->sex == 0) {
                if (id == 0 || (u32)(id - 16) <= 1 || id == 18) {
                    id = body_nude_model[pl->skin];
                }
            } else if (id == 0 || (u32)(id - 12) <= 1 || id == 14) {
                id = body_nude_model[pl->skin + 4];
            }
            break;
        case 4:
            if (pl->sex == 0) {
                if (id == 0 || (u32)(id - 16) <= 1 || id == 18) {
                    id = arm_nude_model[pl->skin];
                }
            } else if (id == 0 || (u32)(id - 12) <= 1 || id == 14) {
                id = arm_nude_model[pl->skin + 4];
            }
            break;
        }
        load_armor_model((s16)id, i, pl->sex);
        h = get_start_mdlw(1);
        if (h < 0) {
            return;
        }
        if (i == 0) {
            pl->body_no = h;
            pl->body_n = 1;
            pl->body_ptr = (s32)get_mdlw_ptr(h);
        } else {
            *(s16 *)(p2 + 0x558) = h;
            *(s16 *)(p2 + 0x54C) = 1;
            *(s32 *)(p4 + 0x534) = (s32)get_mdlw_ptr(h);
        }
        set_used_mdlw(h, 1);
        tex = parts_tex[pl->sex * 6 + i] + id * 8;
        if (i == 0) {
            model_work_set2(h, pl_area_top, (s16)(pl->id * 12 + 0x3A + off), (int *)tex, 0x900, (u16)(pl->skin + pl->sex * 4), 2);
        } else {
            model_work_set2(h, pl_area_top, (s16)(pl->id * 12 + 0x3A + off), (int *)tex, 0x900, (u16)(pl->skin + pl->sex * 4), 1);
        }
        i++;
        p2 += 2;
        p4 += 4;
        off += 2;
    } while (i < 6);
}
#endif
