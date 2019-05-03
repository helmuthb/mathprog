#ifndef __KMST_ILP__H__
#define __KMST_ILP__H__

#include "Tools.h"
#include "Digraph.h"
#include "CutCallback.h"
#include <ilcplex/ilocplex.h>

using namespace std;

ILOSTLBEGIN

class kMST_ILP
{

protected:

	Digraph& digraph;
	string model_type;
	int k;
	u_int m, n, a;

	IloEnv env;
	IloModel model;
	IloCplex cplex;

	IloBoolVarArray x; // edge or arc selection variables
	IloBoolVarArray z; // node selection variables

	IloNumArray values; // to store result values of x

	double epInt, epOpt;

	void initCPLEX();

	virtual void createModel() = 0;
	virtual void outputVars() = 0;

public:

	kMST_ILP( Digraph& _digraph, string _model_type, int _k );
	virtual ~kMST_ILP();
	void solve( bool verbose, bool quiet );

};

#endif //__KMST_ILP__H__
