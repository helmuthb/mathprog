#ifndef __MAIN__CPP__
#define __MAIN__CPP__

#include <iostream>
#include "Tools.h"
#include "Digraph.h"
#include "kMST_ILP.h"
#include "kMST_SCF.h"
#include "kMST_MCF.h"
#include "kMST_MTZ.h"
#include "kMST_CEC.h"
#include "kMST_DCC.h"

using namespace std;

void usage()
{
    cout << "USAGE:\t<program> -f filename -m model [-q][-v][-c][-n <max_cuts>] [-k <nodes to connect>]\n";
    cout << "EXAMPLE:\t" << "./kmst -f data/g01.dat -m dcc -k 5\n\n";
    exit( 1 );
} // usage

int main( int argc, char *argv[] )
{
    // read parameters
    int opt;
    // default values
    string file( "data/g01.dat" );
    string model_type( "dcc" );
    bool verbose( 0 );
    bool quiet( 0 );
    bool cut_early( 0 );
    int max_cuts = 1;
    int k = 5;
    while( (opt = getopt( argc, argv, "f:m:k:vqcn:" )) != EOF) {
        switch( opt ) {
            case 'f': // instance file
                file = optarg;
                break;
            case 'm': // algorithm to use
                model_type = optarg;
                break;
            case 'k': // nodes to connect
                k = atoi( optarg );
                break;
            case 'v': // verbose
                verbose = 1;
                break;
            case 'q': // quiet
                quiet = 1;
                break;
            case 'c': // cut early
                cut_early = 1;
                break;
            case 'n': // max. cuts per step
                max_cuts = atoi( optarg );
                break;
            default:
                usage();
                break;
        }
    }
    // read instance
    Digraph instance( file, quiet );
    // solve instance
    kMST_ILP *ilp;
    if ( model_type == "scf" ) {
        ilp = new kMST_SCF( instance, k, quiet );
    }
    else if ( model_type == "mcf" ) {
        ilp = new kMST_MCF( instance, k, quiet );
    }
    else if ( model_type == "mtz" ) {
        ilp = new kMST_MTZ( instance, k, quiet );
    }
    else if ( model_type == "cec" ) {
        ilp = new kMST_CEC( instance, k, quiet, cut_early, max_cuts );
    }
    else if ( model_type == "dcc" ) {
        ilp = new kMST_DCC( instance, k, quiet, cut_early, max_cuts );
    }
    else {
        cerr << "Sorry, this model '" << model_type << "' is not yet implemented!" << endl;
        cerr << "You can currently only use 'scf', 'mcf', 'mtz', 'cec' and 'dcc'." << endl;
        return 1;
    }
    ilp->solve( verbose );
    delete ilp;

    return 0;
} // main

#endif // __MAIN__CPP__
