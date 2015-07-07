#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HINTLENGTH		2
#define BUFFER 			1024
#define DICTIONARYSIZE	58110	// # of lines/words in the word list (words.txt); this is known beforehand
#define MAXWORDLENGTH	22		// length of longest word in word list; this is known beforehand
#define FILENAME 		"words.txt"

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
		fgets(dictionary + i*maxWordLength, maxWordLength+2, fp); // +2 for newline and null terminator
		trimWhitespace(dictionary + i*maxWordLength);
	}
}

void printCipher(char* cipher){
	int i;
	for (i = 0; i < 26; i++){
		printf("(%c, %c) ", i+'A', *(cipher+i));
	}
	printf("\n");
}

/**
 * Just check the case.
 */
int isUpper(char c){
	return c >= 'A' && c <= 'Z';
}
int isLower(char c){
	return c >= 'a' && c <= 'z';
}

/**
 * Check if a word matches a given format.
 */
int wordMatchesFormat(char* word, char* format){
	int i;
	int wordLen = strlen(word);
	int formatLen = strlen(format);
	if (wordLen != formatLen)
		return 0;

	char formatMap[1024] = {0};
	for (i = 0; i < formatLen; i++){
		char c = *(format + i);
		if (isLower(c)){ // found a known letter
			char c2 = *(word + i);
			if (c != c2){ // check if the known letter matches up
				return 0;
			}
		}
		else if (isUpper(c)){ // make sure the format matches
			char index = c-'A';
			if (!formatMap[index]){
				formatMap[index] = *(word + i); // match the uppercase letter to the current lowercase letter
			}
			else if (formatMap[index] != *(word + i)){
				return 0; // if the current lowercase does not match a previously matched one, no match
			}
		}
	}
	return 1;
}

/**
 * Save all possible words that match te given format.
 */
int getPossibleWords(char** possibleWords, char* format, char* dictionary, int dictionarySize, int maxWordLength){
	int i;
	int count = 0;
	for (i = 0; i < dictionarySize; i++){
		if (wordMatchesFormat( dictionary+i*maxWordLength, format )){
			*possibleWords = (char*)realloc( *possibleWords, ( (count+1)*(maxWordLength) )*sizeof(char) );
			memcpy(*possibleWords + count*maxWordLength, dictionary + i*maxWordLength, maxWordLength);
			count++;
		}
	}
	return count;
}

/**
 * Before is the format (upper or lowercase). After is the possible word from the dict (all lowercase).
 */
int cipherFromMap(char* before, char* after, char* base, char* newCipher){
	int i, j;
	int len = strlen(before);
	for (i = 0; i < 26; i++){
		*(newCipher + i) = *(base + i);
	}
	for (i = 0; i < len; i++){
		char b = *(before + i);
		char a = *(after + i);
		if (isUpper(b)){
			// If the value exists in the cipher and it is not equal to the corresponding letter
			int letterIndex = b - 'A';
			if ( *(newCipher + letterIndex) ){
				if ( *(newCipher + letterIndex) != a ){
					return 0;
				}	
			}
			*(newCipher + letterIndex) = a;
		}
	}

	// Check for any duplicates
	for (i = 0; i < 26; i++){
		for (j = i+1; j < 26; j++){
			if ( *(newCipher+i) ){
				if ( *(newCipher+i) == *(newCipher+j) ) // found dup
					return 0;
			}
		}
	}

	return 1;
}

/**
 * Find all available ciphers.
 */
int unscramble(char** ciphers, char* words, int wordCount, int depth, char* cipher, char* dictionary, int dictionarySize, int maxWordLength){
	if (depth >= wordCount){
		*ciphers = (char*)realloc( *ciphers, 26*sizeof(char) );
		memcpy(*ciphers, cipher, 26);
		return 1;
	}

	int i;
	char* possibleWords = (char*)malloc(1);
	int cipherCount = 0;
	int matchCount = getPossibleWords(&possibleWords, words + depth*maxWordLength, dictionary, dictionarySize, maxWordLength);
	for (i = 0; i < matchCount; i++){
		// option = possibleWords + i*maxWordLength
		char newCipher[26] = {0};
		int success = cipherFromMap(words + depth*maxWordLength, possibleWords + i*maxWordLength, cipher, newCipher);
		if (success){
			char* solutions = (char*)malloc(1);
			int solutionCount = unscramble(&solutions, words, wordCount, depth+1, newCipher, (char*)dictionary, dictionarySize, maxWordLength);
			
			int j;
			printf("found %d solutions\n", solutionCount);
			for (j = 0; j < solutionCount; j++){
				*ciphers = (char*)realloc( *ciphers, sizeof(char)*(cipherCount+1)*26 );
				memcpy(*ciphers + j*26, solutions + j*26, 26);
				cipherCount++;
			}

			free(solutions);
		}
	}
	free(possibleWords);

	return 0;
}

int main(){
	/* Read form stdin */
	char text[BUFFER];
	fgets(text, BUFFER, stdin); // ciphered text
	trimWhitespace(text);

	/* Split the text into words */
	char* words = (char*)malloc(1);
	int wordCount = 0;
	char* word = strtok(text, " ");
	while (word != NULL){
		words = (char*)realloc(words, (wordCount+1)*MAXWORDLENGTH*sizeof(char));
		strncpy(words + wordCount*MAXWORDLENGTH, word, MAXWORDLENGTH);
		wordCount++;
		word = strtok(NULL, " ");
	}

	/* Create the cipher mapping the second val to the first  */
	char cipher[26] = {0};
	char hintCountBuffer[BUFFER];
	fgets(hintCountBuffer, BUFFER, stdin);
	int hintCount = atoi(hintCountBuffer);
	int i;
	for (i = 0; i < hintCount; i++){
		char* buffer = malloc( HINTLENGTH*sizeof(char) );
		fgets(buffer, HINTLENGTH+2, stdin);
		char first = *(buffer);
		char second = *(buffer+1);
		cipher[second-'A'] = first;
	}

	// Create the dictionary
	char dictionary[DICTIONARYSIZE][MAXWORDLENGTH];
	createDictionary((char*) dictionary, FILENAME, DICTIONARYSIZE, MAXWORDLENGTH);

	/* Do the thing */
	char* solutions = (char*)malloc(1);
	int solutionCount = unscramble(&solutions, words, wordCount, 0, cipher, (char*)dictionary, DICTIONARYSIZE, MAXWORDLENGTH);
	printf("%d solutions\n", solutionCount);
	for (i = 0; i < solutionCount; i++){
		printCipher(solutions + i*26);
	}
	free(solutions);

	return 0;
}