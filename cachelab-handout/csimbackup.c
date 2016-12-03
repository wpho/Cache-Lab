// Vo Tran Thanh Luong - 1551020



#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>
#include <math.h>

#include "cachelab.h"

/* Always use a 64-bit variable to hold memory addresses*/
typedef unsigned long long int memoryAddress;

/* a struct that groups cache parameters together */
struct cacheParameter {
	int s; /* 2**s cache sets */
	int b; /* cacheline block size 2**b bytes */
	int E; /* number of cachelines per set */
	int S; /* number of sets, derived from S = 2**s */
	int B; /* cacheline block size (bytes), derived from B = 2**b */

	int hits;
	int misses;
	int evicts;
};


struct setLine{
	int latestUsed;
	int valid;
	memoryAddress tag;
	char *block;
};

struct cacheSet{
	struct setLine *lines;
}; 

struct cache{
	 struct cacheSet *sets;
};




long long bit_pow(int exp) 
{
	long long result = 1;
	result = result << exp;
	return result;
}

/*
 * printUsage - Print usage info
 */
// void printUsage(char* argv[])
// {
//     printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
//     printf("Options:\n");
//     printf("  -h         Print this help message.\n");
//     printf("  -v         Optional verbose flag.\n");
//     printf("  -s <num>   Number of set index bits.\n");
//     printf("  -E <num>   Number of lines per set.\n");
//     printf("  -b <num>   Number of block offset bits.\n");
//     printf("  -t <file>  Trace file.\n");
//     printf("\nExamples:\n");
//     printf("  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
//     printf("  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
//     exit(0);
// }



struct cache buidlingTheCache(long long numberOfSets, int numberOfLines, long long sizeOfBlock) 
{

	struct cache newCache;	
	struct cacheSet set;
	struct setLine line;
	int indexOfSet;
	int indexOfLine;

	newCache.sets = (struct cacheSet*) malloc(sizeof(struct cacheSet) * numberOfSets);

	for (indexOfSet = 0; indexOfSet < numberOfSets; indexOfSet ++) 
	{
		
		set.lines =  (struct setLine *) malloc(sizeof(struct setLine) * numberOfLines);
		newCache.sets[indexOfSet] = set;

		for (indexOfLine = 0; indexOfLine < numberOfLines; indexOfLine ++) 
		{
			line.latestUsed = 0;
			line.valid = 0;
			line.tag = 0; 
			set.lines[indexOfLine] = line;	
		}
		
	} 

	return newCache;
	
}


void cleanFunction(struct cache myCache, long long numberOfSets, int numberOfLines, long long sizeOfBlock) 
{
	int indexOfSet;
	

	for (indexOfSet = 0; indexOfSet < numberOfSets; indexOfSet ++) 
	{
		struct cacheSet set = myCache.sets[indexOfSet];
		
		if (set.lines != NULL) {	
			free(set.lines);
		}
		
	} 
	if (myCache.sets != NULL) {
		free(myCache.sets);
	}
}

int detectEmptyLine(struct cacheSet exampleSet, struct cacheParameter exampleParameter) {
	int numberOfLines = exampleParameter.E;
	int index;
	struct setLine line;

	for (index = 0; index < numberOfLines; index ++) {
		line = exampleSet.lines[index];
		if (line.valid == 0) {
			return index;
		}
	}
	//Control flow should not fall here. Method is only called if checkFullCache flag is set to false.
	return -1;
}

int detectEvictLine(struct cacheSet exampleSet, struct cacheParameter exampleParameter, int *usedLines) {
	
	//Returns index of least recently used line.
	//usedLines[0] gives least recently used line, usedLines[1] gives current lru counter or most recently used line.
	int numberOfLines = exampleParameter.E;
	int maxFreqUsage = exampleSet.lines[0].latestUsed;
	int minFreqUsage = exampleSet.lines[0].latestUsed;
	int minFreqUsage_index = 0;

	struct setLine line; 
	int indexOfLine;

	for (indexOfLine = 1; indexOfLine < numberOfLines; indexOfLine ++) {
		line = exampleSet.lines[indexOfLine];

		if (minFreqUsage > line.latestUsed) {
			minFreqUsage_index = indexOfLine;	
			minFreqUsage = line.latestUsed;
		}

		if (maxFreqUsage < line.latestUsed) {
			maxFreqUsage = line.latestUsed;
		}
	}

	usedLines[0] = minFreqUsage;
	usedLines[1] = maxFreqUsage;
	return minFreqUsage_index;
}

struct cacheParameter testCacheFunction(struct cache myCache, struct cacheParameter exampleParameter, memoryAddress address) {
		
		int indexOfLine;
		int checkFullCache = 1;

		int numberOfLines = exampleParameter.E;
		int previousHit = exampleParameter.hits;

		int tagSize = (64 - (exampleParameter.s + exampleParameter.b));
		memoryAddress inputTag = address >> (exampleParameter.s + exampleParameter.b);
		unsigned long long temp = address << (tagSize);
		unsigned long long indexOfSet = temp >> (tagSize + exampleParameter.b);
		
  		struct cacheSet exampleSet = myCache.sets[indexOfSet];

		for (indexOfLine = 0; indexOfLine < numberOfLines; indexOfLine ++) 	{
			
			struct setLine line = exampleSet.lines[indexOfLine];
			
			if (line.valid) {
					
				if (line.tag == inputTag) {
						
					line.latestUsed ++;
					exampleParameter.hits ++;
					exampleSet.lines[indexOfLine] = line;
				}

			} else if (!(line.valid) && (checkFullCache)) {
				//We found an empty line
				checkFullCache = 0;		
			}

		}	

		if (previousHit == exampleParameter.hits) {
			//Miss in cache;
			exampleParameter.misses++;
		} else {
			//Data is in cache
			return exampleParameter;
		}

		//We missed, so evict if necessary and write data into cache.
		
		int *usedLines = (int*) malloc(sizeof(int) * 2);
		int minFreqUsage_index = detectEvictLine(exampleSet, exampleParameter, usedLines);	

		if (checkFullCache) 
		{
			exampleParameter.evicts++;

			//Found least-recently-used line, overwrite it.
			exampleSet.lines[minFreqUsage_index].tag = inputTag;
			exampleSet.lines[minFreqUsage_index].latestUsed = usedLines[1] + 1;
		
		}

		else
	        {
			int empty_index = detectEmptyLine(exampleSet, exampleParameter);

			//Found first empty line, write to it.
			exampleSet.lines[empty_index].tag = inputTag;
			exampleSet.lines[empty_index].valid = 1;
			exampleSet.lines[empty_index].latestUsed = usedLines[1] + 1;
		}						

		free(usedLines);
		return exampleParameter;
}

int main(int argc, char **argv)
{
	
	struct cache myCache;
	struct cacheParameter exampleParameter;
	bzero(&exampleParameter, sizeof(exampleParameter));

	long long numberOfSets;
	long long sizeOfBlock;	


 
	FILE *openTrace;
	char instructionInTraceFile;
	memoryAddress address;
	int size;
	
	char *trace_file;
	char c;
    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1)
	{
        switch(c)
		{
        case 's':
            exampleParameter.s = atoi(optarg);
            break;
        case 'E':
            exampleParameter.E = atoi(optarg);
            break;
        case 'b':
            exampleParameter.b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'h':
            //printUsage(argv);
            exit(0);
        default:
            //printUsage(argv);
            exit(1);
        }
    }


	
	// you need to compute S and B yourself
 	numberOfSets = pow(2.0, exampleParameter.s);
	sizeOfBlock = bit_pow(exampleParameter.b);	
	exampleParameter.hits = 0;
	exampleParameter.misses = 0;
	exampleParameter.evicts = 0;
	
	myCache = buidlingTheCache (numberOfSets, exampleParameter.E, sizeOfBlock);
 	
	// fill in rest of the simulator routine
	openTrace  = fopen(trace_file, "r");
	
	
	if (openTrace != NULL) {
		while (fscanf(openTrace, " %c %llx,%d", &instructionInTraceFile, &address, &size) == 3) {
			switch(instructionInTraceFile) {
				case 'I':
					break;
				case 'L':
					exampleParameter = testCacheFunction(myCache, exampleParameter, address);
					break;
				case 'S':
					exampleParameter = testCacheFunction(myCache, exampleParameter, address);
					break;
				case 'M':
					exampleParameter = testCacheFunction(myCache, exampleParameter, address);
					exampleParameter = testCacheFunction(myCache, exampleParameter, address);	
					break;
				default:
					break;
			}
		}
	}
	
    printSummary(exampleParameter.hits, exampleParameter.misses, exampleParameter.evicts);
	cleanFunction(myCache, numberOfSets, exampleParameter.E, sizeOfBlock);
	fclose(openTrace);

    return 0;
}
