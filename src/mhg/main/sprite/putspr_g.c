/* G-only sprite helpers between Put_F and Paint_square (SLPM_658.69 0x0012A800-0x0012A984): like Put_2TF / Put_F they scale
 * the x coordinates of a 640-wide screen primitive by 0.8 before handing it to flps0005 / flps000C (flat and shaded
 * triangles, G has no MH1 twin). */
#include "types.h"

typedef struct PUT_3 {          /* 24 bytes */
    s16 x0, y0;
    s16 x1, y1;
    s32 a, b, c, d;
} PUT_3;

typedef struct PUT_4 {          /* 28 bytes */
    s16 x0, y0;
    s16 x1, y1;
    s16 x2, y2;
    s32 a, b, c, d;
} PUT_4;

void flps0005(PUT_3 *);
void flps000C(PUT_4 *);

void func_0012A800(PUT_3 *p) {
    PUT_3 q = *p;

    q.x0 = 0.8f * q.x0;
    q.x1 = 0.8f * q.x1;
    flps0005(&q);
}

void func_0012A8B0(PUT_4 *p) {
    PUT_4 q = *p;

    q.x0 = 0.8f * q.x0;
    q.x1 = 0.8f * q.x1;
    q.x2 = 0.8f * q.x2;
    flps000C(&q);
}
