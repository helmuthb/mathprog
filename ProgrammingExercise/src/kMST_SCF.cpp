#include "kMST_SCF.h"

kMST_SCF::kMST_SCF( Digraph& _digraph, int _k, bool _quiet ) :
  kMST_ILP( _digraph, "scf", _k, _quiet )
{
}

void kMST_SCF::createModel()
{
  // SCF is a directed formulation which will then be "cast" back to an
  // undirected solution
  // We have two times as many arcs as we have edges (always in two directions)
  // And we have an artificial root node 0 - so our tree should have (k+1) nodes.
  // k will take the role of (n-1) in the normal MST formulation.
  // Variables:
  //   f(i,j) - flow from i to j
  //   x(i,j) - edge (i,j) selected
  //   y(i,j) - arc (i,j) selected
  //   z(i) - node i selected
  //   f ... float >= 0
  //   x ... boolean from {0, 1}
  //   y ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  // Constraints:
  //   (1): f(i,j) <= k * y(i,j), i, j != 0
  //        y(i,j) + y(j,i) = x(i,j)
  //   (3): x(i,j) <= z(i)
  //   (4): x(i,j) <= z(j)
  //   (5): f going from 0 to i = k * y(0,i)
  //   (6): sum over f going from i - sum over f going to i = -z(i), i != 0
  //   (7): sum x(i,j) = k
  //   (8): sum z(j) = k + 1
  //   (9): sum y(0,i) = 1
  // Objective function:
  //   min( sum of w[i]*x[i] )
  f = IloNumVarArray( env, a );
  x = IloBoolVarArray( env, m );
  IloBoolVarArray y = IloBoolVarArray( env, a );
  z = IloBoolVarArray( env, n );
  for ( u_int j = 0; j < n; j++ ) {
    // add variables for nodes
    char varname[16];
    sprintf( varname, "z(%d)", j );
    z[j] = IloBoolVar( env, varname );
  }
  for ( u_int i = 0; i < m; i++ ) {
    // add variables for edges
    char varname[16];
    sprintf( varname, "x(%d,%d)", digraph.edges[i].v1, digraph.edges[i].v2 );
    x[i] = IloBoolVar( env, varname );
  }
  for ( u_int i = 0; i < a; i++ ) {
    // add variable for arcs
    char varname[16];
    sprintf( varname, "y(%d,%d)", digraph.arcs[i].v1, digraph.arcs[i].v2 );
    y[i] = IloBoolVar( env, varname );
    sprintf( varname, "f(%d,%d)", digraph.arcs[i].v1, digraph.arcs[i].v2 );
    f[i] = IloNumVar( env, 0, k, varname );
  }
  for ( u_int i = 0; i < a; i++ ) {
    // add constraints (1) and (2)
    if ( digraph.arcs[i].v1 != 0 ) {
      model.add( f[i] <= k*y[i] );
    }
    int e = digraph.arcs[i].e;
    int o = digraph.arcs[i].o;
    if ( o >= 0 ) {
      model.add( y[i] + y[o] == x[e] );
    }
    else {
      model.add( y[i] == x[e] );
    }
    // add constraints (3) and (4)
    model.add( x[e] <= z[digraph.arcs[i].v1] );
    model.add( x[e] <= z[digraph.arcs[i].v2] );
  }
  // Constraint 5
  for ( u_int i = 0; i < a; i++ ) {
    // we look for edges starting from 0
    if ( digraph.arcs[i].v1 == 0 ) {
      model.add( f[i] == k * y[i] );
    }
  }
  // Constraint 6
  // j ... the node we look at currently
  for ( u_int j = 1; j < n; j++ ) {
    IloExpr constraint6( env );
    for ( u_int i = 0; i < a; i++ ) {
      if ( digraph.arcs[i].v1 == j ) {
        constraint6 += f[i];
      }
      else if ( digraph.arcs[i].v2 == j ) {
        constraint6 -= f[i];
      }
    }
    model.add( constraint6 == -1*z[j] );
    constraint6.end();
  }
  // Constraint 7
  IloNumExpr constraint7 ( env );
  for ( u_int e = 0; e < m; e++ ) {
    constraint7 += x[e];
  }
  model.add( constraint7 == k);
  constraint7.end();
  // Constraint 8
  IloExpr constraint8 ( env );
  for ( u_int j = 0; j < n; j++ ) {
    constraint8 += z[j];
  }
  model.add( constraint8 == k+1 );
  constraint8.end();
  // Constraint 9
  IloNumExpr constraint9 ( env );
  for ( u_int i = 0; i < a; i++ ) {
    if ( digraph.arcs[i].v1 == 0 ) {
      constraint9 += y[i];
    }
  }
  model.add( constraint9 == 1 );
  constraint9.end();
  // Objective function
  IloExpr objective ( env );
  for ( u_int e = 0; e < m; e ++ ) {
    objective += digraph.edges[e].weight * x[e];
  }
  model.add( IloMinimize( env, objective ) );
  // give it a name for output
  model.setName("k-MST (SCF)");
}

void kMST_SCF::outputVars()
{
  // Edge variables
  for ( u_int i = 0; i < m; i++ ) {
    if ( cplex.getValue( x[i] ) ) {
      cout << "Edge " << digraph.edges[i].v1 << "->" << digraph.edges[i].v2 << endl;
    }
  }
  // Node selections
  for ( u_int j = 0; j < n; j++ ) {
    if ( cplex.getValue( z[j] ) ) {
      cout << "Node " << j << endl;
    }
  }
  // Flow variables
  for ( u_int i = 0; i < a; i++ ) {
    int flow = cplex.getValue( f[i] );
    cout << "Flow " << digraph.arcs[i].v1 << "->" << digraph.arcs[i].v2;
    cout << "=" << flow << endl;
  }

}
