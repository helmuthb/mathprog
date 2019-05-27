#include "kMST_CEC.h"

kMST_CEC::kMST_CEC( Digraph& _digraph, int _k, bool _quiet,
                    bool _cut_early, int _max_cuts ) :
  kMST_ILP( _digraph, "cec", _k, _quiet, _cut_early, _max_cuts )
{
}

void kMST_CEC::createModel()
{
  // CEC is a directed formulation which will then be "cast" back to an
  // undirected solution
  // We have two times as many arcs as we have edges (always in two directions)
  // And we have an artificial root node 0 - so our tree should have (k+1) nodes.
  // k will take the role of (n-1) in the normal MST formulation.
  // Variables:
  //   x(i,j) - edge (i,j) selected
  //   y(i,j) - arc (i,j) selected
  //   z(i) - node i selected
  //   x ... boolean from {0, 1}
  //   y ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  // Constraints:
  //   (1): x(i,j) <= z(i)
  //   (2): x(i,j) <= z(j)
  //   (3): y(i,j)+y(j,i) = x(i,j)
  //   (4): sum x(i,j) = k
  //   (5): sum z(j) = k + 1
  //   (6): sum y(0,i) = 1
  //   ... all other constraints are added dynamically in callbacks ...
  // Target function:
  //   min( sum of w[i]*x[i] )
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
    // add constraint (1) and (2)
    model.add( x[i] <= z[digraph.edges[i].v1] );
    model.add( x[i] <= z[digraph.edges[i].v2] );
  }
  for ( u_int i = 0; i < a; i++ ) {
    // add variable for arcs
    char varname[16];
    sprintf( varname, "y(%d,%d)", digraph.arcs[i].v1, digraph.arcs[i].v2 );
    y[i] = IloBoolVar( env, varname );
  }
  for ( u_int i = 0; i < a; i++ ) {
    int e = digraph.arcs[i].e;
    int o = digraph.arcs[i].o;
    // constraint 3
    if ( o >= 0 ) {
      model.add( y[i] + y[o] == x[e] );
    }
    else {
      model.add( y[i] == x[e] );
    }
  }
  // Constraint 4
  IloExpr constraint4 ( env );
  for ( u_int i = 0; i < m; i++ ) {
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
      constraint6 += y[i];
    }
  }
  model.add( constraint6 == 1 );
  constraint6.end();
  // Objective function
  IloExpr objective ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    objective += digraph.edges[i].weight * x[i];
  }
  model.add( IloMinimize( env, objective ) );
  // give it a name for output
  model.setName("k-MST (CEC)");
}

void kMST_CEC::outputVars()
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
}
