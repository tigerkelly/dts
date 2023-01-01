/*
 * asciitritree.h
 *
 *  Created on: Dec 28, 2022
 *      Author: Kelly Wiles
 */

#ifndef TRIETREE_H_
#define TRIETREE_H_

#include <sys/types.h>
#include "dts.h"
#include "miscutils.h"

#ifndef TRIE_NULL
#define TRIE_NULL ((void *) 0)
#endif

// The *next array on a 64bit system is 760 bytes in size,
// cause on 64bit systems pointers are 8 bytes long.
typedef struct _trieTreeNode {
	DtsData_t *data;
	unsigned int useCount;
	unsigned short inUse;
	struct _trieTreeNode *next[95];
} TrieTreeNode;

typedef struct _trieTree {
	TrieTreeNode *root;
} TrieTree;

TrieTree *ttInit();
TrieTreeNode *ttFindEnd(TrieTree *trie, char *ip);
int ttInsert(TrieTree *trie, char *ip, DtsData_t *value);
void *ttLookup(TrieTree *trie, char *ip);
int ttNumEntries(TrieTree *trie);



#endif /* INCS_HEXTRIETREE_H_ */
