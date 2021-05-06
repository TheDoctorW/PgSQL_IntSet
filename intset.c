/*
 * src/tutorial/intset.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"

#include "fmgr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>
#include <stdbool.h>

PG_MODULE_MAGIC;

typedef struct IntSet {
    int32 length;
    int data[FLEXIBLE_ARRAY_MEMBER];
} IntSet;


/*****************************************************************************
 * Helper functions
 *****************************************************************************/
void swap(int* xp, int* yp);
int removeChar(char *input, char garbage);
int checkVaild(char *input, int counts);
int *stringToArray(char* input, int len);
void selectionSort(int arr[], int n);
int removeDuplicate(int arr[], int n);
int arrayLength(char *input);
bool supersetIntSet(IntSet *a, IntSet *b);
bool equalIntSet(IntSet *a, IntSet *b);


void swap(int* xp, int* yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

int removeChar(char *input, char garbage) {
    char *src, *output;
    int count = strlen(input);
    for (src = output = input; *src != '\0'; src++) {
        *output = *src;
        if (*output != garbage) {
            output++;
            count--;
        }
    }
    *output = '\0';
    return count;
}

int checkVaild(char *input, int counts) {
    int countLb = 0; 
    int countRb = 0;
    int countCo = 0;
    int countDi = 0;
    int slen = strlen(input);
    for (int i = 0; i < slen; i++) {
        int j = i + 1;
        if (isdigit(input[i]) != 0) {
            countDi++;
        }
        if (isalpha(input[i]) != 0) {
            return -1;
        }
        if (input[i] == '-' || input[i] == '.' ||
            input[i] == '[' || input[i] == ']' ||
            input[i] == '(' || input[i] == ')') {
            return -1;
        }
        if (input[i] == '{') {
            countLb++;
        }
        if (input[i] == '}') {
            countRb++;
        }
        if (input[i] == ',') {
            countCo++;
        }
        if (input[i] == ',' && input[j] == ',') {
            return -1;
        }
    }
    if (countLb != 1 || countRb != 1) {
        return -1;
    }
    if (input[0] != '{' || input[slen - 1] != '}') {
        return -1;
    }
    if (slen > 2) {
        if (isdigit(input[1]) == 0 || isdigit(input[slen - 2]) == 0) {
            return -1;
        }
        if (counts != 0 && countCo == 0 && countDi > 1) {
            return -1;
        }
    }
    return 0;
}

int *stringToArray(char* input, int len) {
    int *output;
    int i = 0;
    int countl, countr;
	char *num;
    output = palloc(len * sizeof(int));
    countl = removeChar(input, '{');
    countr = removeChar(input, '}');
    countl++;
    countr++;
	num = strtok(input, ",");
    while (num != NULL) {
        output[i] = atoi(num);
        i++;
        num = strtok(NULL, ",");
    }
    return output;
}

void selectionSort(int arr[], int n) {
    int i, j, min_idx;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        swap(&arr[min_idx], &arr[i]);
    }
}

int removeDuplicate(int arr[], int n) {
	int *temp = arr;
	int j = 0;
    int i;
    if (n == 0 || n == 1) {
        return n;
    }
    for (i = 0; i < n - 1; i++) {
        if (arr[i] != arr[i + 1]) {
            temp[j++] = arr[i];
        }
    }
    temp[j++] = arr[n - 1];
    for (i = 0; i < j; i++) {
        arr[i] = temp[i];
    }
    return j;
}

int arrayLength(char *input) {
    int len = 1;
    for (char *tmp = input; *tmp != '\0'; tmp++) {
        if (*tmp == ',') {
            len++;
        }
    }
    if (len == 1 && strlen(input) == 2) {
        len--;
    }
    return len;
}

bool supersetIntSet(IntSet *a, IntSet *b) {
	int check = 0;
    int alen = ((a->length) / 16) - 1;
    int blen = ((b->length) / 16) - 1;
    if (alen < blen) {
        return false;
    }
    for (int i = 0; i < blen; i++) {
        for (int j = 0; j < alen; j++) {
            if (a->data[j] == b->data[i]) {
                check++;
            }
        }
    }
    if (check != blen) {
        return false;
    }
    return true;
}

bool equalIntSet(IntSet *a, IntSet *b) {
    int alen = ((a->length) / 16) - 1;
    int blen = ((b->length) / 16) - 1;
    if (alen != blen) {
        return false;
    }
    for (int i = 0; i < alen; i++) {
        if (a->data[i] != b->data[i]) {
            return false;
        }
    }
    return true;
}


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intset_in);

Datum
intset_in(PG_FUNCTION_ARGS) {
	char *str = PG_GETARG_CSTRING(0);
	IntSet *output; 
    int check, counts, alen;
    int *array;
    
    counts = removeChar(str, ' ');
	check = checkVaild(str, counts);
	if (check != 0) {
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				errmsg("invalid input syntax for intset: \"%s\"", str)));
	}

    alen = arrayLength(str);
    array = stringToArray(str, alen);
    selectionSort(array, alen);
    alen = removeDuplicate(array, alen);
    output = (IntSet *) palloc(VARHDRSZ + alen * sizeof(int));
    SET_VARSIZE(output, VARHDRSZ + alen * sizeof(int));
    memcpy(output->data, array, alen * sizeof(int));
    pfree(array);
	PG_RETURN_POINTER(output);
}

PG_FUNCTION_INFO_V1(intset_out);

Datum
intset_out(PG_FUNCTION_ARGS) {
	IntSet *set = (IntSet *) PG_GETARG_POINTER(0);
	char *result = "{";
    int i = 1;
    int len = ((set->length) / 16) - 1;
    if (len == 0) {
        result = "{}";
        PG_RETURN_CSTRING(result);
    }
    result = psprintf("%s%d", result, set->data[0]);
    while (i < len) {
        result = psprintf("%s,%d", result, set->data[i]);
        i++;
    }
    result = psprintf("%s}", result);
	PG_RETURN_CSTRING(result);
}


/*****************************************************************************
 * New Operators
 *
 * A practical IntSet datatype would provide much more than this, of course.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intset_check_contain);

Datum
intset_check_contain(PG_FUNCTION_ARGS) {
	int n = PG_GETARG_INT32(0);
	IntSet *set = (IntSet *) PG_GETARG_POINTER(1);
    int len = ((set->length) / 16) - 1;
	for (int i = 0; i < len; i++) {
        if (set->data[i] == n) {
            PG_RETURN_BOOL(1);
        }
    }
    PG_RETURN_BOOL(0);
}


PG_FUNCTION_INFO_V1(intset_length);

Datum
intset_length(PG_FUNCTION_ARGS) {
	IntSet *set = (IntSet *) PG_GETARG_POINTER(0);
    int len = ((set->length) / 16) - 1;
	PG_RETURN_INT32(len);
}


PG_FUNCTION_INFO_V1(intset_superset);

Datum
intset_superset(PG_FUNCTION_ARGS) {
	IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
	IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
	bool check = supersetIntSet(a, b);
	if (check == true) {
		PG_RETURN_BOOL(1);
	}
    PG_RETURN_BOOL(0);
}


PG_FUNCTION_INFO_V1(intset_subset);

Datum
intset_subset(PG_FUNCTION_ARGS) {
	IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
	IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
	bool check = supersetIntSet(b, a);
	if (check == true) {
		PG_RETURN_BOOL(1);
	}
    PG_RETURN_BOOL(0);	
}


PG_FUNCTION_INFO_V1(intset_equal);

Datum
intset_equal(PG_FUNCTION_ARGS) {
	IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
	IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
	bool check = equalIntSet(a, b);
	if (check) {
		PG_RETURN_BOOL(1);
	}
    PG_RETURN_BOOL(0);
}


PG_FUNCTION_INFO_V1(intset_not_equal);

Datum
intset_not_equal(PG_FUNCTION_ARGS) {
	IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
	IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
	bool check = !equalIntSet(a, b);
	if (check) {
		PG_RETURN_BOOL(1);
	}
    PG_RETURN_BOOL(0);	
}


PG_FUNCTION_INFO_V1(intset_union);

Datum
intset_union(PG_FUNCTION_ARGS) {
    IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
    IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
    int alen = ((a->length) / 16) - 1;
    int blen = ((b->length) / 16) - 1;
    int i = 0;
    int j = 0;
    int *array = (int *) palloc((alen + blen) * sizeof(int));
    int count = 0;
    IntSet *output;
    while (i < alen && j < blen) {
        if (a->data[i] < b->data[j]) {
            array[count] = a->data[i];
            i++;
        } else if (a->data[i] > b->data[j]) {
            array[count] = b->data[j];
            j++;
        } else {
            array[count] = b->data[j];
            i++;
            j++;
        }
        count++;
    }
    while (i < alen) {
        array[count] = a->data[i];
        i++;
        count++;
    }
    while (j < blen) {
        array[count] = b->data[j];
        j++;
        count++;
    }
    output = (IntSet *) palloc(VARHDRSZ + count * sizeof(int));
    SET_VARSIZE(output, VARHDRSZ + count * sizeof(int));
    memcpy(output->data, array, count * sizeof(int));
    pfree(array);
    PG_RETURN_POINTER(output);
}


PG_FUNCTION_INFO_V1(intset_intersection);

Datum
intset_intersection(PG_FUNCTION_ARGS) {
    IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
    IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
    int alen = ((a->length) / 16) - 1;
    int blen = ((b->length) / 16) - 1;
    int i = 0;
    int j = 0;
    int nlen;
    int *array;
    int count = 0;
    IntSet *output;	
    if (alen > blen) {
        nlen = alen;
    } else {
        nlen = blen;
    }
    array = (int *) palloc(nlen * sizeof(int));
    while (i < alen && j < blen) {
        if (a->data[i] < b->data[j]) {
            i++;
        } else if (a->data[i] > b->data[j]) {
            j++;
        } else {
            array[count] = b->data[j];
            i++;
            j++;
            count++;
        }
    }
    output = (IntSet *) palloc(VARHDRSZ + count * sizeof(int));
    SET_VARSIZE(output, VARHDRSZ + count * sizeof(int));
    memcpy(output->data, array, count * sizeof(int));
    pfree(array);
    PG_RETURN_POINTER(output);
}


PG_FUNCTION_INFO_V1(intset_disjunction);

Datum
intset_disjunction(PG_FUNCTION_ARGS) {
    IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
    IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
    int alen = ((a->length) / 16) - 1;
    int blen = ((b->length) / 16) - 1;
    int i = 0;
    int j = 0;
    int *array = (int *) palloc((alen + blen) * sizeof(int));
    int count = 0;
    IntSet *output;
    while (i < alen && j < blen) {
        if (a->data[i] < b->data[j]) {
            array[count] = a->data[i];
            i++;
            count++;
        } else if (a->data[i] > b->data[j]) {
            array[count] = b->data[j];
            j++;
            count++;
        } else {
            i++;
            j++;
        }
    }
    while (i < alen) {
        array[count] = a->data[i];
        i++;
        count++;
    }
    while (j < blen) {
        array[count] = b->data[j];
        j++;
        count++;
    }
    output = (IntSet *) palloc(VARHDRSZ + count * sizeof(int));
    SET_VARSIZE(output, VARHDRSZ + count * sizeof(int));
    memcpy(output->data, array, count * sizeof(int));
    pfree(array);
    PG_RETURN_POINTER(output);
}


PG_FUNCTION_INFO_V1(intset_difference);

Datum
intset_difference(PG_FUNCTION_ARGS) {
    IntSet *a = (IntSet *) PG_GETARG_POINTER(0);
    IntSet *b = (IntSet *) PG_GETARG_POINTER(1);
    int alen = ((a->length) / 16) - 1;
    int blen = ((b->length) / 16) - 1;
    int i = 0;
    int j = 0;
    int *array = (int *) palloc(alen * sizeof(int));
    int count = 0;
    IntSet *output;
    while (i < alen && j < blen) {
        if (a->data[i] < b->data[j]) {
            array[count] = a->data[i];
            i++;
            count++;
        } else if (a->data[i] > b->data[j]) {
            j++;
        } else {
            i++;
            j++;
        }
    }
    while (i < alen) {
        array[count] = a->data[i];
        i++;
        count++;
    }
    output = (IntSet *) palloc(VARHDRSZ + count * sizeof(int));
    SET_VARSIZE(output, VARHDRSZ + count * sizeof(int));
    memcpy(output->data, array, count * sizeof(int));
    pfree(array);
    PG_RETURN_POINTER(output);
}