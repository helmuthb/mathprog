#include "kMST_MTZ.h"

kMST_MTZ::kMST_MTZ( Instance& _instance, int _k ) :
  kMST_ILP( _instance, "scf", _k )
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
  //   o(i) - order of node i
  //   y ... boolean from {0, 1}
  //   x ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  //   o ... number >= 0
  // Constraints:
  //   (1): !!! o(i) <= k * z(i)
  //        o(i) >= z(i) for i != 0
  //   (2): y(i,j)+y(j,i) = x(i,j)
  //   (3): o(j) >= o(i) + 1 - (1-y(i,j))*k
  //   (4): !!! sum over j y(i,j) <= z(i)
  //   (5): sum over j y(j,i) <= z(i)
  //   (6): sum x(i,j) = k
  //   (7): sum z(j) = k + 1
  //   (8): sum x(0,i) = 1
  // Target function:
  //   min( sum of w[i]*x[i] )
  y = IloBoolVarArray( env, 2 * m );
  x = IloBoolVarArray( env, m );
  z = IloBoolVarArray( env, n );
  o = IloNumVarArray( env, n );
  for ( u_int j = 0; j < n; j++ ) {
    // add variables for nodes
    char varname[16];
    sprintf( varname, "z(%d)", j );
    z[j] = IloBoolVar( env, varname );
    sprintf( varname, "o(%d)", j );
    o[j] = IloNumVar( env, 0, k, varname );
    // o[j] = IloIntVar( env, varname );
    // add constraint (1)
    // model.add( o[j] <= k * z[j] );
  }
  for ( u_int i = 0; i < m; i++ ) {
    // add variables for edges
    char varname[16];
    sprintf( varname, "x(%d,%d)", instance.edges[i].v1, instance.edges[i].v2 );
    x[i] = IloBoolVar( env, varname );
    sprintf( varname, "y(%d,%d)", instance.edges[i].v1, instance.edges[i].v2 );
    y[2*i] = IloBoolVar( env, varname );
    sprintf( varname, "y(%d,%d)", instance.edges[i].v2, instance.edges[i].v1 );
    y[2*i+1] = IloBoolVar( env, varname );
    // add constraint (2)
    model.add( y[2*i] + y[2*i+1] == x[i] );
    // add constraint (3)
    model.add( o[instance.edges[i].v2] >= o[instance.edges[i].v1] + 1 - (1 - y[2*i])*(k+1) );
    model.add( o[instance.edges[i].v1] >= o[instance.edges[i].v2] + 1 - (1 - y[2*i+1])*(k+1) );
  }
  // constraints (4) and (5)
  for ( u_int i = 0; i < n; i++ ) {
    IloExpr constraint4 ( env );
    IloExpr constraint5 ( env );
    for ( u_int j = 0; j < m; j++ ) {
      if ( instance.edges[j].v1 == i ) {
        model.add( x[j] <= z[i] );
        constraint4 += y[2*j];
        constraint5 += y[2*j+1];
      }
      else if ( instance.edges[j].v2 == i ) {
        model.add( x[j] <= z[i] );
        constraint4 += y[2*j+1];
        constraint5 += y[2*j];
      }
    }
    // model.add( constraint4 <= z[i] );
    model.add( constraint5 <= z[i] );
    constraint4.end();
    constraint5.end();
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
    if ( instance.edges[i].v1 == 0 || instance.edges[i].v2 == 0 ) {
      constraint8 += x[i];
    }
  }
  model.add( constraint8 == 1 );
  constraint8.end();
  model.add( o[0] == 0 );
  // Target function
  IloExpr target ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    target += instance.edges[i].weight * x[i];
  }
  model.add( IloMinimize( env, target ) );
  // give it a name for output
  model.setName("k-MST (MTZ)");
}

void kMST_MTZ::outputVars()
{
  // Arc variables
  for ( u_int i = 0; i < m; i++ ) {
    if ( cplex.getValue( y[2*i] ) ) {
      cout << "Arc " << instance.edges[i].v1 << "->" << instance.edges[i].v2 << endl;
    }
    if ( cplex.getValue( y[2*i+1] ) ) {
      cout << "Arc " << instance.edges[i].v2 << "->" << instance.edges[i].v1 << endl;
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
    int order = cplex.getValue( o[i] );
    if (order > 0) {
      cout << "Order (" << i << ") = " << order << endl;
    }
  }

}
