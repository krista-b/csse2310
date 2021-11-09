#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* The default path for dictionary if no file is provided in the comand line.
 */
#define STD_DICT_PATH "/usr/share/dict/words"

/* The error codes for the program
 */
typedef enum ErrorCode {
    E_OK,
    E_PARAMS,
    E_INVALID_FILE,
    E_MORE_LETTERS,
    E_INVALID_LETTERS,
    E_NO_MATCHES
} ErrorCode;

/* Prints the associated error message to the given error code to stderr.
 * Returns the exit code.
 */
int error_msg(ErrorCode code) {
    struct ErrorPair {
        int code;
        char* msg;
    };

    const struct ErrorPair errors[] = {
            {0, ""},
            {1, "Usage: unjumble [-alpha|-len|-longest] [-include letter]"
            " letters [dictionary]\n"},
            {2, ""},
            {3, "unjumble: must supply at least three letters\n"},
            {4, "unjumble: can only unjumble alphabetic characters\n"},
            {10, ""}
            };
    fprintf(stderr, "%s", errors[code].msg);
    return errors[code].code;
}

/* Checks if the given filepath can be opened.
 * Returns false if an error ocurrs and true otherwise. 
 */
bool file_accessible(char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        return false;
    }
    fclose(file);
    return true;
}

/* Represents the given command line arguments.
 */
typedef struct Args {
    char* sort;
    char* include;
    char* letters;
    char* dict;
} Args;

/* Checks if the given dictionary filepath is accessible.
 * Returns the error code for an invalid file if not and the code for ok 
 * otherwise.
 */
ErrorCode check_dict_file(char* filepath) {
    if (strcmp(filepath, STD_DICT_PATH) && !file_accessible(filepath)) {
        return E_INVALID_FILE;
    }
    return E_OK;
}

/* Checks if the correct number of letters were provided in the command line.
 * Returns false if there are less than 3 and true otherwise.
 */
bool check_letters_length(char* letters) {
    if (strlen(letters) < 3) {
        return false;
    }
    return true;
}

/* Checks if all the letters provided in the command line are alphabetic 
 * characters.
 * Returns false if not and true otherwise. 
 */
bool check_letters(char* letters) {
    for (int i = 0; i < strlen(letters); i++) {
        if (!isalpha(letters[i])) {
            return false;
        }
    }
    return true;
}

/* Checks if a sort argument was provided in the command line.
 * Returns true if one is present and false otherwise. 
 */
bool check_if_sort(char* sort) {
    if (!strcmp(sort, "-alpha") || !strcmp(sort, "-len") || !strcmp(sort, \
            "-longest")) {
        return true;
    }
    return false;
}

/* Checks if an include argument was provided in the command line.
 * Returns true if one is present and false otherwise. 
 */
bool check_if_include(char* include) {
    if (!strcmp(include, "-include")) {
        return true;
    }
    return false;
}

/* Checks if the include character provided in the command line is an 
 * alphabetic character and that there is only 1.
 * Returns true if both condictions are met and false otherwise.
 */
bool check_include(char* include) {
    if ((strlen(include) == 1)) {
        if (isalpha(include[0])) {
            return true;
        }
    } 
    return false;
}

/* Parses all arguments given to the command line and checks for any invalid 
 * inputs. If there are no invalid inputs then the arguments are stored in an 
 * args struct to be passed back to main. 
 * Returns the error code of which error is caught first or ok otherwise. 
 */
ErrorCode parse_args(int argc, char** argv, char** sort, char** include, \
        char** letters, char** dict) {
    char* std = STD_DICT_PATH;
    int sortCount = 0, incluCount = 0;

    if ((argc < 2) || (argc > 6)) {
        return E_PARAMS;
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (check_if_sort(argv[i]) && sortCount == 0) {
                memcpy(sort, &argv[i], sizeof(char*));
                sortCount++;
            } else if (check_if_include(argv[i]) && incluCount == 0) {
                if (check_include(argv[i + 1])) {
                    memcpy(include, &argv[i + 1], sizeof(char*));
                    incluCount++;
                } else {
                    return E_PARAMS;
                }
            } else {
                return E_PARAMS;
            }
        } else {
            if (incluCount == 0) {
                if (check_letters_length(argv[i])) {
                    if (check_letters(argv[i])) {
                        memcpy(letters, &argv[i], sizeof(char*));
                        if (argv[i + 1] != NULL) {
                            memcpy(dict, &argv[i + 1], sizeof(char*));
                            if (argv[i + 2] != NULL) {
                                return E_PARAMS;
                            }
                        } else {
                            memcpy(dict, &std, strlen(std) + 1);
                        }
                    } else {
                        return E_INVALID_LETTERS;
                    }
                } else {
                    return E_MORE_LETTERS;
                }
            } 
            if (incluCount == 1) {
                incluCount--;
                continue;
            }
            return E_OK;
        }
    } 
    return E_PARAMS;
}

/* Converts every character in a given string to lowercase.
 * Returns the given string in all lowercase. 
 */
char* change_case(char* letters) {
    for (int i = 0; i < strlen(letters); i++) {
        letters[i] = tolower(letters[i]);
    }

    return letters;
}

/* Removes the first occurrence of the given character from the given string.
 */
void remove_char(char* string, char c) {
    int i = 0;
    int len = strlen(string);

    while (i < len && string[i] != c) {
	i++;
    }
    while (i < len) {
  	string[i] = string[i + 1];
        i++;
    }
}

/* Compares two strings alphabetically.
 * Returns the int return value of strcasecamp which the given two strings 
 * are passed to as inputs.
 */
static int alpha_compare(const void* a, const void* b) {  
    return strcasecmp(*(const char**)a, *(const char**)b);
}

/* Compares the two input strings by length.
 * Returns 1 if the first string is longer, -1 if the second string is longer 
 * or the int return value of strcasecamp which the given two strings 
 * are passed to as inputs if they are the same length.
 */
static int len_compare(const void* a, const void* b) {
    const char* stringA = *(const char**)a;
    const char* stringB = *(const char**)b;
    size_t lenA = strlen(stringA);
    size_t lenB = strlen(stringB);
    if (lenA > lenB) {
        return -1;
    }
    if (lenB > lenA) {
        return 1;
    }
    return strcasecmp(stringA, stringB);
}

/* Calls the qsort function accordingly when a sorting method is included in
 * the command line arguments. The given array is sorted either by length 
 * or alphabetically. 
 */
void sort_array(char* sort, char** words, int wordCount) {
    if (!strcmp(sort, "-alpha")) {
        qsort(words, wordCount, sizeof(char*), alpha_compare);
    }
    if (!strcmp(sort, "-len")) {
        qsort(words, wordCount, sizeof(char*), len_compare);
    }
    if (!strcmp(sort, "-longest")) {
        qsort(words, wordCount, sizeof(char*), len_compare);
    }
}

/* The given array is printed to stdout line by line if no include character 
 * is provided. If an include character is provided then only array elements 
 * that include that character are printed. 
 */
void print_words(int count, char* include, char** words) {
    for (int i = 0; i < count; i++) {
        if (include != NULL) {
            if (strchr(words[i], include[0]) != NULL) {
                printf("%s", words[i]);
            }
        } else {
            printf("%s", words[i]);
        }
        free(words[i]);
    }
}

/* Reads the dictionary file line by line and checks if the word can be made 
 * with the provided letters. The list of matched words is then passed to the
 * sort_array function (if a sort method is provided) and then that sorted 
 * array is passed to the print_words function.
 *
 * Returns the error code of which error occurs first and ok if no error 
 * occurs. 
 */
ErrorCode check_match(Args args) {
    FILE* file = fopen(args.dict, "r");
    char line[50];
    char* lowerLetters = change_case(args.letters);
    char toSearch[strlen(lowerLetters)];
    int wordCount = 0, j = 0;
    char** words = malloc(100000 * sizeof(char *));

    while (fgets(line, sizeof(line), file)) {
        strcpy(toSearch, lowerLetters);
        int lettersPresent = true;
        if (strlen(line) > 3) {
            for (int i = 0; i < strlen(line) - 1; i++) {
                if (strchr(toSearch, tolower(line[i])) == NULL) {
                    lettersPresent = false;
                } else {
                    remove_char(toSearch, tolower(line[i]));
                }
            }
            if (lettersPresent) {
                words[wordCount] = (char *)malloc(strlen(line) + 1);
                strcpy(words[wordCount], line);
                wordCount++;
            } 
        }
    }
    if (args.sort != NULL) {
        sort_array(args.sort, words, wordCount);
    }
    char** wordsSorted = malloc(100000 * sizeof(char *));
    for (int i = 0; i < wordCount; i++) {
        if (!strcmp(args.sort, "-longest")) {
            if (strlen(words[i]) == strlen(words[0])) {
                wordsSorted[j] = (char *)malloc(strlen(words[i]) + 1);
                strcpy(wordsSorted[j], words[i]);
                j++;
            }
        } else {
            wordsSorted[j] = (char *)malloc(strlen(words[i]) + 1);
            strcpy(wordsSorted[j], words[i]);
            j++;
        } 
    }
    print_words(j, args.include, wordsSorted);
    free(words);
    free(wordsSorted);
    if (wordCount == 0) {
        return E_NO_MATCHES;
    }
    return E_OK;
}

int main(int argc, char** argv) {
    ErrorCode error;

    Args args;

    error = parse_args(argc, argv, &args.sort, &args.include, &args.letters, \
            &args.dict);
    if (error != E_OK) {
        return error_msg(error);
    }

    error = check_dict_file(args.dict);
    if (error != E_OK) {
        fprintf(stderr, "unjumble: file \"%s\" can not be "
                "opened\n", args.dict);
        return error;
    }

    error = check_match(args);
    if (error != E_OK) {
        return error_msg(error);
    }
    
    return error;
}
