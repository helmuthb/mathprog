#ifndef __KMST_MCF_H__
#define __KMST_MCF_H__

#include "kMST_ILP.h"

class kMST_MCF : public kMST_ILP {

protected:
  // MCF variables
	IloBoolVarArray f; // flow variables

  void createModel();
  void outputVars();
  
public:
  	kMST_MCF( Instance& _instance, int _k );

};

#endif