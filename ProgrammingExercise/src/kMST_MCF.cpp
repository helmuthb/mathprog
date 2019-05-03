#include "kMST_MCF.h"

kMST_MCF::kMST_MCF( Instance& _instance, int _k ) :
  kMST_ILP( _instance, "scf", _k )
{
}

void kMST_MCF::createModel()
{
  // MCF is a directed formulation which will then be "cast" back to an
  // undirected solution
  // We have two times as many arcs as we have edges (always in two directions)
  // And we have an artificial root node 0 - so our tree should have (k+1) nodes.
  // k will take the role of (n-1) in the normal MST formulation.
  // Variables:
  //   f^l(i,j), f^l(j,i) - flow targeted to l from i to j and from j to i, l>0
  //   x(i,j) - edge (i,j) selected
  //   z(i) - node i selected
  //   f ... float in the range [0, 1]
  //   x ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  // Constraints:
  //   (1): f^l(i,j) <= x(i,j), l != 0
  //   (2): f^l(j,i) <= x(i,j), l != 0
  //   (3): f^l(i,j) <= z(l), l != 0
  //   (5): x(i,j) <= z(i)
  //   (6): x(i,j) <= z(j)
  //   (7): sum over f^l going from 0 = 1 * z(l), l != 0
  //        f^l going to 0 = 0
  //   (8): sum over f^l going from l - sum over f^l going to l = -1 * z(l), l != 0
  //   (9): sum over f^l going from j - sum over f^l going to j = 0 (for j != l, j != 0, l != 0)
  //   (10): sum x(i,j) = k
  //   (11): sum z(j) = k + 1
  //   (12): sum x(0,i) = 1
  // Target function:
  //   min( sum of w[i]*x[i] )
  f = IloNumVarArray( env, 2 * m * (n - 1) );
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
    sprintf( varname, "x(%d,%d)", instance.edges[i].v1, instance.edges[i].v2 );
    x[i] = IloBoolVar( env, varname );
    for ( u_int l = 1; l < n; l++ ) {
      sprintf( varname, "f(%d)(%d,%d)", l, instance.edges[i].v1, instance.edges[i].v2 );
      f[2*(l-1)*m+2*i] = IloNumVar( env, 0, 1, varname );
      sprintf( varname, "f(%d)(%d,%d)", l, instance.edges[i].v2, instance.edges[i].v1 );
      f[2*(l-1)*m+2*i+1] = IloNumVar( env, 0, 1, varname );
      // add constraints (1), (2), (3)
      model.add( f[2*(l-1)*m+2*i] <= x[i] );
      model.add( f[2*(l-1)*m+2*i+1] <= x[i] );
      model.add( f[2*(l-1)*m+2*i] <= z[l] );
      model.add( f[2*(l-1)*m+2*i+1] <= z[l] );
    }
    // add constraints (5) and (6)
    model.add( x[i] <= z[instance.edges[i].v1] );
    model.add( x[i] <= z[instance.edges[i].v2] );
  }
  // Constraint 7
  for ( u_int l = 1; l < n; l++ ) {
    IloExpr constraint7( env );
    for ( u_int i = 0; i < m; i++ ) {
      // we look for edges involving 0
      if ( instance.edges[i].v1 == 0 ) {
        constraint7 += f[2*(l-1)*m+2*i];
        // constraint7 -= f[2*(l-1)*m+2*i+1];
        model.add( f[2*(l-1)*m+2*i+1] == 0 );
      }
      else if ( instance.edges[i].v2 == 0 ) {
        constraint7 += f[2*(l-1)*m+2*i+1];
        // constraint7 -= f[2*(l-1)*m+2*i];
        model.add( f[2*(l-1)*m+2*i] == 0 );
      }
    }
    model.add( constraint7 == z[l] );
    constraint7.end();
  }
  // Constraint 8
  for ( u_int l = 1; l < n; l++ ) {
    IloExpr constraint8( env );
    for ( u_int i = 0; i < m; i++ ) {
      if ( instance.edges[i].v1 == l ) {
        constraint8 += f[2*(l-1)*m+2*i];
        constraint8 -= f[2*(l-1)*m+2*i+1];
      }
      else if ( instance.edges[i].v2 == l ) {
        constraint8 += f[2*(l-1)*m+2*i+1];
        constraint8 -= f[2*(l-1)*m+2*i];
      }
    }
    model.add( constraint8 == -z[l] );
    constraint8.end();
  }
  // Constraint 9
  // j ... the node we look at currently
  // l ... the target node
  for ( u_int j = 1; j < n; j++ ) {
    for (u_int l = 1; l < n; l++ ) {
      if ( j != l ) {
        IloExpr constraint9( env );
        for ( u_int i = 0; i < m; i++ ) {
          if ( instance.edges[i].v1 == j ) {
            constraint9 += f[2*(l-1)*m+2*i];
            constraint9 -= f[2*(l-1)*m+2*i+1];
          }
          else if ( instance.edges[i].v2 == j ) {
            constraint9 += f[2*(l-1)*m+2*i+1];
            constraint9 -= f[2*(l-1)*m+2*i];
          }
        }
        model.add( constraint9 == 0 );
        constraint9.end();
      }
    }
  }
  // Constraint 10
  IloExpr constraint10 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    constraint10 += x[i];
  }
  model.add( constraint10 == k);
  constraint10.end();
  // Constraint 11
  IloExpr constraint11 ( env );
  for ( u_int j = 0; j < n; j++ ) {
    constraint11 += z[j];
  }
  model.add( constraint11 == k+1 );
  constraint11.end();
  // Constraint 12
  IloExpr constraint12 ( env );
  for ( u_int i = 0; i < m; i++ ) {
    if ( instance.edges[i].v1 == 0 || instance.edges[i].v2 == 0 ) {
      constraint12 += x[i];
    }
  }
  model.add( constraint12 == 1 );
  constraint12.end();
  // Target function
  IloExpr target ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    target += instance.edges[i].weight * x[i];
  }
  model.add( IloMinimize( env, target ) );
  // give it a name for output
  model.setName("k-MST (MCF)");
}

void kMST_MCF::outputVars()
{
  // Edge variables
  for ( u_int i = 0; i < m; i++ ) {
    if ( cplex.getValue( x[i] ) ) {
      cout << "Edge " << instance.edges[i].v1 << "->" << instance.edges[i].v2 << endl;
    }
  }
  // Node selections
  for ( u_int j = 0; j < n; j++ ) {
    if ( cplex.getValue( z[j] ) ) {
      cout << "Node " << j << endl;
    }
  }
  // Flow variables
  for ( u_int i = 0; i < m; i++ ) {
    for ( u_int j = 1; j < n; j++ ) {
      int flow = cplex.getValue( f[2*m*(j-1)+2*i] );
      if (flow > 0) {
        cout << "Flow (" << j << ") " << instance.edges[i].v1 << "->" << instance.edges[i].v2;
        cout << "=" << flow << endl;
      }
      flow = cplex.getValue( f[2*m*(j-1)+2*i+1] );
      if (flow > 0) {
        cout << "Flow (" << j << ") " << instance.edges[i].v2 << "->" << instance.edges[i].v1;
        cout << "=" << flow << endl;
      }
    }
  }

}
