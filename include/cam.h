/* cam.h - game camera (f_cam, f_cam_223B50): CameraWork and its five
 * camera slots. Field names are mostly by offset; meanings are guesses from
 * the code that uses them. */
#ifndef CAM_H
#define CAM_H
#include "types.h"
#include "pl.h"

/* One zoom entry of a camera config (32 bytes). */
typedef struct CAMCNFE {
    f32 x00;            /* 0x00 */
    f32 y;              /* 0x04 eye offset up (rotated by the camera yaw) */
    f32 z;              /* 0x08 eye offset back */
    f32 x0C;            /* 0x0C */
    f32 tar_y;          /* 0x10 target height above the player */
    f32 x14;            /* 0x14 */
    f32 gnd;            /* 0x18 eye kept this far above the ground */
    f32 x1C;            /* 0x1C */
} CAMCNFE;

/* Camera config: angle of view and roll, then the zoom levels. */
typedef struct CAMCNF {
    f32 fov;            /* 0x00 */
    f32 roll;           /* 0x04 */
    u8 _pad08[0x20 - 0x08];
    CAMCNFE e[7];       /* 0x20 zoom levels (CameraWork.zoom), [4] = special */
} CAMCNF;

/* Camera area (stage camera data, CameraWork.area). */
typedef struct CAMAREA {
    u8 x00;             /* 0x00 */
    u8 no;              /* 0x01 area number */
    u8 type;            /* 0x02 0 = follow player, 1 fixed, 2 rail, 3 fixed offset */
    u8 _pad03[0x1C - 0x03];
    u8 *blend;          /* 0x1C 16 pairs (from area, frames to blend) */
    union {
        CAMCNF cnf;     /* 0x20 type 0 */
        struct {
            f32 pos[3]; /* 0x20 eye (type 1) or offset from the player (type 3) */
            u8 _pad2C[0x50 - 0x2C];
            f32 fov;    /* 0x50 */
            f32 roll;   /* 0x54 */
        } fix;
    } u;
} CAMAREA;

/* Per-mode work at slot+0x90 (0x70 bytes). */
typedef struct CAMD_STD {
    f32 eye[3];         /* 0x00 */
    f32 tar[3];         /* 0x0C */
    f32 old[3];         /* 0x18 eye last frame */
    f32 gnd;            /* 0x24 ground under the target */
    s8 gflag;           /* 0x28 */
#ifdef VERSION_MHG
    u8 x29;             /* 0x29 (G only) */
    s16 x2A;            /* 0x2A (G only) */
    f32 x2C;            /* 0x2C (G only) */
#else
    u8 _pad29[0x30 - 0x29];
#endif
    f32 eye_d[3];       /* 0x30 wanted eye */
    f32 tar_d[3];       /* 0x3C wanted target */
    f32 fov;            /* 0x48 */
    f32 fov_d;          /* 0x4C */
    f32 roll;           /* 0x50 */
    f32 roll_d;         /* 0x54 */
    CAMCNF *cnf;        /* 0x58 */
    CAMCNFE *cnfe;      /* 0x5C current zoom entry */
    f32 hit_h;          /* 0x60 eye raised by k_HitWallCamera */
    f32 rate;           /* 0x64 blend step */
#ifdef VERSION_MHG
    f32 x68;            /* 0x68 (G only, zeroed by cam_init_sub_std) */
    f32 x6C;            /* 0x6C (G only) */
    s16 ang;            /* 0x70 wanted yaw */
    s16 ang2;           /* 0x72 camera yaw (MH1: CAMS.ang) */
    u16 on;             /* 0x74 buttons held */
    u16 trg;            /* 0x76 buttons pressed */
    u8 x6E;             /* 0x78 */
    u8 wall;            /* 0x79 GetWallHitLine result */
    u8 x7A;             /* 0x7A (G only) */
    u8 x7B;             /* 0x7B (G only) */
#else
    s16 ang;            /* 0x68 wanted yaw */
    u16 on;             /* 0x6A buttons held */
    u16 trg;            /* 0x6C buttons pressed */
    u8 x6E;             /* 0x6E */
    u8 wall;            /* 0x6F GetWallHitLine result */
#endif
} CAMD_STD;

typedef struct CAMD_STG {
    f32 eye[3];         /* 0x00 */
    f32 tar[3];         /* 0x0C */
    f32 eye_f[3];       /* 0x18 blend from */
    f32 tar_f[3];       /* 0x24 */
    f32 fov;            /* 0x30 */
    f32 roll;           /* 0x34 */
    f32 fov_f;          /* 0x38 */
    f32 roll_f;         /* 0x3C */
#ifdef VERSION_MHG
    s16 vx;             /* 0x40 pitch speed */
    s16 vy;             /* 0x42 yaw speed */
    u8 _pad44[0x48 - 0x44];
    s16 ax;             /* 0x48 pitch (MH1: CAMS.ax) */
    s16 ay;             /* 0x4A yaw (MH1: CAMS.ay) */
    u8 _pad4C[0x50 - 0x4C];
    f32 rate;           /* 0x50 */
    s16 cnt;            /* 0x54 blend frames left */
#else
    u8 _pad40[0x42 - 0x40];
    s16 cnt;            /* 0x42 blend frames left */
    f32 rate;           /* 0x44 */
    s16 vx;             /* 0x48 pitch speed */
    s16 vy;             /* 0x4A yaw speed */
#endif
} CAMD_STG;

typedef struct CAMD_PCH {
    f32 m[4][4];        /* 0x00 cannon matrix */
    f32 pos[3];         /* 0x40 player position last frame */
    f32 min;            /* 0x4C fov range */
    f32 max;            /* 0x50 */
    f32 range;          /* 0x54 */
    f32 fov0;           /* 0x58 */
    f32 fov1;           /* 0x5C */
#ifdef VERSION_MHG
    s16 x60;            /* 0x60 (G only) */
    s16 ang;            /* 0x62 */
    s8 type;            /* 0x64 PachiTypeCheck */
    u8 x63;             /* 0x65 */
    u8 zoom;            /* 0x66 */
#else
    s16 ang;            /* 0x60 */
    s8 type;            /* 0x62 PachiTypeCheck */
    u8 x63;             /* 0x63 */
    u8 zoom;            /* 0x64 */
#endif
} CAMD_PCH;

typedef struct CAMFISH {
    u8 step;            /* 0x00 */
    s32 on;             /* 0x04 */
    void *fish;         /* 0x08 */
} CAMFISH;

typedef struct CAMZOOM {
    u8 step;            /* 0x00 */
    PLW *npc;           /* 0x04 */
    f32 pos[3];         /* 0x08 */
    f32 tar[3];         /* 0x14 */
#ifdef VERSION_MHG
    u8 req;             /* 0x20 (a byte in G) */
    u8 x21;             /* 0x21 (G only) mode: 0 NPC, 1 manual, 2 fixed */
#else
    u16 req;            /* 0x20 */
#endif
    s16 cnt;            /* 0x22 */
#ifdef VERSION_MHG
    f32 x24;            /* 0x24 manual: height above the hunter */
    f32 x28;            /* 0x28 manual: distance */
    f32 x2C;            /* 0x2C field of view at full zoom */
#endif
} CAMZOOM;

typedef struct CAMD_EX {
    u8 _pad00[4];
    CAMFISH fish;       /* 0x04 */
    CAMZOOM zoom;       /* 0x10 */
} CAMD_EX;

typedef struct CAMD_DEMO {
    f32 eye[3];         /* 0x00 */
    f32 eye_o[3];       /* 0x0C */
    f32 tar[3];         /* 0x18 */
    f32 tar_o[3];       /* 0x24 */
    f32 roll;           /* 0x30 */
    f32 roll_o;         /* 0x34 */
    f32 fov;            /* 0x38 */
    f32 fov_o;          /* 0x3C */
    s8 *cmd;            /* 0x40 command stream */
    s8 *loop;           /* 0x44 */
    u8 move;            /* 0x48 */
    u8 pos_mode;        /* 0x49 */
    u8 tar_mode;        /* 0x4A */
    u8 pos_part;        /* 0x4B */
    u8 tar_part;        /* 0x4C */
    u8 x4D;             /* 0x4D */
    u8 no;              /* 0x4E demo camera number */
    s8 state;           /* 0x4F 1 running, -1 failed */
    s32 arg;            /* 0x50 */
    f32 x54;            /* 0x54 */
    f32 x58;            /* 0x58 */
#ifdef VERSION_MHG
    s16 ax;             /* 0x5C (MH1: CAMS.ax) */
    s16 ay;             /* 0x5E */
    s16 ax0;            /* 0x60 */
    s16 ay0;            /* 0x62 */
    u8 stop;            /* 0x64 */
    u8 x65;             /* 0x65 (G only) */
    u8 x66;             /* 0x66 (G only) */
#else
    u8 stop;            /* 0x5C */
#endif
} CAMD_DEMO;

/* One camera (0x100 bytes). The head has the same layout as CAMW's. */
typedef struct CAMS {
    f32 eye[3];         /* 0x00 */
    f32 tar[3];         /* 0x0C */
    u8 _pad18[0x24 - 0x18];
    f32 vec[3];         /* 0x24 eye - target (cam_sub_stg) */
    f32 eye_f[3];       /* 0x30 blend from */
    f32 tar_f[3];       /* 0x3C */
    u8 _pad48[0x60 - 0x48];
    f32 roll;           /* 0x60 */
    f32 roll_f;         /* 0x64 */
    f32 fov;            /* 0x68 */
    f32 fov_f;          /* 0x6C */
    u8 no;              /* 0x70 slot number */
    u8 act;             /* 0x71 this camera wants the view */
    u8 req;             /* 0x72 slot to take the yaw from, -1 none */
    u8 _pad73;
    s16 cnt;            /* 0x74 */
    s16 cnt_max;        /* 0x76 */
    union {
        s32 w;
        u8 b;
    } mode;             /* 0x78 */
    union {
        s32 w;
        u8 b;
    } step;             /* 0x7C */
#ifndef VERSION_MHG
    s16 ax;             /* 0x80 */
    s16 ay;             /* 0x82 */
    s16 ax0;            /* 0x84 */
    s16 ay0;            /* 0x86 */
    s16 ang;            /* 0x88 yaw */
    u8 _pad8A[0x90 - 0x8A];
#endif
    union {
#ifdef VERSION_MHG
        u8 b[0x80];
#else
        u8 b[0x70];
#endif
        CAMD_STD std;
        CAMD_STG stg;
        CAMD_PCH pch;
        CAMD_EX ex;
        CAMD_DEMO demo;
    } d;                /* 0x90 (G: 0x80, the five angles live in the per-mode work) */
} CAMS;

/* The camera angles: in CAMS for MH1, in the per-mode work (std, stage, demo) for G. */
#ifdef VERSION_MHG
#define CS_STD_ANG(cs) ((cs)->d.std.ang2)
#define CS_STG_AX(cs) ((cs)->d.stg.ax)
#define CS_STG_AY(cs) ((cs)->d.stg.ay)
#define CS_DEMO_AX(cs) ((cs)->d.demo.ax)
#define CS_DEMO_AY(cs) ((cs)->d.demo.ay)
#define CS_DEMO_AX0(cs) ((cs)->d.demo.ax0)
#define CS_DEMO_AY0(cs) ((cs)->d.demo.ay0)
#else
#define CS_STD_ANG(cs) ((cs)->ang)
#define CS_STG_AX(cs) ((cs)->ax)
#define CS_STG_AY(cs) ((cs)->ay)
#define CS_DEMO_AX(cs) ((cs)->ax)
#define CS_DEMO_AY(cs) ((cs)->ay)
#define CS_DEMO_AX0(cs) ((cs)->ax0)
#define CS_DEMO_AY0(cs) ((cs)->ay0)
#endif

typedef struct CAMQUAKE {
    f32 pos[3];         /* 0x00 */
    u8 on;              /* 0x0C */
    u8 type;            /* 0x0D 0x80 = everywhere */
    s16 time;           /* 0x0E */
} CAMQUAKE;

typedef struct CAMW {
    f32 eye[3];         /* 0x000 resulting view */
    f32 tar[3];         /* 0x00C */
    u8 _pad018[0x60 - 0x18];
    f32 roll;           /* 0x060 */
    u8 _pad064[0x68 - 0x64];
    f32 fov;            /* 0x068 */
    u8 _pad06C[0x80 - 0x6C];
    CAMS sl[5];         /* 0x080 std, stage, pachinger, player EX, demo */
    union {
        s32 w;
        u8 b;
    } init;             /* 0x580 */
    PLW *pl;            /* 0x584 */
    void *data;         /* 0x588 SetCameraData */
    CAMAREA *area;      /* 0x58C */
    u8 x590[0x18];      /* 0x590 Get_cam_grid_XZ */
    s16 gx;             /* 0x5A8 */
    s16 gz;             /* 0x5AA */
    f32 rail_t;         /* 0x5AC */
    f32 rail_u;         /* 0x5B0 */
    u8 rail_no;         /* 0x5B4 */
    u8 _pad5B5[0x5BC - 0x5B5];
    CAMQUAKE qk[2];     /* 0x5BC [1] pachinger */
    s8 grid;            /* 0x5DC */
    u8 area_no;         /* 0x5DD */
    u8 area_old;        /* 0x5DE */
    u8 area_chg;        /* 0x5DF (u8: lbu in cam_sub_std) */
    u8 cam_old;         /* 0x5E0 */
    u8 cam_no;          /* 0x5E1 0 = std, 1 = stage */
    u16 sw_on;          /* 0x5E2 */
    u16 sw_trg;         /* 0x5E4 */
    u16 sw_x5E6;        /* 0x5E6 */
    u16 an_ang;         /* 0x5E8 analog stick */
    u16 an_pow;         /* 0x5EA */
    s32 reset;          /* 0x5EC (WyvernFindPlayer stores a pointer) */
    s32 demo_arg;       /* 0x5F0 */
    u8 demo_req;        /* 0x5F4 */
    u8 _pad5F5;
    u8 zoom;            /* 0x5F6 */
#ifdef VERSION_MHG
    u8 x5F7;            /* 0x5F7 (G only, cleared by cam_init_sub_std) */
    f32 x5F8;           /* 0x5F8 (G only: 0x3E99999A when the camera is close to the hunter, else 1.0) */
    u8 x5FC;            /* 0x5FC (G only: cleared at the start of CameraMove; if set, func_001DD5E0 runs) */
    u8 x5FD;            /* 0x5FD (G only, cleared by CameraInit) */
    u8 _pad5FE[2];
#else
    u8 _pad5F7;
#endif
} CAMW;

extern CAMW CameraWork;

#endif
