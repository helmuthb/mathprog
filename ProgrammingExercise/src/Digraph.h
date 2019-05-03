#ifndef __DIGRAPH__H__
#define __DIGRAPH__H__

#include "Instance.h"

class Digraph : public Instance
{

public:

  struct Arc
  {
    u_int v1, v2;
    int weight;
  };

  // number of arcs
  u_int n_arcs;
  // array of edges
  vector<Arc> arcs;

  // constructor
  Digraph( string file );

};
// Digraph

#endif //__DIGRAPH__H__
