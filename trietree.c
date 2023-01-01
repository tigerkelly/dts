/*
 * trietree.c
 *
 * Non-compressed Trie Tree for printable ascii storage and lookup.
 *
 * To be used to storage info using ascii strings.
 *
 *  Created on: Dec 28, 2022
 *      Author: Kelly Wiles
 */

#include "trietree.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "strutils.h"

int _trieTreeInit = 0;

static inline int _toIdx(char ch) __attribute__((always_inline));

TrieTree *ttInit() {

	TrieTree *ttRoot = (TrieTree *) calloc(1, sizeof(TrieTree));
	if (ttRoot == NULL)
		return NULL;
	ttRoot->root = NULL;

	_trieTreeInit = 1;

	return ttRoot;
}

static inline int _toIdx(char ch) {

	// Only allow printable ASCII characters.
	if (ch > 31 && ch < 127)
		return (int)ch - ' ';
	else {
		fprintf(stderr, "ERROR: Not a printable character.\n");
		return 0;
	}
}

/*
 * Function to find end of tree for a given name.
 *
 *   name = A ascii string
 */
TrieTreeNode *ttFindEnd(TrieTree *trie, char *name) {
	TrieTreeNode *node;
	char *p;

	if (_trieTreeInit == 0) {
		fprintf(stderr, "Must call ttInit() first.\n");
		return NULL;
	}

	// Search down the trie until the end of string is reached

	node = trie->root;

	for (p = name; *p != '\0'; ++p) {

		if (node == NULL) {
			// Not found in the tree. Return.
			return NULL;
		}

		// Jump to the next node
		node = node->next[_toIdx(*p)];
	}

	// This IP is present if the value at the last node is not NULL

	return node;
}

/*
 * Function _ttInsertRollback is private to this file.
 */
static void _ttInsertRollback(TrieTree *trie, char *name) {
	TrieTreeNode *node;
	TrieTreeNode **prev_ptr;
	TrieTreeNode *next_node;
	TrieTreeNode **next_prev_ptr;
	char *p;

	// Follow the chain along.  We know that we will never reach the
	// end of the string because ttInsert never got that far.  As a
	// result, it is not necessary to check for the end of string
	// delimiter (NUL)

	node = trie->root;
	prev_ptr = &trie->root;
	p = name;

	while (node != NULL) {

		/* Find the next node now. We might free this node. */

		next_prev_ptr = &node->next[_toIdx(*p)];
		next_node = *next_prev_ptr;
		++p;

		// Decrease the use count and free the node if it
		// reaches zero.

		AtomicSub(&node->useCount, 1);

		if (node->useCount == 0) {
			free(node);

			if (prev_ptr != NULL) {
				*prev_ptr = NULL;
			}

			next_prev_ptr = NULL;
		}

		/* Update pointers */

		node = next_node;
		prev_ptr = next_prev_ptr;
	}
}

/*
 * Function ttInsert is used to insert data into trie tree.
 */
int ttInsert(TrieTree *trie, char *name, DtsData_t *value) {
	TrieTreeNode **rover;
	TrieTreeNode *node;
	char *p;

	if (_trieTreeInit == 0) {
		fprintf(stderr, "Must call ttInit() first.\n");
		return 0;
	}

	/* Cannot insert NULL values */

	if (value == TRIE_NULL) {
		return 0;
	}

	// Search down the trie until we reach the end of string,
	// creating nodes as necessary

	rover = &trie->root;

	p = name;

	TrieTreeNode *tmp = NULL;

	for (;;) {

		node = *rover;

		if (tmp == NULL) {
			// tmp will be freed if it is unused at end of loop.
			tmp = (TrieTreeNode *) calloc(1, sizeof(TrieTreeNode));
			if (tmp != NULL)
				tmp->inUse = 1;
		}

		if (tmp == NULL) {
			// Allocation failed.  Go back and undo
			// what we have done so far.
			_ttInsertRollback(trie, name);

			return 0;
		}

		TrieTreeNode *expect = NULL;

		// Trying to avoid locks here.
		if (AtomicExchange(&node, &expect, &tmp) == 1) {
			*rover = tmp;
			tmp = NULL;		// Set tmp so another will be allocated.
		} else {
			// Another thread beat us in adding node.
			// Do not free tmp here.
		}

		// Increase the node useCount
		AtomicAdd(&node->useCount, 1);

		// Reached the end of string?  If so, we're finished.
		if (*p == '\0') {
			DtsData_t *td = (DtsData_t *)calloc(1, sizeof(DtsData_t));

			DtsData_t *expData = NULL;
			if (AtomicExchange(&node->data, &expData, &td) == 0)
				free(td);	// node->data already allocated

			// Set the data at the node we have reached
			node->data->name = strdup(value->name);
			node->data->type = value->type;
			node->data->b = value->b;
			node->data->n = value->n;
			node->data->d = value->d;
			if (value->type == STRING_TYPE && value->s != NULL)
				node->data->s = strdup(value->s);

			break;
		}

		// Advance to the next node in the chain
		rover = &node->next[_toIdx(*p)];
		++p;
	}

	if (tmp != NULL)
		free(tmp);

	return 1;
}

void *ttLookup(TrieTree *trie, char *name) {
	TrieTreeNode *node;

	if (_trieTreeInit == 0) {
		fprintf(stderr, "Must call ttInit() first.\n");
		return NULL;
	}

	node = ttFindEnd(trie, name);

	if (node != NULL) {
		return node->data;
	} else {
		return TRIE_NULL;
	}
}

int ttNumEntries(TrieTree *trie) {
	// To find the number of entries, simply look at the use count
	// of the root node.

	if (_trieTreeInit == 0) {
		fprintf(stderr, "Must call ttInit() first.\n");
		return 0;
	}

	if (trie->root == NULL) {
		return 0;
	} else {
		return AtomicGet(&trie->root->useCount);
	}
}
