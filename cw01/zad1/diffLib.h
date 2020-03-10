#pragma once
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <regex.h>

typedef struct {
	char* diffOffset;
	int size;
} EditionBlock;

typedef struct {
	EditionBlock** edition_blocks;
	int numblocks;
} MainBlockEntry;

typedef struct {
	MainBlockEntry** entries;
	int size;
	char** sequence;
	int sequenceNum;
} MainBlock;

void create_main_block(int size);
void free_block(void);
void define_pairs_sequence(char** pairs, int len);
void compare_pairs(void);
int get_operations_num_in_edition_block(int index);
void remove_operation_block(int idx);
void remove_edition_operation_from_block(int block_index, int operation_index);

void __printData(void);