#include "kMST_DCC.h"

kMST_DCC::kMST_DCC( Digraph& _digraph, int _k, bool _quiet ) :
  kMST_ILP( _digraph, "dcc", _k, _quiet )
{
}

void kMST_DCC::createModel()
{
  // DCC is a directed formulation which will then be "cast" back to an
  // undirected solution
  // We have two times as many arcs as we have edges (always in two directions)
  // And we have an artificial root node 0 - so our tree should have (k+1) nodes.
  // k will take the role of (n-1) in the normal MST formulation.
  // Variables:
  //   x(i,j) - arc (i,j) selected(!)
  //   x0(i,j) - edge (i,j) selected
  //   z(i) - node i selected
  //   x ... boolean from {0, 1}
  //   x0 ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  // Constraints:
  //   (1): x0(i,j) <= z(i)
  //   (2): x0(i,j) <= z(j)
  //   (3): x(i,j)+x(j,i) = x0(i,j)
  //   (4): sum x(i,j) = k
  //   (5): sum z(j) = k + 1
  //   (6): sum x(0,i) = 1
  //   ... all other constraints are added dynamically in callbacks ...
  // Target function:
  //   min( sum of w[i]*x[i] )
  x = IloBoolVarArray( env, a );
  IloBoolVarArray x0 = IloBoolVarArray( env, m );
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
    sprintf( varname, "x0(%d,%d)", digraph.edges[i].v1, digraph.edges[i].v2 );
    x0[i] = IloBoolVar( env, varname );
    // add constraint (1) and (2)
    model.add( x0[i] <= z[digraph.edges[i].v1] );
    model.add( x0[i] <= z[digraph.edges[i].v2] );
  }
  for ( u_int i = 0; i < a; i++ ) {
    // add variable for arcs
    char varname[16];
    sprintf( varname, "x(%d,%d)", digraph.arcs[i].v1, digraph.arcs[i].v2 );
    x[i] = IloBoolVar( env, varname );
  }
  for ( u_int i = 0; i < a; i++ ) {
    int e = digraph.arcs[i].e;
    int o = digraph.arcs[i].o;
    // constraint 3
    if ( o >= 0 ) {
      model.add( x[i] + x[o] == x0[e] );
    }
    else {
      model.add( x[i] == x0[e] );
    }
  }
  // Constraint 4
  IloExpr constraint4 ( env );
  for ( u_int i = 0; i < a; i++ ) {
    constraint4 += x[i];
  }
  model.add( constraint4 == k);
  constraint4.end();
  // Constraint 5
  IloExpr constraint5 ( env );
  for ( u_int j = 0; j < n; j++ ) {
    constraint5 += z[j];
  }
  model.add( constraint5 == k+1 );
  constraint5.end();
  // Constraint 6
  IloNumExpr constraint6 ( env );
  for ( u_int i = 0; i < a; i++ ) {
    if ( digraph.arcs[i].v1 == 0 ) {
      constraint6 += x[i];
    }
  }
  model.add( constraint6 == 1 );
  constraint6.end();
  // Objective function
  IloExpr objective ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    objective += digraph.edges[i].weight * x0[i];
  }
  model.add( IloMinimize( env, objective ) );
  // give it a name for output
  model.setName("k-MST (DCC)");
}

void kMST_DCC::outputVars()
{
  // Edge variables
  for ( u_int i = 0; i < a; i++ ) {
    if ( cplex.getValue( x[i] ) ) {
      cout << "Arc " << digraph.arcs[i].v1 << "->" << digraph.arcs[i].v2 << endl;
    }
  }
  // Node selections
  for ( u_int j = 0; j < n; j++ ) {
    if ( cplex.getValue( z[j] ) ) {
      cout << "Node " << j << endl;
    }
  }
}
