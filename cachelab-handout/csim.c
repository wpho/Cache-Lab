// Vo Tran Thanh Luong - 1551020
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>
#include <math.h>
#include "cachelab.h"

typedef unsigned long long int memoryAddress;


struct cacheParameter {
	int s; 
	int b; 
	int E; 
	int S; 
	int B; 
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
	return -1;
}

int detectEvictLine(struct cacheSet exampleSet, struct cacheParameter exampleParameter, int *usedLines) {
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
				checkFullCache = 0;		
			}
		}	

		if (previousHit == exampleParameter.hits) {
			exampleParameter.misses++;
		} else {
			return exampleParameter;
		}
		int *usedLines = (int*) malloc(sizeof(int) * 2);
		int minFreqUsage_index = detectEvictLine(exampleSet, exampleParameter, usedLines);	

		if (checkFullCache) 
		{
			exampleParameter.evicts++;
			exampleSet.lines[minFreqUsage_index].tag = inputTag;
			exampleSet.lines[minFreqUsage_index].latestUsed = usedLines[1] + 1;
		
		}

		else
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
 	numberOfSets = pow(2.0, exampleParameter.s);
	sizeOfBlock = bit_pow(exampleParameter.b);	
	exampleParameter.hits = 0;
	exampleParameter.misses = 0;
	exampleParameter.evicts = 0;
	myCache = buidlingTheCache (numberOfSets, exampleParameter.E, sizeOfBlock);
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
