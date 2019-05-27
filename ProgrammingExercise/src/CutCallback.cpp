#include "CutCallback.h"
#include <strstream>
#include <set>

CutCallback::CutCallback( IloEnv& _env, string _cut_type, double _eps,
                          Digraph& _digraph, IloBoolVarArray& _x,
                          IloBoolVarArray& _z, u_int _k, bool _cut_early,
                          u_int _max_cuts ) :
    LazyConsI( _env ), UserCutI( _env ), env( _env ),
        cut_type( _cut_type ), eps( _eps ), digraph( _digraph ),
        x( _x ), z( _z ), k( _k ), cut_early( _cut_early ),
        max_cuts(_max_cuts), arc_weights( 2 * digraph.n_edges )
{
}

CutCallback::~CutCallback()
{
}

void CutCallback::separate()
{
    if( cut_type == "dcc" ) connectionCuts();
    else if( cut_type == "cec" ) cycleEliminationCuts();
}

/*
 * separation of directed connection cut inequalities
 */
void CutCallback::connectionCuts()
{
    if ( !lazy && !cut_early ) {
        // if not specified by the user, do not cut early
        return;
    }
    try {

        u_int n = digraph.n_nodes;
        u_int a = digraph.n_arcs;

        IloNumArray xval( env, a );
        IloNumArray zval( env, n );

        if( lazy ) {
            LazyConsI::getValues( xval, x );
            LazyConsI::getValues( zval, z );
        }
        else {
            UserCutI::getValues( xval, x );
            UserCutI::getValues( zval, z );
        }

        // allocate memory (to be free'd at the end)
        double *capacity = new double[a];
        int *cuts = new int[n];

        // initialize arc capacities with x
        list<pair<u_int, u_int> > arcs;
        for ( u_int i = 0; i < a; i++ ) {
            // we count capacity if the arc is used, 0 if not
            capacity[i] = xval[i] > eps ? 1 : 0;
            u_int v1 = digraph.arcs[i].v1;
            u_int v2 = digraph.arcs[i].v2;
            arcs.push_back(pair<u_int, u_int>( v1, v2 ));
        }

        // initialize MaxFlow algorithm
        Maxflow mflow ( n, a, arcs );
        bool mflow_initialized = false;

        // we look for minimum capacity cut < 2
        // and before we shuffle the indices != 0
        vector<u_int> shuffled ( n );
        for ( u_int i = 1; i < n; i++ ) {
            // add those which are used in the current solution
            if ( zval[i] > eps ) {
                shuffled.push_back( i );
            }
        }
        random_shuffle( shuffled.begin(), shuffled.end() );
        // count the cuts added so far
        u_int cut_count = 0;
        // and keep count of all involved nodes in cuts so far
        set<u_int> used_nodes;
        for ( vector<u_int>::iterator it1 = shuffled.begin();
                    it1 != shuffled.end(); ++it1 ) {
            u_int i1 = *it1;
            if ( used_nodes.find( i1 ) != used_nodes.end() ) {
                // try the next one
                continue;
            }
            if ( mflow_initialized ) {
                mflow.update( 0, i1 );
            }
            else {
                mflow.update( 0, i1, capacity );
            }
            // get the minimal flow
            // idea: only look at the selected nodes!!!
            double min_cut = mflow.min_cut( 1, cuts );
            if ( min_cut < 1 ) {
                // we found a cut which might violate the DCC constraint
                // we have to count the number of nodes on the left side -
                // they must be less or equal than k
                u_int count = 0;
                for ( u_int i = 0; i < n; i++ ) {
                    if ( cuts[i] == 1 ) count++;
                }
                // did we find one with less than k nodes?
                if ( count <= k ) {
                    // add the new nodes to the set of used ones
                    for ( u_int i = 0; i < n; i++ ) {
                        if ( cuts[i] == 1 ) {
                            used_nodes.insert( i );
                        }
                    }
                    IloExpr constraint ( env );
                    for ( u_int j = 0; j < a; j++ ) {
                        u_int v1 = digraph.arcs[j].v1;
                        u_int v2 = digraph.arcs[j].v2;
                        if ( cuts[v1] == 1 && cuts[v2] != 1 ) {
                            constraint += x[j];
                        }
                    }
                    if ( lazy ) {
                        LazyConsI::add( constraint >= 1 );
                    }
                    else {
                        UserCutI::add( constraint >= 1 );
                    }
                    constraint.end();
                    cut_count++;
                    // check if we should look for more cuts
                    if ( cut_count >= max_cuts ) {
                        break;
                    }
                }
            }
        }
        // free memory
        delete[] ( capacity );
        delete[] ( cuts );
        xval.end();
        zval.end();
    }
    catch( IloException& e ) {
        cerr << "CutCallback: exception " << e.getMessage();
        exit( -1 );
    }
    catch( ... ) {
        cerr << "CutCallback: unknown exception.\n";
        exit( -1 );
    }
}

/*
 * separation of cycle elimination cut inequalities
 */
void CutCallback::cycleEliminationCuts()
{
    try {

        u_int n = digraph.n_nodes;
        u_int m = digraph.n_edges;

        IloNumArray xval( env, m );
        IloNumArray zval( env, n );

        if( lazy ) {
            LazyConsI::getValues( xval, x );
            LazyConsI::getValues( zval, z );
        }
        else {
            UserCutI::getValues( xval, x );
            UserCutI::getValues( zval, z );
        }

        // initialize arc weights: 1 - x
        for ( u_int i = 0; i < m; i++ ) {
            // we count edges as binary - 0 or 1
            if ( xval[i] < eps ) {
                arc_weights[i] = 1;
                arc_weights[i+m] = 1;
            }
            else {
                arc_weights[i] = 0;
                arc_weights[i+m] = 0;
            }
            arc_weights[i] = 1 - xval[i];
            arc_weights[i+m] = 1 - xval[i];
        }

        // we look for cycles, and before we shuffle the
        // indices of the arcs
        vector<u_int> shuffled ( m );
        for ( u_int i = 0; i < m; i++ ) {
            shuffled.push_back(i);
        }
        random_shuffle( shuffled.begin(), shuffled.end() );
        // we count the number of cuts identified
        u_int cut_count = 0;
        set<u_int> used_edges;
        for ( vector<u_int>::iterator it1 = shuffled.begin();
                    it1 != shuffled.end(); ++it1 ) {
            // did we use the edge already?
            u_int i1 = *it1;
            if ( used_edges.find( i1 ) != used_edges.end() ) {
                continue;
            }
            u_int v1 = digraph.edges[i1].v1;
            u_int v2 = digraph.edges[i1].v2;
            // exclude this edge by setting the weight to high number
            double oldWeight = arc_weights[i1];
            arc_weights[i1] = MAXFLOAT;
            arc_weights[i1+m] = MAXFLOAT;
            // calculate shortest path from v1 to v2
            CutCallback::SPResultT spResult = shortestPath( v1, v2 );
            // reset weight
            arc_weights[i1] = oldWeight;
            arc_weights[i1+m] = oldWeight;
            double compareValue = spResult.weight + arc_weights[i1];
            int spSize = spResult.path.size();
            if (spSize >= 1 && compareValue < 1 - eps) {
                // we found a new constraint to add!
                IloNumExpr constraint( env );
                list<u_int>::iterator it;
                for ( it = spResult.path.begin();
                      it != spResult.path.end();
                      ++it ) {
                    u_int e = *it;
                    if ( e >= m ) {
                        e -= m;
                    }
                    used_edges.insert(e);
                    constraint += x[e];
                }
                // add edge i1
                used_edges.insert(i1);
                constraint += x[i1];
                if ( lazy ) {
                    LazyConsI::add( constraint <= spSize );
                }
                else {
                    UserCutI::add( constraint <= spSize );
                }
                constraint.end();
                // check if we shall look for more cuts to add
                cut_count++;
                if ( cut_count >= max_cuts ) {
                    break;
                }
            }
        }
        xval.end();
        zval.end();

    }
    catch( IloException& e ) {
        cerr << "CutCallback: exception " << e.getMessage();
        exit( -1 );
    }
    catch( ... ) {
        cerr << "CutCallback: unknown exception.\n";
        exit( -1 );
    }
}

/*
 * Dijkstra's algorithm to find a shortest path
 * (slow implementation in time O(n^2))
 */
CutCallback::SPResultT CutCallback::shortestPath( u_int source, u_int target )
{
    u_int n = digraph.n_nodes;
    u_int m = digraph.n_edges;
    vector<SPNodeT> nodes( n );
    vector<bool> finished( n, false ); // indicates finished nodes

    // initialization
    for( u_int v = 0; v < n; v++ ) {
        nodes[v].pred = -1;
        nodes[v].pred_arc_id = -1;
        if( v == source ) nodes[v].weight = 0;
        else nodes[v].weight = numeric_limits<double>::max();
    }

    while( true ) {

        // find unfinished node with minimum weight to examine next
        // (should usually be done with heap or similar data structures)
        double wmin = numeric_limits<double>::max();
        u_int v;
        for( u_int u = 0; u < n; u++ ) {
            if( !finished[u] && nodes[u].weight < wmin ) {
                wmin = nodes[u].weight;
                v = u;
            }
        }

        // if all reachable nodes are finished
        // or target node is reached -> stop
        if( wmin == numeric_limits<double>::max() || v == target ) break;

        // this node is finished now
        finished[v] = true;

        // update all adjacent nodes on outgoing arcs
        list<u_int>::iterator it;
        for( it = digraph.incidentEdges[v].begin(); it != digraph.incidentEdges[v].end(); it++ ) {
            u_int e = *it; // edge id
            u_int a; // according arc id
            u_int u; // adjacent node
            if( digraph.edges[e].v1 == v ) {
                a = e;
                u = digraph.edges[e].v2;
            }
            else {
                a = e + m;
                u = digraph.edges[e].v1;
            }
            // only examine adjacent node if unfinished
            if( !finished[u] ) {
                // check if weight at node u can be decreased
                if( nodes[u].weight > nodes[v].weight + arc_weights[a] ) {
                    nodes[u].weight = nodes[v].weight + arc_weights[a];
                    nodes[u].pred = v;
                    nodes[u].pred_arc_id = a;
                }
            }
        }
    }

    SPResultT sp;
    sp.weight = 0;
    int v = target;
    while( v != (int) source && v != -1 ) {
        int a = nodes[v].pred_arc_id;
        if( a < 0 ) break;
        sp.weight += arc_weights[a];
        sp.path.push_back( a );
        v = nodes[v].pred;
    }
    return sp;
}

