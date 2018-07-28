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
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "run_crown/symbolic_execution.h"

using namespace crown;
using namespace std;

/*
 * comments written by Hyunwoo Kim (17.07.26)
 * input
 * cmd: command to execute.
 * output: result of command in the form of string.
 * exec_cmd takes command and returns output after performing command.
 */
string exec_cmd(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[512];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 512, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}

/*
 * comments written by Hyunwoo Kim (17.07.26)
 * input
 * s: a string to split.
 * delim: a delimeter character.
 * result: a vector to save the split sub-strings.
 * output: output is recorded in result.
 * split_string gives sub-strings by spliting a string with delimeter.
 */
void split_string(const std::string &s, char delim, vector<string> &result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
}

int main(void) {
	SymbolicExecution ex;

	ifstream in("szd_execution", ios::in | ios::binary);
	assert(ex.Parse(in));
	in.close();

    /*
     * comments written by Hyunwoo Kim (17.07.13)
     * output of print_execution was refined.
     */

	// Print input.
	cout << "\nSymbolic variables & input values" << endl;
	for (size_t i = 0; i < ex.inputs().size(); i++) {
		if(ex.vars().at(i) == types::FLOAT || ex.vars().at(i) == types::DOUBLE){
			cout << "("<<ex.var_names()[i]<<" = "<<ex.inputs()[i].floating << ") FP\t[ Line: " << ex.locations()[i].lineno<<", File: " << ex.locations()[i].fname << " ]\n";
		}else{
			cout << "("<<ex.var_names()[i]<<" = "<<ex.inputs()[i].integral << ")\t[ Line: " << ex.locations()[i].lineno<<", File: " << ex.locations()[i].fname << " ]\n";
		}
	}
	cout << endl;

	cout << "Symbolic path for the input" <<endl;
	{ // Print the constraints.
        
		string tmp;
		for (size_t i = 0; i < ex.path().constraints().size(); i++) {
			tmp.clear();
			ex.path().constraints()[i]->AppendToString(&tmp);
			cout << tmp << "\t[ Line: " << ex.path().locations()[i].lineno<<", File: " << ex.path().locations()[i].fname<< " ]" << endl;
		}
		cout << endl;
	}

    /*
     * comments written by Hyunwoo Kim (17.07.26)
     * This part parses line number per bid from bid-lnum-exp
     * and filename per bid from functions_in_file.
     * bid-lnum-exp and functions_in_file are outputs of parsing_cil.
     */
    vector<string> list_cil_files;
    string res_cmd;
    map<int, int> bid_line_map;
    map<int, string> bid_filename_map;
    
    res_cmd = exec_cmd("ls *.cil.c");
    split_string(res_cmd, '\n', list_cil_files);

    for(vector<string>::iterator it = list_cil_files.begin(); it!=list_cil_files.end(); ++it){
        exec_cmd("rm -rf bid-lnum-exp functions_in_file");
        exec_cmd(string(string("parsing_cil ") + *it + " &> /dev/null").c_str());

        int bid,lnum;

        ifstream fin_ble("bid-lnum-exp");
        while(!fin_ble.eof()) {
            fin_ble>>bid>>lnum;
            bid_line_map[bid]=lnum;
            
            fin_ble.get();
            fin_ble.ignore(1073741824, '\n');
        }
        fin_ble.close();

        ifstream fin_fif("functions_in_file");

        string filename, funcname;
        char deli_tok;

        while(!fin_fif.eof()) {
            fin_fif>>filename>>funcname;
            fin_fif.get();
            while(1) {
                streampos oldpos = fin_fif.tellg();
                fin_fif.get(deli_tok);
                if(deli_tok=='\n' || fin_fif.eof())
                    break;
                fin_fif.seekg (oldpos);
                fin_fif>>bid;
                fin_fif.get();

                bid_filename_map[bid]=filename.substr(0,filename.length()-4);
            }
        }
        fin_fif.close();
    }
    exec_cmd("rm -rf bid-lnum-exp functions_in_file");

	// Print the branches.
	cout << "Sequence of reached branch ids" << endl;
	for (size_t i = 0; i < ex.path().branches().size(); i++) {
		cout << ex.path().branches()[i] << "\t";
        if(ex.path().branches()[i] == -1)
            cout << "[Function enters]" << endl;
        else if(ex.path().branches()[i] == -2)
            cout << "[Function exits]" << endl;
        else {
            cout << "[ Line: " << bid_line_map[ex.path().branches()[i]] \
                << ", File: " << bid_filename_map[ex.path().branches()[i]] \
                << " ]" << endl;
        }
	}

	return 0;
}
