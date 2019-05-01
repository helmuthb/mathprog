#ifndef __KMST_MTZ_H__
#define __KMST_MTZ_H__

#include "kMST_ILP.h"

class kMST_MTZ : public kMST_ILP {

protected:
  // MTZ variables
	IloBoolVarArray y; // arc variables
  IloIntVarArray o; // order variables

  void createModel();
  void outputVars();
  
public:
  	kMST_MTZ( Instance& _instance, int _k );

};

#endif