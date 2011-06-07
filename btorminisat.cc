/*  Boolector: Satisfiablity Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2011 Armin Biere, FMV, JKU.
 *
 *  This file is part of Boolector.
 *
 *  Boolector is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Boolector is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef BTOR_USE_MINISAT

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "minisat/simp/SimpSolver.h"

extern "C" {

#include "btorminisat.h"
#include "btorsat.h"

#include <cassert>
#include <cstring>

using namespace Minisat;

class BtorMiniSAT : public Solver
{
  vec<Lit> assumptions, clause;
  signed char *fmap;
  int szfmap;
  bool nomodel;
  Lit import (int lit)
  {
    assert (0 < abs (lit) && abs (lit) <= nVars ());
    return mkLit (Var (abs (lit) - 1), (lit < 0));
  }
  void reset ()
  {
    if (fmap) delete[] fmap, fmap = 0, szfmap = 0;
  }
  void ana ()
  {
    fmap = new signed char[szfmap = nVars ()];
    memset (fmap, 0, szfmap);
    for (int i = 0; i < conflict.size (); i++)
    {
      int tmp = var (conflict[i]);
      assert (0 <= tmp && tmp < szfmap);
      fmap[tmp] = 1;
    }
  }

 public:
  BtorMiniSAT () : szfmap (0), fmap (0), nomodel (true) {}
  ~BtorMiniSAT () { reset (); }
  int inc ()
  {
    int res = newVar ();
    assert (0 <= res && res == nVars () - 1);
    return res + 1;
  }
  void assume (int lit)
  {
    nomodel = true;
    assumptions.push (import (lit));
  }
  void add (int lit)
  {
    nomodel = true;
    if (lit)
      clause.push (import (lit));
    else
      addClause (clause), clause.clear ();
  }
  int sat ()
  {
    reset ();
    bool res = solve (assumptions);
    assumptions.clear ();
    nomodel = !res;
    return res ? 10 : 20;
  }
  int failed (int lit)
  {
    if (!fmap) ana ();
    int tmp = var (import (lit));
    assert (0 <= tmp && tmp < nVars ());
    return fmap[tmp];
  }
  int fixed (int lit)
  {
    Var v   = var (import (lit));
    int idx = v, res;
    assert (0 <= idx && idx < nVars ());
    lbool val = assigns[idx];
    if (val == l_Undef || level (v))
      res = 0;
    else
      res = (val == l_True) ? 1 : -1;
    if (lit < 0) res = -res;
    return res;
  }
  int deref (int lit)
  {
    if (nomodel) return fixed (lit);
    lbool res = modelValue (import (lit));
    return (res == l_True) ? 1 : -1;
  }
};

void *
btor_minisat_init (BtorSATMgr *)
{
  return new BtorMiniSAT ();
}

const char *
btor_minisat_version (void)
{
  return "unknown";
}

void
btor_minisat_add (BtorSATMgr *smgr, int lit)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  solver->add (lit);
}

int
btor_minisat_sat (BtorSATMgr *smgr)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  return solver->sat ();
}

int
btor_minisat_deref (BtorSATMgr *smgr, int lit)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  return solver->deref (lit);
}

void
btor_minisat_reset (BtorSATMgr *smgr)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  delete solver;
}

int
btor_minisat_inc_max_var (BtorSATMgr *smgr)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  return solver->inc ();
}

int
btor_minisat_variables (BtorSATMgr *smgr)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  return solver->nVars ();
}

void
btor_minisat_assume (BtorSATMgr *smgr, int lit)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  solver->assume (lit);
}

int
btor_minisat_fixed (BtorSATMgr *smgr, int lit)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  solver->fixed (lit);
}

int
btor_minisat_failed (BtorSATMgr *smgr, int lit)
{
  BtorMiniSAT *solver = (BtorMiniSAT *) BTOR_GET_SOLVER_SAT (smgr);
  return solver->failed (lit);
}

void
btor_minisat_set_output (BtorSATMgr *, FILE *)
{
}

void
btor_minisat_set_prefix (BtorSATMgr *, const char *)
{
}

void
btor_minisat_enable_verbosity (BtorSATMgr *)
{
}

void
btor_minisat_stats (BtorSATMgr *)
{
}

int
btor_minisat_changed (BtorSATMgr *)
{
  return 1;
}

int
btor_minisat_inconsistent (BtorSATMgr *)
{
  return 0;
}
};

#endif