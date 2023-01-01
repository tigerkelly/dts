
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "strutils.h"
#include "dts.h"
#include "trietree.h"

TrieTree *tt = NULL;
char *_packet = NULL;

DtsTypes_t dtsType(char *type);
DtsActions_t fdtsAction(char *action);
DtsData_t *dtsProcess(char *name, DtsTypes_t type, DtsActions_t action, char *value);
char *dtsFindData(char *name);
DtsActions_t dtsAction(char *action);
char *buildPacket(DtsData_t *dd, bool clearFlag);

int dtsInit() {

	tt = ttInit();

	return 0;
}

char *dtsParse(char *dtsData, int dataSize) {

	if (dtsData == NULL) {
		printf("dtsData is null.\n");
		return NULL;
	}

	printf("dtsData: %s, %d\n", dtsData, dataSize);

	char *packet = NULL;
	bool clearFlag = true;
	char *n, *t, *a, *v;
	DtsTypes_t type = UNKNOWN_TYPE;
	DtsActions_t action = UNKNOWN_ACTION;
	char *args[128];
	int nargs = parse(dtsData, ";", args, 128);	// split multiple data.

	for (int i = 0; i < nargs; i++) {
		trim(args[i]);

		// printf("++ Parsing %s\n", args[i]);

		n = a = t = v = NULL;

		char *args2[5];
		int nargs = qparse(args[i], ": ", args2, 5);
		if (nargs == 4) {
			n = args2[0];
			t = args2[1];
			a = args2[2];
			v = args2[3];		// qparse() does remove the single quote marks from string.
		} else if (nargs == 3) {
			n = args2[0];
			t = args2[1];
			a = args2[2];
		}

		type = dtsType(t);
		action = dtsAction(a);

		// printf("n=%s, t=%d, a=%d, v=%s\n", n, type, action, v);

		DtsData_t *dd = dtsProcess(n, type, action, v);

		// printf("dd->type %d\n", dd->type);

		if (dd != NULL && dd->type != UNKNOWN_TYPE) {
			packet = buildPacket(dd, clearFlag);
			clearFlag = false;

			// Free all allocated stuff.
			if (dd->name != NULL)
				free(dd->name);
			if (dd->type == STRING_TYPE && dd->s != NULL)
				free(dd->s);
			free(dd);
		} else {
			printf("---dd is null.\n");
		}
	}

	return packet;
}

char *buildPacket(DtsData_t *dd, bool clearFlag) {

	if (dd == NULL)
		return NULL;

	if (clearFlag == true) {
		if (_packet != NULL)
			free(_packet);
		_packet = NULL;
	}

	int len = strlen(dd->name);
	char t[64];
	char b[2];
	char *s = NULL;
	int idx = 0;

	switch(dd->type) {
		case BOOLEAN_TYPE:
			len += 2;
			b[0] = dd->b? 'T':'F';
			break;
		case STRING_TYPE:
			len += strlen(dd->s) + 1;
			s = strdup(dd->s);
			break;
		case NUMBER_TYPE:
			sprintf(t, "%ld", dd->n);
			len += strlen(t) + 1;
			break;
		case DOUBLE_TYPE:
			sprintf(t, "%f", dd->d);
			len += strlen(t) + 1;
			break;
		case UNKNOWN_TYPE:
			break;
	}

	if (_packet == NULL)
		_packet = (char *)calloc(len, 1);
	else {
		_packet = (char *)realloc(_packet, strlen(_packet) + 2 + len);
		strcat(_packet, ";");
		idx += strlen(_packet);
	}

	switch(dd->type) {
		case BOOLEAN_TYPE:
			sprintf(&_packet[idx], "%s:%c %s", dd->name, 'b', b);
			break;
		case STRING_TYPE:
			sprintf(&_packet[idx], "%s:%c '%s'", dd->name, 's', s);
			break;
		case NUMBER_TYPE:
			sprintf(&_packet[idx], "%s:%c %s", dd->name, 'n', t);
			break;
		case DOUBLE_TYPE:
			sprintf(&_packet[idx], "%s:%c %s", dd->name, 'd', t);
			break;
		case UNKNOWN_TYPE:
			break;
	}

	// printf("   _packet: %s\n", _packet);
	
	return _packet;
}

unsigned long stringToLong(char *value) {
	unsigned long n = 0;

	if (value == NULL)
		return n;

	if (value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {	// hex
		n = strtol(value, NULL, 16);
	} else if (value[0] == '0') {	// octal
		n = strtol(value, NULL, 8);
	} else {
		n = strtol(value, NULL, 10);
	}


	return n;
}

double stringToDouble(char *value) {

	return strtod(value, NULL);
}

bool stringToBoolean(char *value) {
	bool b = false;

	switch(value[0]) {
		case 'Y':
		case 'y':
		case 'T':
		case 't':
		case true:
			b = true;
			break;
	}
	return b;
}

DtsData_t *dtsProcess(char *name, DtsTypes_t type, DtsActions_t action, char *value) {
	DtsData_t *dd = NULL;
	// DtsData_t *fd = calloc(sizeof(DtsData_t), 1);
	DtsData_t *nv = NULL;
	bool b = false;
	unsigned long n = 0;
	double d = 0.0;

#if(0)
	fd->name = strdup(name);
	fd->type = type;
	if (value != NULL) {
		switch(type) {
			case UNKNOWN_TYPE:
				break;
			case NUMBER_TYPE:
				fd->n = stringToLong(value);
				// printf("     value %s, %ld\n", value, fd->n);
				break;
			case DOUBLE_TYPE:
				fd->d = stringToDouble(value);
				break;
			case STRING_TYPE:
				fd->s = strdup(value);
				break;
			case BOOLEAN_TYPE:
				fd->b = stringToBoolean(value);
				break;
		}
	}
#endif

	switch(action) {
		case GET_ACTION:
			dd = (DtsData_t *)ttLookup(tt, name);
			if (dd != NULL) {
				nv = (DtsData_t *)calloc(sizeof(DtsData_t), 1);
				switch(dd->type) {
					case NUMBER_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						nv->n = dd->n;
						break;
					case DOUBLE_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						nv->d = dd->d;
						break;
					case BOOLEAN_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						nv->b = dd->b;
						break;
					case STRING_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						if (dd->s != NULL) {
							nv->s = strdup(dd->s);
						}
						break;
					case UNKNOWN_TYPE:
						printf("GET null.\n");
						return NULL;
				}
			} else {
				printf("ttLookup failed for Get.\n");
			}
			break;
		case SET_ACTION:
			dd = (DtsData_t *)ttLookup(tt, name);
			if (dd != NULL) {
				nv = (DtsData_t *)calloc(sizeof(DtsData_t), 1);
				switch(dd->type) {
					case NUMBER_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						n = stringToLong(value);
						dd->n = n;
						nv->n = n;
						break;
					case DOUBLE_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						d = stringToDouble(value);
						dd->d = d;
						nv->d = d;
						break;
					case BOOLEAN_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						b = stringToBoolean(value);
						dd->b = b;
						nv->b = b;
						break;
					case STRING_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						if (dd->s != NULL) {
							free(dd->s);
							dd->s = NULL;
						}
						if (value != NULL) {
							dd->s = strdup(value);
							nv->s = strdup(value);
						}
						break;
					case UNKNOWN_TYPE:
						printf("SET null.\n");
						return NULL;
				}
			} else {
				nv = (DtsData_t *)calloc(sizeof(DtsData_t), 1);
				nv->name = strdup(name);
				nv->type = type;
				switch(type) {
					case NUMBER_TYPE:
						n = stringToLong(value);
						nv->n = n;
						break;
					case DOUBLE_TYPE:
						d = stringToDouble(value);
						nv->d = d;
						break;
					case BOOLEAN_TYPE:
						b = stringToBoolean(value);
						nv->b = b;
						break;
					case STRING_TYPE:
						if (value != NULL) {
							nv->s = strdup(value);
						}
						break;
					case UNKNOWN_TYPE:
						printf("SET null.\n");
						return NULL;
				}

				ttInsert(tt, name, nv);
				// nv = ttLookup(tt, name);
			}
			break;
		case INCREASE_ACTION:
			dd = (DtsData_t *)ttLookup(tt, name);
			if (dd != NULL) {
				nv = (DtsData_t *)calloc(sizeof(DtsData_t), 1);
				switch(dd->type) {
					case NUMBER_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						n = stringToLong(value);
						dd->n += n;
						nv->n = dd->n + n;
						break;
					case DOUBLE_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						d = stringToDouble(value);
						dd->d += d;
						nv->d = dd->d + d;
						break;
					case BOOLEAN_TYPE:
					case STRING_TYPE:
					case UNKNOWN_TYPE:
						printf("INCREASE null.\n");
						return NULL;
				}
			}
			break;
		case DECREASE_ACTION:
			dd = (DtsData_t *)ttLookup(tt, name);
			if (dd != NULL) {
				nv = (DtsData_t *)calloc(sizeof(DtsData_t), 1);
				switch(dd->type) {
					case NUMBER_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						n = stringToLong(value);
						dd->n -= n;
						nv->n = dd->d - n;
						break;
					case DOUBLE_TYPE:
						nv->name = strdup(name);
						nv->type = type;
						d = stringToDouble(value);
						dd->d -= d;
						nv->d = dd->d - d;
						break;
					case BOOLEAN_TYPE:
					case STRING_TYPE:
					case UNKNOWN_TYPE:
						printf("DECREASE null.\n");
						return NULL;
				}
			}
			break;
		case UNKNOWN_ACTION:
			printf("Actino unknown.\n");
			break;
	}

#if(0)
	if (fd != NULL) {
		if (fd->name != NULL)
			free(fd->name);
		if (fd->type == STRING_TYPE && fd->s != NULL)
			free(fd->s);
		free(fd);
	}
#endif

	return nv;
}

DtsActions_t dtsAction(char *action) {
	DtsActions_t a = UNKNOWN_ACTION;

	switch(action[0]) {
		case 'D':
		case 'd':
			a = DECREASE_ACTION;
			break;
		case 'I':
		case 'i':
			a = INCREASE_ACTION;
			break;
		case 'G':
		case 'g':
			a = GET_ACTION;
			break;
		case 'S':
		case 's':
			a = SET_ACTION;
			break;
	}

	return a;
}

DtsTypes_t dtsType(char *type) {

	DtsTypes_t t = UNKNOWN_TYPE;

	switch(type[0]) {
		case 'B':
		case 'b':
			t = BOOLEAN_TYPE;
			break;
		case 'D':
		case 'd':
			t =  DOUBLE_TYPE;
			break;
		case 'N':
		case 'n':
			t =  NUMBER_TYPE;
			break;
		case 'S':
		case 's':
			t =  STRING_TYPE;
			break;
	}

	return t;
}
