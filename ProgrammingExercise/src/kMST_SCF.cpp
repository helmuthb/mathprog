#include "kMST_SCF.h"

kMST_SCF::kMST_SCF( Digraph& _digraph, int _k ) :
  kMST_ILP( _digraph, "scf", _k )
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
  //   f(i,j), f(j,i) - flow from i to j and from j to i
  //   x(i,j) - edge (i,j) selected
  //   z(i) - node i selected
  //   f ... float >= 0
  //   x ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  // Constraints:
  //   (1): f(i,j) <= k * x(i,j), i, j != 0
  //   (2): f(j,i) <= k * x(i,j), i, j != 0
  //   (3): x(i, j) <= z(i)
  //   (4): x(i, j) <= z(j)
  //   (5): f going to 0 = 0
  //        f going from 0 to i = k * x(0,i)
  //   (6): i not 0, sum over f going from i - sum over f going to i = -1 * z(i)
  //   (7): sum x(i,j) = k
  //   (8): sum z(j) = k + 1
  //   (9): sum x(0,i) = 1
  // Target function:
  //   min( sum of w[i]*x[i] )
  f = IloNumVarArray( env, 2 * m );
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
    sprintf( varname, "f(%d,%d)", digraph.edges[i].v1, digraph.edges[i].v2 );
    f[2*i] = IloNumVar( env, 0, k, varname );
    sprintf( varname, "f(%d,%d)", digraph.edges[i].v2, digraph.edges[i].v1 );
    f[2*i+1] = IloNumVar( env, 0, k, varname );
    sprintf( varname, "x(%d,%d)", digraph.edges[i].v1, digraph.edges[i].v2 );
    x[i] = IloBoolVar( env, varname );
    // add constraints (1) and (2)
    if ( digraph.edges[i].v1 != 0 && digraph.edges[i].v2 != 0 ) {
      model.add( f[2*i] <= k*x[i] );
      model.add( f[2*i+1] <= k*x[i] );
    }
    // add constraints (3) and (4)
    model.add( x[i] <= z[digraph.edges[i].v1] );
    model.add( x[i] <= z[digraph.edges[i].v2] );
  }
  // Constraint 5
  for ( u_int i = 0; i < m; i++ ) {
    // we look for edges involving 0
    if ( digraph.edges[i].v1 == 0 ) {
      model.add( f[2*i+1] == 0 );
      model.add( f[2*i] == k * x[i] );
    }
    else if ( digraph.edges[i].v2 == 0 ) {
      model.add( f[2*i] == 0 );
      model.add( f[2*i+1] == k * x[i] );
    }
  }
  // Constraint 6
  // j ... the node we look at currently
  for ( u_int j = 1; j < n; j++ ) {
    IloExpr constraint6( env );
    for ( u_int i = 0; i < m; i++ ) {
      if ( digraph.edges[i].v1 == j ) {
        constraint6 -= f[2*i+1];
        if ( digraph.edges[i].v2 != 0 ) {
          constraint6 += f[2*i];
        }
      }
      else if ( digraph.edges[i].v2 == j ) {
        constraint6 -= f[2*i];
        if ( digraph.edges[i].v1 != 0 ) {
          constraint6 += f[2*i+1];
        }
      }
    }
    model.add( constraint6 == -1*z[j] );
    constraint6.end();
  }
  // Constraint 7
  IloExpr constraint7 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    constraint7 += x[i];
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
  IloExpr constraint9 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    if ( digraph.edges[i].v1 == 0 || digraph.edges[i].v2 == 0 ) {
      constraint9 += x[i];
    }
  }
  model.add( constraint9 == 1 );
  constraint9.end();
  // Target function
  IloExpr target ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    target += digraph.edges[i].weight * x[i];
  }
  model.add( IloMinimize( env, target ) );
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
  for ( u_int i = 0; i < 0; i++ ) {
    int flow = cplex.getValue( f[2*i] );
    cout << "Flow " << digraph.edges[i].v1 << "->" << digraph.edges[i].v2;
    cout << "=" << flow << endl;
    flow = cplex.getValue( f[2*i+1] );
    cout << "Flow " << digraph.edges[i].v2 << "->" << digraph.edges[i].v1;
    cout << "=" << flow << endl;
  }

}
