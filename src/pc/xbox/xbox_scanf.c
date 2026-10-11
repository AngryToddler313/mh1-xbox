/* sscanf for the Xbox build with floating point. nxdk's pdclib accepts
 * %f/%e/%g/%a in scanf formats but converts nothing (the conversion fails and
 * the scan stops), so every sscanf("%d,%f,%f", ...) in the PC code returned
 * too few fields on the Xbox: the RT_* test aids with coordinates (village
 * warps, RT_PL_WARP ...) and --cam / --follow never took effect (found in xemu,
 * 12 Oct 2026). This object's sscanf replaces pdclib's (the linker takes the
 * library member only for undefined symbols): literal text, white space,
 * %n, %% and the float conversions are handled here, every other conversion
 * is handed to pdclib's vsscanf one at a time. */
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* pdclib's vsscanf behind a variadic front (sscanf itself is ours now) */
static int sscanf_pd(const char *s, const char *f, ...)
{
    va_list ap;
    int r;
    va_start(ap, f);
    r = vsscanf(s, f, ap);
    va_end(ap);
    return r;
}

/* one non-float conversion through pdclib: "%<spec>%n" */
static int one(const char *s, const char *spec, void *arg, int *used)
{
    char f[40];
    *used = -1;
    snprintf(f, sizeof f, "%s%%n", spec);
    if (arg)
        return sscanf_pd(s, f, arg, used);
    return sscanf_pd(s, f, used);
}

/* decimal float: [sign] digits [. digits] [e [sign] digits], at most w chars */
static int scan_float(const char *s, int w, double *out)
{
    const char *p = s, *end = s + (w > 0 ? w : 1 << 20);
    double v = 0, f = 1;
    int neg = 0, digits = 0, e = 0, eneg = 0;
    if (p < end && (*p == '+' || *p == '-'))
        neg = *p++ == '-';
    while (p < end && isdigit((unsigned char)*p)) {
        v = v * 10 + (*p++ - '0');
        digits++;
    }
    if (p < end && *p == '.')
        for (p++; p < end && isdigit((unsigned char)*p); p++, digits++)
            v += (*p - '0') * (f /= 10);
    if (!digits)
        return 0;
    if (p + 1 < end && (*p == 'e' || *p == 'E')
        && (isdigit((unsigned char)p[1]) || ((p[1] == '+' || p[1] == '-') && p + 2 < end && isdigit((unsigned char)p[2])))) {
        p++;
        if (*p == '+' || *p == '-')
            eneg = *p++ == '-';
        while (p < end && isdigit((unsigned char)*p))
            e = e * 10 + (*p++ - '0');
        while (e--)
            v = eneg ? v / 10 : v * 10;
    }
    *out = neg ? -v : v;
    return (int)(p - s);
}

int sscanf(const char *str, const char *fmt, ...)
{
    va_list ap;
    const char *s = str;
    int n = 0;
    va_start(ap, fmt);
    while (*fmt) {
        if (isspace((unsigned char)*fmt)) {
            while (isspace((unsigned char)*fmt))
                fmt++;
            while (isspace((unsigned char)*s))
                s++;
            continue;
        }
        if (*fmt != '%' || fmt[1] == '%') {
            if (*fmt == '%')
                fmt++;
            if (*s != *fmt)
                break;
            s++;
            fmt++;
            continue;
        }
        {
            const char *start = fmt++;
            int skip = 0, w = 0, longf = 0;
            char conv, spec[40];
            if (*fmt == '*') {
                skip = 1;
                fmt++;
            }
            while (isdigit((unsigned char)*fmt))
                w = w * 10 + (*fmt++ - '0');
            while (strchr("hljztL", *fmt)) {
                if (*fmt == 'l' || *fmt == 'L')
                    longf = 1;
                fmt++;
            }
            conv = *fmt;
            if (conv == '[') {              /* scan set: up to the closing ] (a leading ] belongs to the set) */
                fmt++;
                if (*fmt == '^')
                    fmt++;
                if (*fmt == ']')
                    fmt++;
                while (*fmt && *fmt != ']')
                    fmt++;
            }
            if (!*fmt)
                break;
            fmt++;
            if (conv == 'n') {
                if (!skip)
                    *va_arg(ap, int *) = (int)(s - str);
                continue;
            }
            if (strchr("fFeEgGaA", conv)) {
                double v;
                int k;
                while (isspace((unsigned char)*s))
                    s++;
                if (!*s && !n) {
                    n = EOF;
                    break;
                }
                if (!(k = scan_float(s, w, &v)))
                    break;
                s += k;
                if (!skip) {
                    if (longf)
                        *va_arg(ap, double *) = v;
                    else
                        *va_arg(ap, float *) = (float)v;
                    n++;
                }
                continue;
            }
            if ((size_t)(fmt - start) >= sizeof spec - 3)
                break;
            memcpy(spec, start, (size_t)(fmt - start));
            spec[fmt - start] = 0;
            {
                int used, r = one(s, spec, skip ? NULL : va_arg(ap, void *), &used);
                if (r == EOF && !n) {
                    n = EOF;
                    break;
                }
                if (used < 0)               /* the conversion failed */
                    break;
                s += used;
                if (!skip)
                    n++;
            }
        }
    }
    va_end(ap);
    return n;
}
