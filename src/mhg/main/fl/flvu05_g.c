/* G-only VU0 routine (SLPM_658.69 0x0013E750): v = v * m (4-vector times 4x4 matrix, in place; a0 = v, a1 = m).
 * MWCC inline assembly like flvu05.c; on the PC the rt supplies a C version. */
#ifdef __MWERKS__
asm void func_0013E750(void)
{
    lwc1 f3, 0x0(a0)
    addiu sp, sp, -0x10
    lwc1 f2, 0x4(a0)
    addiu v1, sp, 0x0
    lwc1 f1, 0x8(a0)
    lwc1 f0, 0xC(a0)
    swc1 f3, 0x0(v1)
    swc1 f2, 0x4(v1)
    swc1 f1, 0x8(v1)
    swc1 f0, 0xC(v1)
    lqc2 vf4, 0x0(v1)
    lqc2 vf5, 0x0(a1)
    lqc2 vf6, 0x10(a1)
    lqc2 vf7, 0x20(a1)
    lqc2 vf8, 0x30(a1)
    vmulax.xyzw ACC, vf5, vf4x
    vmadday.xyzw ACC, vf6, vf4y
    vmaddaz.xyzw ACC, vf7, vf4z
    vmaddw.xyzw vf4, vf8, vf4w
    sqc2 vf4, 0x0(v1)
    addiu v1, sp, 0x0
    lwc1 f3, 0x0(v1)
    lwc1 f2, 0x4(v1)
    lwc1 f1, 0x8(v1)
    lwc1 f0, 0xC(v1)
    swc1 f3, 0x0(a0)
    swc1 f2, 0x4(a0)
    swc1 f1, 0x8(a0)
    swc1 f0, 0xC(a0)
    jr ra
    addiu sp, sp, 0x10
}
#endif
