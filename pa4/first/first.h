#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

typedef struct linkedList{
	int numblockindex;
	
	struct linkedList* next;
	
}linkedList;

typedef struct cacheLocation{
	char* tag;
	//int setindex;
	//int blockindex;
}cacheLocation;

typedef struct cacheBlock{
	cacheLocation* line;
}cacheBlock;

typedef struct cacheSet{
	cacheBlock* blocks;
	
	struct linkedList* front;
	struct linkedList* rear;
}cacheSet;

typedef struct cacheMain{
	cacheSet* sets;
}cacheMain;

void initialize(cacheLocation* location);
int calcAssociativity(char* assoc);
char* longToBinary(long num);
void printResults(int memread, int memwrite, int cachehit, int cachemiss);

