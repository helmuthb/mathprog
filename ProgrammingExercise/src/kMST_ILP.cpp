#include "kMST_ILP.h"

kMST_ILP::kMST_ILP( Instance& _instance, string _model_type, int _k ) :
	instance( _instance ), model_type( _model_type ), k( _k ), epInt( 0.0 ), epOpt( 0.0 )
{
	n = instance.n_nodes;
	m = instance.n_edges;
	if( k == 0 ) k = n;
}

void kMST_ILP::solve( bool verbose )
{
	// initialize CPLEX solver
	initCPLEX();

	try {
		// build model (calling model-specific implementation)
		createModel();

		cplex = IloCplex( model );
		cplex.exportModel( "model.lp" );

		// set parameters
		epInt = cplex.getParam( IloCplex::EpInt );
		epOpt = cplex.getParam( IloCplex::EpOpt );
		// only use a single thread
		cplex.setParam( IloCplex::Threads, 1 );

		// set cut- and lazy-constraint-callback for
		// cycle-elimination cuts ("cec") or directed connection cuts ("dcc")
		CutCallback* usercb = new CutCallback( env, model_type, epOpt, instance, x, z );
		CutCallback* lazycb = new CutCallback( env, model_type, epOpt, instance, x, z );
		cplex.use( (UserCutI*) usercb );
		cplex.use( (LazyConsI*) lazycb );

		// solve model
		cout << "Calling CPLEX solve ...\n";
		cplex.solve();
		cout << "CPLEX finished.\n\n";
		cout << "CPLEX status: " << cplex.getStatus() << "\n";
		cout << "Branch-and-Bound nodes: " << cplex.getNnodes() << "\n";
		if ( cplex.getStatus() != CPX_STAT_INFEASIBLE ) {
			cout << "Objective value: " << cplex.getObjValue() << "\n";
			if ( verbose ) {
				outputVars();
			}
		}
		cout << "CPU time: " << Tools::CPUtime() << "\n\n";

	}
	catch( IloException& e ) {
		cerr << "kMST_ILP: exception " << e.getMessage();
		exit( -1 );
	}
	catch( ... ) {
		cerr << "kMST_ILP: unknown exception.\n";
		exit( -1 );
	}
}

// ----- protected methods -----------------------------------------------

void kMST_ILP::initCPLEX()
{
	cout << "initialize CPLEX ... ";
	try {
		env = IloEnv();
		model = IloModel( env );
		values = IloNumArray( env );
	}
	catch( IloException& e ) {
		cerr << "kMST_ILP: exception " << e.getMessage();
	}
	catch( ... ) {
		cerr << "kMST_ILP: unknown exception.\n";
	}
	cout << "done.\n";
}

kMST_ILP::~kMST_ILP()
{
	// free CPLEX resources
	cplex.end();
	model.end();
	env.end();
}
