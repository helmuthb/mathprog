#include "kMST_ILP.h"

kMST_ILP::kMST_ILP( Digraph& _digraph, string _model_type, int _k, bool _quiet ) :
	digraph( _digraph ), model_type( _model_type ), k( _k ),
	epInt( 0.0 ), epOpt( 0.0 ), quiet( _quiet )
{
	n = digraph.n_nodes;
	m = digraph.n_edges;
	a = digraph.n_arcs;
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
		if ( quiet ) {
			cplex.setOut( env.getNullStream() );
		}
		cplex.exportModel( "model.lp" );

		// set parameters
		epInt = cplex.getParam( IloCplex::EpInt );
		epOpt = cplex.getParam( IloCplex::EpOpt );
		// only use a single thread
		cplex.setParam( IloCplex::Threads, 1 );

		// set cut- and lazy-constraint-callback for
		// cycle-elimination cuts ("cec") or directed connection cuts ("dcc")
		if (model_type == "cec" || model_type == "dcc") {
			CutCallback* usercb = new CutCallback( env, model_type, epOpt, digraph, x, z, k );
			CutCallback* lazycb = new CutCallback( env, model_type, epOpt, digraph, x, z, k );
			cplex.use( (UserCutI*) usercb );
			cplex.use( (LazyConsI*) lazycb );
		}

		// solve model
		if ( !quiet ) {
			cout << "Calling CPLEX solve ...\n";
		}
		cplex.solve();
		if ( !quiet ) {
			cout << "CPLEX finished.\n\n";
			cout << "CPLEX status: " << cplex.getStatus() << "\n";
			cout << "Branch-and-Bound nodes: " << cplex.getNnodes() << "\n";
			if ( cplex.getStatus() != CPX_STAT_INFEASIBLE ) {
				cout << "Objective value: " << cplex.getObjValue() << "\n";
				if ( verbose ) {
					outputVars();
				}
			}
		}
		double cpuTime = Tools::CPUtime();
		if ( !quiet ) {
			cout << "CPU time: " << cpuTime << "\n\n";
		}
		else {
			cout << cpuTime << "," << cplex.getNnodes() << ",";
			if ( cplex.getStatus() != CPX_STAT_INFEASIBLE ) {
				cout << cplex.getObjValue() << endl;
			}
			else {
				cout << -1 << endl;
			}
		}

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
	if ( !quiet ) {
		cout << "initialize CPLEX ... ";
	}
	try {
		env = IloEnv();
		if ( quiet ) {
			env.setOut( env.getNullStream() );
		}
		model = IloModel( env );
		values = IloNumArray( env );
	}
	catch( IloException& e ) {
		cerr << "kMST_ILP: exception " << e.getMessage();
	}
	catch( ... ) {
		cerr << "kMST_ILP: unknown exception.\n";
	}
	if ( !quiet ) {
		cout << "done.\n";
	}
}

kMST_ILP::~kMST_ILP()
{
	// free CPLEX resources
	cplex.end();
	model.end();
	env.end();
}
