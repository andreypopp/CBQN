#include "../core.h"
#include "../utils/each.h"
#include <math.h>

static inline B arith_recm(BB2B f, B x) {
  B fx = getFillQ(x);
  B r = eachm_fn(f, bi_N, x);
  return withFill(r, fx);
}
#ifdef CATCH_ERRORS
static inline B arith_recd(BBB2B f, B w, B x) {
  B fx = getFillQ(x);
  if (noFill(fx)) return eachd_fn(f, bi_N, w, x);
  B fw = getFillQ(w);
  B r = eachd_fn(f, bi_N, w, x);
  if (noFill(fw)) return r;
  if (CATCH) { dec(catchMessage); return r; }
  B fr = f(bi_N, fw, fx);
  popCatch();
  return withFill(r, asFill(fr));
}
#else
static inline B arith_recd(BBB2B f, B w, B x) {
  return eachd_fn(f, bi_N, w, x);
}
#endif


#define P1(N) { if(         isArr(x)) return arith_recm(N##_c1,    x); }
#define P2(N) { if(isArr(w)|isArr(x)) return arith_recd(N##_c2, w, x); }
#if TYPED_ARITH
  static B f64_maybe_i32(B x) {
    f64* xp = f64arr_ptr(x);
    usz ia = a(x)->ia;
    if (ia==0) { dec(x); return inc(bi_emptyIVec); }
    if (xp[0] != (i32)xp[0]) return x;
    i32* rp; B r = m_i32arrc(&rp, x);
    for (usz i = 0; i < ia; i++) {
      f64 cf = xp[i];
      i32 c = (i32)cf;
      if (cf!=c) { dec(r); return x; }
      rp[i] = c;
    }
    dec(x);
    return r;
  }
  #define GC2f(NAME, EXPR, EXTRA) B NAME##_c2(B t, B w, B x) {               \
    if (isF64(w) & isF64(x)) return m_f64(EXPR);                             \
    EXTRA                                                                    \
    if (isArr(w)|isArr(x)) { B ow=w; B ox=x;                                 \
      if (isArr(w)&isArr(x) && rnk(w)==rnk(x)) {                             \
        usz ia = a(x)->ia;                                                   \
        u8 we = TI(w).elType;                                                \
        u8 xe = TI(x).elType;                                                \
        if (isNumEl(we)&isNumEl(xe)) {                                       \
          f64* rp; B r = m_f64arrc(&rp, x);                                  \
          if (we==el_i32) { B w,x/*shadow*/; i32* wp = i32any_ptr(ow);       \
            if (xe==el_i32) { i32* xp = i32any_ptr(ox); for (usz i = 0; i < ia; i++) {w.f=wp[i];x.f=xp[i];rp[i]=EXPR;} } \
            else            { f64* xp = f64any_ptr(ox); for (usz i = 0; i < ia; i++) {w.f=wp[i];x.f=xp[i];rp[i]=EXPR;} } \
          } else {          B w,x/*shadow*/; f64* wp = f64any_ptr(ow);       \
            if (xe==el_i32) { i32* xp = i32any_ptr(ox); for (usz i = 0; i < ia; i++) {w.f=wp[i];x.f=xp[i];rp[i]=EXPR;} } \
            else            { f64* xp = f64any_ptr(ox); for (usz i = 0; i < ia; i++) {w.f=wp[i];x.f=xp[i];rp[i]=EXPR;} } \
          }                                                                  \
          dec(w); dec(x); return f64_maybe_i32(r);                           \
        }                                                                    \
      } else if (isF64(w)&isArr(x)) { usz ia = a(x)->ia;                     \
        u8 xe = TI(x).elType; f64*rp;                                        \
        if (xe==el_i32) { B r=m_f64arrc(&rp, x); i32*xp=i32any_ptr(x);       \
          for (usz i = 0; i < ia; i++) {B x/*shadow*/;x.f=xp[i];rp[i]=EXPR;} \
          dec(x); return f64_maybe_i32(r);                                   \
        }                                                                    \
        if (xe==el_f64) { B r=m_f64arrc(&rp, x); f64*xp=f64any_ptr(x);       \
          for (usz i = 0; i < ia; i++) {B x/*shadow*/;x.f=xp[i];rp[i]=EXPR;} \
          dec(x); return f64_maybe_i32(r);                                   \
        }                                                                    \
      } else if (isF64(x)&isArr(w)) { usz ia = a(w)->ia;                     \
        u8 we = TI(w).elType; f64*rp;                                        \
        if (we==el_i32) { B r=m_f64arrc(&rp, w); i32*wp=i32any_ptr(w);       \
          for (usz i = 0; i < ia; i++) {B w/*shadow*/;w.f=wp[i];rp[i]=EXPR;} \
          dec(w); return f64_maybe_i32(r);                                   \
        }                                                                    \
        if (we==el_f64) { B r=m_f64arrc(&rp, w); f64*wp=f64any_ptr(w);       \
          for (usz i = 0; i < ia; i++) {B w/*shadow*/;w.f=wp[i];rp[i]=EXPR;} \
          dec(w); return f64_maybe_i32(r);                                   \
        }                                                                    \
      }                                                                      \
      P2(NAME)                                                               \
    }                                                                        \
    thrM(#NAME ": invalid arithmetic");                                      \
  }
  
  #define PF(N) f64* N##p = f64any_ptr(N);
  #define PI(N) i32* N##p = i32any_ptr(N);
  #define RI(A) i32* rp; B r=m_i32arrc(&rp, A);
  #define RF(A) f64* rp; B r=m_f64arrc(&rp, A);
  
  #define DOF(EXPR,A,W,X) {        \
    for (usz i = 0; i < ia; i++) { \
      f64 wv = W; f64 xv = X;      \
      rp[i] = EXPR;                \
    }                              \
  }
  #define DOI(EXPR,A,W,X,BASE) {   \
    RI(A)                          \
    for (usz i = 0; i < ia; i++) { \
      i64 wv = W; i64 xv = X;      \
      i64 rv = EXPR;               \
      if (RARE(rv!=(i32)rv)) {     \
        mm_free(v(r));             \
        goto BASE;                 \
      }                            \
      rp[i] = rv;                  \
    }                              \
    dec(w); dec(x);                \
    return r;                      \
  }
  #define GC2i(NAME, EXPR, EXTRA) B NAME##_c2(B t, B w, B x) {       \
    if (isF64(w) & isF64(x)) {f64 wv=w.f,xv=x.f;return m_f64(EXPR);} \
    EXTRA                                                            \
    if (isArr(w)|isArr(x)) {                                         \
      if (isArr(w)&isArr(x) && rnk(w)==rnk(x)) {                     \
        usz ia = a(x)->ia;                                           \
        u8 we = TI(w).elType;                                        \
        u8 xe = TI(x).elType;                                        \
        if (isNumEl(we)&isNumEl(xe)) {                               \
          bool wei = we==el_i32; bool xei = xe==el_i32;              \
          if (wei&xei) { PI(w) PI(x) DOI(EXPR,w,wp[i],xp[i],aaB); }  \
          aaB:; RF(x)                                                \
          if (wei) { PI(w)                                           \
            if (xei) { PI(x) DOF(EXPR,w,wp[i],xp[i]) }               \
            else     { PF(x) DOF(EXPR,w,wp[i],xp[i]) }               \
          } else { PF(w)                                             \
            if (xei) { PI(x) DOF(EXPR,w,wp[i],xp[i]) }               \
            else     { PF(x) DOF(EXPR,w,wp[i],xp[i]) }               \
          }                                                          \
          dec(w); dec(x); return f64_maybe_i32(r);                   \
        }                                                            \
      } else if (isF64(w)&isArr(x)) { usz ia = a(x)->ia; u8 xe = TI(x).elType;                 \
        if (xe==el_i32 && q_i32(w)) { PI(x) i32 wc=o2iu(w); DOI(EXPR,x,wc,xp[i],naB) } naB:;   \
        if (xe==el_i32) { RF(x) PI(x) DOF(EXPR,w,w.f,xp[i]) dec(x); return f64_maybe_i32(r); } \
        if (xe==el_f64) { RF(x) PF(x) DOF(EXPR,w,w.f,xp[i]) dec(x); return f64_maybe_i32(r); } \
      } else if (isF64(x)&isArr(w)) { usz ia = a(w)->ia; u8 we = TI(w).elType;                 \
        if (we==el_i32 && q_i32(x)) { PI(w) i32 xc=o2iu(x); DOI(EXPR,w,wp[i],xc,anB) } anB:;   \
        if (we==el_i32) { RF(w) PI(w) DOF(EXPR,x,wp[i],x.f) dec(w); return f64_maybe_i32(r); } \
        if (we==el_f64) { RF(w) PF(w) DOF(EXPR,x,wp[i],x.f) dec(w); return f64_maybe_i32(r); } \
      }                                                              \
      P2(NAME)                                                       \
    }                                                                \
    thrM(#NAME ": invalid arithmetic");                              \
  }
#else // if !TYPED_ARITH
  #define GC2i(NAME, EXPR, EXTRA) B NAME##_c2(B t, B w, B x) { \
    if (isF64(w) & isF64(x)) { f64 wv=w.f; f64 xv=x.f; return m_f64(EXPR); } \
    EXTRA \
    P2(NAME) \
    thrM(#NAME ": invalid arithmetic"); \
  }
  #define GC2f(NAME, EXPR, EXTRA) B NAME##_c2(B t, B w, B x) { \
    if (isF64(w) & isF64(x)) return m_f64(EXPR); \
    EXTRA \
    P2(NAME) \
    thrM(#NAME ": invalid arithmetic"); \
  }
#endif // TYPED_ARITH

static f64 pfmod(f64 a, f64 b) {
  f64 r = fmod(a, b);
  if (a<0 != b<0 && r!=0) r+= b;
  return r;
}

GC2i(add, wv+xv, {
  if (isC32(w) & isF64(x)) { u64 r = (u64)(o2cu(w)+o2i64(x)); if(r>CHR_MAX)thrM("+: Invalid character"); return m_c32((u32)r); }
  if (isF64(w) & isC32(x)) { u64 r = (u64)(o2cu(x)+o2i64(w)); if(r>CHR_MAX)thrM("+: Invalid character"); return m_c32((u32)r); }
})
GC2i(sub, wv-xv, {
  if (isC32(w) & isF64(x)) { u64 r = (u64)(o2cu(w)-o2u64(x)); if(r>CHR_MAX)thrM("-: Invalid character"); return m_c32((u32)r); }
  if (isC32(w) & isC32(x)) return m_f64((i32)(u32)w.u - (i32)(u32)x.u);
})
GC2i(mul, wv*xv, {})
GC2i(and, wv*xv, {})
GC2i(or , (wv+xv)-(wv*xv), {})
GC2i(not, 1+wv-xv, {})

GC2f(div  ,           w.f/x.f, {})
GC2f(pow  ,     pow(w.f, x.f), {})
GC2f(floor,    fmin(w.f, x.f), {})
GC2f(ceil ,    fmax(w.f, x.f), {})
GC2f(stile,   pfmod(x.f, w.f), {})
GC2f(log  , log(x.f)/log(w.f), {})

#undef GC2i
#undef GC2f

B decp_c1(B t, B x);
#define CMP_IMPL(OP,FC,CF) \
  if (isF64(w)&isF64(x)) return m_i32(w.f OP x.f); \
  if (isC32(w)&isC32(x)) return m_f64(w.u OP x.u); \
  if (isF64(w)&isC32(x)) return m_f64(FC);         \
  if (isC32(w)&isF64(x)) return m_f64(CF);         \
  bool wa = isArr(w);                              \
  bool xa = isArr(x);                              \
  if (wa|xa) {                                     \
    u8 we = wa? TI(w).elType : selfElType(w);      \
    u8 xe = xa? TI(x).elType : selfElType(x);      \
    if (we==el_i32 && xe==el_i32) {                \
      i32* rp; B r = m_i32arrc(&rp, isArr(x)? x : w); usz ria=a(r)->ia;                                    \
      if      (!wa) { i32 wv=o2iu(w); i32* xp=i32any_ptr(x); for(usz i=0;i<ria;i++)rp[i]=wv    OP xp[i]; } \
      else if (!xa) { i32 xv=o2iu(x); i32* wp=i32any_ptr(w); for(usz i=0;i<ria;i++)rp[i]=wp[i] OP xv;    } \
      else { i32* wp=i32any_ptr(w);   i32* xp=i32any_ptr(x); for(usz i=0;i<ria;i++)rp[i]=wp[i] OP xp[i]; } \
      if(wa) dec(w); if(xa) dec(x); return r;      \
    }                                              \
    if (we==el_c32 && xe==el_c32) {                \
      i32* rp; B r = m_i32arrc(&rp, isArr(x)? x : w); usz ria=a(r)->ia;                                    \
      if      (!wa) { u32 wv=o2cu(w); u32* xp=c32any_ptr(x); for(usz i=0;i<ria;i++)rp[i]=wv    OP xp[i]; } \
      else if (!xa) { u32 xv=o2cu(x); u32* wp=c32any_ptr(w); for(usz i=0;i<ria;i++)rp[i]=wp[i] OP xv;    } \
      else { u32* wp=c32any_ptr(w);   u32* xp=c32any_ptr(x); for(usz i=0;i<ria;i++)rp[i]=wp[i] OP xp[i]; } \
      if(wa) dec(w); if(xa) dec(x); return r;      \
    }                                              \
  }

#define CMP(NAME,OP,FC,CF)          \
B NAME##_c2(B t, B w, B x) {        \
  CMP_IMPL(OP, FC, CF);             \
  P2(NAME);                         \
  return m_i32(compare(w, x) OP 0); \
}
CMP(le, <=, 1, 0)
CMP(ge, >=, 0, 1)
CMP(lt, < , 1, 0)
CMP(gt, > , 0, 1)
#undef CMP

B eq_c2(B t, B w, B x) {
  CMP_IMPL(==, 0, 0);
  P2(eq);
  B r = m_i32(atomEqual(w, x));
  dec(w); dec(x);
  return r;
}
B ne_c2(B t, B w, B x) {
  CMP_IMPL(!=, 1, 1);
  P2(ne);
  B r = m_i32(!atomEqual(w, x));
  dec(w); dec(x);
  return r;
}

extern B rt_merge;
B gt_c1(B t, B x) {
  if (isAtm(x)) return x;
  return bqn_merge(x);
}

B   add_c1(B t, B x) { return x; }
B   sub_c1(B t, B x) { if (isF64(x)) return m_f64(     -x.f ); P1(  sub); thrM("-: Negating non-number"); }
B   not_c1(B t, B x) { if (isF64(x)) return m_f64(    1-x.f ); P1(  not); thrM("¬: Argument was not a number"); }
B   mul_c1(B t, B x) { if (isF64(x)) return m_f64(x.f==0?0:x.f>0?1:-1); P1(mul); thrM("×: Getting sign of non-number"); }
B   div_c1(B t, B x) { if (isF64(x)) return m_f64(    1/x.f ); P1(  div); thrM("÷: Getting reciprocal of non-number"); }
B   pow_c1(B t, B x) { if (isF64(x)) return m_f64(  exp(x.f)); P1(  pow); thrM("⋆: Getting exp of non-number"); }
B floor_c1(B t, B x) { if (isF64(x)) return m_f64(floor(x.f)); P1(floor); thrM("⌊: Argument was not a number"); }
B  ceil_c1(B t, B x) { if (isF64(x)) return m_f64( ceil(x.f)); P1( ceil); thrM("⌈: Argument was not a number"); }
B stile_c1(B t, B x) { if (isF64(x)) return m_f64( fabs(x.f)); P1(stile); thrM("|: Argument was not a number"); }
B   log_c1(B t, B x) { if (isF64(x)) return m_f64(  log(x.f)); P1(  log); thrM("⋆⁼: Getting log of non-number"); }

B lt_c1(B t, B x) { return m_unit(x); }
B eq_c1(B t, B x) { B r = m_i32(isArr(x)? rnk(x) : 0); decR(x); return r; }
B ne_c1(B t, B x) { B r = m_f64(isArr(x)&&rnk(x)? *a(x)->sh : 1); decR(x); return r; }

extern B rt_sortDsc;
B or_c1(B t, B x) { return c1(rt_sortDsc, x); }
B and_c1(B t, B x); // defined in sort.c

#undef P1
#undef P2



void arith_init() {
  c(BFn,bi_add)->ident = c(BFn,bi_sub)->ident = c(BFn,bi_or )->ident = c(BFn,bi_ne)->ident = c(BFn,bi_gt)->ident = m_i32(0);
  c(BFn,bi_mul)->ident = c(BFn,bi_div)->ident = c(BFn,bi_and)->ident = c(BFn,bi_eq)->ident = c(BFn,bi_ge)->ident = c(BFn,bi_pow)->ident = c(BFn,bi_not)->ident = m_i32(1);
  c(BFn,bi_floor)->ident = m_f64(1.0/0.0);
  c(BFn,bi_ceil )->ident = m_f64(-1.0/0.0);
}
