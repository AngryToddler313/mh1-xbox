/*
 * xbox_libc.c - C library pieces nxdk's pdclib does not have yet (Xbox build
 * only, tools/build_xbox.py). atof: decimal and exponent forms only, enough
 * for the viewer's command-line numbers. The printf family with floating
 * point (xbox_compat.h renames the callers' names to these).
 */
#include <ctype.h>

double atof(const char *s)
{
    double v = 0, f = 1;
    int neg = 0, e = 0, eneg = 0;

    while (isspace((unsigned char)*s))
        s++;
    if (*s == '+' || *s == '-')
        neg = *s++ == '-';
    while (isdigit((unsigned char)*s))
        v = v * 10 + (*s++ - '0');
    if (*s == '.')
        for (s++; isdigit((unsigned char)*s); s++)
            v += (*s - '0') * (f /= 10);
    if (*s == 'e' || *s == 'E') {
        s++;
        if (*s == '+' || *s == '-')
            eneg = *s++ == '-';
        while (isdigit((unsigned char)*s))
            e = e * 10 + (*s++ - '0');
        while (e--)
            v = eneg ? v / 10 : v * 10;
    }
    return neg ? -v : v;
}

/* fopen with '/' turned into '\' (xbox_compat.h); this file is compiled
 * without that header, so fopen here is pdclib's */
#include <stdio.h>
#include <string.h>

FILE *xbox_fopen(const char *name, const char *mode)
{
    char p[512];
    size_t i, n = strlen(name);

    if (n >= sizeof p)
        return NULL;
    for (i = 0; i <= n; i++)
        p[i] = name[i] == '/' ? '\\' : name[i];
    return fopen(p, mode);
}

/* ------------------------------------------------------------ printf
 * pdclib's printf skips %f/%e/%g/%a without taking the argument. These
 * versions do the floating-point conversions themselves and hand every other
 * conversion, one at a time, to pdclib's snprintf. Float output is exact to
 * about 15 significant digits, plenty for logs and on-screen numbers. */
#include <stdarg.h>
#include <stdlib.h>

typedef struct {
    char *buf;          /* string output: buf[0..cap-1], always terminated */
    size_t cap;
    FILE *f;            /* or stream output through chunk[] */
    char chunk[256];
    size_t nchunk;
    size_t total;       /* characters produced (the return value) */
} Sink;

static void sink_put(Sink *k, const char *s, size_t n)
{
    if (k->f) {
        while (n) {
            size_t m = sizeof k->chunk - k->nchunk;
            if (m > n)
                m = n;
            memcpy(k->chunk + k->nchunk, s, m);
            k->nchunk += m;
            s += m;
            n -= m;
            k->total += m;
            if (k->nchunk == sizeof k->chunk) {
                fwrite(k->chunk, 1, k->nchunk, k->f);
                k->nchunk = 0;
            }
        }
        return;
    }
    if (k->total + 1 < k->cap) {
        size_t m = k->cap - 1 - k->total;
        memcpy(k->buf + k->total, s, m < n ? m : n);
    }
    k->total += n;
}

static void sink_pad(Sink *k, char c, int n)
{
    char b[32];
    memset(b, c, sizeof b);
    for (; n > 0; n -= (int)sizeof b)
        sink_put(k, b, n < (int)sizeof b ? (size_t)n : sizeof b);
}

enum { F_LEFT = 1, F_PLUS = 2, F_SPACE = 4, F_ALT = 8, F_ZERO = 16 };

/* v > 0 as m x 10^e with 1 <= m < 10; returns e */
static int float_exp(double v, double *m)
{
    static const double big[9] = { 1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256 };
    int e = 0, k;

    if (v >= 10) {
        for (k = 8; k >= 0; k--)
            if (v >= big[k]) {
                v /= big[k];
                e += 1 << k;
            }
    } else if (v < 1) {
        for (k = 8; k >= 0; k--)
            if (v * big[k] < 1) {
                v *= big[k];
                e -= 1 << k;
            }
        v *= 10;
        e--;
    }
    if (v >= 10) {          /* rounding in the scaling */
        v /= 10;
        e++;
    } else if (v < 1) {
        v *= 10;
        e--;
    }
    *m = v;
    return e;
}

/* nd (1..17) significant digits of v > 0, rounded, into d; returns the
 * decimal exponent of the first digit (v ~ d[0].d[1]... x 10^e) */
static int float_digits(double v, int nd, char *d)
{
    static const unsigned long long p10[18] = { 1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL,
        1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL,
        1000000000000ULL, 10000000000000ULL, 100000000000000ULL, 1000000000000000ULL,
        10000000000000000ULL, 100000000000000000ULL };
    unsigned long long r;
    double m;
    int e = float_exp(v, &m), k;

    m *= (double)p10[nd - 1];
    r = (unsigned long long)(m + 0.5);
    if ((double)r - m == 0.5 && (r & 1))   /* a tie: to even, like glibc */
        r--;
    if (r >= p10[nd]) {     /* 9.99.. rounded up to 10.0.. */
        r /= 10;
        e++;
    }
    for (k = nd - 1; k >= 0; k--, r /= 10)
        d[k] = (char)('0' + r % 10);
    return e;
}

/* the digits of a conversion: nd significant digits (any count; past 17
 * they are zeros) */
static int float_digits_n(double v, int nd, char *d)
{
    int e;
    if (v == 0) {
        memset(d, '0', (size_t)nd);
        return 0;
    }
    e = float_digits(v, nd > 17 ? 17 : nd, d);
    if (nd > 17)
        memset(d + 17, '0', (size_t)(nd - 17));
    return e;
}

#define FLOAT_PREC_MAX 60

/* one %f/%e/%g of |v| without sign or padding into out (room for 380 chars) */
static int float_body(char *out, double v, int conv, int prec, int flags)
{
    char d[400];
    int up = conv == 'E' || conv == 'G' || conv == 'A';
    int n = 0, e = 0, i, nd, strip = 0, have = 0;

    if (v != v || v - v != 0) {         /* NaN, infinity */
        memcpy(out, v != v ? (up ? "NAN" : "nan") : (up ? "INF" : "inf"), 3);
        return 3;
    }
    if (prec < 0)
        prec = 6;
    if (prec > FLOAT_PREC_MAX)
        prec = FLOAT_PREC_MAX;
    conv |= 0x20;
    if (conv == 'a')
        conv = 'e';
    if (conv == 'g') {                  /* %e or %f from prec significant digits */
        if (prec == 0)
            prec = 1;
        e = float_digits_n(v, prec, d);
        have = 1;
        strip = !(flags & F_ALT);
        if (e < prec && e >= -4) {
            conv = 'f';
            prec = prec - 1 - e;
        } else {
            conv = 'e';
            prec--;
        }
    }
    if (conv == 'e') {
        if (!have)
            e = float_digits_n(v, prec + 1, d);
        out[n++] = d[0];
        if (prec || (flags & F_ALT))
            out[n++] = '.';
        memcpy(out + n, d + 1, (size_t)prec);
        n += prec;
        if (strip) {
            while (out[n - 1] == '0')
                n--;
            if (out[n - 1] == '.')
                n--;
        }
        out[n++] = up ? 'E' : 'e';
        out[n++] = e < 0 ? '-' : '+';
        if (e < 0)
            e = -e;
        if (e >= 100)
            out[n++] = (char)('0' + e / 100);
        out[n++] = (char)('0' + e / 10 % 10);
        out[n++] = (char)('0' + e % 10);
        return n;
    }
    /* %f: the digits down to 10^-prec, nd = e + 1 + prec of them */
    if (have) {
        nd = e + 1 + prec;              /* = %g's precision */
    } else if (v == 0) {
        nd = 0;
    } else {
        double m;
        e = float_exp(v, &m);           /* unrounded: rounding may add a digit */
        nd = e + 1 + prec;
        if (nd > 0) {
            int e2 = float_digits_n(v, nd, d);
            if (e2 > e)                 /* rounded up a decade: 99.9 -> 100.0 */
                d[nd++] = '0';
        } else if (nd == 0 && m > 5) {    /* 5 exactly: a tie, to the even 0 */
            d[0] = '1';                 /* rounds up to 10^-prec */
            nd = 1;
        } else {
            nd = 0;                     /* below half of 10^-prec: 0.000 */
        }
    }
    if (nd < prec + 1) {                /* leading zeros: 0.00ddd */
        int z = prec + 1 - nd;
        memmove(d + z, d, (size_t)nd);
        memset(d, '0', (size_t)z);
        nd += z;
    }
    for (i = 0; i < nd - prec; i++)
        out[n++] = d[i];
    if (prec || (flags & F_ALT))
        out[n++] = '.';
    memcpy(out + n, d + nd - prec, (size_t)prec);
    n += prec;
    if (strip && prec) {
        while (out[n - 1] == '0')
            n--;
        if (out[n - 1] == '.')
            n--;
    }
    return n;
}

static void put_float(Sink *k, double v, int conv, int prec, int flags, int width)
{
    char body[400];
    char sign = 0;
    int n, len;

    if (v < 0 || (v == 0 && 1 / v < 0)) {
        sign = '-';
        v = -v;
    } else if (flags & F_PLUS) {
        sign = '+';
    } else if (flags & F_SPACE) {
        sign = ' ';
    }
    n = float_body(body, v, conv, prec, flags);
    len = n + (sign != 0);
    if (body[0] > '9')                  /* inf / nan: no zero padding */
        flags &= ~F_ZERO;
    if (!(flags & (F_LEFT | F_ZERO)))
        sink_pad(k, ' ', width - len);
    if (sign)
        sink_put(k, &sign, 1);
    if ((flags & (F_LEFT | F_ZERO)) == F_ZERO)
        sink_pad(k, '0', width - len);
    sink_put(k, body, (size_t)n);
    if (flags & F_LEFT)
        sink_pad(k, ' ', width - len);
}

static void xfmt(Sink *k, const char *fmt, va_list ap)
{
    while (*fmt) {
        const char *s = fmt, *p;
        char spec[40], tmp[512];
        int flags = 0, width = 0, prec = -1, n, len = 0, sl;
        char conv;

        while (*fmt && *fmt != '%')
            fmt++;
        sink_put(k, s, (size_t)(fmt - s));
        if (!*fmt)
            break;
        s = fmt++;
        for (;; fmt++) {
            if (*fmt == '-') flags |= F_LEFT;
            else if (*fmt == '+') flags |= F_PLUS;
            else if (*fmt == ' ') flags |= F_SPACE;
            else if (*fmt == '#') flags |= F_ALT;
            else if (*fmt == '0') flags |= F_ZERO;
            else break;
        }
        if (*fmt == '*') {
            fmt++;
            width = va_arg(ap, int);
            if (width < 0) {
                flags |= F_LEFT;
                width = -width;
            }
        } else {
            while (*fmt >= '0' && *fmt <= '9')
                width = width * 10 + (*fmt++ - '0');
        }
        if (*fmt == '.') {
            fmt++;
            prec = 0;
            if (*fmt == '*') {
                fmt++;
                prec = va_arg(ap, int);
            } else {
                while (*fmt >= '0' && *fmt <= '9')
                    prec = prec * 10 + (*fmt++ - '0');
            }
        }
        p = fmt;                        /* length modifier */
        while (*fmt && strchr("hljztL", *fmt))
            fmt++;
        len = (int)(fmt - p);
        conv = *fmt;
        if (!conv)
            break;
        fmt++;
        /* the conversion for pdclib, width and precision written out */
        sl = 0;
        spec[sl++] = '%';
        if (flags & F_LEFT) spec[sl++] = '-';
        if (flags & F_PLUS) spec[sl++] = '+';
        if (flags & F_SPACE) spec[sl++] = ' ';
        if (flags & F_ALT) spec[sl++] = '#';
        if (flags & F_ZERO) spec[sl++] = '0';
        if (width > 400)
            width = 400;
        if (width)
            sl += sprintf(spec + sl, "%d", width);
        if (prec >= 0)
            sl += sprintf(spec + sl, ".%d", prec > 400 ? 400 : prec);
        memcpy(spec + sl, p, (size_t)len);
        sl += len;
        spec[sl++] = conv;
        spec[sl] = 0;
        n = -1;
        switch (conv) {
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
            put_float(k, len && p[0] == 'L' ? (double)va_arg(ap, long double) : va_arg(ap, double),
                      conv, prec, flags, width);
            continue;
        case 's': {                     /* strings can be long: no temporary */
            const char *str = va_arg(ap, const char *);
            size_t m;
            if (!str)
                str = "(null)";
            for (m = 0; str[m] && (prec < 0 || m < (size_t)prec); m++)
                ;
            if (!(flags & F_LEFT))
                sink_pad(k, ' ', width - (int)m);
            sink_put(k, str, m);
            if (flags & F_LEFT)
                sink_pad(k, ' ', width - (int)m);
            continue;
        }
        case '%':
            sink_put(k, "%", 1);
            continue;
        case 'n':
            *va_arg(ap, int *) = (int)k->total;
            continue;
        case 'c':
            n = snprintf(tmp, sizeof tmp, spec, va_arg(ap, int));
            break;
        case 'p':
            n = snprintf(tmp, sizeof tmp, spec, va_arg(ap, void *));
            break;
        case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
            if (len == 2 && p[0] == 'l')        /* ll */
                n = snprintf(tmp, sizeof tmp, spec, va_arg(ap, long long));
            else if (len == 1 && p[0] == 'j')
                n = snprintf(tmp, sizeof tmp, spec, va_arg(ap, long long));
            else if (len == 1 && (p[0] == 'l' || p[0] == 'z' || p[0] == 't'))
                n = snprintf(tmp, sizeof tmp, spec, va_arg(ap, long));
            else
                n = snprintf(tmp, sizeof tmp, spec, va_arg(ap, int));
            break;
        default:                        /* unknown: copy it as written */
            sink_put(k, s, (size_t)(fmt - s));
            continue;
        }
        if (n > (int)sizeof tmp - 1)
            n = (int)sizeof tmp - 1;
        if (n > 0)
            sink_put(k, tmp, (size_t)n);
    }
}

int xbox_vsnprintf(char *buf, size_t cap, const char *fmt, va_list ap)
{
    Sink k;
    k.buf = buf;
    k.cap = cap;
    k.f = NULL;
    k.total = 0;
    xfmt(&k, fmt, ap);
    if (cap)
        buf[k.total < cap ? k.total : cap - 1] = 0;
    return (int)k.total;
}

int xbox_vfprintf(FILE *f, const char *fmt, va_list ap)
{
    Sink k;
    k.f = f;
    k.nchunk = 0;
    k.total = 0;
    xfmt(&k, fmt, ap);
    if (k.nchunk)
        fwrite(k.chunk, 1, k.nchunk, f);
    return (int)k.total;
}

int xbox_vsprintf(char *buf, const char *fmt, va_list ap)
{
    return xbox_vsnprintf(buf, (size_t)-1 / 2, fmt, ap);
}

int xbox_vprintf(const char *fmt, va_list ap)
{
    return xbox_vfprintf(stdout, fmt, ap);
}

int xbox_snprintf(char *buf, size_t cap, const char *fmt, ...)
{
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = xbox_vsnprintf(buf, cap, fmt, ap);
    va_end(ap);
    return n;
}

int xbox_sprintf(char *buf, const char *fmt, ...)
{
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = xbox_vsprintf(buf, fmt, ap);
    va_end(ap);
    return n;
}

int xbox_fprintf(FILE *f, const char *fmt, ...)
{
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = xbox_vfprintf(f, fmt, ap);
    va_end(ap);
    return n;
}

int xbox_printf(const char *fmt, ...)
{
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = xbox_vfprintf(stdout, fmt, ap);
    va_end(ap);
    return n;
}

/* Data the game C names but gcc's PC build never references (gcc drops the
 * unused strchr(num_tbl, c) in hk_kbd_input_sub; nxdk's clang keeps the
 * call). The result is unused, so the contents do not matter. */
char num_tbl[16] = "0123456789";
