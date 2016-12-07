// Vo Tran Thanh Luong - 1551020
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>
#include <math.h>
#include "cachelab.h"

typedef unsigned long long int memoryAddress;


struct cacheParameter {      // store all the  basic info of cache
	int s; 
	int b; 
	int E; 
	int S; 
	int B; 
	int hits;
	int misses;
	int evicts;
};

struct setLine {          // store necessary info of a Line
	int latestUsed;
	int valid;
	memoryAddress tag;
	char *block;
};

struct cacheSet{    // multi lines build up a set
	struct setLine *lines;
}; 

struct cache{      // a cache has many sets
	 struct cacheSet *sets;
};

struct cache initCache(long long numberOfSets, int numberOfLines, long long sizeOfBlock) 
{    // this function constructs the cache by the given information

	struct cache newCache;	 
	struct cacheSet set;
	struct setLine line;
	int indexOfSet;
	int indexOfLine;

	newCache.sets = (struct cacheSet*) malloc(sizeof(struct cacheSet) * numberOfSets);      // do memory allocation in C

	for (indexOfSet = 0; indexOfSet < numberOfSets; indexOfSet ++) 
	{        // this loop loops through every line in every set and put the default value 0 inside every slot. (because we contain nothing in the cache)
		
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
{     //this function cleans up everything  when done because memory is expensive in cache and C doens't handle memory itself.



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
	// check is whether the line that is matched is empty or not
	


	int numberOfLines = exampleParameter.E;
	int index;
	struct setLine line;

	for (index = 0; index < numberOfLines; index ++) {
		line = exampleSet.lines[index];
		if (line.valid == 0) {    // one line is available
			return index;
		}
	}
	return -1;     // no line is available
}

int detectEvictLine(struct cacheSet exampleSet, struct cacheParameter exampleParameter, int *usedLines) {
	// this function detects the line that can be evicted ( the least recently used line)



	int numberOfLines = exampleParameter.E;
	int maxFreqUsage = exampleSet.lines[0].latestUsed;     //store usage frequency
	int minFreqUsage = exampleSet.lines[0].latestUsed;	 // store usage frequency
	int minFreqUsage_index = 0;
	int indexOfLine;


	//very basic loop, compare each line with max & min to decide
	for (indexOfLine = 1; indexOfLine < numberOfLines; indexOfLine ++) {
		if (minFreqUsage > exampleSet.lines[indexOfLine].latestUsed) {
			minFreqUsage_index = indexOfLine;	
			minFreqUsage = exampleSet.lines[indexOfLine].latestUsed;
		}

		if (maxFreqUsage < exampleSet.lines[indexOfLine].latestUsed) {
			maxFreqUsage = exampleSet.lines[indexOfLine].latestUsed;
		}
	}

	usedLines[0] = minFreqUsage;
	usedLines[1] = maxFreqUsage;
	return minFreqUsage_index;
}

/* this is the most important operation*/
struct cacheParameter accessTheCacheData(struct cache myCache, struct cacheParameter exampleParameter, memoryAddress address) {
		int indexOfLine;
		int checkFullCache = 1;     // assume that cache is full

		int numberOfLines = exampleParameter.E;
		int previousHit = exampleParameter.hits;

		int tagSize = (64 - (exampleParameter.s + exampleParameter.b));    // take the valid tag out t = m-s-b
		memoryAddress inputTag = address >> (exampleParameter.s + exampleParameter.b);
		unsigned long long temp = address << (tagSize);
		unsigned long long indexOfSet = temp >> (tagSize + exampleParameter.b);
		
  		struct cacheSet exampleSet = myCache.sets[indexOfSet];

		for (indexOfLine = 0; indexOfLine < numberOfLines; indexOfLine ++) 	{
				
			if (exampleSet.lines[indexOfLine].valid) {   // check the valid tag != 0
					
				if (exampleSet.lines[indexOfLine].tag == inputTag) {
						//check for matching tag


					exampleSet.lines[indexOfLine].latestUsed ++;  // update for later use of eviction
					exampleParameter.hits ++;    // tag match -> raise hit
				}
				// If the valid tag is different from 0 and the input tag matches that line tag, then it is safe for us to raise the hit because we did cache hit


			} else if (!(exampleSet.lines[indexOfLine].valid) && (checkFullCache)) {
				// valid tag = 0, fullcache != 0
				
				checkFullCache = 0;	    // reset this to 0	because there is empty space left.
			}
			// 
		}	

		if (previousHit == exampleParameter.hits) {   // if after the above loop nothing hit -> we miss
			exampleParameter.misses++;    // raise miss
		} else {
			return exampleParameter;
		}
		int *usedLines = (int*) malloc(sizeof(int) * 2);
		int minFreqUsage_index = detectEvictLine(exampleSet, exampleParameter, usedLines);	

		if (checkFullCache)     // if cache is full (checkFullCache!=0) do eviction
		{
			exampleParameter.evicts++;
			exampleSet.lines[minFreqUsage_index].tag = inputTag;
			exampleSet.lines[minFreqUsage_index].latestUsed = usedLines[1] + 1;
		
		}

		else        // else write to tge empty line
	        {
			int empty_index = detectEmptyLine(exampleSet, exampleParameter);
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
	bzero(&exampleParameter, sizeof(exampleParameter));  // read the report for this function's purpose
	long long numberOfSets;
	long long sizeOfBlock;	
	FILE *openTrace;
	char instructionInTraceFile;
	memoryAddress address;
	int size;
	char *trace_file;
	char c;
	/* this part takes in argument. More on how do I do this-> read report file */
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
            exit(0);
        default:
            exit(1);
        }
    }
   /* end of take in arguments from command line */ 

 	numberOfSets = pow(2.0, exampleParameter.s);   // get Number of set by 2^s
	sizeOfBlock = pow(2.0, exampleParameter.b);  //  get sizeOfBlock by 2^b
	exampleParameter.hits = 0;
	exampleParameter.misses = 0;
	exampleParameter.evicts = 0;
	myCache = initCache (numberOfSets, exampleParameter.E, sizeOfBlock);

	/* this part read file. More on how do I do this-> read report file */
	openTrace  = fopen(trace_file, "r");
	if (openTrace != NULL) {
		while (fscanf(openTrace, " %c %llx,%d", &instructionInTraceFile, &address, &size) == 3) {
			switch(instructionInTraceFile) {
				case 'I':
					break;
				case 'L':
					exampleParameter = accessTheCacheData(myCache, exampleParameter, address);
					break;
				case 'S':
					exampleParameter = accessTheCacheData(myCache, exampleParameter, address);
					break;
				case 'M':
					exampleParameter = accessTheCacheData(myCache, exampleParameter, address);
					exampleParameter = accessTheCacheData(myCache, exampleParameter, address);	
					break;
				default:
					break;
			}
		}
	}
	/* end of read file */
	
    printSummary(exampleParameter.hits, exampleParameter.misses, exampleParameter.evicts);
	cleanFunction(myCache, numberOfSets, exampleParameter.E, sizeOfBlock);
	fclose(openTrace);
    return 0;
}
