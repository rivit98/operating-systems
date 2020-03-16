#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

#include "io_lib.h"


double calc_time(clock_t s, clock_t e) {
    return ((long int) (e - s) / (double) sysconf(_SC_CLK_TCK));
}

void print_times(struct tms* times_start_buffer, struct tms* times_end_buffer){
	printf("> user %.3fs   sys %.3fs\n", calc_time(times_start_buffer->tms_utime, times_end_buffer->tms_utime), calc_time(times_start_buffer->tms_stime, times_end_buffer->tms_stime));
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		printf("Too few arguments\n");
		exit(1);
	}

	struct tms times_start_buffer, times_end_buffer;
	const char* cmd = argv[1];
	if(strcmp(cmd, "generate") == 0){
		if(argc != 5){
			printf("Usage: generate filename records_num record_size\n");
			exit(1);
		}

		const char* filename = argv[2];
		int records_num = atoi(argv[3]);
		int record_size = atoi(argv[4]);
		generate_records(filename, records_num, record_size);

	}else if(strcmp(cmd, "sort") == 0){
		if(argc != 6){
			printf("Usage: sort filename records_num record_size method\n");
			exit(1);
		}
		
		const char* filename = argv[2];
		int records_num = atoi(argv[3]);
		int record_size = atoi(argv[4]);
		const char* type = argv[5];

		times(&times_start_buffer);
		sort_records(filename, records_num, record_size, type);
		times(&times_end_buffer);
		print_times(&times_start_buffer, &times_end_buffer);

	}else if(strcmp(cmd, "copy") == 0){
		if(argc != 7){
			printf("Usage: copy source dest records_num record_size\n");
			exit(1);
		}

		const char* filename = argv[2];
		const char* dest = argv[3];
		int records_num = atoi(argv[4]);
		int record_size = atoi(argv[5]);
		const char* type = argv[6];

		times(&times_start_buffer);
		copy_records(filename, dest, records_num, record_size, type);
		times(&times_end_buffer);
		print_times(&times_start_buffer, &times_end_buffer);

	}else{
		printf("Bad arguments\n");
	}
}
