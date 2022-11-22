#include "powers.h"

map<unsigned, list<unsigned>> map_powers(struct map_powers_args args, ifstream& file) {
	// number of numbers
	unsigned N;
	// number to check
	unsigned number;

	file >> N;

	// returned map
	map<unsigned, list<unsigned>> result;
	list<unsigned> emptyList;

	// looping through all numbers
	for(unsigned i = 0; i < N; i++) {
		file >> number;

		if(number < 1)
			continue;
		
		// binary search for a base for each exponent
		for(unsigned exp = 2; exp < args.reducers_count + 2; exp++) {
			unsigned upper = number;
			unsigned lower = 1;
			unsigned base;

			while(upper >= lower) {
				base = (upper + lower) / 2;
				if(pow(base, exp) == number) {
					if(!result.count(exp))
						result.insert(pair<unsigned, list<unsigned>>(exp, emptyList));

					result.at(exp).push_back(number);
					break;
				}
				else if(pow(base, exp) < number)
					lower = base + 1;
				else
					upper = base - 1;
			}
		}
	}
		
	return result;
}

unsigned reduce_powers(struct reduce_powers_args args, unsigned idx,
	vector<map<unsigned, list<unsigned>>> mapper_results) {
		// set of all numbers for an exponent
		set<unsigned> final_list;

		// looping through mapper results to gather the numbers for the current exponent(idx)
		for(auto map_res : mapper_results) {
			if(!map_res.count(idx))
				continue;

			for(auto it = map_res.at(idx).begin(); it != map_res.at(idx).end(); it++)
				final_list.insert(*it);
		}

		// return the count of individual numbers for the current exponent
		return final_list.size();
	}

int main(int argc, char* argv[]) {
	ifstream main_file(argv[3]);
	vector<string> files;
	unsigned mapper_c = atoi(argv[1]);
	unsigned left_mappers = mapper_c;
	unsigned reducer_c = atoi(argv[2]);
	unsigned file_c;
	string file_name;
	pthread_mutex_t map_m;
	pthread_mutex_t red_m;
	vector<map<unsigned, list<unsigned>>> mappers_results;
	pthread_t map_threads[mapper_c];
	pthread_t red_threads[reducer_c];

	main_file >> file_c;

	if(pthread_mutex_init(&map_m, NULL)) {
		perror("Mutex init failure!\n");
			exit(-1);
	}

	if(pthread_mutex_init(&red_m, NULL)) {
		perror("Mutex init failure!\n");
			exit(-1);
	}
	
	// locking the reducer mutex
	if(pthread_mutex_lock(&red_m)) {
			perror("Mutex lock failure!\n");
			exit(-1);
	}

	// thread arguments
	mapping_args<unsigned, unsigned, map_powers_args> map_arg(files, map_m, red_m, left_mappers,
		map_powers_args(reducer_c), mappers_results, map_powers);

	vector<reducing_args<unsigned, unsigned, reduce_powers_args, unsigned>*> red_args;

	getline(main_file, file_name);

	// getting files
	for(unsigned i = 0; i < file_c; i++) {
		getline(main_file, file_name);
		files.push_back(file_name);
	}

	// running threads
	for(unsigned i = 0; i < mapper_c + reducer_c; i++) {
		if(i < mapper_c) {
			if(pthread_create(&map_threads[i], NULL, mapping<unsigned, unsigned, map_powers_args>,
				(void*)&map_arg))
			{
				perror("Mapper thread creation failed!\n");
				exit(-1);
			}
		} else {
			red_args.push_back(new reducing_args<unsigned, unsigned, reduce_powers_args, unsigned>
				(mappers_results, red_m, i + 2 - mapper_c, reduce_powers_args(), reduce_powers));

			if(pthread_create(&red_threads[i - mapper_c], NULL,
				reducing<unsigned, unsigned, reduce_powers_args, unsigned>, (void*)red_args[i - mapper_c]))
			{
				perror("Reducer thread creation failed!\n");
				exit(-1);
			}
		}
	}

	// waiting for threads to finish
	for(unsigned i = 0; i < mapper_c + reducer_c; i++) {
		if(i < mapper_c) {
			if(pthread_join(map_threads[i], NULL))
			{
				perror("Mapper thread join failed!\n");
				exit(-1);
			}
		} else {
			ofstream f("out" + to_string(i + 2 - mapper_c) + ".txt");
			unsigned *count;

			if(pthread_join(red_threads[i - mapper_c], (void**)&count))
			{
				perror("Reducer thread join failed!\n");
				exit(-1);
			}

			f << *count;
			delete(count);
			delete(red_args[i - mapper_c]);
			f.close();
		}
	}

	if(pthread_mutex_destroy(&map_m)) {
		perror("Mutex destroy failure!\n");
			exit(-1);
	}

	if(pthread_mutex_destroy(&red_m)) {
		perror("Mutex destroy failure!\n");
			exit(-1);
	}

	pthread_exit(0);
}