/*
 * Issue phyz#3322: search traces for cross-engine comparison.
 *
 * OFF unless IQTREE_TRACE names a file. When on, one JSON object per line is
 * written to that file and nowhere else: stdout, stderr, .log, .iqtree and
 * .treefile are untouched. Every traced value is one IQ-TREE already computes
 * at the point of emission; the trace only reads node, neighbour and NNIMove
 * fields (no likelihood, partial-lh or branch-length write). A field the trace
 * cannot fill without an extra computation is emitted as null instead (see
 * g_nr_final_f_valid_2479).
 *
 * Single-threaded: run traced builds at -T 1 (as the [I369] counters require).
 * Lines are flushed at nni_exit and iter; an abort can lose the tail after the
 * last of those. An unopenable IQTREE_TRACE path is a fatal error (exit 2).
 *
 * Event types ("e"):
 *   header       format/version
 *   nni_enter    one IQTree::optimizeNNI call starts: call, logl (= curScore)
 *   round_start  first candidate of a round: call, round, tree (whole-tree dump)
 *   topo         one scored NNI candidate (the NNI2479 fields, plus call/round)
 *   compat       the sorted positive NNIs passed to getCompatibleNNIs
 *   doNNIs       one doNNIs batch (applied, reverted, or the fallback single)
 *   opt_enter    optimizeAllBranches(int,double,int) entry: tree
 *   visit        one optimizeOneBranch inside that sweep: n1, n2, len_in, len_out
 *   opt_exit     its return value and the tree after it
 *   nni_exit     optimizeNNI returns: steps, applied
 *   perturb      one stochastic iteration's perturbation, with the lnL after it
 *                (none under bootstrap-quartet perturbation, IQP_BOOTSTRAP)
 *   iter         one NNI-search iteration (phase "init": initCandidateTreeSet;
 *                "stochastic": doTreeSearch): post-search lnL and admission
 *   pool         after each iter: the whole candidate set, best first, as the
 *                set holds it, and popSize (phyz#3327)
 *   taxa         once, before the first pool: taxon id -> name (phyz#3327)
 * iter also carries "tree" (the post-search tree string the set was offered).
 * perturb also carries "parent" (the perturbed candidate's tree string) and
 * "random_nni_central" (each random NNI's split before the swap), phyz#3327.
 * nni_enter..nni_exit use the event names and fields of the gdb capture
 * (phyz experiments/2026-09-28-first-divergence-0474/scripts/iqcap.py), with
 * node ids in place of node pointers.
 */
#ifndef IQTRACE_H
#define IQTRACE_H

#include <ostream>
#include <string>
#include <vector>

class Node;
class MTree;

struct IqTraceState {
    int call;          /* optimizeNNI calls so far (1-based once inside one) */
    int round;         /* numSteps of the current optimizeNNI round */
    bool in_nni;       /* inside IQTree::optimizeNNI */
    bool need_round;   /* next candidate starts a round (iqcap.py's rule) */
    bool in_opt;       /* inside optimizeAllBranches(int,double,int) under in_nni */
    bool in_perturb;   /* inside IQTree::doTreePerturbation */
    std::vector<std::string> perturb_nnis;  /* random NNIs applied, each a JSON split */
    std::vector<std::string> perturb_central;  /* their central splits before the swap (phyz#3327) */
    int perturb_attempts;                   /* doRandomNNIs' cntNNI */
};

extern bool iqtrace_enabled;
extern IqTraceState iqtrace_st;

/* The trace file (opened on first use, truncated). */
std::ostream &iqtrace_out();

/* JSON number in %.17g (enough digits to round-trip a double), null if not finite. */
std::string iqtrace_num(double x);
/* JSON string: escapes quote, backslash and control characters; other bytes
 * pass through, so a name that is not UTF-8 gives a line that is not JSON. */
std::string iqtrace_str(const std::string &s);

/* {"<id>": [[nbr_id, length], ...], ...} for the component holding `start`. */
void iqtrace_dump_tree(std::ostream &os, Node *start);
/* {"<id>": "<name>", ...} for the leaves of the component holding `start`. */
void iqtrace_dump_names(std::ostream &os, Node *start);
/* Sorted leaf names on a's side of edge (a, b), as a JSON array. */
std::string iqtrace_split_json(MTree *tree, Node *a, Node *b);

#endif
