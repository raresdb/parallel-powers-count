#pragma once

#include <set>

#include "map-reducer.h"
#include "math.h"

using namespace std;

// arguments used by the map_powers function
struct map_powers_args {
	unsigned reducers_count;

	map_powers_args(unsigned reducers_c) {
		reducers_count = reducers_c;
	}

	map_powers_args() {};
};

// arguments used by the reduce_powers function
struct reduce_powers_args {

};

// function used to map a file in the powers problem
map<unsigned, list<unsigned>> map_powers(struct map_powers_args args, ifstream& file);

// function used to count the bases for each exponent
// idx is the key in the map -> exponent
unsigned reduce_powers(struct reduce_powers_args args, unsigned idx,
	vector<map<unsigned, list<unsigned>>> mapper_results);