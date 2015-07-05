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

/* Update an array of words witha given cipher */
void applyCipher(char* words, char cipher[26], char* wordsCopy, int wordCount, int maxWordLength){
	int i, j, k;
	for (i = 0; i < wordCount; i++){
		int len = strlen(words + i*maxWordLength);
		for (j = 0; j < len; j++){
			char c = *(words + i*maxWordLength + j);
			if (isUpper(c)){
				for (k = 0; k < 26; k++){
					if (cipher[k] == c){
						*(wordsCopy + i*maxWordLength + j) = k+'a';
					}
				}
			}
		}
	}
}

/* Check if the format exists in a given dictionary */
int formatExists(char* dictionary, char* word, int dictionarySize, int maxWordLength){
	int i, j;
	int len = strlen(word);
	for (i = 0; i < dictionarySize; i++){
		if (strlen( dictionary + i*maxWordLength ) == len){
			for (j = 0; j < len; j++){
				char c = *(word+j);
				if (isLower(c)){
					char c2 = *(dictionary + i*maxWordLength + j);
					if (c != c2){
						break;
					}
				}
				if (j == len-1){
					return 1; // found a match
				}
			}
		}
	}
	return 0;
}

/* Check if a given array of words exists. */
int formatsExist(char* dictionary, char* words, int dictionarySize, int maxWordLength, int wordCount){
	int i;
	for (i = 0; i < wordCount; i++){
		if (!formatExists(dictionary, words + i*maxWordLength, dictionarySize, maxWordLength)){
			return 0;
		}
	}
	return 1;
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

	Need:
	- copy of original sentence since it will by edited
	- copy of original list of words (split original sentence)

	Code:
	// 0 if the key is not used
	// 1-26 to map the corresponding letter
	// value represents uppercase CIPHER
	// key is the lowercase mapped letter
	char cipher[26] = {0}; 
	applyCipher(words, cipher);
	int skips[words.length*MAXWORDLENGTH] = {0};
	int wordOffets[words.length*MAXWORDLENGTH] = {0};
	char changes[26] = {0};
	int lastChangeIndex = 0; // keep track of the last change made to the cipher
	for (int i = 0; i < words.length; i++){
		wordOffsets[i+1] = words[i].length + wordOffsets[i];
	}
	for (int i = 0; i < words.length; i++){
		char* word = words[i];
		for (int j = 0; j < word.length; j++){
			char c = word[j];
			if (isUpper(c)){
				// Get first available cipher
				int k;
				for (k = skips[wordOffsets[i]+j]; k < cipher.length; k++){
					if (!cipher[k]){
						cipher[k] = c;
						changes[lastChangeIndex] = k;
						lastChangeIndex++;
						break;
					}
				}

				// Apply this guess onto the sentence
				applyCipher(words, cipher);

				// The previously applied cipher does not result in any matches. Go back to that one and try again.
				// Move back 1 index, but skip 1 this time.
				if (k == 26){
					skip[wordOffsets[i]+j] = 0;
					skip[wordOffsets[i]+j-1]++;
					// Remove the last change
					lastChangeIndex--;
					int lastChange = changes[lastChangeIndex];
					changes[lastChangeIndex] = 0;
					cipher[lastChange] = 0;
					j -= 2;
				}
				// Check if the words exist
				else if (!formatExists(words)){
					cipher[k] = 0;
					skip[wordOffsets[i]+j]++;
					j--; // repeat the current loop to apply a new cipher
				}
			}
		}
	}

	int formatExists(format){
		return format in dictionary;
	}

	int formatsExists(words){
		foreach (format in words){
			if (!formarExists(format))
				return 0;
		}
		return 1;
	}

	void applyCipher(words, cipher){
		foreach (word in words){
			for (int i = 0; i < word.length; i++){
				char c = word[i];
				if (c in cipher){
					word[i] = cipher[i]; // or something along these lines
				}
			}
		}
	}

	 */

	// Create the dictionary
	char dictionary[DICTIONARYSIZE][MAXWORDLENGTH];
	createDictionary((char*) dictionary, fileName, DICTIONARYSIZE, MAXWORDLENGTH);

	// Apply the hints
	substituteHints((char*)hints, text, hintCount, HINTLENGTH);

	// Split the text into words
	char* words = (char*)malloc(1);
	int wordCount = 0;
	char* word = strtok(text, " ");
	while (word != NULL){
		words = (char*)realloc(words, (wordCount+1)*MAXWORDLENGTH*sizeof(char));
		strncpy(words + wordCount*MAXWORDLENGTH, word, MAXWORDLENGTH);
		wordCount++;
		word = strtok(NULL, " ");
	}

	char cipher[26] = {0};
	for (i = 0; i < hintCount; i++){
		cipher[hints[i][0] - 'a'] = hints[i][1];
	}
	int skips[wordCount*MAXWORDLENGTH];
	memset(skips, 0, wordCount*MAXWORDLENGTH*sizeof(int));
	int wordOffsets[wordCount*MAXWORDLENGTH];
	memset(wordOffsets, 0, wordCount*MAXWORDLENGTH*sizeof(int));
	char changes[26] = {0};
	int lastChangeIndex = 0; // keep track of the last change made to the cipher
	for (i = 0; i < wordCount; i++){
		wordOffsets[i+1] = (int)strlen(words + i*MAXWORDLENGTH) + wordOffsets[i];
	}
	char* wordsCopy = malloc( sizeof(char)*wordCount*MAXWORDLENGTH );
	for (i = 0; i < wordCount; i++){
		int j;
		int len = strlen(words + i*MAXWORDLENGTH);
		for (j = 0; j < len; j++){
			char c = *(words + i*MAXWORDLENGTH + j);
			if (isUpper(c)){
				// Get first available cipher
				int k;
				for (k = skips[wordOffsets[i]+j]; k < 26; k++){
					if (!cipher[k]){
						cipher[k] = c;
						changes[lastChangeIndex] = k;
						lastChangeIndex++;
						break;
					}
				}

				// Apply this guess to the words
				applyCipher(words, cipher, wordsCopy, wordCount, MAXWORDLENGTH);

				// The previously applied cipher does not result in any matches. Go back to that one and try again.
				// Move back 1 index, but skip 1 this time.
				if (k == 26){
					skips[wordOffsets[i]+j] = 0;
					skips[wordOffsets[i]+j-1]++;
					// Remove the last change
					lastChangeIndex--;
					int lastChange = changes[lastChangeIndex];
					changes[lastChangeIndex] = 0;
					cipher[lastChange] = 0;
					j -= 2;
				}
				// Check if the words exist
				else if (!formatsExist( (char*)dictionary, wordsCopy, DICTIONARYSIZE, MAXWORDLENGTH, wordCount )){
					cipher[k] = 0;
					skips[wordOffsets[i]+j]++;
					j--; // repeat the current loop to apply a new cipher
				}
			}
		}
	}

	free(wordsCopy);
	free(words);

	/* Print stuff */
	for (i = 0; i < wordCount; i++){
		printf("word: %s\n", words + i*MAXWORDLENGTH);
	}
	for (i = 0; i < 26; i++){
		printf("(%c,%c) \n", i+'a', cipher[i]);
	}
	char testFormat[] = "###bie";
	printf("format (%s) exists: %d\n", testFormat, formatExists((char*)dictionary, testFormat, DICTIONARYSIZE, MAXWORDLENGTH));
	printf("formats exist for words: %d\n", formatsExist( (char*)dictionary, words, DICTIONARYSIZE, MAXWORDLENGTH, wordCount ));

	return 0;
}