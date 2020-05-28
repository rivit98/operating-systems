#include "diffLib.h"

int get_free_index();
long get_file_size(FILE* fp);
MainBlockEntry* compare_one_pair(char* pair);
void fillEntryBlock(MainBlockEntry* block, char* buffer, int len);

MainBlock* mb = NULL;

void create_main_block(int s) {
	if (s <= 0) {
		fprintf(stderr, "[create_main_block] Wrong block size\n");
		return;
	}

	if (mb != NULL) {
		fprintf(stderr, "[create_main_block] Block already created\n");
		return;
	}

	//create main block
	mb = (MainBlock*)calloc(1, sizeof(MainBlock));
	mb->size = s;
	mb->sequence = NULL;
	mb->sequenceNum = 0;
	mb->entries = (MainBlockEntry**)calloc(s, sizeof(MainBlockEntry*));
	if (mb->entries == NULL) {
		fprintf(stderr, "[create_main_block] calloc failed\n");
		exit(1);
	}
}

void free_block(void) {
	if (mb == NULL || mb->entries == NULL) {
		return;
	}

	MainBlockEntry* main_block_entry = NULL;
	for (int i = 0; i < mb->size; i++) {
		main_block_entry = mb->entries[i];

		if (main_block_entry == NULL) {
			continue;
		}

		for(int edition = 0; edition < main_block_entry->numblocks; edition++){
		 	if(main_block_entry->edition_blocks[edition] == NULL){
		 		continue;
		 	}

			free(main_block_entry->edition_blocks[edition]->diffOffset);
		 	free(main_block_entry->edition_blocks[edition]);
		}

		free(main_block_entry->edition_blocks);
		free(main_block_entry);
	}

	free(mb->entries);
	free(mb);
}

void define_pairs_sequence(char** pairs, int len) {
	if (mb == NULL) {
		fprintf(stderr, "[define_pairs_sequence] You should create_table first\n");
		exit(1);
	}

	if (mb->size < len) {
		fprintf(stderr, "[define_pairs_sequence] Too many elements for this table [%d/%d]\n", len, mb->size);
		exit(1);
	}

	mb->sequence = pairs;
	mb->sequenceNum = len;
}

void compare_pairs(void) {
	if (mb == NULL || mb->entries == NULL) {
		fprintf(stderr, "[compare_pairs] You should create_table first\n");
		exit(1);
	}

	if (mb->sequenceNum <= 0) {
		fprintf(stderr, "[compare_pairs] Nothing to compare\n");
		exit(1);
	}

	int freeEntryIndex = -1;
	for (int i = 0; i < mb->sequenceNum; i++) {
		freeEntryIndex = get_free_index();
		if (freeEntryIndex != -1) {
			MainBlockEntry* block = compare_one_pair(mb->sequence[i]);
			if(block != NULL){
				mb->entries[freeEntryIndex] = block;
			}
		}else{
			fprintf(stderr, "[compare_pairs] no space in table left\n");
			return;
		}
	}
}

int get_free_index(void) {
	if (mb == NULL) {
		fprintf(stderr, "[getFreeIndex] You should create_table first\n");
		return -1;
	}

	for (int i = 0; i < mb->size; i++) {
		if (mb->entries[i] == NULL) {
			return i;
		}
	}

	return -1;
}

MainBlockEntry* compare_one_pair(char* pair){
	char* source = strdup(pair);
	char* start_address = source;
	const char* delimiter = ":";
	const char* tempFile = "temp.txt";
	char* token = NULL;
	int idx = 0;
	char filename[2][64];

	//split pair by ':'
	while ((token = strsep(&source, delimiter)) != NULL) {
		strncpy(filename[idx++], token, sizeof filename[0]);
	}
	free(start_address);

	if(access(filename[0], F_OK) == -1 || access(filename[1], F_OK) == -1){
		fprintf(stderr, "No such files: %s or %s\n", filename[0], filename[1]);
		return NULL;
	}

	char cmd[256];
	sprintf(cmd, "diff %s %s > %s", filename[0], filename[1], tempFile);
	system(cmd);

	FILE* fp = fopen(tempFile, "r");
	if (!fp) {
		fprintf(stderr, "[compare_one_pair] File open error %s", tempFile);
		exit(1);
	}
	long f_size = get_file_size(fp);
	char* buffer = calloc(f_size + 1, sizeof(char));
	fread(buffer, sizeof(char), f_size, fp);
	buffer[f_size] = '\0';
	fclose(fp);

	MainBlockEntry* block = calloc(1, sizeof(MainBlockEntry));
	
	fillEntryBlock(block, buffer, f_size+1);
	free(buffer);

	return block;
}

long get_file_size(FILE* fp) {
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}

void fillEntryBlock(MainBlockEntry* block, char* buffer, int buf_len) {
	char* tempBuffer = strdup(buffer);
	char* start_address = tempBuffer;

	regex_t re;
	int ret = regcomp(&re, "[0-9,]+[adc][0-9,]+", REG_EXTENDED);
	if (ret != 0) {
		char errmsg[256];
		regerror(ret, &re, errmsg, sizeof(errmsg));
		fprintf(stderr, "Compiling regex error | %s\n", errmsg);
		exit(1);
	}

	int numblocks = 0;
	const char separator[3] = "\r\n";
	// zlicz ile blokow bedzie
	char* line = strtok(tempBuffer, separator);
	while (line != NULL) {
		ret = regexec(&re, line, 0, NULL, 0);
		if (ret == 0) {
			numblocks++;
		}

		line = strtok(NULL, separator);
	}

	free(start_address);
	tempBuffer = strdup(buffer);
	start_address = tempBuffer;

	//stworz pointer na tablice pointerow do blokow edycyjnych
	block->edition_blocks = calloc(numblocks, sizeof(EditionBlock*));
	block->numblocks = numblocks;
	for (int b = 0; b < block->numblocks; b++) {
		//stworz bloki edycyjne i przypnij pointer
		block->edition_blocks[b] = calloc(1, sizeof(EditionBlock));
	}

	int block_indexer = 0;
	char* prev_block = NULL;
	char* fragment_pointer = NULL;
	int sum_segments = 0;
	int segmentSize = 0;
	line = strtok(tempBuffer, separator);
	while (line != NULL && block_indexer < block->numblocks) {
		ret = regexec(&re, line, 0, NULL, 0);
		if (ret == 0) {
			//tutaj jest poczatek nowego bloku i koniec starego
			if (line != start_address) {
				//jesli to pierwszy blok, to nie ma poprzednika
				segmentSize = (line - prev_block);

				fragment_pointer = calloc(segmentSize+1, sizeof(char));
				memcpy(fragment_pointer, (buffer + sum_segments), segmentSize);
				fragment_pointer[segmentSize] = '\0';
				block->edition_blocks[block_indexer]->diffOffset = fragment_pointer;
				block->edition_blocks[block_indexer]->size = segmentSize;

				sum_segments = (line - start_address);
				block_indexer++;
			}

			prev_block = line;
		}

		line = strtok(NULL, separator);
	}

	//obsluga ostatniego
	segmentSize = (buf_len - sum_segments) - 1;
	fragment_pointer = calloc(segmentSize+1, sizeof(char));
	memcpy(fragment_pointer, (buffer + sum_segments), segmentSize);
	fragment_pointer[segmentSize] = '\0';
	block->edition_blocks[block_indexer]->diffOffset = fragment_pointer;
	block->edition_blocks[block_indexer]->size = segmentSize;

	// for (int b = 0; b < block->numblocks; b++) {
	// 	printf("%s", block->edition_blocks[b]->diffOffset);
	// }

	free(start_address);
	regfree(&re);
}

int get_operations_num_in_edition_block(int index){
	if (mb == NULL) {
		fprintf(stderr, "[get_operations_in_edition_block] You should create_table first\n");
		return -1;
	}

	return mb->entries[index]->numblocks;
}

void remove_operation_block(int index){
	if (mb == NULL) {
		fprintf(stderr, "[remove_operation_block] You should create_table first\n");
		return;
	}

	MainBlockEntry* main_block_entry = mb->entries[index];

	if(main_block_entry == NULL || main_block_entry->numblocks <= 0){
		fprintf(stderr, "[remove_operation_block] Nothing to remove\n");
		return;	
	}

	if(index >= mb->size){
		fprintf(stderr, "[remove_operation_block] Block with index %d does not exists\n", index);
		return;
	}

	int numblocks = mb->entries[index]->numblocks;
	for(int i = 0; i < numblocks; i++){
		if(main_block_entry->edition_blocks[i] == NULL){
		 	continue;
		}

		free(main_block_entry->edition_blocks[i]->diffOffset);
		free(main_block_entry->edition_blocks[i]);
	}

	free(main_block_entry->edition_blocks);
	free(main_block_entry);
	mb->entries[index] = NULL;
}

void remove_edition_operation_from_block(int block_index, int operation_index){
	if (mb == NULL) {
		fprintf(stderr, "[remove_edition_operation_from_block] You should create_table first\n");
		return;
	}

	MainBlockEntry* main_block_entry = mb->entries[block_index];

	if(main_block_entry == NULL || main_block_entry->numblocks <= 0){
		fprintf(stderr, "[remove_edition_operation_from_block] Nothing to remove\n");
		return;	
	}

	if(block_index >= mb->size){
		fprintf(stderr, "[remove_edition_operation_from_block] Block with index %d does not exists\n", block_index);
		return;
	}

	int numblocks = mb->entries[block_index]->numblocks;
	if(operation_index >= numblocks){
		fprintf(stderr, "[remove_edition_operation_from_block] Edition with index %d does not exists\n", operation_index);
		return;
	}

	if(main_block_entry->edition_blocks[operation_index] == NULL){
		fprintf(stderr, "[remove_edition_operation_from_block] Nothing to remove from edition block\n");
		return;	
	}

	free(main_block_entry->edition_blocks[operation_index]->diffOffset);
	free(main_block_entry->edition_blocks[operation_index]);
	main_block_entry->edition_blocks[operation_index] = NULL;
}


void __printData(void){
	if(mb == NULL){
		return;
	}
	printf("\n\n\n-----------------\n");
	printf("> MainBlock has %d operation blocks available at %p\n", mb->size, mb->entries);
	MainBlockEntry* main_block_entry = NULL;
	for (int i = 0; i < mb->size; i++) {
		printf(">  MainBlockEntry %d - ", i);
		main_block_entry = mb->entries[i];
		if (main_block_entry == NULL) {
			printf("empty\n");
			continue;
		}
		printf("has %d edition blocks at %p\n", main_block_entry->numblocks, main_block_entry->edition_blocks);
		for(int edition = 0; edition < main_block_entry->numblocks; edition++){
			printf(">   Edition block %d - ", edition);
			if(main_block_entry->edition_blocks[edition] != NULL){
				if(main_block_entry->edition_blocks[edition]->diffOffset != NULL){
					printf("has data at %p\n", main_block_entry->edition_blocks[edition]->diffOffset);
					//printf("\n##\n%s\n##\n", main_block_entry->edition_blocks[edition]->diffOffset);
				}else{
					printf("has no data\n");
				}
			}else{
				printf("empty\n##\n");
			}
		}
	}

	printf("-----------------\n\n\n");
}