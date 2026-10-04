/*
 * EPIC #369 operation counters.
 *
 * Plain non-atomic counters, incremented at the single dispatch point of each
 * operation. Read/emitted only when IQTREE_I369 is set. They are NOT
 * thread-safe: every number reported from them was produced at -T 1, where
 * IQ-TREE runs one packet on one thread and no race is possible. Under -T >1
 * these counts would be unreliable and must not be used.
 */
#ifndef I369_H
#define I369_H

struct I369Counters {
    /* A: downward/lower CLV at an internal node, one node x one pattern block */
    unsigned long long clv_partial;
    /* B: nearest thing to a distinct upper/root-directed partial */
    unsigned long long reorient;          /* calls, most of which early-return */
    unsigned long long reorient_takeover; /* calls that actually moved a memory slot */
    /* C: one objective-function call inside the per-branch optimiser */
    unsigned long long lk_derv;       /* Newton path: value+1st+2nd derivative */
    unsigned long long lk_function;   /* Brent path: value only */
    /* D: complete traversal ending in a root log-likelihood */
    unsigned long long lk_branch;
    unsigned long long lk_full;       /* computeLikelihood() wrapper */
    /* E: P(t) construction */
    unsigned long long transmat;        /* explicit P(t) build: NONREV kernel only */
    unsigned long long eigen_decomp;    /* rate-matrix eigendecomposition */
    /* F: root likelihood summation over patterns */
    unsigned long long lk_frombuffer;
    unsigned long long lk_frombuffer_fast;      /* served from the buffer */
    unsigned long long lk_frombuffer_fallback;  /* fell through to a full branch traversal */
    /* branch-length optimisation shape */
    unsigned long long bl_one;            /* optimizeOneBranch calls */
    unsigned long long bl_all;            /* optimizeAllBranches calls */
    unsigned long long bl_sweeps;         /* whole-tree sweeps inside those calls */
    unsigned long long bl_branch_visits;  /* branch visits issued by sweeps */
    /* observed Newton behaviour */
    unsigned long long nr_calls, nr_iters;
    unsigned long long nr_exit_immediate;        /* converged before any NR step */
    unsigned long long nr_exit_grad;             /* |f| < xacc with df > 0 */
    unsigned long long nr_exit_dx;               /* |dx| < xacc */
    unsigned long long nr_exit_cap;              /* j == maxNRStep */
    unsigned long long nr_exit_bisect_collapse;  /* xl == rts */
    unsigned long long nr_exit_step_collapse;    /* temp == rts */
    unsigned long long nr_hist[12];              /* iterations, 11 = >10 */
    /* CLV invalidation: blanket vs targeted */
    unsigned long long inval_blanket_calls;
    unsigned long long inval_blanket_slots;
    unsigned long long inval_blanket_valid;  /* held a VALID partial -> swept in */
    unsigned long long inval_target_calls;
    unsigned long long inval_target_valid;
    /* Issue phyz#3322: #2815's two census counters the set above lacked */
    unsigned long long trav_partial;   /* traversal_info entries: partial-lh vectors to recompute */
    unsigned long long bl_recursive;   /* recursive optimizeAllBranches(node, dad, ...) calls */
};

extern I369Counters i369c;
extern bool i369_enabled;

/* Issue #2930: cumulative counters after the final model optimisation. */
void i369_emit_end_totals();

/* Issue phyz#3322: #2815's census line ("OPS2815 mark=<mark> ..."), cumulative,
 * written to the IQTREE_I369 file instead of #2815's ungated cout. */
void i369_emit_ops2815(const char *mark);

/* record one completed Newton call: iteration count + which exit fired */
inline void i369_nr_done(int iters, unsigned long long &reason) {
    if (!i369_enabled) return;
    i369c.nr_iters += (unsigned long long) iters;
    i369c.nr_hist[iters > 10 ? 11 : iters]++;
    reason++;
}

#endif
