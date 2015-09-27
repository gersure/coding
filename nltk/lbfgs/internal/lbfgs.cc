// Limited memory BFGS (L-BFGS).
//
// Copyright (c) 1990, Jorge Nocedal
// Copyright (c) 2007-2010 Naoaki Okazaki
// All rights reserved.

#include "nltk/lbfgs/public/lbfgs.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <emmintrin.h>
#include <algorithm>

namespace nltk {
namespace lbfgs {

inline static void* vecalloc(size_t size) {
  void *memblock = NULL, *p = NULL;
  if (posix_memalign(&p, 16, size) == 0) {
    memblock = p;
  }
  if (memblock != NULL) {
    memset(memblock, 0, size);
  }
  return memblock;
}

inline static void vecfree(void *memblock) { free(memblock); }

#define fsigndiff(x, y) \
  ((_mm_movemask_pd(_mm_set_pd(*(x), *(y))) + 1) & 0x002)

#define vecset(x, c, n) { \
  int i; \
  __m128d XMM0 = _mm_set1_pd(c); \
  for (i = 0;i < (n); i += 8) { \
    _mm_store_pd((x) + i  , XMM0); \
    _mm_store_pd((x) + i + 2, XMM0); \
    _mm_store_pd((x) + i + 4, XMM0); \
    _mm_store_pd((x) + i + 6, XMM0); \
  } \
}

#define veccpy(y, x, n) { \
  int i; \
  for (i = 0;i < (n); i += 8) { \
    __m128d XMM0 = _mm_load_pd((x) + i); \
    __m128d XMM1 = _mm_load_pd((x) + i + 2); \
    __m128d XMM2 = _mm_load_pd((x) + i + 4); \
    __m128d XMM3 = _mm_load_pd((x) + i + 6); \
    _mm_store_pd((y) + i, XMM0); \
    _mm_store_pd((y) + i + 2, XMM1); \
    _mm_store_pd((y) + i + 4, XMM2); \
    _mm_store_pd((y) + i + 6, XMM3); \
  } \
}

#define vecncpy(y, x, n) { \
  int i; \
  for (i = 0;i < (n); i += 8) { \
    __m128d XMM0 = _mm_setzero_pd(); \
    __m128d XMM1 = _mm_setzero_pd(); \
    __m128d XMM2 = _mm_setzero_pd(); \
    __m128d XMM3 = _mm_setzero_pd(); \
    __m128d XMM4 = _mm_load_pd((x) + i); \
    __m128d XMM5 = _mm_load_pd((x) + i + 2); \
    __m128d XMM6 = _mm_load_pd((x) + i + 4); \
    __m128d XMM7 = _mm_load_pd((x) + i + 6); \
    XMM0 = _mm_sub_pd(XMM0, XMM4); \
    XMM1 = _mm_sub_pd(XMM1, XMM5); \
    XMM2 = _mm_sub_pd(XMM2, XMM6); \
    XMM3 = _mm_sub_pd(XMM3, XMM7); \
    _mm_store_pd((y) + i, XMM0); \
    _mm_store_pd((y) + i + 2, XMM1); \
    _mm_store_pd((y) + i + 4, XMM2); \
    _mm_store_pd((y) + i + 6, XMM3); \
  } \
}

#define vecadd(y, x, c, n) { \
  int i; \
  __m128d XMM7 = _mm_set1_pd(c); \
  for (i = 0;i < (n); i += 4) { \
    __m128d XMM0 = _mm_load_pd((x) + i); \
    __m128d XMM1 = _mm_load_pd((x) + i + 2); \
    __m128d XMM2 = _mm_load_pd((y) + i); \
    __m128d XMM3 = _mm_load_pd((y) + i + 2); \
    XMM0 = _mm_mul_pd(XMM0, XMM7); \
    XMM1 = _mm_mul_pd(XMM1, XMM7); \
    XMM2 = _mm_add_pd(XMM2, XMM0); \
    XMM3 = _mm_add_pd(XMM3, XMM1); \
    _mm_store_pd((y) + i, XMM2); \
    _mm_store_pd((y) + i + 2, XMM3); \
  } \
}

#define vecdiff(z, x, y, n) { \
  int i; \
  for (i = 0;i < (n); i += 8) { \
    __m128d XMM0 = _mm_load_pd((x) + i); \
    __m128d XMM1 = _mm_load_pd((x) + i + 2); \
    __m128d XMM2 = _mm_load_pd((x) + i + 4); \
    __m128d XMM3 = _mm_load_pd((x) + i + 6); \
    __m128d XMM4 = _mm_load_pd((y) + i); \
    __m128d XMM5 = _mm_load_pd((y) + i + 2); \
    __m128d XMM6 = _mm_load_pd((y) + i + 4); \
    __m128d XMM7 = _mm_load_pd((y) + i + 6); \
    XMM0 = _mm_sub_pd(XMM0, XMM4); \
    XMM1 = _mm_sub_pd(XMM1, XMM5); \
    XMM2 = _mm_sub_pd(XMM2, XMM6); \
    XMM3 = _mm_sub_pd(XMM3, XMM7); \
    _mm_store_pd((z) + i, XMM0); \
    _mm_store_pd((z) + i + 2, XMM1); \
    _mm_store_pd((z) + i + 4, XMM2); \
    _mm_store_pd((z) + i + 6, XMM3); \
  } \
}

#define vecscale(y, c, n) { \
  int i; \
  __m128d XMM7 = _mm_set1_pd(c); \
  for (i = 0;i < (n); i += 4) { \
    __m128d XMM0 = _mm_load_pd((y) + i); \
    __m128d XMM1 = _mm_load_pd((y) + i + 2); \
    XMM0 = _mm_mul_pd(XMM0, XMM7); \
    XMM1 = _mm_mul_pd(XMM1, XMM7); \
    _mm_store_pd((y) + i, XMM0); \
    _mm_store_pd((y) + i + 2, XMM1); \
  } \
}

#define vecmul(y, x, n) { \
  int i; \
  for (i = 0;i < (n); i += 8) { \
    __m128d XMM0 = _mm_load_pd((x) + i); \
    __m128d XMM1 = _mm_load_pd((x) + i + 2); \
    __m128d XMM2 = _mm_load_pd((x) + i + 4); \
    __m128d XMM3 = _mm_load_pd((x) + i + 6); \
    __m128d XMM4 = _mm_load_pd((y) + i); \
    __m128d XMM5 = _mm_load_pd((y) + i + 2); \
    __m128d XMM6 = _mm_load_pd((y) + i + 4); \
    __m128d XMM7 = _mm_load_pd((y) + i + 6); \
    XMM4 = _mm_mul_pd(XMM4, XMM0); \
    XMM5 = _mm_mul_pd(XMM5, XMM1); \
    XMM6 = _mm_mul_pd(XMM6, XMM2); \
    XMM7 = _mm_mul_pd(XMM7, XMM3); \
    _mm_store_pd((y) + i, XMM4); \
    _mm_store_pd((y) + i + 2, XMM5); \
    _mm_store_pd((y) + i + 4, XMM6); \
    _mm_store_pd((y) + i + 6, XMM7); \
  } \
}

// Horizontal add with SSE instruction. The work register (rw) is used.
#define __horizontal_sum(r, rw) \
  rw = r; \
  r = _mm_shuffle_ps(r, rw, _MM_SHUFFLE(1, 0, 3, 2)); \
  r = _mm_add_ps(r, rw); \
  rw = r; \
  r = _mm_shuffle_ps(r, rw, _MM_SHUFFLE(2, 3, 0, 1)); \
  r = _mm_add_ps(r, rw);

#define vecdot(s, x, y, n) { \
  int i; \
  __m128d XMM0 = _mm_setzero_pd(); \
  __m128d XMM1 = _mm_setzero_pd(); \
  __m128d XMM2, XMM3, XMM4, XMM5; \
  for (i = 0;i < (n); i += 4) { \
    XMM2 = _mm_load_pd((x) + i); \
    XMM3 = _mm_load_pd((x) + i + 2); \
    XMM4 = _mm_load_pd((y) + i); \
    XMM5 = _mm_load_pd((y) + i + 2); \
    XMM2 = _mm_mul_pd(XMM2, XMM4); \
    XMM3 = _mm_mul_pd(XMM3, XMM5); \
    XMM0 = _mm_add_pd(XMM0, XMM2); \
    XMM1 = _mm_add_pd(XMM1, XMM3); \
  } \
  XMM0 = _mm_add_pd(XMM0, XMM1); \
  XMM1 = _mm_shuffle_pd(XMM0, XMM0, _MM_SHUFFLE2(1, 1)); \
  XMM0 = _mm_add_pd(XMM0, XMM1); \
  _mm_store_sd((s), XMM0); \
}

#define vec2norm(s, x, n) { \
  int i; \
  __m128d XMM0 = _mm_setzero_pd(); \
  __m128d XMM1 = _mm_setzero_pd(); \
  __m128d XMM2, XMM3, XMM4, XMM5; \
  for (i = 0;i < (n); i += 4) { \
    XMM2 = _mm_load_pd((x) + i); \
    XMM3 = _mm_load_pd((x) + i + 2); \
    XMM4 = XMM2; \
    XMM5 = XMM3; \
    XMM2 = _mm_mul_pd(XMM2, XMM4); \
    XMM3 = _mm_mul_pd(XMM3, XMM5); \
    XMM0 = _mm_add_pd(XMM0, XMM2); \
    XMM1 = _mm_add_pd(XMM1, XMM3); \
  } \
  XMM0 = _mm_add_pd(XMM0, XMM1); \
  XMM1 = _mm_shuffle_pd(XMM0, XMM0, _MM_SHUFFLE2(1, 1)); \
  XMM0 = _mm_add_pd(XMM0, XMM1); \
  XMM0 = _mm_sqrt_pd(XMM0); \
  _mm_store_sd((s), XMM0); \
}

#define vec2norminv(s, x, n) { \
  int i; \
  __m128d XMM0 = _mm_setzero_pd(); \
  __m128d XMM1 = _mm_setzero_pd(); \
  __m128d XMM2, XMM3, XMM4, XMM5; \
  for (i = 0;i < (n); i += 4) { \
    XMM2 = _mm_load_pd((x) + i); \
    XMM3 = _mm_load_pd((x) + i + 2); \
    XMM4 = XMM2; \
    XMM5 = XMM3; \
    XMM2 = _mm_mul_pd(XMM2, XMM4); \
    XMM3 = _mm_mul_pd(XMM3, XMM5); \
    XMM0 = _mm_add_pd(XMM0, XMM2); \
    XMM1 = _mm_add_pd(XMM1, XMM3); \
  } \
  XMM2 = _mm_set1_pd(1.0); \
  XMM0 = _mm_add_pd(XMM0, XMM1); \
  XMM1 = _mm_shuffle_pd(XMM0, XMM0, _MM_SHUFFLE2(1, 1)); \
  XMM0 = _mm_add_pd(XMM0, XMM1); \
  XMM0 = _mm_sqrt_pd(XMM0); \
  XMM2 = _mm_div_pd(XMM2, XMM0); \
  _mm_store_sd((s), XMM2); \
}

#define max3(a, b, c) std::max(std::max((a), (b)), (c));

struct tag_callback_data {
  int n;
  void * instance;
  lbfgs_evaluate_t proc_evaluate;
  lbfgs_progress_t proc_progress;
};
typedef struct tag_callback_data callback_data_t;

struct tag_iteration_data {
  double alpha;
  double * s;  // [n]
  double * y;  // [n]
  double ys;  // vecdot(y, s)
};
typedef struct tag_iteration_data iteration_data_t;

static const lbfgs_parameter_t _defparam = {
  6, 1e-5, 0, 1e-5,
  0, LBFGS_LINESEARCH_DEFAULT, 40,
  1e-20, 1e20, 1e-4, 0.9, 0.9, 1.0e-16,
  0.0, 0, -1,
};

typedef int (*line_search_proc)(int n, double * x, double * f, double * g,
                                double * s, double * stp, const double * xp,
                                const double * gp, double * wa,
                                callback_data_t * cd,
                                const lbfgs_parameter_t * param);

static int line_search_backtracking(int n, double * x, double * f, double * g,
                                    double * s, double * stp, const double * xp,
                                    const double * gp, double * wa,
                                    callback_data_t * cd,
                                    const lbfgs_parameter_t * param);

static int line_search_backtracking_owlqn(int n, double * x, double * f,
                                          double * g, double * s,
                                          double * stp, const double * xp,
                                          const double * gp, double * wp,
                                          callback_data_t * cd,
                                          const lbfgs_parameter_t * param);

static int line_search_morethuente(int n, double * x, double * f, double * g,
                                   double * s, double * stp, const double * xp,
                                   const double * gp, double * wa,
                                   callback_data_t * cd,
                                   const lbfgs_parameter_t * param);

static int update_trial_interval(double * x, double * fx, double * dx,
                                 double * y, double * fy, double * dy,
                                 double * t, double * ft, double * dt,
                                 const double tmin, const double tmax,
                                 int * brackt);

static double owlqn_x1norm(const double * x, const int start, const int n);

static void owlqn_pseudo_gradient(double * pg, const double * x,
                                  const double * g, const int n, const double c,
                                  const int start, const int end);

static void owlqn_project(double * d, const double * sign, const int start,
                          const int end);

static int round_out_variables(int n) {
  n += 7;
  n /= 8;
  n *= 8;
  return n;
}

double* lbfgs_malloc(int n) {
  n = round_out_variables(n);
  return (double*)vecalloc(sizeof(double) * n);  // NOLINT
}

void lbfgs_free(double *x) { vecfree(x); }

void lbfgs_parameter_init(lbfgs_parameter_t *param) {
  memcpy(param, &_defparam, sizeof(*param));
}

int lbfgs(int n, double * x, double * ptr_fx, lbfgs_evaluate_t proc_evaluate,
          lbfgs_progress_t proc_progress, void * instance,
          lbfgs_parameter_t *_param) {
  int ret;
  int i, j, k, ls, end, bound;
  double step;

  // Constant parameters and their default values.
  lbfgs_parameter_t param = (_param != NULL) ? (*_param) : _defparam;
  const int m = param.m;

  double * xp = NULL;
  double * g = NULL, * gp = NULL, * pg = NULL;
  double * d = NULL, * w = NULL, * pf = NULL;
  iteration_data_t *lm = NULL, *it = NULL;
  double ys, yy;
  double xnorm, gnorm, beta;
  double fx = 0.;
  double rate = 0.;
  line_search_proc linesearch = line_search_morethuente;

  // Construct a callback data.
  callback_data_t cd;
  cd.n = n;
  cd.instance = instance;
  cd.proc_evaluate = proc_evaluate;
  cd.proc_progress = proc_progress;

  // Round out the number of variables.
  n = round_out_variables(n);

  // Check the input parameters for errors.
  if (n <= 0) {
    return LBFGSERR_INVALID_N;
  }
  if (n % 8 != 0) {
    return LBFGSERR_INVALID_N_SSE;
  }
  if ((uintptr_t)(const void*)x % 16 != 0) {
    return LBFGSERR_INVALID_X_SSE;
  }
  if (param.epsilon < 0.) {
    return LBFGSERR_INVALID_EPSILON;
  }
  if (param.past < 0) {
    return LBFGSERR_INVALID_TESTPERIOD;
  }
  if (param.delta < 0.) {
    return LBFGSERR_INVALID_DELTA;
  }
  if (param.min_step < 0.) {
    return LBFGSERR_INVALID_MINSTEP;
  }
  if (param.max_step < param.min_step) {
    return LBFGSERR_INVALID_MAXSTEP;
  }
  if (param.ftol < 0.) {
    return LBFGSERR_INVALID_FTOL;
  }
  if (param.linesearch == LBFGS_LINESEARCH_BACKTRACKING_WOLFE ||
      param.linesearch == LBFGS_LINESEARCH_BACKTRACKING_STRONG_WOLFE) {
    if (param.wolfe <= param.ftol || 1. <= param.wolfe) {
      return LBFGSERR_INVALID_WOLFE;
    }
  }
  if (param.gtol < 0.) {
    return LBFGSERR_INVALID_GTOL;
  }
  if (param.xtol < 0.) {
    return LBFGSERR_INVALID_XTOL;
  }
  if (param.max_linesearch <= 0) {
    return LBFGSERR_INVALID_MAXLINESEARCH;
  }
  if (param.orthantwise_c < 0.) {
    return LBFGSERR_INVALID_ORTHANTWISE;
  }
  if (param.orthantwise_start < 0 || n < param.orthantwise_start) {
    return LBFGSERR_INVALID_ORTHANTWISE_START;
  }
  if (param.orthantwise_end < 0) {
    param.orthantwise_end = n;
  }
  if (n < param.orthantwise_end) {
    return LBFGSERR_INVALID_ORTHANTWISE_END;
  }
  if (param.orthantwise_c != 0.) {
    switch (param.linesearch) {
      case LBFGS_LINESEARCH_BACKTRACKING: {
        linesearch = line_search_backtracking_owlqn;  // NOLINT
        break;
      }
      default: {
        // Only the backtracking method is available.
        return LBFGSERR_INVALID_LINESEARCH;
      }
    }
  } else {
    switch (param.linesearch) {
      case LBFGS_LINESEARCH_MORETHUENTE: {
        linesearch = line_search_morethuente;
        break;
      }
      case LBFGS_LINESEARCH_BACKTRACKING_ARMIJO:
      case LBFGS_LINESEARCH_BACKTRACKING_WOLFE:
      case LBFGS_LINESEARCH_BACKTRACKING_STRONG_WOLFE: {
        linesearch = line_search_backtracking;
        break;
      }
      default: {
        return LBFGSERR_INVALID_LINESEARCH;
      }
    }
  }

  // Allocate working space.
  xp = (double*)vecalloc(n * sizeof(double));  // NOLINT
  g = (double*)vecalloc(n * sizeof(double));  // NOLINT
  gp = (double*)vecalloc(n * sizeof(double));  // NOLINT
  d = (double*)vecalloc(n * sizeof(double));  // NOLINT
  w = (double*)vecalloc(n * sizeof(double));  // NOLINT
  if (xp == NULL || g == NULL || gp == NULL || d == NULL || w == NULL) {
    ret = LBFGSERR_OUTOFMEMORY;
    goto lbfgs_exit;
  }

  if (param.orthantwise_c != 0.) {
    // Allocate working space for OW-LQN.
    pg = (double*)vecalloc(n * sizeof(double));  // NOLINT
    if (pg == NULL) {
      ret = LBFGSERR_OUTOFMEMORY;
      goto lbfgs_exit;
    }
  }

  // Allocate limited memory storage.
  lm = (iteration_data_t*)vecalloc(m * sizeof(iteration_data_t));  // NOLINT
  if (lm == NULL) {
    ret = LBFGSERR_OUTOFMEMORY;
    goto lbfgs_exit;
  }

  // Initialize the limited memory.
  for (i = 0;i < m;++i) {
    it = &lm[i];
    it->alpha = 0;
    it->ys = 0;
    it->s = (double*)vecalloc(n * sizeof(double));  // NOLINT
    it->y = (double*)vecalloc(n * sizeof(double));  // NOLINT
    if (it->s == NULL || it->y == NULL) {
      ret = LBFGSERR_OUTOFMEMORY;
      goto lbfgs_exit;
    }
  }

  // Allocate an array for storing previous values of the objective function.
  if (0 < param.past) {
    pf = (double *)vecalloc(param.past * sizeof(double));  // NOLINT
  }

  // Evaluate the function value and its gradient.
  fx = cd.proc_evaluate(cd.instance, x, g, cd.n, 0);
  if (0. != param.orthantwise_c) {
    // Compute the L1 norm of the variable and add it to the object value.
    xnorm = owlqn_x1norm(x, param.orthantwise_start, param.orthantwise_end);
    fx += xnorm * param.orthantwise_c;
    owlqn_pseudo_gradient(pg, x, g, n, param.orthantwise_c,
                          param.orthantwise_start, param.orthantwise_end);
  }

  // Store the initial value of the objective function.
  if (pf != NULL) {
    pf[0] = fx;
  }

  // Compute the direction;
  // we assume the initial hessian matrix H_0 as the identity matrix.
  if (param.orthantwise_c == 0.) {
    vecncpy(d, g, n);
  } else {
    vecncpy(d, pg, n);
  }

  // Make sure that the initial variables are not a minimizer.
  vec2norm(&xnorm, x, n);
  if (param.orthantwise_c == 0.) {
    vec2norm(&gnorm, g, n);
  } else {
    vec2norm(&gnorm, pg, n);
  }
  if (xnorm < 1.0) xnorm = 1.0;
  if (gnorm / xnorm <= param.epsilon) {
    ret = LBFGS_ALREADY_MINIMIZED;
    goto lbfgs_exit;
  }

  // Compute the initial step:
  // step = 1.0 / sqrt(vecdot(d, d, n))
  vec2norminv(&step, d, n);

  k = 1;
  end = 0;
  for (;;) {
    // Store the current position and gradient vectors.
    veccpy(xp, x, n);
    veccpy(gp, g, n);

    // Search for an optimal step.
    if (param.orthantwise_c == 0.) {
      ls = linesearch(n, x, &fx, g, d, &step, xp, gp, w, &cd, &param);
    } else {
      ls = linesearch(n, x, &fx, g, d, &step, xp, pg, w, &cd, &param);
      owlqn_pseudo_gradient(pg, x, g, n, param.orthantwise_c,
                            param.orthantwise_start, param.orthantwise_end);
    }
    if (ls < 0) {
      // Revert to the previous point.
      veccpy(x, xp, n);
      veccpy(g, gp, n);
      ret = ls;
      goto lbfgs_exit;
    }

    // Compute x and g norms.
    vec2norm(&xnorm, x, n);
    if (param.orthantwise_c == 0.) {
      vec2norm(&gnorm, g, n);
    } else {
      vec2norm(&gnorm, pg, n);
    }

    // Report the progress.
    if (cd.proc_progress) {
      if ((ret = cd.proc_progress(cd.instance, x, g, fx, xnorm,
                                  gnorm, step, cd.n, k, ls))) {
        goto lbfgs_exit;
      }
    }

    // Convergence test.
    // The criterion is given by the following formula:
    // |g(x)| / \max(1, |x|) < \epsilon
    if (xnorm < 1.0) xnorm = 1.0;
    if (gnorm / xnorm <= param.epsilon) {
      // Convergence.
      ret = LBFGS_SUCCESS;
      break;
    }

    // Test for stopping criterion.
    // The criterion is given by the following formula:
    // (f(past_x) - f(x)) / f(x) < \delta
    if (pf != NULL) {
      // We don't test the stopping criterion while k < past.
      if (param.past <= k) {
        // Compute the relative improvement from the past.
        rate = (pf[k % param.past] - fx) / fx;

        // The stopping criterion.
        if (rate < param.delta) {
          ret = LBFGS_STOP;
          break;
        }
      }

      // Store the current value of the objective function.
      pf[k % param.past] = fx;
    }

    if (param.max_iterations != 0 && param.max_iterations < k+1) {
      // Maximum number of iterations.
      ret = LBFGSERR_MAXIMUMITERATION;
      break;
    }

    // Update vectors s and y:
    // s_{k+1} = x_{k+1} - x_{k} = \step * d_{k}.
    // y_{k+1} = g_{k+1} - g_{k}.
    it = &lm[end];
    vecdiff(it->s, x, xp, n);
    vecdiff(it->y, g, gp, n);

    // Compute scalars ys and yy:
    // ys = y^t \cdot s = 1 / \rho.
    // yy = y^t \cdot y.
    // Notice that yy is used for scaling the hessian matrix H_0
    // (Cholesky factor).
    vecdot(&ys, it->y, it->s, n);
    vecdot(&yy, it->y, it->y, n);
    it->ys = ys;

    // Recursive formula to compute dir = -(H \cdot g).
    // This is described in page 779 of:
    // Jorge Nocedal.
    // Updating Quasi-Newton Matrices with Limited Storage.
    // Mathematics of Computation, Vol. 35, No. 151,
    // pp. 773--782, 1980.
    bound = (m <= k) ? m : k;
    ++k;
    end = (end + 1) % m;

    // Compute the steepest direction.
    if (param.orthantwise_c == 0.) {
      // Compute the negative of gradients.
      vecncpy(d, g, n);
    } else {
      vecncpy(d, pg, n);
    }

    j = end;
    for (i = 0;i < bound;++i) {
      j = (j + m - 1) % m;  // if (--j == -1) j = m-1;
      it = &lm[j];
      // \alpha_{j} = \rho_{j} s^{t}_{j} \cdot q_{k+1}.
      vecdot(&it->alpha, it->s, d, n);
      it->alpha /= it->ys;
      // q_{i} = q_{i+1} - \alpha_{i} y_{i}.
      vecadd(d, it->y, -it->alpha, n);
    }

    vecscale(d, ys / yy, n);

    for (i = 0;i < bound;++i) {
      it = &lm[j];
      // \beta_{j} = \rho_{j} y^t_{j} \cdot \gamma_{i}.
      vecdot(&beta, it->y, d, n);
      beta /= it->ys;
      // \gamma_{i+1} = \gamma_{i} + (\alpha_{j} - \beta_{j}) s_{j}.
      vecadd(d, it->s, it->alpha - beta, n);
      j = (j + 1) % m;  // if (++j == m) j = 0;
    }

    // Constrain the search direction for orthant-wise updates.
    if (param.orthantwise_c != 0.) {
      for (i = param.orthantwise_start;i < param.orthantwise_end;++i) {
        if (d[i] * pg[i] >= 0) {
          d[i] = 0;
        }
      }
    }

    // Now the search direction d is ready. We try step = 1 first.
    step = 1.0;
  }

 lbfgs_exit:
  // Return the final value of the objective function.
  if (ptr_fx != NULL) {
    *ptr_fx = fx;
  }

  vecfree(pf);

  // Free memory blocks used by this function.
  if (lm != NULL) {
    for (i = 0;i < m;++i) {
      vecfree(lm[i].s);
      vecfree(lm[i].y);
    }
    vecfree(lm);
  }
  vecfree(pg);
  vecfree(w);
  vecfree(d);
  vecfree(gp);
  vecfree(g);
  vecfree(xp);

  return ret;
}

static int line_search_backtracking(int n, double * x, double * f, double * g,
                                    double * s, double * stp, const double * xp,
                                    const double * gp, double * wp,
                                    callback_data_t * cd,
                                    const lbfgs_parameter_t * param) {
  int count = 0;
  double width, dg;
  double finit, dginit = 0., dgtest;
  const double dec = 0.5, inc = 2.1;

  // Check the input parameters for errors.
  if (*stp <= 0.) {
    return LBFGSERR_INVALIDPARAMETERS;
  }

  // Compute the initial gradient in the search direction.
  vecdot(&dginit, g, s, n);

  // Make sure that s points to a descent direction.
  if (0 < dginit) {
    return LBFGSERR_INCREASEGRADIENT;
  }

  // The initial value of the objective function.
  finit = *f;
  dgtest = param->ftol * dginit;

  for (;;) {
    veccpy(x, xp, n);
    vecadd(x, s, *stp, n);

    // Evaluate the function and gradient values.
    *f = cd->proc_evaluate(cd->instance, x, g, cd->n, *stp);

    ++count;

    if (*f > finit + *stp * dgtest) {
      width = dec;
    } else {
      // The sufficient decrease condition (Armijo condition).
      if (param->linesearch == LBFGS_LINESEARCH_BACKTRACKING_ARMIJO) {
        // Exit with the Armijo condition.
        return count;
      }

      // Check the Wolfe condition.
      vecdot(&dg, g, s, n);
      if (dg < param->wolfe * dginit) {
        width = inc;
      } else {
        if (param->linesearch == LBFGS_LINESEARCH_BACKTRACKING_WOLFE) {
          // Exit with the regular Wolfe condition.
          return count;
        }

        // Check the strong Wolfe condition.
        if (dg > -param->wolfe * dginit) {
          width = dec;
        } else {
          // Exit with the strong Wolfe condition.
          return count;
        }
      }
    }

    if (*stp < param->min_step) {
      // The step is the minimum value.
      return LBFGSERR_MINIMUMSTEP;
    }
    if (*stp > param->max_step) {
      // The step is the maximum value.
      return LBFGSERR_MAXIMUMSTEP;
    }
    if (param->max_linesearch <= count) {
      // Maximum number of iteration.
      return LBFGSERR_MAXIMUMLINESEARCH;
    }

    (*stp) *= width;
  }
}



static int line_search_backtracking_owlqn(int n, double * x, double * f,
                                          double * g, double * s,
                                          double * stp, const double * xp,
                                          const double * gp, double * wp,
                                          callback_data_t * cd,
                                          const lbfgs_parameter_t *param) {
  int i, count = 0;
  double width = 0.5, norm = 0.;
  double finit = *f, dgtest;

  // Check the input parameters for errors.
  if (*stp <= 0.) {
    return LBFGSERR_INVALIDPARAMETERS;
  }

  // Choose the orthant for the new point.
  for (i = 0;i < n;++i) {
    wp[i] = (xp[i] == 0.) ? -gp[i] : xp[i];
  }

  for (;;) {
    // Update the current point.
    veccpy(x, xp, n);
    vecadd(x, s, *stp, n);

    // The current point is projected onto the orthant.
    owlqn_project(x, wp, param->orthantwise_start, param->orthantwise_end);

    // Evaluate the function and gradient values.
    *f = cd->proc_evaluate(cd->instance, x, g, cd->n, *stp);

    // Compute the L1 norm of the variables and add it to the object value.
    norm = owlqn_x1norm(x, param->orthantwise_start, param->orthantwise_end);
    *f += norm * param->orthantwise_c;

    ++count;

    dgtest = 0.;
    for (i = 0;i < n;++i) {
      dgtest += (x[i] - xp[i]) * gp[i];
    }

    if (*f <= finit + param->ftol * dgtest) {
      // The sufficient decrease condition.
      return count;
    }

    if (*stp < param->min_step) {
      // The step is the minimum value.
      return LBFGSERR_MINIMUMSTEP;
    }
    if (*stp > param->max_step) {
      // The step is the maximum value.
      return LBFGSERR_MAXIMUMSTEP;
    }
    if (param->max_linesearch <= count) {
      // Maximum number of iteration.
      return LBFGSERR_MAXIMUMLINESEARCH;
    }

    (*stp) *= width;
  }
}

static int line_search_morethuente(int n, double * x, double * f, double * g,
                                   double * s, double * stp,
                                   const double * xp, const double * gp,
                                   double * wa, callback_data_t * cd,
                                   const lbfgs_parameter_t *param) {
  int count = 0;
  int brackt, stage1, uinfo = 0;
  double dg;
  double stx, fx, dgx;
  double sty, fy, dgy;
  double fxm, dgxm, fym, dgym, fm, dgm;
  double finit, ftest1, dginit, dgtest;
  double width, prev_width;
  double stmin, stmax;

  // Check the input parameters for errors.
  if (*stp <= 0.) {
    return LBFGSERR_INVALIDPARAMETERS;
  }

  // Compute the initial gradient in the search direction.
  vecdot(&dginit, g, s, n);

  // Make sure that s points to a descent direction.
  if (0 < dginit) {
    return LBFGSERR_INCREASEGRADIENT;
  }

  // Initialize local variables.
  brackt = 0;
  stage1 = 1;
  finit = *f;
  dgtest = param->ftol * dginit;
  width = param->max_step - param->min_step;
  prev_width = 2.0 * width;

  // The variables stx, fx, dgx contain the values of the step,
  // function, and directional derivative at the best step.
  // The variables sty, fy, dgy contain the value of the step,
  // function, and derivative at the other endpoint of
  // the interval of uncertainty.
  // The variables stp, f, dg contain the values of the step,
  // function, and derivative at the current step.
  stx = sty = 0.;
  fx = fy = finit;
  dgx = dgy = dginit;

  for (;;) {
    // Set the minimum and maximum steps to correspond to the
    // present interval of uncertainty.
    if (brackt) {
      stmin = std::min(stx, sty);
      stmax = std::max(stx, sty);
    } else {
      stmin = stx;
      stmax = *stp + 4.0 * (*stp - stx);
    }

    // Clip the step in the range of [stpmin, stpmax].
    if (*stp < param->min_step) *stp = param->min_step;
    if (param->max_step < *stp) *stp = param->max_step;

    // If an unusual termination is to occur then let
    // stp be the lowest point obtained so far.
    if ((brackt && ((*stp <= stmin || stmax <= *stp) ||
         param->max_linesearch <= count + 1 || uinfo != 0)) ||
        (brackt && (stmax - stmin <= param->xtol * stmax))) {
      *stp = stx;
    }

    // Compute the current value of x:
    // x <- x + (*stp) * s.
    veccpy(x, xp, n);
    vecadd(x, s, *stp, n);

    // Evaluate the function and gradient values.
    *f = cd->proc_evaluate(cd->instance, x, g, cd->n, *stp);
    vecdot(&dg, g, s, n);

    ftest1 = finit + *stp * dgtest;
    ++count;

    // Test for errors and convergence.
    if (brackt && ((*stp <= stmin || stmax <= *stp) || uinfo != 0)) {
      // Rounding errors prevent further progress.
      return LBFGSERR_ROUNDING_ERROR;
    }
    if (*stp == param->max_step && *f <= ftest1 && dg <= dgtest) {
      // The step is the maximum value.
      return LBFGSERR_MAXIMUMSTEP;
    }
    if (*stp == param->min_step && (ftest1 < *f || dgtest <= dg)) {
      // The step is the minimum value.
      return LBFGSERR_MINIMUMSTEP;
    }
    if (brackt && (stmax - stmin) <= param->xtol * stmax) {
      // Relative width of the interval of uncertainty is at most xtol.
      return LBFGSERR_WIDTHTOOSMALL;
    }
    if (param->max_linesearch <= count) {
      // Maximum number of iteration.
      return LBFGSERR_MAXIMUMLINESEARCH;
    }
    if (*f <= ftest1 && fabs(dg) <= param->gtol * (-dginit)) {
      // The sufficient decrease condition and the directional derivative
      // condition hold.
      return count;
    }

    // In the first stage we seek a step for which the modified
    // function has a nonpositive value and nonnegative derivative.
    if (stage1 && *f <= ftest1 &&
        std::min(param->ftol, param->gtol) * dginit <= dg) {
      stage1 = 0;
    }

    // A modified function is used to predict the step only if
    // we have not obtained a step for which the modified
    // function has a nonpositive function value and nonnegative
    // derivative, and if a lower function value has been
    // obtained but the decrease is not sufficient.
    if (stage1 && ftest1 < *f && *f <= fx) {
      // Define the modified function and derivative values.
      fm = *f - *stp * dgtest;
      fxm = fx - stx * dgtest;
      fym = fy - sty * dgtest;
      dgm = dg - dgtest;
      dgxm = dgx - dgtest;
      dgym = dgy - dgtest;

      // Call update_trial_interval() to update the interval of
      // uncertainty and to compute the new step.
      uinfo = update_trial_interval(&stx, &fxm, &dgxm,
                                    &sty, &fym, &dgym,
                                    stp, &fm, &dgm,
                                    stmin, stmax, &brackt);

      // Reset the function and gradient values for f.
      fx = fxm + stx * dgtest;
      fy = fym + sty * dgtest;
      dgx = dgxm + dgtest;
      dgy = dgym + dgtest;
    } else {
      // Call update_trial_interval() to update the interval of
      // uncertainty and to compute the new step.
      uinfo = update_trial_interval(&stx, &fx, &dgx,
                                    &sty, &fy, &dgy,
                                    stp, f, &dg,
                                    stmin, stmax, &brackt);
    }

    // Force a sufficient decrease in the interval of uncertainty.
    if (brackt) {
      if (0.66 * prev_width <= fabs(sty - stx)) {
        *stp = stx + 0.5 * (sty - stx);
      }
      prev_width = width;
      width = fabs(sty - stx);
    }
  }

  return LBFGSERR_LOGICERROR;
}



// Define the local variables for computing minimizers.
#define USES_MINIMIZER \
    double a, d, gamma, theta, p, q, r, s;

// Find a minimizer of an interpolated cubic function.
//  @param  cm      The minimizer of the interpolated cubic.
//  @param  u       The value of one point, u.
//  @param  fu      The value of f(u).
//  @param  du      The value of f'(u).
//  @param  v       The value of another point, v.
//  @param  fv      The value of f(v).
//  @param  du      The value of f'(v).
#define CUBIC_MINIMIZER(cm, u, fu, du, v, fv, dv) \
    d = (v) - (u); \
    theta = ((fu) - (fv)) * 3 / d + (du) + (dv); \
    p = fabs(theta); \
    q = fabs(du); \
    r = fabs(dv); \
    s = max3(p, q, r); \
    /* gamma = s*sqrt((theta/s)**2 - (du/s) * (dv/s)) */ \
    a = theta / s; \
    gamma = s * sqrt(a * a - ((du) / s) * ((dv) / s)); \
    if ((v) < (u)) gamma = -gamma; \
    p = gamma - (du) + theta; \
    q = gamma - (du) + gamma + (dv); \
    r = p / q; \
    (cm) = (u) + r * d;

// Find a minimizer of an interpolated cubic function.
//  @param  cm      The minimizer of the interpolated cubic.
//  @param  u       The value of one point, u.
//  @param  fu      The value of f(u).
//  @param  du      The value of f'(u).
//  @param  v       The value of another point, v.
//  @param  fv      The value of f(v).
//  @param  du      The value of f'(v).
//  @param  xmin    The maximum value.
//  @param  xmin    The minimum value.
#define CUBIC_MINIMIZER2(cm, u, fu, du, v, fv, dv, xmin, xmax) \
    d = (v) - (u); \
    theta = ((fu) - (fv)) * 3 / d + (du) + (dv); \
    p = fabs(theta); \
    q = fabs(du); \
    r = fabs(dv); \
    s = max3(p, q, r); \
    /* gamma = s*sqrt((theta/s)**2 - (du/s) * (dv/s)) */ \
    a = theta / s; \
    gamma = s * sqrt(std::max(0.0, a * a - ((du) / s) * ((dv) / s))); \
    if ((u) < (v)) gamma = -gamma; \
    p = gamma - (dv) + theta; \
    q = gamma - (dv) + gamma + (du); \
    r = p / q; \
    if (r < 0. && gamma != 0.) { \
        (cm) = (v) - r * d; \
    } else if (a < 0) { \
        (cm) = (xmax); \
    } else { \
        (cm) = (xmin); \
    }

// Find a minimizer of an interpolated quadratic function.
//  @param  qm      The minimizer of the interpolated quadratic.
//  @param  u       The value of one point, u.
//  @param  fu      The value of f(u).
//  @param  du      The value of f'(u).
//  @param  v       The value of another point, v.
//  @param  fv      The value of f(v).
#define QUARD_MINIMIZER(qm, u, fu, du, v, fv) \
    a = (v) - (u); \
    (qm) = (u) + (du) / (((fu) - (fv)) / a + (du)) / 2 * a;

// Find a minimizer of an interpolated quadratic function.
//  @param  qm      The minimizer of the interpolated quadratic.
//  @param  u       The value of one point, u.
//  @param  du      The value of f'(u).
//  @param  v       The value of another point, v.
//  @param  dv      The value of f'(v).
#define QUARD_MINIMIZER2(qm, u, du, v, dv) \
    a = (u) - (v); \
    (qm) = (v) + (dv) / ((dv) - (du)) * a;

// Update a safeguarded trial value and interval for line search.
//
//  The parameter x represents the step with the least function value.
//  The parameter t represents the current step. This function assumes
//  that the derivative at the point of x in the direction of the step.
//  If the bracket is set to true, the minimizer has been bracketed in
//  an interval of uncertainty with endpoints between x and y.
//
//  @param  x       The pointer to the value of one endpoint.
//  @param  fx      The pointer to the value of f(x).
//  @param  dx      The pointer to the value of f'(x).
//  @param  y       The pointer to the value of another endpoint.
//  @param  fy      The pointer to the value of f(y).
//  @param  dy      The pointer to the value of f'(y).
//  @param  t       The pointer to the value of the trial value, t.
//  @param  ft      The pointer to the value of f(t).
//  @param  dt      The pointer to the value of f'(t).
//  @param  tmin    The minimum value for the trial value, t.
//  @param  tmax    The maximum value for the trial value, t.
//  @param  brackt  The pointer to the predicate if the trial value is
//                  bracketed.
//  @retval int     Status value. Zero indicates a normal termination.
//
//  @see
//      Jorge J. More and David J. Thuente. Line search algorithm with
//      guaranteed sufficient decrease. ACM Transactions on Mathematical
//      Software (TOMS), Vol 20, No 3, pp. 286-307, 1994.
static int update_trial_interval(double * x, double * fx, double * dx,
                                 double * y, double * fy, double * dy,
                                 double * t, double * ft, double * dt,
                                 const double tmin, const double tmax,
                                 int *brackt) {
  int bound;
  int dsign = fsigndiff(dt, dx);
  double mc;  // minimizer of an interpolated cubic.
  double mq;  // minimizer of an interpolated quadratic.
  double newt;  // new trial value.
  USES_MINIMIZER;  // for CUBIC_MINIMIZER and QUARD_MINIMIZER.

  // Check the input parameters for errors.
  if (*brackt) {
    if (*t <= std::min(*x, *y) || std::max(*x, *y) <= *t) {
      // The trival value t is out of the interval.
      return LBFGSERR_OUTOFINTERVAL;
    }
    if (0. <= *dx * (*t - *x)) {
      // The function must decrease from x.
      return LBFGSERR_INCREASEGRADIENT;
    }
    if (tmax < tmin) {
      // Incorrect tmin and tmax specified.
      return LBFGSERR_INCORRECT_TMINMAX;
    }
  }

  // Trial value selection.
  if (*fx < *ft) {
    // Case 1: a higher function value.
    // The minimum is brackt. If the cubic minimizer is closer
    // to x than the quadratic one, the cubic one is taken, else
    // the average of the minimizers is taken.
    *brackt = 1;
    bound = 1;
    CUBIC_MINIMIZER(mc, *x, *fx, *dx, *t, *ft, *dt);
    QUARD_MINIMIZER(mq, *x, *fx, *dx, *t, *ft);
    if (fabs(mc - *x) < fabs(mq - *x)) {
      newt = mc;
    } else {
      newt = mc + 0.5 * (mq - mc);
    }
  } else if (dsign) {
    // Case 2: a lower function value and derivatives of
    // opposite sign. The minimum is brackt. If the cubic
    // minimizer is closer to x than the quadratic (secant) one,
    // the cubic one is taken, else the quadratic one is taken.
    *brackt = 1;
    bound = 0;
    CUBIC_MINIMIZER(mc, *x, *fx, *dx, *t, *ft, *dt);
    QUARD_MINIMIZER2(mq, *x, *dx, *t, *dt);
    if (fabs(mc - *t) > fabs(mq - *t)) {
      newt = mc;
    } else {
      newt = mq;
    }
  } else if (fabs(*dt) < fabs(*dx)) {
    // Case 3: a lower function value, derivatives of the
    // same sign, and the magnitude of the derivative decreases.
    // The cubic minimizer is only used if the cubic tends to
    // infinity in the direction of the minimizer or if the minimum
    // of the cubic is beyond t. Otherwise the cubic minimizer is
    // defined to be either tmin or tmax. The quadratic (secant)
    // minimizer is also computed and if the minimum is brackt
    // then the the minimizer closest to x is taken, else the one
    // farthest away is taken.
    bound = 1;
    CUBIC_MINIMIZER2(mc, *x, *fx, *dx, *t, *ft, *dt, tmin, tmax);
    QUARD_MINIMIZER2(mq, *x, *dx, *t, *dt);
    if (*brackt) {
      if (fabs(*t - mc) < fabs(*t - mq)) {
        newt = mc;
      } else {
        newt = mq;
      }
    } else {
      if (fabs(*t - mc) > fabs(*t - mq)) {
        newt = mc;
      } else {
        newt = mq;
      }
    }
  } else {
    // Case 4: a lower function value, derivatives of the
    // same sign, and the magnitude of the derivative does
    // not decrease. If the minimum is not brackt, the step
    // is either tmin or tmax, else the cubic minimizer is taken.
    bound = 0;
    if (*brackt) {
      CUBIC_MINIMIZER(newt, *t, *ft, *dt, *y, *fy, *dy);
    } else if (*x < *t) {
      newt = tmax;
    } else {
      newt = tmin;
    }
  }

  // Update the interval of uncertainty. This update does not
  // depend on the new step or the case analysis above.

  // - Case a: if f(x) < f(t),
  // x <- x, y <- t.
  // - Case b: if f(t) <= f(x) && f'(t)*f'(x) > 0,
  // x <- t, y <- y.
  // - Case c: if f(t) <= f(x) && f'(t)*f'(x) < 0,
  // x <- t, y <- x.
  if (*fx < *ft) {
    // Case a
    *y = *t;
    *fy = *ft;
    *dy = *dt;
  } else {
    // Case c
    if (dsign) {
      *y = *x;
      *fy = *fx;
      *dy = *dx;
    }
    // Cases b and c
    *x = *t;
    *fx = *ft;
    *dx = *dt;
  }

  // Clip the new trial value in [tmin, tmax].
  if (tmax < newt) newt = tmax;
  if (newt < tmin) newt = tmin;

  // Redefine the new trial value if it is close to the upper bound
  // of the interval.
  if (*brackt && bound) {
    mq = *x + 0.66 * (*y - *x);
    if (*x < *y) {
      if (mq < newt) newt = mq;
    } else {
      if (newt < mq) newt = mq;
    }
  }

  // Return the new trial value.
  *t = newt;
  return 0;
}

static double owlqn_x1norm(const double * x, const int start, const int n) {
  int i;
  double norm = 0.;

  for (i = start;i < n;++i) {
    norm += fabs(x[i]);
  }
  return norm;
}

static void owlqn_pseudo_gradient(double * pg, const double * x,
                                  const double * g, const int n, const double c,
                                  const int start, const int end) {
  int i;
  // Compute the negative of gradients.
  for (i = 0;i < start;++i) {
    pg[i] = g[i];
  }

  // Compute the psuedo-gradients.
  for (i = start;i < end;++i) {
    if (x[i] < 0.) {
      // Differentiable.
      pg[i] = g[i] - c;
    } else if (0. < x[i]) {
      // Differentiable.
      pg[i] = g[i] + c;
    } else {
      if (g[i] < -c) {
        // Take the right partial derivative.
        pg[i] = g[i] + c;
      } else if (c < g[i]) {
        // Take the left partial derivative.
        pg[i] = g[i] - c;
      } else {
        pg[i] = 0.;
      }
    }
  }

  for (i = end;i < n;++i) {
    pg[i] = g[i];
  }
}

static void owlqn_project(double * d, const double * sign, const int start,
                          const int end) {
  int i;
  for (i = start;i < end;++i) {
    if (d[i] * sign[i] <= 0) {
      d[i] = 0;
    }
  }
}

}  // namespace lbfgs
}  // namespace nltk
