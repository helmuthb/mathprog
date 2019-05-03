#ifndef __KMST_CEC_H__
#define __KMST_CEC_H__

#include "kMST_ILP.h"

class kMST_CEC : public kMST_ILP {

protected:
  // CEC variables

  void createModel();
  void outputVars();

public:
  	kMST_CEC( Digraph& _digraph, int _k );

};

#endif
