#include "kMST_MTZ.h"

kMST_MTZ::kMST_MTZ( Digraph& _digraph, int _k, bool _quiet ) :
  kMST_ILP( _digraph, "mtz", _k, _quiet )
{
}

void kMST_MTZ::createModel()
{
  // MTZ is a directed formulation which will then be "cast" back to an
  // undirected solution
  // We have two times as many arcs as we have edges (always in two directions)
  // And we have an artificial root node 0 - so our tree should have (k+1) nodes.
  // k will take the role of (n-1) in the normal MST formulation.
  // Variables:
  //   y(i,j) - arc from i to j is used
  //   x(i,j) - edge (i,j) selected
  //   z(i) - node i selected
  //   u(i) - order of node i
  //   y ... boolean from {0, 1}
  //   x ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  //   u ... float number in [0-k]
  // Constraints:
  //   (1): y(i,j)+y(j,i) = x(i,j)
  //   (2): u(j) >= u(i) + 1 - (1-y(i,j))*k
  //   (3): sum over j y(j,i) <= z(i)
  //   (4): x(i,j) <= z(i)
  //   (5): x(j,i) <= z(i)
  //   (6): sum x(i,j) = k
  //   (7): sum z(j) = k + 1
  //   (8): sum x(0,i) = 1
  //   (9): u(0) = 0
  // Objective function:
  //   min( sum of w[i]*x[i] )
  y = IloBoolVarArray( env, a );
  x = IloBoolVarArray( env, m );
  z = IloBoolVarArray( env, n );
  u = IloNumVarArray( env, n );
  for ( u_int j = 0; j < n; j++ ) {
    // add variables for nodes
    char varname[16];
    sprintf( varname, "z(%d)", j );
    z[j] = IloBoolVar( env, varname );
    sprintf( varname, "u(%d)", j );
    u[j] = IloNumVar( env, 0, k, varname );
  }
  for ( u_int i = 0; i < m; i++ ) {
    // add variables for edges
    char varname[16];
    sprintf( varname, "x(%d,%d)", digraph.edges[i].v1, digraph.edges[i].v2 );
    x[i] = IloBoolVar( env, varname );
  }
  for ( u_int i = 0; i < a; i++ ) {
    char varname[16];
    sprintf( varname, "y(%d,%d)", digraph.arcs[i].v1, digraph.arcs[i].v2 );
    y[i] = IloBoolVar( env, varname );
  }
  for ( u_int i = 0; i < a; i++ ) {
    int o = digraph.arcs[i].o;
    int e = digraph.arcs[i].e;
    // add constraint (1)
    if ( o >= 0 ) {
      model.add( y[i] + y[o] == x[e] );
    }
    else {
      model.add( y[i] == x[e] );
    }
    // add constraint (2)
    model.add( u[digraph.arcs[i].v2] >= u[digraph.arcs[i].v1] + 1 - (1 - y[i])*(k+1) );
  }
  // constraints (3), (4) and (5)
  for ( u_int i = 0; i < n; i++ ) {
    IloExpr constraint3 ( env );
    for ( u_int j = 0; j < a; j++ ) {
      int e = digraph.arcs[j].e;
      if ( digraph.arcs[j].v1 == i ) {
        model.add( x[e] <= z[i] );
      }
      if ( digraph.arcs[j].v2 == i ) {
        model.add( x[e] <= z[i] );
        constraint3 += y[j];
      }
    }
    model.add( constraint3 <= z[i] );
    constraint3.end();
  }
  // Constraint 6
  IloExpr constraint6 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    constraint6 += x[i];
  }
  model.add( constraint6 == k);
  constraint6.end();
  // Constraint 7
  IloExpr constraint7 ( env );
  for ( u_int j = 0; j < n; j++ ) {
    constraint7 += z[j];
  }
  model.add( constraint7 == k+1 );
  constraint7.end();
  // Constraint 8
  IloExpr constraint8 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    if ( digraph.edges[i].v1 == 0 || digraph.edges[i].v2 == 0 ) {
      constraint8 += x[i];
    }
  }
  model.add( constraint8 == 1 );
  constraint8.end();
  model.add( u[0] == 0 );
  // Objective function
  IloExpr objective ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    objective += digraph.edges[i].weight * x[i];
  }
  model.add( IloMinimize( env, objective ) );
  // give it a name for output
  model.setName("k-MST (MTZ)");
}

void kMST_MTZ::outputVars()
{
  // Arc variables
  for ( u_int i = 0; i < a; i++ ) {
    if ( cplex.getValue( y[i] ) ) {
      cout << "Arc " << digraph.edges[i].v1 << "->" << digraph.edges[i].v2 << endl;
    }
  }
  // Node selections
  for ( u_int j = 0; j < n; j++ ) {
    if ( cplex.getValue( z[j] ) ) {
      cout << "Node " << j << endl;
    }
  }
  // Order variables
  for ( u_int i = 0; i < n; i++ ) {
    int order = cplex.getValue( u[i] );
    cout << "Order (" << i << ") = " << order << endl;
  }

}
