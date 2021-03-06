/* blas/slamch.f -- translated by f2c (version 20050501).
   You must link the resulting object file with libf2c:
        on Microsoft Windows system, link with libf2c.lib;
        on Linux or Unix systems, link with .../path/to/libf2c.a -lm
        or, if you install libf2c.a in a standard place, with -lf2c -lm
        -- in that order, at the end of the command line, as in
                cc *.o -lf2c -lm
        Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

                http://www.netlib.org/f2c/libf2c.zip
*/

/* This code expects correct IEEE rounding behaviour which is not
   always provided.  The source should be built with -ffloat-store.
   A note from the GCC man page:

   -ffloat-store
    Do  not  store floating point variables in registers.  This pre-
    vents undesirable excess precision on machines such as the 68000
    where  the floating registers (of the 68881) keep more precision
    than a double is supposed to have.

    For most programs, the excess precision does only  good,  but  a
    few  programs  rely  on  the precise definition of IEEE floating
    point.  Use `-ffloat-store' for such programs.  */

/* Disable "global optimizations" to avoid optimizer bugs in this file.
   For GCC the file should be compiled with the -fno-gcse option.  Here
   is a note from the GCC man page:

    Note: When compiling a program using computed gotos, a GCC exten-
    sion, you may get better runtime performance if you disable the
    global common subexpression elimination pass by adding -fno-gcse to
    the command line.
*/
#ifdef _MSC_VER
# pragma optimize ("g",off)
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include "v3p_netlib.h"

#include <stdio.h>

/* Initialization function just calls the function once so that its
   runtime-initialized constants are initialized.  After the first
   call it is safe to call the function from multiple threads at
   once.  */
void v3p_netlib_slamch_init()
{
  slamch_(" ", 1);
}

/* Table of constant values */

static real c_b32 = (float)0.;

/*<       REAL             FUNCTION SLAMCH( CMACH ) >*/
doublereal slamch_(char *cmach, ftnlen cmach_len)
{
    /* Initialized data */

    static logical first = TRUE_; /* runtime-initialized constant */

    /* System generated locals */
    integer i__1;
    real ret_val;

    /* Builtin functions */
    double pow_ri(real *, integer *);

    /* Local variables */
    static real t; /* runtime-initialized constant */
    integer it;
    static real rnd, eps, base; /* runtime-initialized constant */
    integer beta;
    static real emin, prec, emax; /* runtime-initialized constant */
    integer imin, imax;
    logical lrnd;
    static real rmin, rmax; /* runtime-initialized constant */
    real rmach=0;
    extern logical lsame_(const char *, const char *, ftnlen, ftnlen);
    real small;
    static real sfmin; /* runtime-initialized constant */
    extern /* Subroutine */ int slamc2_(integer *, integer *, logical *, real
            *, integer *, real *, integer *, real *);
    (void)cmach_len;

/*  -- LAPACK auxiliary routine (version 1.1) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     October 31, 1992 */

/*     .. Scalar Arguments .. */
/*<       CHARACTER          CMACH >*/
/*     .. */

/*  Purpose */
/*  ======= */

/*  SLAMCH determines single precision machine parameters. */

/*  Arguments */
/*  ========= */

/*  CMACH   (input) CHARACTER*1 */
/*          Specifies the value to be returned by SLAMCH: */
/*          = 'E' or 'e',   SLAMCH := eps */
/*          = 'S' or 's ,   SLAMCH := sfmin */
/*          = 'B' or 'b',   SLAMCH := base */
/*          = 'P' or 'p',   SLAMCH := eps*base */
/*          = 'N' or 'n',   SLAMCH := t */
/*          = 'R' or 'r',   SLAMCH := rnd */
/*          = 'M' or 'm',   SLAMCH := emin */
/*          = 'U' or 'u',   SLAMCH := rmin */
/*          = 'L' or 'l',   SLAMCH := emax */
/*          = 'O' or 'o',   SLAMCH := rmax */

/*          where */

/*          eps   = relative machine precision */
/*          sfmin = safe minimum, such that 1/sfmin does not overflow */
/*          base  = base of the machine */
/*          prec  = eps*base */
/*          t     = number of (base) digits in the mantissa */
/*          rnd   = 1.0 when rounding occurs in addition, 0.0 otherwise */
/*          emin  = minimum exponent before (gradual) underflow */
/*          rmin  = underflow threshold - base**(emin-1) */
/*          emax  = largest exponent before overflow */
/*          rmax  = overflow threshold  - (base**emax)*(1-eps) */

/* ===================================================================== */

/*     .. Parameters .. */
/*<       REAL               ONE, ZERO >*/
/*<       PARAMETER          ( ONE = 1.0E+0, ZERO = 0.0E+0 ) >*/
/*     .. */
/*     .. Local Scalars .. */
/*<       LOGICAL            FIRST, LRND >*/
/*<       INTEGER            BETA, IMAX, IMIN, IT >*/
/*<    >*/
/*     .. */
/*     .. External Functions .. */
/*<       LOGICAL            LSAME >*/
/*<       EXTERNAL           LSAME >*/
/*     .. */
/*     .. External Subroutines .. */
/*<       EXTERNAL           SLAMC2 >*/
/*     .. */
/*     .. Save statement .. */
/*<    >*/
/*     .. */
/*     .. Data statements .. */
/*<       DATA               FIRST / .TRUE. / >*/
/*     .. */
/*     .. Executable Statements .. */

/*<       IF( FIRST ) THEN >*/
    if (first) {
/*<          FIRST = .FALSE. >*/
        first = FALSE_;
/*<          CALL SLAMC2( BETA, IT, LRND, EPS, IMIN, RMIN, IMAX, RMAX ) >*/
        slamc2_(&beta, &it, &lrnd, &eps, &imin, &rmin, &imax, &rmax);
/*<          BASE = BETA >*/
        base = (real) beta;
/*<          T = IT >*/
        t = (real) it;
/*<          IF( LRND ) THEN >*/
        if (lrnd) {
/*<             RND = ONE >*/
            rnd = (float)1.;
/*<             EPS = ( BASE**( 1-IT ) ) / 2 >*/
            i__1 = 1 - it;
            eps = pow_ri(&base, &i__1) / 2;
/*<          ELSE >*/
        } else {
/*<             RND = ZERO >*/
            rnd = (float)0.;
/*<             EPS = BASE**( 1-IT ) >*/
            i__1 = 1 - it;
            eps = pow_ri(&base, &i__1);
/*<          END IF >*/
        }
/*<          PREC = EPS*BASE >*/
        prec = eps * base;
/*<          EMIN = IMIN >*/
        emin = (real) imin;
/*<          EMAX = IMAX >*/
        emax = (real) imax;
/*<          SFMIN = RMIN >*/
        sfmin = rmin;
/*<          SMALL = ONE / RMAX >*/
        small = (float)1. / rmax;
/*<          IF( SMALL.GE.SFMIN ) THEN >*/
        if (small >= sfmin) {

/*           Use SMALL plus a bit, to avoid the possibility of rounding */
/*           causing overflow when computing  1/sfmin. */

/*<             SFMIN = SMALL*( ONE+EPS ) >*/
            sfmin = small * (eps + (float)1.);
/*<          END IF >*/
        }
/*<       END IF >*/
    }

/*<       IF( LSAME( CMACH, 'E' ) ) THEN >*/
    if (lsame_(cmach, "E", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = EPS >*/
        rmach = eps;
/*<       ELSE IF( LSAME( CMACH, 'S' ) ) THEN >*/
    } else if (lsame_(cmach, "S", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = SFMIN >*/
        rmach = sfmin;
/*<       ELSE IF( LSAME( CMACH, 'B' ) ) THEN >*/
    } else if (lsame_(cmach, "B", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = BASE >*/
        rmach = base;
/*<       ELSE IF( LSAME( CMACH, 'P' ) ) THEN >*/
    } else if (lsame_(cmach, "P", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = PREC >*/
        rmach = prec;
/*<       ELSE IF( LSAME( CMACH, 'N' ) ) THEN >*/
    } else if (lsame_(cmach, "N", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = T >*/
        rmach = t;
/*<       ELSE IF( LSAME( CMACH, 'R' ) ) THEN >*/
    } else if (lsame_(cmach, "R", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = RND >*/
        rmach = rnd;
/*<       ELSE IF( LSAME( CMACH, 'M' ) ) THEN >*/
    } else if (lsame_(cmach, "M", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = EMIN >*/
        rmach = emin;
/*<       ELSE IF( LSAME( CMACH, 'U' ) ) THEN >*/
    } else if (lsame_(cmach, "U", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = RMIN >*/
        rmach = rmin;
/*<       ELSE IF( LSAME( CMACH, 'L' ) ) THEN >*/
    } else if (lsame_(cmach, "L", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = EMAX >*/
        rmach = emax;
/*<       ELSE IF( LSAME( CMACH, 'O' ) ) THEN >*/
    } else if (lsame_(cmach, "O", (ftnlen)1, (ftnlen)1)) {
/*<          RMACH = RMAX >*/
        rmach = rmax;
/*<       END IF >*/
    }

/*<       SLAMCH = RMACH >*/
    ret_val = rmach;
/*<       RETURN >*/
    return ret_val;

/*     End of SLAMCH */

/*<       END >*/
} /* slamch_ */


/* *********************************************************************** */

/*<       SUBROUTINE SLAMC1( BETA, T, RND, IEEE1 ) >*/
/* Subroutine */ int slamc1_(integer *beta, integer *t, logical *rnd, logical
        *ieee1)
{
    /* Initialized data */

    static logical first = TRUE_; /* runtime-initialized constant */

    /* System generated locals */
    real r__1, r__2;

    /* Local variables */
    real a, b, c__, f, t1, t2;
    static integer lt; /* runtime-initialized constant */
    real one, qtr;
    static logical lrnd; /* runtime-initialized constant */
    static integer lbeta; /* runtime-initialized constant */
    real savec;
    static logical lieee1; /* runtime-initialized constant */
    extern doublereal slamc3_(real *, real *);


/*  -- LAPACK auxiliary routine (version 1.1) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     October 31, 1992 */

/*     .. Scalar Arguments .. */
/*<       LOGICAL            IEEE1, RND >*/
/*<       INTEGER            BETA, T >*/
/*     .. */

/*  Purpose */
/*  ======= */

/*  SLAMC1 determines the machine parameters given by BETA, T, RND, and */
/*  IEEE1. */

/*  Arguments */
/*  ========= */

/*  BETA    (output) INTEGER */
/*          The base of the machine. */

/*  T       (output) INTEGER */
/*          The number of ( BETA ) digits in the mantissa. */

/*  RND     (output) LOGICAL */
/*          Specifies whether proper rounding  ( RND = .TRUE. )  or */
/*          chopping  ( RND = .FALSE. )  occurs in addition. This may not */
/*          be a reliable guide to the way in which the machine performs */
/*          its arithmetic. */

/*  IEEE1   (output) LOGICAL */
/*          Specifies whether rounding appears to be done in the IEEE */
/*          'round to nearest' style. */

/*  Further Details */
/*  =============== */

/*  The routine is based on the routine  ENVRON  by Malcolm and */
/*  incorporates suggestions by Gentleman and Marovich. See */

/*     Malcolm M. A. (1972) Algorithms to reveal properties of */
/*        floating-point arithmetic. Comms. of the ACM, 15, 949-951. */

/*     Gentleman W. M. and Marovich S. B. (1974) More on algorithms */
/*        that reveal properties of floating point arithmetic units. */
/*        Comms. of the ACM, 17, 276-277. */

/* ===================================================================== */

/*     .. Local Scalars .. */
/*<       LOGICAL            FIRST, LIEEE1, LRND >*/
/*<       INTEGER            LBETA, LT >*/
/*<       REAL               A, B, C, F, ONE, QTR, SAVEC, T1, T2 >*/
/*     .. */
/*     .. External Functions .. */
/*<       REAL               SLAMC3 >*/
/*<       EXTERNAL           SLAMC3 >*/
/*     .. */
/*     .. Save statement .. */
/*<       SAVE               FIRST, LIEEE1, LBETA, LRND, LT >*/
/*     .. */
/*     .. Data statements .. */
/*<       DATA               FIRST / .TRUE. / >*/
/*     .. */
/*     .. Executable Statements .. */

/*<       IF( FIRST ) THEN >*/
    if (first) {
/*<          FIRST = .FALSE. >*/
        first = FALSE_;
/*<          ONE = 1 >*/
        one = (float)1.;

/*        LBETA,  LIEEE1,  LT and  LRND  are the  local values  of  BETA, */
/*        IEEE1, T and RND. */

/*        Throughout this routine  we use the function  SLAMC3  to ensure */
/*        that relevant values are  stored and not held in registers,  or */
/*        are not affected by optimizers. */

/*        Compute  a = 2.0**m  with the  smallest positive integer m such */
/*        that */

/*           fl( a + 1.0 ) = a. */

/*<          A = 1 >*/
        a = (float)1.;
/*<          C = 1 >*/
        c__ = (float)1.;

/* +       WHILE( C.EQ.ONE )LOOP */
/*<    10    CONTINUE >*/
L10:
/*<          IF( C.EQ.ONE ) THEN >*/
        if (c__ == one) {
/*<             A = 2*A >*/
            a *= 2;
/*<             C = SLAMC3( A, ONE ) >*/
            c__ = slamc3_(&a, &one);
/*<             C = SLAMC3( C, -A ) >*/
            r__1 = -a;
            c__ = slamc3_(&c__, &r__1);
/*<             GO TO 10 >*/
            goto L10;
/*<          END IF >*/
        }
/* +       END WHILE */

/*        Now compute  b = 2.0**m  with the smallest positive integer m */
/*        such that */

/*           fl( a + b ) .gt. a. */

/*<          B = 1 >*/
        b = (float)1.;
/*<          C = SLAMC3( A, B ) >*/
        c__ = slamc3_(&a, &b);

/* +       WHILE( C.EQ.A )LOOP */
/*<    20    CONTINUE >*/
L20:
/*<          IF( C.EQ.A ) THEN >*/
        if (c__ == a) {
/*<             B = 2*B >*/
            b *= 2;
/*<             C = SLAMC3( A, B ) >*/
            c__ = slamc3_(&a, &b);
/*<             GO TO 20 >*/
            goto L20;
/*<          END IF >*/
        }
/* +       END WHILE */

/*        Now compute the base.  a and c  are neighbouring floating point */
/*        numbers  in the  interval  ( beta**t, beta**( t + 1 ) )  and so */
/*        their difference is beta. Adding 0.25 to c is to ensure that it */
/*        is truncated to beta and not ( beta - 1 ). */

/*<          QTR = ONE / 4 >*/
        qtr = one / 4;
/*<          SAVEC = C >*/
        savec = c__;
/*<          C = SLAMC3( C, -A ) >*/
        r__1 = -a;
        c__ = slamc3_(&c__, &r__1);
/*<          LBETA = C + QTR >*/
        lbeta = c__ + qtr;

/*        Now determine whether rounding or chopping occurs,  by adding a */
/*        bit  less  than  beta/2  and a  bit  more  than  beta/2  to  a. */

/*<          B = LBETA >*/
        b = (real) lbeta;
/*<          F = SLAMC3( B / 2, -B / 100 ) >*/
        r__1 = b / 2;
        r__2 = -b / 100;
        f = slamc3_(&r__1, &r__2);
/*<          C = SLAMC3( F, A ) >*/
        c__ = slamc3_(&f, &a);
/*<          IF( C.EQ.A ) THEN >*/
        if (c__ == a) {
/*<             LRND = .TRUE. >*/
            lrnd = TRUE_;
/*<          ELSE >*/
        } else {
/*<             LRND = .FALSE. >*/
            lrnd = FALSE_;
/*<          END IF >*/
        }
/*<          F = SLAMC3( B / 2, B / 100 ) >*/
        r__1 = b / 2;
        r__2 = b / 100;
        f = slamc3_(&r__1, &r__2);
/*<          C = SLAMC3( F, A ) >*/
        c__ = slamc3_(&f, &a);
/*<    >*/
        if (lrnd && c__ == a) {
            lrnd = FALSE_;
        }

/*        Try and decide whether rounding is done in the  IEEE  'round to */
/*        nearest' style. B/2 is half a unit in the last place of the two */
/*        numbers A and SAVEC. Furthermore, A is even, i.e. has last  bit */
/*        zero, and SAVEC is odd. Thus adding B/2 to A should not  change */
/*        A, but adding B/2 to SAVEC should change SAVEC. */

/*<          T1 = SLAMC3( B / 2, A ) >*/
        r__1 = b / 2;
        t1 = slamc3_(&r__1, &a);
/*<          T2 = SLAMC3( B / 2, SAVEC ) >*/
        r__1 = b / 2;
        t2 = slamc3_(&r__1, &savec);
/*<          LIEEE1 = ( T1.EQ.A ) .AND. ( T2.GT.SAVEC ) .AND. LRND >*/
        lieee1 = t1 == a && t2 > savec && lrnd;

/*        Now find  the  mantissa, t.  It should  be the  integer part of */
/*        log to the base beta of a,  however it is safer to determine  t */
/*        by powering.  So we find t as the smallest positive integer for */
/*        which */

/*           fl( beta**t + 1.0 ) = 1.0. */

/*<          LT = 0 >*/
        lt = 0;
/*<          A = 1 >*/
        a = (float)1.;
/*<          C = 1 >*/
        c__ = (float)1.;

/* +       WHILE( C.EQ.ONE )LOOP */
/*<    30    CONTINUE >*/
L30:
/*<          IF( C.EQ.ONE ) THEN >*/
        if (c__ == one) {
/*<             LT = LT + 1 >*/
            ++lt;
/*<             A = A*LBETA >*/
            a *= lbeta;
/*<             C = SLAMC3( A, ONE ) >*/
            c__ = slamc3_(&a, &one);
/*<             C = SLAMC3( C, -A ) >*/
            r__1 = -a;
            c__ = slamc3_(&c__, &r__1);
/*<             GO TO 30 >*/
            goto L30;
/*<          END IF >*/
        }
/* +       END WHILE */

/*<       END IF >*/
    }

/*<       BETA = LBETA >*/
    *beta = lbeta;
/*<       T = LT >*/
    *t = lt;
/*<       RND = LRND >*/
    *rnd = lrnd;
/*<       IEEE1 = LIEEE1 >*/
    *ieee1 = lieee1;
/*<       RETURN >*/
    return 0;

/*     End of SLAMC1 */

/*<       END >*/
} /* slamc1_ */


/* *********************************************************************** */

/*<       SUBROUTINE SLAMC2( BETA, T, RND, EPS, EMIN, RMIN, EMAX, RMAX ) >*/
/* Subroutine */ int slamc2_(integer *beta, integer *t, logical *rnd, real *
        eps, integer *emin, real *rmin, integer *emax, real *rmax)
{
    /* Initialized data */

    static logical first = TRUE_; /* runtime-initialized constant */
    static logical iwarn = FALSE_; /* runtime-initialized constant */

    /* System generated locals */
    integer i__1;
    real r__1, r__2, r__3, r__4, r__5;

    /* Builtin functions */
    double pow_ri(real *, integer *);

    /* Local variables */
    real a, b, c__;
    integer i__;
    static integer lt; /* runtime-initialized constant */
    real one, two;
    logical ieee;
    real half;
    logical lrnd = 0; //variable 'lrnd' is used uninitialized whenever 'if' condition is false [-Wsometimes-uninitialized]
    static real leps; /* runtime-initialized constant */
    real zero;
    static integer lbeta; /* runtime-initialized constant */
    real rbase;
    static integer lemin, lemax; /* runtime-initialized constant */
    integer gnmin;
    real small;
    integer gpmin;
    real third;
    static real lrmin, lrmax; /* runtime-initialized constant */
    real sixth;
    logical lieee1;
    extern /* Subroutine */ int slamc1_(integer *, integer *, logical *,
            logical *);
    extern doublereal slamc3_(real *, real *);
    extern /* Subroutine */ int slamc4_(integer *, real *, integer *),
            slamc5_(integer *, integer *, integer *, logical *, integer *,
            real *);
    integer ngnmin, ngpmin;

/*  -- LAPACK auxiliary routine (version 1.1) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     October 31, 1992 */

/*     .. Scalar Arguments .. */
/*<       LOGICAL            RND >*/
/*<       INTEGER            BETA, EMAX, EMIN, T >*/
/*<       REAL               EPS, RMAX, RMIN >*/
/*     .. */

/*  Purpose */
/*  ======= */

/*  SLAMC2 determines the machine parameters specified in its argument */
/*  list. */

/*  Arguments */
/*  ========= */

/*  BETA    (output) INTEGER */
/*          The base of the machine. */

/*  T       (output) INTEGER */
/*          The number of ( BETA ) digits in the mantissa. */

/*  RND     (output) LOGICAL */
/*          Specifies whether proper rounding  ( RND = .TRUE. )  or */
/*          chopping  ( RND = .FALSE. )  occurs in addition. This may not */
/*          be a reliable guide to the way in which the machine performs */
/*          its arithmetic. */

/*  EPS     (output) REAL */
/*          The smallest positive number such that */

/*             fl( 1.0 - EPS ) .LT. 1.0, */

/*          where fl denotes the computed value. */

/*  EMIN    (output) INTEGER */
/*          The minimum exponent before (gradual) underflow occurs. */

/*  RMIN    (output) REAL */
/*          The smallest normalized number for the machine, given by */
/*          BASE**( EMIN - 1 ), where  BASE  is the floating point value */
/*          of BETA. */

/*  EMAX    (output) INTEGER */
/*          The maximum exponent before overflow occurs. */

/*  RMAX    (output) REAL */
/*          The largest positive number for the machine, given by */
/*          BASE**EMAX * ( 1 - EPS ), where  BASE  is the floating point */
/*          value of BETA. */

/*  Further Details */
/*  =============== */

/*  The computation of  EPS  is based on a routine PARANOIA by */
/*  W. Kahan of the University of California at Berkeley. */

/* ===================================================================== */

/*     .. Local Scalars .. */
/*<       LOGICAL            FIRST, IEEE, IWARN, LIEEE1, LRND >*/
/*<    >*/
/*<    >*/
/*     .. */
/*     .. External Functions .. */
/*<       REAL               SLAMC3 >*/
/*<       EXTERNAL           SLAMC3 >*/
/*     .. */
/*     .. External Subroutines .. */
/*<       EXTERNAL           SLAMC1, SLAMC4, SLAMC5 >*/
/*     .. */
/*     .. Intrinsic Functions .. */
/*<       INTRINSIC          ABS, MAX, MIN >*/
/*     .. */
/*     .. Save statement .. */
/*<    >*/
/*     .. */
/*     .. Data statements .. */
/*<       DATA               FIRST / .TRUE. / , IWARN / .FALSE. / >*/
/*     .. */
/*     .. Executable Statements .. */

/*<       IF( FIRST ) THEN >*/
    if (first) {
/*<          FIRST = .FALSE. >*/
        first = FALSE_;
/*<          ZERO = 0 >*/
        zero = (float)0.;
/*<          ONE = 1 >*/
        one = (float)1.;
/*<          TWO = 2 >*/
        two = (float)2.;

/*        LBETA, LT, LRND, LEPS, LEMIN and LRMIN  are the local values of */
/*        BETA, T, RND, EPS, EMIN and RMIN. */

/*        Throughout this routine  we use the function  SLAMC3  to ensure */
/*        that relevant values are stored  and not held in registers,  or */
/*        are not affected by optimizers. */

/*        SLAMC1 returns the parameters  LBETA, LT, LRND and LIEEE1. */

/*<          CALL SLAMC1( LBETA, LT, LRND, LIEEE1 ) >*/
        slamc1_(&lbeta, &lt, &lrnd, &lieee1);

/*        Start to find EPS. */

/*<          B = LBETA >*/
        b = (real) lbeta;
/*<          A = B**( -LT ) >*/
        i__1 = -lt;
        a = pow_ri(&b, &i__1);
/*<          LEPS = A >*/
        leps = a;

/*        Try some tricks to see whether or not this is the correct  EPS. */

/*<          B = TWO / 3 >*/
        b = two / 3;
/*<          HALF = ONE / 2 >*/
        half = one / 2;
/*<          SIXTH = SLAMC3( B, -HALF ) >*/
        r__1 = -half;
        sixth = slamc3_(&b, &r__1);
/*<          THIRD = SLAMC3( SIXTH, SIXTH ) >*/
        third = slamc3_(&sixth, &sixth);
/*<          B = SLAMC3( THIRD, -HALF ) >*/
        r__1 = -half;
        b = slamc3_(&third, &r__1);
/*<          B = SLAMC3( B, SIXTH ) >*/
        b = slamc3_(&b, &sixth);
/*<          B = ABS( B ) >*/
        b = dabs(b);
/*<    >*/
        if (b < leps) {
            b = leps;
        }

/*<          LEPS = 1 >*/
        leps = (float)1.;

/* +       WHILE( ( LEPS.GT.B ).AND.( B.GT.ZERO ) )LOOP */
/*<    10    CONTINUE >*/
L10:
/*<          IF( ( LEPS.GT.B ) .AND. ( B.GT.ZERO ) ) THEN >*/
        if (leps > b && b > zero) {
/*<             LEPS = B >*/
            leps = b;
/*<             C = SLAMC3( HALF*LEPS, ( TWO**5 )*( LEPS**2 ) ) >*/
            r__1 = half * leps;
/* Computing 5th power */
            r__3 = two, r__4 = r__3, r__3 *= r__3;
/* Computing 2nd power */
            r__5 = leps;
            r__2 = r__4 * (r__3 * r__3) * (r__5 * r__5);
            c__ = slamc3_(&r__1, &r__2);
/*<             C = SLAMC3( HALF, -C ) >*/
            r__1 = -c__;
            c__ = slamc3_(&half, &r__1);
/*<             B = SLAMC3( HALF, C ) >*/
            b = slamc3_(&half, &c__);
/*<             C = SLAMC3( HALF, -B ) >*/
            r__1 = -b;
            c__ = slamc3_(&half, &r__1);
/*<             B = SLAMC3( HALF, C ) >*/
            b = slamc3_(&half, &c__);
/*<             GO TO 10 >*/
            goto L10;
/*<          END IF >*/
        }
/* +       END WHILE */

/*<    >*/
        if (a < leps) {
            leps = a;
        }

/*        Computation of EPS complete. */

/*        Now find  EMIN.  Let A = + or - 1, and + or - (1 + BASE**(-3)). */
/*        Keep dividing  A by BETA until (gradual) underflow occurs. This */
/*        is detected when we cannot recover the previous A. */

/*<          RBASE = ONE / LBETA >*/
        rbase = one / lbeta;
/*<          SMALL = ONE >*/
        small = one;
/*<          DO 20 I = 1, 3 >*/
        for (i__ = 1; i__ <= 3; ++i__) {
/*<             SMALL = SLAMC3( SMALL*RBASE, ZERO ) >*/
            r__1 = small * rbase;
            small = slamc3_(&r__1, &zero);
/*<    20    CONTINUE >*/
/* L20: */
        }
/*<          A = SLAMC3( ONE, SMALL ) >*/
        a = slamc3_(&one, &small);
/*<          CALL SLAMC4( NGPMIN, ONE, LBETA ) >*/
        slamc4_(&ngpmin, &one, &lbeta);
/*<          CALL SLAMC4( NGNMIN, -ONE, LBETA ) >*/
        r__1 = -one;
        slamc4_(&ngnmin, &r__1, &lbeta);
/*<          CALL SLAMC4( GPMIN, A, LBETA ) >*/
        slamc4_(&gpmin, &a, &lbeta);
/*<          CALL SLAMC4( GNMIN, -A, LBETA ) >*/
        r__1 = -a;
        slamc4_(&gnmin, &r__1, &lbeta);
/*<          IEEE = .FALSE. >*/
        ieee = FALSE_;

/*<          IF( ( NGPMIN.EQ.NGNMIN ) .AND. ( GPMIN.EQ.GNMIN ) ) THEN >*/
        if (ngpmin == ngnmin && gpmin == gnmin) {
/*<             IF( NGPMIN.EQ.GPMIN ) THEN >*/
            if (ngpmin == gpmin) {
/*<                LEMIN = NGPMIN >*/
                lemin = ngpmin;
/*            ( Non twos-complement machines, no gradual underflow; */
/*              e.g.,  VAX ) */
/*<             ELSE IF( ( GPMIN-NGPMIN ).EQ.3 ) THEN >*/
            } else if (gpmin - ngpmin == 3) {
/*<                LEMIN = NGPMIN - 1 + LT >*/
                lemin = ngpmin - 1 + lt;
/*<                IEEE = .TRUE. >*/
                ieee = TRUE_;
/*            ( Non twos-complement machines, with gradual underflow; */
/*              e.g., IEEE standard followers ) */
/*<             ELSE >*/
            } else {
/*<                LEMIN = MIN( NGPMIN, GPMIN ) >*/
                lemin = min(ngpmin,gpmin);
/*            ( A guess; no known machine ) */
/*<                IWARN = .TRUE. >*/
                iwarn = TRUE_;
/*<             END IF >*/
            }

/*<          ELSE IF( ( NGPMIN.EQ.GPMIN ) .AND. ( NGNMIN.EQ.GNMIN ) ) THEN >*/
        } else if (ngpmin == gpmin && ngnmin == gnmin) {
/*<             IF( ABS( NGPMIN-NGNMIN ).EQ.1 ) THEN >*/
            if ((i__1 = ngpmin - ngnmin, abs(i__1)) == 1) {
/*<                LEMIN = MAX( NGPMIN, NGNMIN ) >*/
                lemin = max(ngpmin,ngnmin);
/*            ( Twos-complement machines, no gradual underflow; */
/*              e.g., CYBER 205 ) */
/*<             ELSE >*/
            } else {
/*<                LEMIN = MIN( NGPMIN, NGNMIN ) >*/
                lemin = min(ngpmin,ngnmin);
/*            ( A guess; no known machine ) */
/*<                IWARN = .TRUE. >*/
                iwarn = TRUE_;
/*<             END IF >*/
            }

/*<    >*/
        } else if ((i__1 = ngpmin - ngnmin, abs(i__1)) == 1 && gpmin == gnmin)
                 {
/*<             IF( ( GPMIN-MIN( NGPMIN, NGNMIN ) ).EQ.3 ) THEN >*/
            if (gpmin - min(ngpmin,ngnmin) == 3) {
/*<                LEMIN = MAX( NGPMIN, NGNMIN ) - 1 + LT >*/
                lemin = max(ngpmin,ngnmin) - 1 + lt;
/*            ( Twos-complement machines with gradual underflow; */
/*              no known machine ) */
/*<             ELSE >*/
            } else {
/*<                LEMIN = MIN( NGPMIN, NGNMIN ) >*/
                lemin = min(ngpmin,ngnmin);
/*            ( A guess; no known machine ) */
/*<                IWARN = .TRUE. >*/
                iwarn = TRUE_;
/*<             END IF >*/
            }

/*<          ELSE >*/
        } else {
/*<             LEMIN = MIN( NGPMIN, NGNMIN, GPMIN, GNMIN ) >*/
/* Computing MIN */
            i__1 = min(ngpmin,ngnmin), i__1 = min(i__1,gpmin);
            lemin = min(i__1,gnmin);
/*         ( A guess; no known machine ) */
/*<             IWARN = .TRUE. >*/
            iwarn = TRUE_;
/*<          END IF >*/
        }
/* ** */
/* Comment out this if block if EMIN is ok */
/*<          IF( IWARN ) THEN >*/
        if (iwarn) {
/*<             FIRST = .TRUE. >*/
            first = TRUE_;
/*<             WRITE( 6, FMT = 9999 )LEMIN >*/
            printf("\n\n WARNING. The value EMIN may be incorrect: - ");
            printf("EMIN = %8li\n",lemin);
            printf("If, after inspection, the value EMIN looks acceptable");
            printf(" please comment out\n the IF block as marked within the");
            printf(" code of routine SLAMC2,\n otherwise supply EMIN");
            printf(" explicitly.\n");
/*<          END IF >*/
        }
/* ** */

/*        Assume IEEE arithmetic if we found denormalised  numbers above, */
/*        or if arithmetic seems to round in the  IEEE style,  determined */
/*        in routine SLAMC1. A true IEEE machine should have both  things */
/*        true; however, faulty machines may have one or the other. */

/*<          IEEE = IEEE .OR. LIEEE1 >*/
        ieee = ieee || lieee1;

/*        Compute  RMIN by successive division by  BETA. We could compute */
/*        RMIN as BASE**( EMIN - 1 ),  but some machines underflow during */
/*        this computation. */

/*<          LRMIN = 1 >*/
        lrmin = (float)1.;
/*<          DO 30 I = 1, 1 - LEMIN >*/
        i__1 = 1 - lemin;
        for (i__ = 1; i__ <= i__1; ++i__) {
/*<             LRMIN = SLAMC3( LRMIN*RBASE, ZERO ) >*/
            r__1 = lrmin * rbase;
            lrmin = slamc3_(&r__1, &zero);
/*<    30    CONTINUE >*/
/* L30: */
        }

/*        Finally, call SLAMC5 to compute EMAX and RMAX. */

/*<          CALL SLAMC5( LBETA, LT, LEMIN, IEEE, LEMAX, LRMAX ) >*/
        slamc5_(&lbeta, &lt, &lemin, &ieee, &lemax, &lrmax);
/*<       END IF >*/
    }

/*<       BETA = LBETA >*/
    *beta = lbeta;
/*<       T = LT >*/
    *t = lt;
/*<       RND = LRND >*/
    *rnd = lrnd;
/*<       EPS = LEPS >*/
    *eps = leps;
/*<       EMIN = LEMIN >*/
    *emin = lemin;
/*<       RMIN = LRMIN >*/
    *rmin = lrmin;
/*<       EMAX = LEMAX >*/
    *emax = lemax;
/*<       RMAX = LRMAX >*/
    *rmax = lrmax;

/*<       RETURN >*/
    return 0;

/*<  9 >*/

/*     End of SLAMC2 */

/*<       END >*/
} /* slamc2_ */


/* *********************************************************************** */

/*<       REAL             FUNCTION SLAMC3( A, B ) >*/
doublereal slamc3_(real *a, real *b)
{
    /* System generated locals */
    real ret_val;


/*  -- LAPACK auxiliary routine (version 1.1) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     October 31, 1992 */

/*     .. Scalar Arguments .. */
/*<       REAL               A, B >*/
/*     .. */

/*  Purpose */
/*  ======= */

/*  SLAMC3  is intended to force  A  and  B  to be stored prior to doing */
/*  the addition of  A  and  B ,  for use in situations where optimizers */
/*  might hold one of these in a register. */

/*  Arguments */
/*  ========= */

/*  A, B    (input) REAL */
/*          The values A and B. */

/* ===================================================================== */

/*     .. Executable Statements .. */

/*<       SLAMC3 = A + B >*/
    ret_val = *a + *b;

/*<       RETURN >*/
    return ret_val;

/*     End of SLAMC3 */

/*<       END >*/
} /* slamc3_ */


/* *********************************************************************** */

/*<       SUBROUTINE SLAMC4( EMIN, START, BASE ) >*/
/* Subroutine */ int slamc4_(integer *emin, real *start, integer *base)
{
    /* System generated locals */
    integer i__1;
    real r__1;

    /* Local variables */
    real a;
    integer i__;
    real b1, b2, c1, c2, d1, d2, one, zero, rbase;
    extern doublereal slamc3_(real *, real *);


/*  -- LAPACK auxiliary routine (version 1.1) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     October 31, 1992 */

/*     .. Scalar Arguments .. */
/*<       INTEGER            BASE, EMIN >*/
/*<       REAL               START >*/
/*     .. */

/*  Purpose */
/*  ======= */

/*  SLAMC4 is a service routine for SLAMC2. */

/*  Arguments */
/*  ========= */

/*  EMIN    (output) EMIN */
/*          The minimum exponent before (gradual) underflow, computed by */
/*          setting A = START and dividing by BASE until the previous A */
/*          can not be recovered. */

/*  START   (input) REAL */
/*          The starting point for determining EMIN. */

/*  BASE    (input) INTEGER */
/*          The base of the machine. */

/* ===================================================================== */

/*     .. Local Scalars .. */
/*<       INTEGER            I >*/
/*<       REAL               A, B1, B2, C1, C2, D1, D2, ONE, RBASE, ZERO >*/
/*     .. */
/*     .. External Functions .. */
/*<       REAL               SLAMC3 >*/
/*<       EXTERNAL           SLAMC3 >*/
/*     .. */
/*     .. Executable Statements .. */

/*<       A = START >*/
    a = *start;
/*<       ONE = 1 >*/
    one = (float)1.;
/*<       RBASE = ONE / BASE >*/
    rbase = one / *base;
/*<       ZERO = 0 >*/
    zero = (float)0.;
/*<       EMIN = 1 >*/
    *emin = 1;
/*<       B1 = SLAMC3( A*RBASE, ZERO ) >*/
    r__1 = a * rbase;
    b1 = slamc3_(&r__1, &zero);
/*<       C1 = A >*/
    c1 = a;
/*<       C2 = A >*/
    c2 = a;
/*<       D1 = A >*/
    d1 = a;
/*<       D2 = A >*/
    d2 = a;
/* +    WHILE( ( C1.EQ.A ).AND.( C2.EQ.A ).AND. */
/*    $       ( D1.EQ.A ).AND.( D2.EQ.A )      )LOOP */
/*<    10 CONTINUE >*/
L10:
/*<    >*/
    if (c1 == a && c2 == a && d1 == a && d2 == a) {
/*<          EMIN = EMIN - 1 >*/
        --(*emin);
/*<          A = B1 >*/
        a = b1;
/*<          B1 = SLAMC3( A / BASE, ZERO ) >*/
        r__1 = a / *base;
        b1 = slamc3_(&r__1, &zero);
/*<          C1 = SLAMC3( B1*BASE, ZERO ) >*/
        r__1 = b1 * *base;
        c1 = slamc3_(&r__1, &zero);
/*<          D1 = ZERO >*/
        d1 = zero;
/*<          DO 20 I = 1, BASE >*/
        i__1 = *base;
        for (i__ = 1; i__ <= i__1; ++i__) {
/*<             D1 = D1 + B1 >*/
            d1 += b1;
/*<    20    CONTINUE >*/
/* L20: */
        }
/*<          B2 = SLAMC3( A*RBASE, ZERO ) >*/
        r__1 = a * rbase;
        b2 = slamc3_(&r__1, &zero);
/*<          C2 = SLAMC3( B2 / RBASE, ZERO ) >*/
        r__1 = b2 / rbase;
        c2 = slamc3_(&r__1, &zero);
/*<          D2 = ZERO >*/
        d2 = zero;
/*<          DO 30 I = 1, BASE >*/
        i__1 = *base;
        for (i__ = 1; i__ <= i__1; ++i__) {
/*<             D2 = D2 + B2 >*/
            d2 += b2;
/*<    30    CONTINUE >*/
/* L30: */
        }
/*<          GO TO 10 >*/
        goto L10;
/*<       END IF >*/
    }
/* +    END WHILE */

/*<       RETURN >*/
    return 0;

/*     End of SLAMC4 */

/*<       END >*/
} /* slamc4_ */


/* *********************************************************************** */

/*<       SUBROUTINE SLAMC5( BETA, P, EMIN, IEEE, EMAX, RMAX ) >*/
/* Subroutine */ int slamc5_(integer *beta, integer *p, integer *emin,
        logical *ieee, integer *emax, real *rmax)
{
    /* System generated locals */
    integer i__1;
    real r__1;

    /* Local variables */
    integer i__;
    real y, z__;
    integer try__, lexp;
    real oldy=0;
    integer uexp, nbits;
    extern doublereal slamc3_(real *, real *);
    real recbas;
    integer exbits, expsum;


/*  -- LAPACK auxiliary routine (version 1.1) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     October 31, 1992 */

/*     .. Scalar Arguments .. */
/*<       LOGICAL            IEEE >*/
/*<       INTEGER            BETA, EMAX, EMIN, P >*/
/*<       REAL               RMAX >*/
/*     .. */

/*  Purpose */
/*  ======= */

/*  SLAMC5 attempts to compute RMAX, the largest machine floating-point */
/*  number, without overflow.  It assumes that EMAX + abs(EMIN) sum */
/*  approximately to a power of 2.  It will fail on machines where this */
/*  assumption does not hold, for example, the Cyber 205 (EMIN = -28625, */
/*  EMAX = 28718).  It will also fail if the value supplied for EMIN is */
/*  too large (i.e. too close to zero), probably with overflow. */

/*  Arguments */
/*  ========= */

/*  BETA    (input) INTEGER */
/*          The base of floating-point arithmetic. */

/*  P       (input) INTEGER */
/*          The number of base BETA digits in the mantissa of a */
/*          floating-point value. */

/*  EMIN    (input) INTEGER */
/*          The minimum exponent before (gradual) underflow. */

/*  IEEE    (input) LOGICAL */
/*          A logical flag specifying whether or not the arithmetic */
/*          system is thought to comply with the IEEE standard. */

/*  EMAX    (output) INTEGER */
/*          The largest exponent before overflow */

/*  RMAX    (output) REAL */
/*          The largest machine floating-point number. */

/* ===================================================================== */

/*     .. Parameters .. */
/*<       REAL               ZERO, ONE >*/
/*<       PARAMETER          ( ZERO = 0.0E0, ONE = 1.0E0 ) >*/
/*     .. */
/*     .. Local Scalars .. */
/*<       INTEGER            EXBITS, EXPSUM, I, LEXP, NBITS, TRY, UEXP >*/
/*<       REAL               OLDY, RECBAS, Y, Z >*/
/*     .. */
/*     .. External Functions .. */
/*<       REAL               SLAMC3 >*/
/*<       EXTERNAL           SLAMC3 >*/
/*     .. */
/*     .. Intrinsic Functions .. */
/*<       INTRINSIC          MOD >*/
/*     .. */
/*     .. Executable Statements .. */

/*     First compute LEXP and UEXP, two powers of 2 that bound */
/*     abs(EMIN). We then assume that EMAX + abs(EMIN) will sum */
/*     approximately to the bound that is closest to abs(EMIN). */
/*     (EMAX is the exponent of the required number RMAX). */

/*<       LEXP = 1 >*/
    lexp = 1;
/*<       EXBITS = 1 >*/
    exbits = 1;
/*<    10 CONTINUE >*/
L10:
/*<       TRY = LEXP*2 >*/
    try__ = lexp << 1;
/*<       IF( TRY.LE.( -EMIN ) ) THEN >*/
    if (try__ <= -(*emin)) {
/*<          LEXP = TRY >*/
        lexp = try__;
/*<          EXBITS = EXBITS + 1 >*/
        ++exbits;
/*<          GO TO 10 >*/
        goto L10;
/*<       END IF >*/
    }
/*<       IF( LEXP.EQ.-EMIN ) THEN >*/
    if (lexp == -(*emin)) {
/*<          UEXP = LEXP >*/
        uexp = lexp;
/*<       ELSE >*/
    } else {
/*<          UEXP = TRY >*/
        uexp = try__;
/*<          EXBITS = EXBITS + 1 >*/
        ++exbits;
/*<       END IF >*/
    }

/*     Now -LEXP is less than or equal to EMIN, and -UEXP is greater */
/*     than or equal to EMIN. EXBITS is the number of bits needed to */
/*     store the exponent. */

/*<       IF( ( UEXP+EMIN ).GT.( -LEXP-EMIN ) ) THEN >*/
    if (uexp + *emin > -lexp - *emin) {
/*<          EXPSUM = 2*LEXP >*/
        expsum = lexp << 1;
/*<       ELSE >*/
    } else {
/*<          EXPSUM = 2*UEXP >*/
        expsum = uexp << 1;
/*<       END IF >*/
    }

/*     EXPSUM is the exponent range, approximately equal to */
/*     EMAX - EMIN + 1 . */

/*<       EMAX = EXPSUM + EMIN - 1 >*/
    *emax = expsum + *emin - 1;
/*<       NBITS = 1 + EXBITS + P >*/
    nbits = exbits + 1 + *p;

/*     NBITS is the total number of bits needed to store a */
/*     floating-point number. */

/*<       IF( ( MOD( NBITS, 2 ).EQ.1 ) .AND. ( BETA.EQ.2 ) ) THEN >*/
    if (nbits % 2 == 1 && *beta == 2) {

/*        Either there are an odd number of bits used to store a */
/*        floating-point number, which is unlikely, or some bits are */
/*        not used in the representation of numbers, which is possible, */
/*        (e.g. Cray machines) or the mantissa has an implicit bit, */
/*        (e.g. IEEE machines, Dec Vax machines), which is perhaps the */
/*        most likely. We have to assume the last alternative. */
/*        If this is true, then we need to reduce EMAX by one because */
/*        there must be some way of representing zero in an implicit-bit */
/*        system. On machines like Cray, we are reducing EMAX by one */
/*        unnecessarily. */

/*<          EMAX = EMAX - 1 >*/
        --(*emax);
/*<       END IF >*/
    }

/*<       IF( IEEE ) THEN >*/
    if (*ieee) {

/*        Assume we are on an IEEE machine which reserves one exponent */
/*        for infinity and NaN. */

/*<          EMAX = EMAX - 1 >*/
        --(*emax);
/*<       END IF >*/
    }

/*     Now create RMAX, the largest machine number, which should */
/*     be equal to (1.0 - BETA**(-P)) * BETA**EMAX . */

/*     First compute 1.0 - BETA**(-P), being careful that the */
/*     result is less than 1.0 . */

/*<       RECBAS = ONE / BETA >*/
    recbas = (float)1. / *beta;
/*<       Z = BETA - ONE >*/
    z__ = *beta - (float)1.;
/*<       Y = ZERO >*/
    y = (float)0.;
/*<       DO 20 I = 1, P >*/
    i__1 = *p;
    for (i__ = 1; i__ <= i__1; ++i__) {
/*<          Z = Z*RECBAS >*/
        z__ *= recbas;
/*<    >*/
        if (y < (float)1.) {
            oldy = y;
        }
/*<          Y = SLAMC3( Y, Z ) >*/
        y = slamc3_(&y, &z__);
/*<    20 CONTINUE >*/
/* L20: */
    }
/*<    >*/
    if (y >= (float)1.) {
        y = oldy;
    }

/*     Now multiply by BETA**EMAX to get RMAX. */

/*<       DO 30 I = 1, EMAX >*/
    i__1 = *emax;
    for (i__ = 1; i__ <= i__1; ++i__) {
/*<          Y = SLAMC3( Y*BETA, ZERO ) >*/
        r__1 = y * *beta;
        y = slamc3_(&r__1, &c_b32);
/*<    30 CONTINUE >*/
/* L30: */
    }

/*<       RMAX = Y >*/
    *rmax = y;
/*<       RETURN >*/
    return 0;

/*     End of SLAMC5 */

/*<       END >*/
} /* slamc5_ */

#ifdef __cplusplus
        }
#endif
