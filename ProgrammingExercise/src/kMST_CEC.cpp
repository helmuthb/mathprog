#include "kMST_CEC.h"

kMST_CEC::kMST_CEC( Digraph& _digraph, int _k ) :
  kMST_ILP( _digraph, "scf", _k )
{
}

void kMST_CEC::createModel()
{
  // SCF is a directed formulation which will then be "cast" back to an
  // undirected solution
  // We have two times as many arcs as we have edges (always in two directions)
  // And we have an artificial root node 0 - so our tree should have (k+1) nodes.
  // k will take the role of (n-1) in the normal MST formulation.
  // Variables:
  //   x(i,j) - edge (i,j) selected
  //   z(i) - node i selected
  //   x ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  // Constraints:
  //   (1): x(i,j) <= z(i)
  //   (2): x(i,j) <= z(j)
  //   (3): sum x(i,j) = k
  //   (4): sum z(j) = k + 1
  //   (5): sum x(0,i) = 1
  //   ... all other constraints are added dynamically in callbacks ...
  // Target function:
  //   min( sum of w[i]*x[i] )
  x = IloBoolVarArray( env, m );
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
    // add constraints (1) and (2)
    model.add( x[i] <= z[digraph.edges[i].v1] );
    model.add( x[i] <= z[digraph.edges[i].v2] );
  }
  // Constraint 3
  IloExpr constraint3 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    constraint3 += x[i];
  }
  model.add( constraint3 == k);
  constraint3.end();
  // Constraint 4
  IloExpr constraint4 ( env );
  for ( u_int j = 0; j < n; j++ ) {
    constraint4 += z[j];
  }
  model.add( constraint4 == k+1 );
  constraint4.end();
  // Constraint 5
  IloExpr constraint5 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    if ( digraph.edges[i].v1 == 0 || digraph.edges[i].v2 == 0 ) {
      constraint5 += x[i];
    }
  }
  model.add( constraint5 == 1 );
  constraint5.end();
  // Target function
  IloExpr target ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    target += digraph.edges[i].weight * x[i];
  }
  model.add( IloMinimize( env, target ) );
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
