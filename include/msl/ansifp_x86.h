#ifndef MSL_ANSIFP_X86_H
#define MSL_ANSIFP_X86_H

struct decform {
    char style;
    char unused;
    short digits;
};
typedef struct decform decform;
struct decimal {
    char sgn;
    char unused;
    short exp;
    struct {
        unsigned char length;
        unsigned char text[32];
        unsigned char unused;
    } sig;
};
typedef struct decimal decimal;

#if MWCCPS2_B198
extern long double __scalenum(long double value, int exponent, int divide);
extern void __num2dec(const decform* format, double value, decimal* out);
extern double __dec2num(const decimal* in);

#endif
#endif
