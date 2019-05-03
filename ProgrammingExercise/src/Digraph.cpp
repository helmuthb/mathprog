#include "Digraph.h"

Digraph::Digraph( string file, bool quiet ) : Instance( file, quiet ),
  n_arcs( 2*n_edges - n_nodes + 1 ), arcs( n_arcs )
{
  // add two arc for each edge
  // except for the root node: only from it, not to it
  u_int j = 0;
  for ( u_int i = 0; i < n_edges; i++ ) {
    if ( edges[i].v1 == 0 && edges[i].v2 == 0 ) {
      // do nothing ...
      continue;
    }
    if ( edges[i].v1 == 0 ) {
      // add only arc from root node
      arcs[j].v1 = 0;
      arcs[j].v2 = edges[i].v2;
      arcs[j].e = i;
      arcs[j].o = -1;
      arcs[j].weight = edges[i].weight;
      j++;
    }
    else if ( edges[i].v2 == 0 ) {
      // add only arc from root node
      arcs[j].v1 = 0;
      arcs[j].v2 = edges[i].v1;
      arcs[j].e = i;
      arcs[j].o = -1;
      arcs[j].weight = edges[i].weight;
      j++;
    }
    else {
      arcs[j].v1 = edges[i].v1;
      arcs[j].v2 = edges[i].v2;
      arcs[j].e = i;
      arcs[j].o = j+1;
      arcs[j].weight = edges[i].weight;
      j++;
      arcs[j].v1 = edges[i].v2;
      arcs[j].v2 = edges[i].v1;
      arcs[j].e = i;
      arcs[j].o = j-1;
      arcs[j].weight = edges[i].weight;
      j++;
    }
  }
  if (j < n_arcs) {
    cerr << "Expected " << n_arcs << " but only got " << j << " arcs" << endl;
  }
}
