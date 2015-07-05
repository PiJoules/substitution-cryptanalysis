#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "format.h"

#define HINTLENGTH		2
#define BUFFER 			1024
#define DICTIONARYSIZE	58110	// # of lines/words in the word list (words.txt); this is known beforehand
#define MAXWORDLENGTH	22		// length of longest word in word list; this is known beforehand

/* Utility function for trimming trailing whitespace */
void trimWhitespace(char* str){
	int len = strlen(str);
	int i = 0;
	while (*(str + len-1 - i) == '\n'){
		*(str + len-1 - i) = '\0';
		i++;
	}
}

/* Add all of the words in the word list to an existing 2d array */
void createDictionary(char* dictionary, const char* fileName, int dictionarySize, int maxWordLength){
	int i;
	FILE *fp = fopen(fileName, "r");
	for (i = 0; i < dictionarySize; i++){
		fgets(dictionary + i*maxWordLength, maxWordLength+2, fp); // +2 for mewline and null terminator
		trimWhitespace(dictionary + i*maxWordLength);
	}
}

/* Given a format, check if there exists a word in the citionary matching it */
char* wordMatches(char* dictionary, char* format, int dictionarySize, int maxWordLength, int* matchCount){
	char* matches = (char*)malloc(1);
	int i;
	int count = 0;
	for (i = 0; i < dictionarySize; i++){
		if (strMatchesFormat( dictionary+i*maxWordLength, format )){
			//printf("word (%s) matches format (%s)\n", dictionary+i*maxWordLength, format);
			matches = realloc( matches, ( (count+1)*maxWordLength )*sizeof(char) );
			strncpy(matches + count*maxWordLength, dictionary+i*maxWordLength, maxWordLength);
			count++;
		}
	}
	*matchCount = count;
	return matches;
}

/* Replace each appropriate character with its hint */
void substituteHints(char* hints, char* str, int hintCount, int hintLength){
	int i, j;
	int len = strlen(str);
	for (i = 0; i < hintCount; i++){
		char hint = *(hints + i*(hintLength+1)); // +1 to account for null terminator
		char cipherChar = *(hints + i*(hintLength+1) + 1);
		for (j = 0; j < len; j++){
			char c = *(str + j);
			if (c == cipherChar){
				*(str + j) = hint;
			}
		}
	}
}

int main(){
	/* Read form stdin */
	const char fileName[] = "words.txt";
	char text[BUFFER];
	fgets(text, BUFFER, stdin); // get ciphered text
	trimWhitespace(text);
	char hintCountBuffer[BUFFER];
	fgets(hintCountBuffer, BUFFER, stdin); // get num of hints
	int hintCount = atoi(hintCountBuffer);
	int i;
	char hints[hintCount][HINTLENGTH+1];
	for (i = 0; i < hintCount; i++){
		fgets(hints[i], HINTLENGTH+2, stdin); // get each hint
		trimWhitespace(hints[i]);
	}

	/*
	For each word, guess a new set of keys and apply it to the word.
	As long as there exists unknown letters in the word, keep getting new sets
	of keys until there are no more unknown letters or there are no more possible guesses.

	A guess is made from by finding more words that can fit a given format. The guess is
	assigning the guessed word to its format.
	 */

	// Create the dictionary
	char dictionary[DICTIONARYSIZE][MAXWORDLENGTH];
	createDictionary((char*) dictionary, fileName, DICTIONARYSIZE, MAXWORDLENGTH);

	// Apply the hints
	substituteHints((char*)hints, text, hintCount, HINTLENGTH);

	// Split the text into words
	char* word = strtok(text, " ");
	while (word != NULL){
		char* format = getFormat(word);

		int matchCount;
		char* matches = wordMatches((char*) dictionary, format, DICTIONARYSIZE, MAXWORDLENGTH, &matchCount);
		for (i = 0; i < matchCount; i++){
			printf("match (%s): %s\n", format, matches + i*MAXWORDLENGTH);
		}
		free(matches);

		free(format);

		word = strtok(NULL, " ");
	}

	/* Print stuff */
	printf("text: %s\n", text);
	printf("hintCount: %d\n", hintCount);
	for (i = 0; i < hintCount; i++){
		printf("hint %d: %s\n", i+1, hints[i]);
	}

	return 0;
}