#ifndef __KMST_DCC_H__
#define __KMST_DCC_H__

#include "kMST_ILP.h"

class kMST_DCC : public kMST_ILP {

protected:
  // DCC variables

  void createModel();
  void outputVars();

public:
  kMST_DCC( Digraph& _digraph, int _k, bool quiet );

};

#endif
