#ifndef __KMST_SCF_H__
#define __KMST_SCF_H__

#include "kMST_ILP.h"

class kMST_SCF : public kMST_ILP {

protected:
  // SCF variables
  IloNumVarArray f; // flow variables

  void createModel();
  void outputVars();

public:
  kMST_SCF( Digraph& _digraph, int _k, bool _quiet );

};

#endif
