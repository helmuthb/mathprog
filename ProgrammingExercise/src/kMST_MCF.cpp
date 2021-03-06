#include "kMST_MCF.h"

kMST_MCF::kMST_MCF( Digraph& _digraph, int _k, bool _quiet ) :
  kMST_ILP( _digraph, "mcf", _k, _quiet )
{
}

void kMST_MCF::createModel()
{
  // MCF is a directed formulation which will then be "cast" back to an
  // undirected solution
  // We use the "Digrah" presentation
  // We have an artificial root node 0 - so our tree should have (k+1) nodes.
  // And for each edge there are two arcs - exception is the root node
  // (no arc into the root node)
  // k will take the role of (n-1) in the normal MST formulation.
  // Variables:
  //   f^l(i,j) - flow targeted to l from i to j, l != 0
  //   x(i,j) - edge (i,j) selected
  //   y(i,j) - arc (i,j) selected
  //   z(i) - node i selected
  //   f ... float in the range [0, 1]
  //   x ... boolean from {0, 1}
  //   z ... boolean from {0, 1}
  // Constraints:
  //   (1): f^l(i,j) <= y(i,j), l != 0
  //   (2): y(i,j) + y(j,i) = x(i,j)
  //   (3): x(i,j) <= z(i)
  //   (4): x(i,j) <= z(j)
  //   (5): sum y(i,j) = z(j)
  //   (6): sum over f^l going from j - sum over f^l going to j = 0 (for j != l, j != 0, l != 0)
  //   (7): sum over f^l going from 0 = 1 * z(l), l != 0
  //   (8): sum over f^l going from l - sum over f^l going to l = -1 * z(l), l != 0
  //   (9): sum y(i,j) = k
  //   (10): sum x(0,i) = 1
  // Objective function:
  //   min( sum of w[i]*x[i] )
  f = IloNumVarArray( env, a * (n-1) );
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
    // add variables for arcs
    char varname[16];
    sprintf( varname, "y(%d,%d)", digraph.arcs[i].v1, digraph.arcs[i].v2 );
    y[i] = IloBoolVar( env, varname );
    for ( u_int l = 1; l < n; l++ ) {
      sprintf( varname, "f(%d)(%d,%d)", l, digraph.arcs[i].v1, digraph.arcs[i].v2 );
      f[(l-1)*a+i] = IloNumVar( env, 0, 1, varname );
      // add constraints (1)
      model.add( f[(l-1)*a+i] <= y[i] );
    }
  }
  for ( u_int i = 0; i < a; i++ ) {
    int e = digraph.arcs[i].e;
    int o = digraph.arcs[i].o;
    // constraint (2)
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
  // constraint5
  for ( u_int i = 1; i < n; i++ ) {
    IloNumExpr constraint5 ( env );
    int count = 0;
    for ( u_int j = 0; j < a; j++ ) {
      if ( digraph.arcs[j].v2 == i ) {
        constraint5 += y[j];
        count++;
      }
    }
    if ( count > 0 ) {
      model.add( constraint5 == z[i] );
    }
  }
  // Constraint 6
  // j ... the node we look at currently
  // l ... the target node
  for ( u_int j = 1; j < n; j++ ) {
    for (u_int l = 1; l < n; l++ ) {
      if ( j != l ) {
        IloNumExpr constraint6( env );
        for ( u_int i = 0; i < a; i++ ) {
          if ( digraph.arcs[i].v1 == j ) {
            constraint6 += f[(l-1)*a+i];
          }
          else if ( digraph.arcs[i].v2 == j ) {
            constraint6 -= f[(l-1)*a+i];
          }
        }
        model.add( constraint6 == 0 );
        constraint6.end();
      }
    }
  }
  // Constraint 7
  for ( u_int l = 1; l < n; l++ ) {
    IloNumExpr constraint7( env );
    for ( u_int i = 0; i < a; i++ ) {
      // we look for arcs coming from 0
      if ( digraph.arcs[i].v1 == 0 ) {
        constraint7 += f[(l-1)*a+i];
      }
    }
    model.add( constraint7 == z[l] );
    constraint7.end();
  }
  // Constraint 8
  for ( u_int l = 1; l < n; l++ ) {
    IloNumExpr constraint8( env );
    for ( u_int i = 0; i < a; i++ ) {
      if ( digraph.arcs[i].v1 == l ) {
        constraint8 += f[(l-1)*a+i];
      }
      else if ( digraph.arcs[i].v2 == l ) {
        constraint8 -= f[(l-1)*a+i];
      }
    }
    model.add( constraint8 == -z[l] );
    constraint8.end();
  }
  // Constraint 9
  IloNumExpr constraint9 ( env );
  for ( u_int i = 0; i < a; i++ ) {
    constraint9 += y[i];
  }
  model.add( constraint9 == k);
  constraint9.end();
  // Constraint 10
  IloNumExpr constraint10 ( env );
  for ( u_int i = 0; i < a; i++ ) {
    if ( digraph.arcs[i].v1 == 0 ) {
      constraint10 += y[i];
    }
  }
  model.add( constraint10 == 1 );
  constraint10.end();
  // Objective function
  IloNumExpr objective ( env );
  for ( u_int i = 0; i < m; i ++ ) {
    if ( digraph.edges[i].weight != 0 ) {
      objective += digraph.edges[i].weight * x[i];
    }
  }
  model.add( IloMinimize( env, objective ) );
  // give it a name for output
  model.setName("k-MST (MCF)");
}

void kMST_MCF::outputVars()
{
  // Edge variables
  for ( u_int i = 0; i < m; i++ ) {
    if ( cplex.getValue( x[i] ) ) {
      cout << "Edge " << digraph.edges[i].v1 << "->" <<
                        digraph.edges[i].v2 << endl;
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
    for ( u_int j = 1; j < n; j++ ) {
      int flow = cplex.getValue( f[m*(j-1)+i] );
      if (flow > 0) {
        cout << "Flow (" << j << ") " << digraph.arcs[i].v1 <<
                "->" << digraph.arcs[i].v2;
        cout << "=" << flow << endl;
      }
    }
  }

}
