
#ifndef DTS_H
#define DTS_H

#include <stdbool.h>

typedef enum {
	UNKNOWN_TYPE,
	NUMBER_TYPE,
	DOUBLE_TYPE,
	STRING_TYPE,
	BOOLEAN_TYPE
} DtsTypes_t;

typedef enum {
	UNKNOWN_ACTION,
	GET_ACTION,
	SET_ACTION,
	INCREASE_ACTION,
	DECREASE_ACTION
} DtsActions_t;

typedef struct _dtsData_ {
	char *name;				// Must start with a letter a-z or A-Z
	DtsTypes_t type;
	bool b;
	unsigned long n;
	double d;
	char *s;

} DtsData_t;

char *dtsParse(char *dtsData, int dataSize);
int dtsInit();

#endif
