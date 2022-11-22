#pragma once

#include <fstream>
#include <vector>
#include <map>
#include <list>
#include <pthread.h>

using namespace std;


// struct for holding the arguments required by the mapping function
template<class K, class V, class F_ARG> struct mapping_args {
	vector<string> &files;
	// mutex shared between mappers
	pthread_mutex_t &m;
	// mutex used to stop the reducers
	pthread_mutex_t &r;
	// count of remaining mappers running
	unsigned &remaining_mappers;
	// arguments for f
	F_ARG f_arg;
	// global vector of results from mapper threads
	vector<map<K, list<V>>> &mapper_results;
	// mapper function for a file
	map<K, list<V>>(*f)(F_ARG, ifstream&);

	mapping_args(vector<string> &fs, pthread_mutex_t &mx, pthread_mutex_t &rx,
		unsigned &mappers, F_ARG f_arg, vector<map<K, list<V>>> &map_res,
		map<K, list<V>>(*f)(F_ARG, ifstream&)) : files(fs), m(mx), r(rx),
		remaining_mappers(mappers), mapper_results(map_res) {
			this->f_arg = f_arg;
			this->f = f;
	}
};

// struct for holding arguments required by the reducing function
template<class K, class V, class F_ARG, class F_RES> struct reducing_args {

	vector<map<K, list<V>>> &mapper_results;
	// mutex used to stop reducers
	pthread_mutex_t &r;
	// key that the reducer thread processes
	K id;
	F_ARG f_arg;
	// reducer function for a key(id)
	F_RES(*f)(F_ARG, K, vector<map<K, list<V>>>);

	reducing_args(vector<map<K, list<V>>> &map_res, pthread_mutex_t &rx, K id, F_ARG f_arg,
		F_RES(*f)(F_ARG, K, vector<map<K, list<V>>>)) : mapper_results(map_res), r(rx) {

			this->id = id;
			this->f_arg = f_arg;
			this->f = f;
		}
};

// routine ran by the mapper thread
template<class K, class V, class F_ARG> void* mapping(void* arg) {

	struct mapping_args<K, V, F_ARG> *args = (struct mapping_args<K, V, F_ARG> *)arg;
	ifstream *file;
	// result obtained by the current mapper
	map<K, list<V>> result;
	// result obtained by the function used to map
	map<K, list<V>> function_result;
	list<V> emptyList;

	// looping through available files
	while(args->files.size()) {
		file = NULL;

		if(pthread_mutex_lock(&args->m)) {
			perror("Mutex lock failure!\n");
			exit(-1);
		}

		if(args->files.size()) {
			// getting a file and eliminating it from the list
			file = new ifstream(args->files.at(args->files.size() - 1));
			args->files.pop_back();
		}

		if(pthread_mutex_unlock(&args->m)) {
			perror("Mutex unlock failure!\n");
			exit(-1);
		}

		if(!file)
			break;

		// map the file
		function_result = args->f(args->f_arg, *file);

		// merge the map from the file into the map where all files are included
		for(auto map_it = function_result.begin(); map_it != function_result.end(); map_it++) {
			if(result.find(map_it->first) == result.end())
				result.insert(pair<K, list<V>>(map_it->first, emptyList));
			
			result.at(map_it->first).insert(result.at(map_it->first).end(), map_it->second.begin(),
				map_it->second.end());
		}

		file->close();
		delete(file);
	}
	
	if(pthread_mutex_lock(&args->m)) {
		perror("Mutex lock failure!\n");
		exit(-1);
	}

	// update the global list of results and mark that this mapper finished
	args->mapper_results.push_back(result);
	args->remaining_mappers--;

	if(pthread_mutex_unlock(&args->m)) {
			perror("Mutex unlock failure!\n");
			exit(-1);
	}

	// unlock the reducers if all mappers finished
	if(!args->remaining_mappers)
		if(pthread_mutex_unlock(&args->r)) {
				perror("Mutex unlock failure!\n");
				exit(-1);
		}

	pthread_exit(NULL);
}

template<class K, class V, class F_ARG, class F_RES> void* reducing(void* arg) {
	struct reducing_args<K, V, F_ARG, F_RES> *args =
		(struct reducing_args<K, V, F_ARG, F_RES> *)arg;
	// wait for mappers to finish
	if(pthread_mutex_lock(&args->r)) {
		perror("Mutex lock failure!\n");
		exit(-1);
	}

	if(pthread_mutex_unlock(&args->r)) {
		perror("Mutex unlock failure!\n");
		exit(-1);
	}
	
	// get result of this reducer
	F_RES* reducer_result = new F_RES;
	*reducer_result = args->f(args->f_arg, args->id, args->mapper_results);

	pthread_exit(reducer_result);
}