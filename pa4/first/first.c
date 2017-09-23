#include"first.h"

void initialize(cacheLocation* location){
	location->tag = NULL;
}

int calcAssociativity(char* assoc){
	int i;
	int nway = 0;
		
	char nwaytemp[10];
	for(i = 7; i <= strlen(assoc); i++){
		nwaytemp[i-7] = assoc[i -1];
	}
		
	//end of line char
	nwaytemp[i-7] = assoc[i - 1];
	nway = atoi(nwaytemp);
		
	if((nway == 0) || !((nway & (nway - 1)) == 0)){
		printf("The associativity is not a power of 2.\n");
		return 0;
	}
		
	return nway;
}

char* longToBinary(long num){
	static char binary[49];
	binary[48] = '\0';
	int i;
	
	for(i = 0; i < 48; i++){
		if((num | 1UL << i) == num){
			binary[48 - 1 - i] = '1';
		}
		else{
			binary[48 - 1 - i] = '0';
		}
	
	}
	
	
	return binary;
}

void printResults(int memread, int memwrite, int cachehit, int cachemiss){
	printf("Memory reads: %d\n", memread);
	printf("Memory writes: %d\n", memwrite);
	printf("Cache hits: %d\n", cachehit);
	printf("Cache misses: %d\n", cachemiss);
}

int main(int argc, char** argv){
	if (argc != 5){
		printf("Incorrect number of arguments.\n");
		return 0;
	}
	
	int cachesize = atoi(argv[1]);
	char* assoc = argv[2];
	int blocksize = atoi(argv[3]);
	
	if((cachesize == 0) || !((cachesize & (cachesize - 1)) == 0)){
		printf("Cache size is not a power of 2.\n");
		return 0;
	}
	
	if((blocksize == 0) || !((blocksize & (blocksize - 1)) == 0)){
		printf("Block size is not a power of 2.\n");
		return 0;
	}
	
	bool direct = false;
	bool full = false;
	bool found = false;
	bool nullopen = false;
	
	int associativity = 1;
	int setsize = 0;
	int setbits = 0;
	int blockbits = 0;
	int temp;
	int i;
	int j;
	int k;
	int fifo = 0;
	
	cacheMain cache;
	
	if(strlen(assoc) > 6){
		//1 block per set
		associativity = calcAssociativity(assoc);
		setsize = cachesize / (blocksize * associativity);
		
		//Figures out # bits for set indexing
		temp = setsize;
		while(temp > 0){
			setbits += 1;
			temp = temp >> 1;
		}
		//Figures out #bits for block indexing
		temp = blocksize;
		while(temp > 0){
			blockbits += 1;
			temp = temp >> 1;
		}
		setbits--;
		blockbits--;
		
		cache.sets = (struct cacheSet*) malloc(sizeof(struct cacheSet) * setsize);
		for(i = 0; i < setsize; i++){
			cache.sets[i].blocks = (struct cacheBlock*) malloc(sizeof(struct cacheBlock) * associativity);
			cache.sets[i].front = NULL;
			cache.sets[i].rear = NULL;
			
			for(j = 0; j < associativity; j++){
				cache.sets[i].blocks[j].line = (struct cacheLocation*) malloc(sizeof(struct cacheLocation) * blocksize);
				
				for(k = 0; k < blocksize; k++){
					initialize(&cache.sets[i].blocks[j].line[k]);
				}
			}
		}
	}
	
	else if(strcmp(assoc, "direct") == 0){
		//1 block per set
		setsize = cachesize / blocksize;
		
		direct = true;
		
		//Figures out # bits for set indexing
		temp = setsize;
		while(temp > 0){
			setbits += 1;
			temp = temp >> 1;
		}
		//Figures out #bits for block indexing
		temp = blocksize;
		while(temp > 0){
			blockbits += 1;
			temp = temp >> 1;
		}
		setbits--;
		blockbits--;

		
		cache.sets = (struct cacheSet*) malloc(sizeof(struct cacheSet) * setsize);
		
		for(i = 0; i < setsize; i++){
			cache.sets[i].blocks = (struct cacheBlock*) malloc(sizeof(struct cacheBlock) * associativity);
			cache.sets[i].front = NULL;
			cache.sets[i].rear = NULL;
			for(j = 0; j < associativity; j++){
				cache.sets[i].blocks[j].line = (struct cacheLocation*) malloc(sizeof(struct cacheLocation) * blocksize);
				
				for(k = 0; k < blocksize; k++){
					initialize(&cache.sets[i].blocks[j].line[k]);
				}
			}
		}
	}
	
	else if(strcmp(assoc, "assoc") == 0){
		//one set
		full = true;
		setsize = 1;
	
		associativity = cachesize / blocksize;
	
		//Figures out # bits for block indexing
		temp = blocksize;
		while(temp > 0){
			blockbits += 1;
			temp = temp >> 1;
		}
		blockbits--;
		
		cache.sets = (struct cacheSet*) malloc(sizeof(struct cacheSet) * setsize);
		cache.sets[0].blocks = (struct cacheBlock*) malloc(sizeof(struct cacheBlock) * associativity);
		cache.sets[0].front = NULL;
		cache.sets[0].rear = NULL;
		for(j = 0; j < associativity; j++){
			cache.sets[0].blocks[j].line = (struct cacheLocation*) malloc(sizeof(struct cacheLocation) * blocksize);
		
			for(k = 0; k < blocksize; k++){
				initialize(&cache.sets[0].blocks[j].line[k]);
			}
		}
	}
	
	else{
		printf("Invalid associativity format\n");
		return 0;
	}
	
	
	
	long hexnum = 0;
	char readwrite = NULL;
	char hexpreint[50];
	
	int memread = 0;
	int memwrite = 0;
	int cachehit = 0;
	int cachemiss = 0;
	
	
	int count = 0;
	
	char ignore[50];

	FILE* filepointer = fopen(argv[4], "r");
	if(filepointer == NULL){
		printf("File not found.\n");

	}
	
	//Run through the file convention A
	while(fscanf(filepointer, "%s %c %s\n", ignore, &readwrite, hexpreint) == 3){
		char* binary;
		
		char* tag;
		

		hexnum = (long)strtoul(hexpreint, NULL, 16);
	
		binary = longToBinary(hexnum);
	
		//direct associative cache - 1 block per set
		if(direct){
			//setbits
			char* setbitcomp = (char*) malloc(sizeof(char) * (setbits + 1));
			setbitcomp[setbits] = '\0';
			//blockbits
			char* blockbitcomp = (char*) malloc(sizeof(char) * (setbits + 1));
			blockbitcomp[blockbits] = '\0';
			
			int binsetindex = 0;
			int setindex = 0;
			int binblockindex = 0;
			int blockindex = 0;
			int remainder;
			int base2 = 1;
		
			temp = 48 - setbits - blockbits;
			count = 0;
			tag = (char*) malloc(sizeof(char) * (temp + 1));
			tag[temp] = '\0';
			
			//Retrieve tag from address
			for(i = 0; i < temp; i++){
				tag[i] = binary[i];
			}
			
			//Retrieve set index from address
			for(i = temp; i < 49 - blockbits - 1; i++){
				setbitcomp[count] = binary[i];
				count++;
			}
			binsetindex = atoi(setbitcomp);
			
			while(binsetindex > 0){
				remainder = binsetindex % 10;
				setindex = setindex + remainder * base2;
				binsetindex /= 10;
				base2 = base2 * 2;
			}
	
			count = 0;
			
			//Retrieve block index from address
			for(i = 49 - blockbits - 1; i < 49; i++){
				blockbitcomp[count] = binary[i];
				count++;
				
			}
			
			binblockindex = atoi(blockbitcomp);
			
			base2 = 1;
		
			while(binblockindex > 0){
				remainder = binsetindex % 10;
				blockindex = blockindex + remainder * base2;
				binblockindex /= 10;
				base2 = base2 * 2;
			}
			
			if(readwrite == 'R'){
				
				if((cache.sets[setindex].blocks[0].line[blockindex].tag != NULL) && strcmp(cache.sets[setindex].blocks[0].line[blockindex].tag, tag) == 0){
					cachehit++;
					
				}
				else{
					cachemiss++;
					memread++;
					cache.sets[setindex].blocks[0].line[blockindex].tag = tag;
					
				}
			}
			else{
				
				if((cache.sets[setindex].blocks[0].line[blockindex].tag != NULL) && strcmp(cache.sets[setindex].blocks[0].line[blockindex].tag, tag) == 0){
					cachehit++;
					memwrite++;
					
				}
				else{
					cachemiss++;
					memread++;
					memwrite++;
					cache.sets[setindex].blocks[0].line[blockindex].tag = tag;
					
				}
			
			}
		}
		
		//fully associative cache - 1 set
		else if(full){
			//blockbits
			
			char* blockbitcomp = (char*) malloc(sizeof(char) * (blockbits + 1));
			blockbitcomp[blockbits] = '\0';
			
			found = false;
			nullopen = false;
			
			
			int binblockindex = 0;
			int blockindex = 0;
			int remainder = 0;
			int base2 = 1;
			
	
			temp = 48 - blockbits;
			
			tag = (char*) malloc(sizeof(char) * (temp + 1));
			tag[temp] = '\0';
			
			for(i = 0; i < temp; i++){
				tag[i] = binary[i];
			}
			
			
			
			count = 0;
			
			//Retrieve block index from address
			for(i = temp; i < 49; i++){
				blockbitcomp[count] = binary[i];
				count++;
				
			}
			
			binblockindex = atoi(blockbitcomp);
		
			
			while(binblockindex > 0){
				remainder = binblockindex % 10;
				blockindex = blockindex + remainder * base2;
				binblockindex /= 10;
				base2 = base2 * 2;
			}
			
			for(j = 0; j < associativity; j++){
				if((cache.sets[0].blocks[j].line[blockindex].tag != NULL) && strcmp(cache.sets[0].blocks[j].line[blockindex].tag, tag) == 0){
					found = true;
					break;
				}
			}
			
			if(readwrite == 'R'){
				
				if(found){
					cachehit++;
					
				}
				else{
					cachemiss++;
					memread++;
					
					for(j = 0; j < associativity; j++){
						if(cache.sets[0].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[0].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[0].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[0].front = newFront;
								
								cache.sets[0].rear = newFront;
								
								fifo = 0;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[0].rear)->next = newRear;
								cache.sets[0].rear = (cache.sets[0].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
					
						fifo = (cache.sets[0].front)->numblockindex;
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[0].rear)->next = newRear;
						cache.sets[0].rear = (cache.sets[0].rear)->next;
						cache.sets[0].front = (cache.sets[0].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[0].blocks[fifo].line[k].tag = tag;
						}
						
					}
					
					
					
				}
			}
			else{
				
				if(found){
					cachehit++;
					memwrite++;
					
				}
				else{
					cachemiss++;
					memread++;
					memwrite++;
					
					for(j = 0; j < associativity; j++){
						if(cache.sets[0].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[0].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[0].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[0].front = newFront;
								
								cache.sets[0].rear = newFront;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[0].rear)->next = newRear;
								cache.sets[0].rear = (cache.sets[0].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
						fifo = (cache.sets[0].front)->numblockindex;
						cache.sets[0].front = (cache.sets[0].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[0].blocks[fifo].line[k].tag = tag;
						}
					
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[0].rear)->next = newRear;
						cache.sets[0].rear = (cache.sets[0].rear)->next;
						
					}
				
				}
			
			}
		}
		
		//n-way associative cache
		else{
			
			char* setbitcomp = (char*) malloc(sizeof(char) * (setbits + 1));
			setbitcomp[setbits] = '\0';
			char* blockbitcomp = (char*) malloc(sizeof(char) * (blockbits + 1));
			blockbitcomp[blockbits] = '\0';
			
			found = false;
			nullopen = false;
			
			int binsetindex = 0;
			int setindex = 0;
			int binblockindex = 0;
			int blockindex = 0;
			int remainder = 0;
			int base2 = 1;
			
			temp = 48 - setbits - blockbits;
			count = 0;
			tag = (char*) malloc(sizeof(char) * (temp + 1));
			tag[temp] = '\0';
			
			//Retrieve tag from address
			for(i = 0; i < temp; i++){
				tag[i] = binary[i];
			}
			
			//Retrieve set index from address
			for(i = temp; i < 49 - blockbits - 1; i++){
				setbitcomp[count] = binary[i];
				count++;
			}
			binsetindex = atoi(setbitcomp);
		
			
			while(binsetindex > 0){
				remainder = binsetindex % 10;
				setindex = setindex + remainder * base2;
				binsetindex /= 10;
				base2 = base2 * 2;
			}
		
			count = 0;
			
			//Retrieve block index from address
			for(i = 49 - blockbits - 1; i < 49; i++){
				blockbitcomp[count] = binary[i];
				count++;
			
			}
			
			binblockindex = atoi(blockbitcomp);
			
			base2 = 1;
		
			while(binblockindex > 0){
				remainder = binsetindex % 10;
				blockindex = blockindex + remainder * base2;
				binblockindex /= 10;
				base2 = base2 * 2;
			}
			
			for(j = 0; j < associativity; j++){
				if((cache.sets[setindex].blocks[j].line[blockindex].tag != NULL) && strcmp(cache.sets[setindex].blocks[j].line[blockindex].tag, tag) == 0){
					found = true;
					break;
				}
			}
			
			if(readwrite == 'R'){
		
				if(found){
					cachehit++;
				}
				else{
					cachemiss++;
					memread++;
					
					for(j = 0; j < associativity; j++){
						if(cache.sets[setindex].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[setindex].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[setindex].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[setindex].front = newFront;
								
								cache.sets[setindex].rear = newFront;
								
								fifo = 0;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[setindex].rear)->next = newRear;
								cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
					
						fifo = (cache.sets[setindex].front)->numblockindex;
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[setindex].rear)->next = newRear;
						cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
						cache.sets[setindex].front = (cache.sets[setindex].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[setindex].blocks[fifo].line[k].tag = tag;
						}
					}
				}
			}
			else{
				
				if(found){
					cachehit++;
					memwrite++;
					
				}
				else{
					cachemiss++;
					memread++;
					memwrite++;
					
					for(j = 0; j < associativity; j++){
						
						if(cache.sets[setindex].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[setindex].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[setindex].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[setindex].front = newFront;
								
								cache.sets[setindex].rear = newFront;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[setindex].rear)->next = newRear;
								cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
						
						fifo = (cache.sets[setindex].front)->numblockindex;
						cache.sets[setindex].front = (cache.sets[setindex].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[setindex].blocks[fifo].line[k].tag = tag;
						}
					
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[setindex].rear)->next = newRear;
						cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
						
					}
					
				}
			
			}
		
		}
		
		
	}
	printf("cache A\n");
	printResults(memread, memwrite, cachehit, cachemiss);
	
	for(i = 0; i < setsize; i++){
		cache.sets[i].front = NULL;
		cache.sets[i].rear = NULL;
		for(j = 0; j < associativity; j++){
			for(k = 0; k < blocksize; k++){
				cache.sets[i].blocks[j].line[k].tag = NULL;
			}
		}
	}
	
	memread = 0;
	memwrite = 0;
	cachehit = 0;
	cachemiss = 0;

	
	rewind(filepointer);
	//convention for type B - this is essentially the same code as from cache A, except the way the memory address is divided is different
	while(fscanf(filepointer, "%s %c %s\n", ignore, &readwrite, hexpreint) == 3){
		char* binary;
		
		//printf("%s\n", ignore);
		//printf("%s %c %s\n", ignore, readwrite, hexpreint);
		
		char* tag;
		
		//printf("%s\n", hexpreint);
		hexnum = (long)strtoul(hexpreint, NULL, 16);
		//printf("Hex in base 10 = %ld\n", hexnum);
		binary = longToBinary(hexnum);
		//printf("binary2 = %s\n", binary);
		
		//direct associative cache - 1 block per set
		if(direct){
			//setbits
			char* setbitcomp = (char*) malloc(sizeof(char) * (setbits + 1));
			setbitcomp[setbits] = '\0';
			//blockbits
			char* blockbitcomp = (char*) malloc(sizeof(char) * (setbits + 1));
			blockbitcomp[blockbits] = '\0';
			
			int binsetindex = 0;
			int setindex = 0;
			int binblockindex = 0;
			int blockindex = 0;
			int remainder;
			int base2 = 1;
			
			//printf("setbits = %d\n", setbits);
			//printf("blockbits = %d\n", blockbits);
			
			temp = 48 - blockbits;
			
			tag = (char*) malloc(sizeof(char) * (temp + 1));
			tag[temp] = '\0';
			
			
			
			//printf("tag    = %s\n", tag);
			
			//Retrieve set index from address
			for(i = 0; i < setbits; i++){
				setbitcomp[i] = binary[i];
				
			}
			binsetindex = atoi(setbitcomp);
			//printf("binsetindex = %d\n", binsetindex);
			while(binsetindex > 0){
				remainder = binsetindex % 10;
				setindex = setindex + remainder * base2;
				binsetindex /= 10;
				base2 = base2 * 2;
			}
			//printf("setindex = %d\n", setindex);
			count = 0;
			
			//Retrieve tag from address
			for(i = setbits; i < temp; i++){
				tag[count] = binary[i];
				//printf("tag[%d] = %c\n", i, tag[i]);
				count++;
				//printf("count = %d\n", count);
			}
			
			//printf("tag = %s\n", tag);
			count = 0;
			//Retrieve block index from address
			for(i = temp; i < 49; i++){
				blockbitcomp[count] = binary[i];
				count++;
				//printf("count = %d\n", count);
			}
			
			binblockindex = atoi(blockbitcomp);
			//printf("binblockindex = %d\n", binblockindex);
			base2 = 1;
			
			while(binblockindex > 0){
				remainder = binsetindex % 10;
				blockindex = blockindex + remainder * base2;
				binblockindex /= 10;
				base2 = base2 * 2;
			}
		
			if(readwrite == 'R'){
				
				if((cache.sets[setindex].blocks[0].line[blockindex].tag != NULL) && strcmp(cache.sets[setindex].blocks[0].line[blockindex].tag, tag) == 0){
					cachehit++;
					//printf("hit\n");
				}
				else{
					cachemiss++;
					memread++;
					cache.sets[setindex].blocks[0].line[blockindex].tag = tag;
					//printf("miss\n");
				}
			}
			else{
	
				if((cache.sets[setindex].blocks[0].line[blockindex].tag != NULL) && strcmp(cache.sets[setindex].blocks[0].line[blockindex].tag, tag) == 0){
					cachehit++;
					memwrite++;
					//printf("hit\n");
				}
				else{
					cachemiss++;
					memread++;
					memwrite++;
					cache.sets[setindex].blocks[0].line[blockindex].tag = tag;
					//printf("miss\n");
				}
			
			}
		}
		
		//fully associative cache - 1 set
		else if(full){
			//blockbits
			
			char* blockbitcomp = (char*) malloc(sizeof(char) * (blockbits + 1));
			blockbitcomp[blockbits] = '\0';
			
			found = false;
			nullopen = false;
			
			int binblockindex = 0;
			int blockindex = 0;
			int remainder = 0;
			int base2 = 1;
			
	
			temp = 48 - blockbits;
			
			tag = (char*) malloc(sizeof(char) * (temp + 1));
			tag[temp] = '\0';
			
			for(i = 0; i < temp; i++){
				tag[i] = binary[i];
			}
		
			count = 0;
			
			//Retrieve block index from address
			for(i = temp; i < 49; i++){
				blockbitcomp[count] = binary[i];
				count++;
				//printf("count = %d\n", count);
			}
			
			binblockindex = atoi(blockbitcomp);
		
			
			while(binblockindex > 0){
				remainder = binblockindex % 10;
				blockindex = blockindex + remainder * base2;
				binblockindex /= 10;
				base2 = base2 * 2;
			}
			
			
			for(j = 0; j < associativity; j++){
				if((cache.sets[0].blocks[j].line[blockindex].tag != NULL) && strcmp(cache.sets[0].blocks[j].line[blockindex].tag, tag) == 0){
					found = true;
					break;
				}
			}
			
			if(readwrite == 'R'){
				//printf("%s compare\n%s\n", tag, cache.sets[setindex].blocks[0].line[blockindex].tag); 
				if(found){
					cachehit++;
					//printf("hit\n");
				}
				else{
					cachemiss++;
					memread++;
					
					for(j = 0; j < associativity; j++){
						if(cache.sets[0].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[0].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[0].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[0].front = newFront;
								
								cache.sets[0].rear = newFront;
								
								fifo = 0;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[0].rear)->next = newRear;
								cache.sets[0].rear = (cache.sets[0].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
					
						fifo = (cache.sets[0].front)->numblockindex;
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[0].rear)->next = newRear;
						cache.sets[0].rear = (cache.sets[0].rear)->next;
						cache.sets[0].front = (cache.sets[0].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[0].blocks[fifo].line[k].tag = tag;
						}
						
					}
					
					
					//printf("miss\n");
				}
			}
			else{
				
				if(found){
					cachehit++;
					memwrite++;
					//printf("hit\n");
				}
				else{
					cachemiss++;
					memread++;
					memwrite++;
					
					for(j = 0; j < associativity; j++){
						if(cache.sets[0].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[0].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[0].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[0].front = newFront;
								
								cache.sets[0].rear = newFront;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[0].rear)->next = newRear;
								cache.sets[0].rear = (cache.sets[0].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
						fifo = (cache.sets[0].front)->numblockindex;
						cache.sets[0].front = (cache.sets[0].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[0].blocks[fifo].line[k].tag = tag;
						}
					
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[0].rear)->next = newRear;
						cache.sets[0].rear = (cache.sets[0].rear)->next;
						
					}
					//printf("miss\n");
				}
			
			}
		}
		
		//n-way associative cache
		else{
			
			char* setbitcomp = (char*) malloc(sizeof(char) * (setbits + 1));
			setbitcomp[setbits] = '\0';
			char* blockbitcomp = (char*) malloc(sizeof(char) * (blockbits + 1));
			blockbitcomp[blockbits] = '\0';
			
			found = false;
			nullopen = false;
			
			int binsetindex = 0;
			int setindex = 0;
			int binblockindex = 0;
			int blockindex = 0;
			int remainder = 0;
			int base2 = 1;
			
			temp = 48 - blockbits;
			count = 0;
			tag = (char*) malloc(sizeof(char) * (temp + 1));
			tag[temp] = '\0';
			
			//Retrieve tag from address
			for(i = setbits; i < temp; i++){
				tag[count] = binary[i];
				count++;
			}
			
			//Retrieve set index from address
			for(i = 0; i < setbits; i++){
				setbitcomp[i] = binary[i];
				
			}
			binsetindex = atoi(setbitcomp);
		
			
			while(binsetindex > 0){
				remainder = binsetindex % 10;
				setindex = setindex + remainder * base2;
				binsetindex /= 10;
				base2 = base2 * 2;
			}
		
			count = 0;
			
			//Retrieve block index from address
			for(i = temp; i < 49; i++){
				blockbitcomp[count] = binary[i];
				count++;
			
			}
			
			binblockindex = atoi(blockbitcomp);
			
			base2 = 1;
		
			while(binblockindex > 0){
				remainder = binsetindex % 10;
				blockindex = blockindex + remainder * base2;
				binblockindex /= 10;
				base2 = base2 * 2;
			}
			
			for(j = 0; j < associativity; j++){
				if((cache.sets[setindex].blocks[j].line[blockindex].tag != NULL) && strcmp(cache.sets[setindex].blocks[j].line[blockindex].tag, tag) == 0){
					found = true;
					break;
				}
			}
			
			if(readwrite == 'R'){
		
				if(found){
					cachehit++;
				}
				else{
					cachemiss++;
					memread++;
					
					for(j = 0; j < associativity; j++){
						if(cache.sets[setindex].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[setindex].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[setindex].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[setindex].front = newFront;
								
								cache.sets[setindex].rear = newFront;
								
								fifo = 0;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[setindex].rear)->next = newRear;
								cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
					
						fifo = (cache.sets[setindex].front)->numblockindex;
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[setindex].rear)->next = newRear;
						cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
						cache.sets[setindex].front = (cache.sets[setindex].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[setindex].blocks[fifo].line[k].tag = tag;
						}
					}
				}
			}
			else{
				
				if(found){
					cachehit++;
					memwrite++;
					
				}
				else{
					cachemiss++;
					memread++;
					memwrite++;
					
					for(j = 0; j < associativity; j++){
						
						if(cache.sets[setindex].blocks[j].line[blockindex].tag == NULL){
							for(k = 0; k < blocksize; k++){
								cache.sets[setindex].blocks[j].line[k].tag = tag;
							}
							
							if(cache.sets[setindex].front == NULL){
								linkedList* newFront = (linkedList*) malloc(sizeof(linkedList));
								newFront->numblockindex = j;
								newFront->next = NULL;
								cache.sets[setindex].front = newFront;
								
								cache.sets[setindex].rear = newFront;
							}
							else{
								linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
								newRear->numblockindex = j;
								newRear->next = NULL;
						
								(cache.sets[setindex].rear)->next = newRear;
								cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
							}
							
							nullopen = true;
							break;
						}
					}
					if(nullopen == false){
						
						fifo = (cache.sets[setindex].front)->numblockindex;
						cache.sets[setindex].front = (cache.sets[setindex].front)->next;
						
						for(k = 0; k < blocksize; k++){
							cache.sets[setindex].blocks[fifo].line[k].tag = tag;
						}
					
						linkedList* newRear = (linkedList*) malloc(sizeof(linkedList));
						newRear->numblockindex = fifo;
						newRear->next = NULL;
						
						(cache.sets[setindex].rear)->next = newRear;
						cache.sets[setindex].rear = (cache.sets[setindex].rear)->next;
						
					}
					
				}
			
			}
		
		}
		
		
	}
	
	printf("cache B\n");
	printResults(memread, memwrite, cachehit, cachemiss);
	
	free(cache.sets);
	
	fclose(filepointer);
	return 0;
}
