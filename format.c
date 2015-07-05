#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "format.h"

/* Convert a string into a given format for searching. (Basically lazy regex expressions.) */
char* getFormat(char* str){
	int i;
	int len = strlen(str);
	char* format = malloc( (len+1)*sizeof(char) );
	for (i = 0; i < len; i++){
		char c = *(str+i);
		if (isUpper(c)){
			*(format+i) = UNKNOWNLETTER;
		}
		else {
			*(format+i) = c;
		}
	}
	format[len] = '\0';
	return format;
}

/* Check if a string matches a given format */
int strMatchesFormat(char* str, char* format){
	int i;
	int strLen = strlen(str);
	int formatLen = strlen(format);
	if (strLen != formatLen)
		return 0;

	for (i = 0; i < formatLen; i++){
		char c = *(format + i);
		if (c != UNKNOWNLETTER){ // found a known letter
			char c2 = *(str + i);
			if (c != c2){ // check if the known letter macthes up
				return 0;
			}
		}
	}
	return 1;
}

int isUpper(char c){
	return c >= 'A' && c <= 'Z';
}

int isLower(char c){
    return c >= 'a' && c <= 'z';
}