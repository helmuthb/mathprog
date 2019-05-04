#ifndef __KMST_MCF_H__
#define __KMST_MCF_H__

#include "kMST_ILP.h"

class kMST_MCF : public kMST_ILP {

protected:
  // MCF variables
  IloNumVarArray f; // flow variables

  void createModel();
  void outputVars();
  
public:
  kMST_MCF( Digraph& _digraph, int _k, bool _quiet );

};

#endif
