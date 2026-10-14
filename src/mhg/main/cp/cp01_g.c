/* G-only vector helper between AddVector/ScaleVector-like cp01 routines (SLPM_658.69 0x00111040): dst = normalize(src) * len. */
#include "types.h"

void flvecCopy(f32 *, f32 *);
void flvecNormalize(f32 *);

void func_00111040(f32 *dst, f32 *src, f32 len) {
    flvecCopy(dst, src);
    flvecNormalize(dst);
    dst[0] *= len;
    dst[1] *= len;
    dst[2] *= len;
}
