#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#ifdef LIB_DYNAMIC
	#include <dlfcn.h>
#else
	#include "diffLib.h"
#endif


#ifdef PERFORMANCE_TESTS
#include <sys/times.h>
#include <unistd.h>
clock_t clock_t_begin, clock_t_end;
struct tms times_start_buffer, times_end_buffer;

void start_timer(){
	clock_t_begin = times(&times_start_buffer);
}

void stop_timer(){
	clock_t_end = times(&times_end_buffer);
}

double calc_time(clock_t s, clock_t e) {
    return ((long int) (e - s) / (double) sysconf(_SC_CLK_TCK));
}

void print_times(const char* operation){
	printf("%20s   real %.3fs   user %.3fs   sys %.3fs\n",
			operation,
			calc_time(clock_t_begin, clock_t_end),
			calc_time(times_start_buffer.tms_cutime, times_end_buffer.tms_cutime),
			calc_time(times_start_buffer.tms_cstime, times_end_buffer.tms_cstime));
}
#endif

int main(int argc, char** argv) {

    #ifdef LIB_DYNAMIC
        void *handle = dlopen("../zad1/diffLib.so", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "Cannot load dynamic library diffLib.so\n");
            exit(1);
        }
        
        void (*create_main_block)(int) = (void (*)(int)) dlsym(handle, "create_main_block");
        void (*define_pairs_sequence)(char**, int) = (void (*)(char**, int)) dlsym(handle, "define_pairs_sequence");
        void (*compare_pairs)(void) = (void (*)(void)) dlsym(handle, "compare_pairs");
        int (*get_operations_num_in_edition_block)(int) = (int (*)(int)) dlsym(handle, "get_operations_num_in_edition_block");
        void (*remove_operation_block)(int) = (void (*)(int)) dlsym(handle, "remove_operation_block");
        void (*remove_edition_operation_from_block)(int, int) = (void (*)(int, int)) dlsym(handle, "remove_edition_operation_from_block");
        void (*free_block)(void) = (void (*)(void)) dlsym(handle, "free_block");
        void (*__printData)(void) = (void (*)(void)) dlsym(handle, "__printData");

        if (dlerror()) {
            fprintf(stderr, "Cannot load functions from dynamic library libblocks.so\n");
            exit(1);
        }
    #endif

	if (argc <= 1) {
		printf("Too few arguments\n");
		exit(1);
	}

	regex_t re;
	int ret = regcomp(&re, "[a-zA-Z0-9._-]+:[a-zA-Z0-9._-]+", REG_EXTENDED);
	if (ret != 0) {
		char errmsg[256];
		regerror(ret, &re, errmsg, sizeof(errmsg));
		printf("Compiling regex error | %s\n", errmsg);
		exit(1);
	}

	char* cmd;
	for (int i = 1; i < argc; i++) {
		cmd = argv[i];

		if (strcmp(cmd, "create_table") == 0) {
			//wymaga jednego argumentu
			if (i + 1 >= argc) {
				printf("create_table expects one argument\n");
				exit(1);
			}

			int size = atoi(argv[++i]);
			create_main_block(size);
		}
		else if (strcmp(cmd, "compare_pairs") == 0) {
			//wymaga przynajmniej jednego argumentu
			if (i + 1 >= argc) {
				printf("compare_pairs expects at least one argument\n");
				exit(1);
			}

			//po prostu przewijaj az sa dane z ':' (czyli pary plikow)
			int cnt = 0;
			int start = (++i);
			for (; i < argc; i++) {
				ret = regexec(&re, argv[i], 0, NULL, 0);
				if (ret == 0) {
					cnt++;
				}
				else {
					// char errmsg[256];
					// regerror(ret, &re, errmsg, sizeof(errmsg));
					// printf("ret: %d | %s\n", ret, errmsg);
					i--;
					break;
				}
			}

			if (cnt < 1) {
				printf("compare_pairs has wrong arguments\n");
				exit(1);
			}

			define_pairs_sequence(argv + start, cnt);

			compare_pairs();
		}
		else if (strcmp(cmd, "remove_block") == 0) {
			//wymaga jednego argumentu
			if (i + 1 >= argc) {
				printf("remove_block expects one argument\n");
				exit(1);
			}

			int idx = atoi(argv[++i]);

			remove_operation_block(idx);
		}
		else if (strcmp(cmd, "remove_operation") == 0) {
			//wymaga dwoch argumentu
			if (i + 2 >= argc) {
				printf("remove_operation expects two arguments\n");
				exit(1);
			}

			int block_index = atoi(argv[++i]);
			int operation_index = atoi(argv[++i]);

			remove_edition_operation_from_block(block_index, operation_index);
		}
		#ifdef PERFORMANCE_TESTS
		else if (strcmp(cmd, "start_timer") == 0) {
			start_timer();
		}
		else if (strcmp(cmd, "end_timer") == 0) {
			stop_timer();
			print_times(argv[++i]);
		}
		#endif
		else {
			printf("Bad argument: %s\n", cmd);
			break;
		}
	}

	// __printData();

	free_block();
	regfree(&re);
	#ifdef LIB_DYNAMIC
	dlclose(handle);
	#endif

	#ifdef PERFORMANCE_TESTS
	printf("\n");
	#endif

	return 0;
}