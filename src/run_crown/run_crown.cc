// Copyright (c) 2008, Jacob Burnim (jburnim@cs.berkeley.edu)
//
// This file is part of CROWN, which is distributed under the revised
// BSD license.  A copy of this license can be found in the file LICENSE.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See LICENSE
// for details.

#include <assert.h>
#include <time.h>
#include <cstdio>
#include <iostream>
#include "run_crown/concolic_search.h"


/* TCdir is a relative directory path to save generated test case files 
 * (i.e. input.1, input.2, ...)
 * 2017.07.07 Hyunwoo Kim 
 */
string TCdir;

/* print_command_usage now shows -TCDIR option and more description about
 * search strategies 
 * 2017.07.07 Hyunwoo Kim 
 */

void print_command_usage() {
    std::cerr<<"Usage:"
<<"\nrun_crown 'target args' <num-iter> -<Strategy> [-TCDIR <path>]"
<<"\n-Note that <Strategy> can be one of {random, random_input, cfg, " 
<<"\n cfg_baseline, hybrid, dfs [<max-depth>], uniform_random [<max-depth>]}."
<<std::endl;
}

/* It checks if a given string s is a positive integer 
 * 2017.07.07 Hyunwoo Kim 
 */
bool is_positive_int(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;

    if(s == "0")
        return false;
    return !s.empty() && it == s.end();
}

int main(int argc, char* argv[]) {

    /* max_depth is <max-depth> in commandline arguments which should be a 
     * positive integer. 
     * 2017.07.07 Hyunwoo Kim 
     */
    unsigned int max_depth = 0 ;

    if(argc < 4){
	print_command_usage();
	exit(1);
    }
    assert(argc >= 4);
    string prog = argv[1];
    int num_iters = atoi(argv[2]);
    string search_type = argv[3];

    /* Updated to handle -TCDIR 
     * 2017.07.07 Hyunwoo Kim 
     */
    if(!is_positive_int(argv[2])){
        print_command_usage();
	std::cerr<<"\n<num-iter> (i.e. "<<argv[2]<<") should be a positive";
	std::cerr<<" integer."<<std::endl;
        exit(1);
    }      

    if(search_type == "-dfs" || search_type == "-uniform_random"){
        switch(argc){

	    // ex> run_crown main 100 -dfs
	    // i.e., no <max-depth> given to -dfs or -uniform_random and 
	    // no -TCDIR option given
	    case 4: 
		break;

            // -dfs <max-depth>  or
	    // -dfs -TCDIR w/o <path> (i.e. error) or
            // -dfs 3.14 (i.e. error) 
            case 5:
                if(string(argv[4])=="-TCDIR"){
                    print_command_usage();
		    std::cerr<<"\n <path> is missing." <<std::endl;
                    exit(1);
		}
	    	if(!is_positive_int(argv[4])){
		    print_command_usage();
		    std::cerr<<"\n<max-depth> (" << argv[4]
			     <<") should be a positive integer."<<std::endl;
		    exit(1);
	        }
		max_depth = atoi(argv[4]); 
                break;
		
	    // ex> run_crown main 100 -dfs -TCDIR test 
	    // all other cases are errors
            case 6:
                if(string(argv[4])!="-TCDIR"){
                    print_command_usage();
                    std::cerr<<"\nThe argument ("<<argv[4]<<") is not a valid one." 
			     <<std::endl;
                    exit(1);
                } else { 
                    TCdir = argv[5];
		}
                break;

            // ex> run_crown 'target args' 100 -dfs 10000  -TCDIR test 
            case 7:
                if(string(argv[4])=="-TCDIR"){
                    print_command_usage();
                    std::cerr<<"\nThe argument ("<<argv[6]
                <<") is not a valid one." <<std::endl;
                    exit(1);
                }
                if(!is_positive_int(argv[4])){
                    print_command_usage();
		    std::cerr<<"\n<max-depth> (" << argv[4]
			     <<") should be a positive integer."<<std::endl;
                    exit(1);
                }

                if(string(argv[5])!="-TCDIR"){
                    print_command_usage();
                    std::cerr<<"\nThe argument ("<<argv[5] 
			     <<") is not a valid one." <<std::endl;
                    exit(1);
                } 

                TCdir = argv[6];
		max_depth = atoi(argv[4]); 
                break;

            default:
                print_command_usage();
                exit(1);
        }
    } else { // search strategies which do not receive <max-depth> 
	if(search_type == "-random" || search_type == "-random_input" 
	|| search_type == "-cfg" || search_type == "-cfg_baseline" 
	|| search_type == "-hybrid"){

            switch(argc){
            // ex> run_crown 'target args' 100 -cfg
            case 4:
                break;
            //ex> run_crown 'target args' 100 -cfg -TCDIR test
            case 6:
                if(string(argv[4]) != "-TCDIR"){
                    print_command_usage();
                    std::cerr<<"\nThe argument ("<<argv[4]
			     <<") is not a valid one." <<std::endl;
                    exit(1);
                }
                TCdir = argv[5];
                break;
            default:
                print_command_usage();
                exit(1);
            }
    } else {
        std::cerr<<"\nInvalid search strategy:"<<search_type.c_str()<<std::endl;
        exit(1);
    }
    }
    // Initialize the random number generator.
#if 1
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	srand(ts.tv_nsec);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand((tv.tv_sec << 20) + tv.tv_usec);
#endif

	crown::Search* strategy;
	if (search_type == "-random") {
		strategy = new crown::RandomSearch(prog, num_iters);
	} else if (search_type == "-random_input") {
		strategy = new crown::RandomInputSearch(prog, num_iters);
	} else if (search_type == "-dfs") {

       	    /* if <max-depth> is omitted, 1,000,000 is set as default.
             * 2017.07.07 Hyunwoo Kim 
             */
	    strategy = new crown::BoundedDepthFirstSearch(prog, num_iters, 
			    max_depth>0 ? max_depth: 1000000);
	} else if (search_type == "-cfg") {
		strategy = new crown::CfgHeuristicSearch(prog, num_iters);
	} else if (search_type == "-cfg_baseline") {
		strategy = new crown::CfgBaselineSearch(prog, num_iters);
	} else if (search_type == "-hybrid") {
		strategy = new crown::HybridSearch(prog, num_iters, 100);
	} else if (search_type == "-uniform_random") {

            /* if <max-depth> is omitted, 100,000,000 is set as default.
             * 2017.07.07 Hyunwoo Kim 
             */
	    strategy = new crown::UniformRandomSearch(prog, num_iters, 
			    max_depth>0 ? max_depth :100000000);
	} else {
	    fprintf(stderr, "Unknown search strategy: %s\n", search_type.c_str());
	    return 1;
	}

	strategy->Run();

	delete strategy;
	//if(remove("input")) perror("Error:");
	return 0;
}
