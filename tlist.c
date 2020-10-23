#include "list.h"
#include <stdio.h> // For NULL

	// Static arrays of Nodes and Lists, act as respective pools
	// for all created lists
	Node nodes[LIST_MAX_NUM_NODES];
	List heads[LIST_MAX_NUM_HEADS];

	// My solution to the big "puzzle" of how to
	// create and use a static linked list is to 
	// keep track of the next available index (in nodes or heads)
	// and store these available indexes in two stacks;
	// this way an available node/head is available in O(1).
	// This does require extra space but only sizeof(int) * n
	// for each respective pool

	// Indices for available heads and nodes
	// Stack data structure
	// Arrays begin as being filled from index i to n - 1
	// backwards. So indexes i to n - 1 contain n-1, ..., 1, 0
	// Could have been filled straight-forward, with index matching
	// stored value, but for some reason I did it this way.
	// The way it works is that upon creation
	// of a list, calling List_create, the function gets an index
	// for an available head from av_heads at index head_track,
	// the List stores this given index, the index where it exists in
	// heads[]. Index that was handed over it made equal to -1 (marked
	// as in use) and head_track (index of the next available head) is de-
	// incremented, so that it's ready to return the index of the
	// next available head in heads[]. When a List is freed it sets
	// its List fields to NULL, head_track is incremented and the index
	// the List had is input into av_heads, as the index of this now
	// available head. The above is all done as well for Nodes.
	// The memory may appear "spotted" with NULL items but no
	// searching is ever done
	int av_nodes[LIST_MAX_NUM_NODES];
	int av_heads[LIST_MAX_NUM_HEADS];

	// Iterators for av_heads and av_nodes
	// Each are the index of the next available head/node
	// All indices are available to start, from 0 to LIST_MAX_NUM_*
	int head_track;
	int node_track;

	// Number of heads and nodes in current use
	int head_n = 0;
	int node_n = 0;

	// bool for whether pools have already been initialized
	int initialized = 0;

// Initializes all items in heads, nodes, av_heads, av_nodes, for use
// and sets head_track and node_track. Called only once
void init() {
	head_track = LIST_MAX_NUM_HEADS - 1;
	node_track = LIST_MAX_NUM_NODES - 1;
	for (int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
		(heads[i]).head = NULL;
		(heads[i]).tail = NULL;
		(heads[i]).current = NULL;
		(heads[i]).number = 0;
		(heads[i]).before = 1;
		(heads[i]).after = 1;
		(heads[i]).index = -1;
	}
	for (int i = 0; i < LIST_MAX_NUM_NODES; i++) {
		(nodes[i]).item = NULL;
		(nodes[i]).next = NULL;
		(nodes[i]).prev = NULL;
		(nodes[i]).index = -1;
	}
	for (int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
		av_heads[LIST_MAX_NUM_HEADS - 1 - i] = i;
	}
	for (int i = 0; i < LIST_MAX_NUM_NODES; i++) {
		av_nodes[LIST_MAX_NUM_NODES - 1 - i] = i;
	}
	initialized = 1;
	return;
}

// Returns a pointer to a List stored in heads[]
// If all Lists are in use returns NULL
List* List_create() {
    if (head_n + 1 > LIST_MAX_NUM_HEADS) {
		return NULL;	
	}
	
	// If there are no Lists in use, intialize
	if (head_n <= 0 && initialized == 0) {
		init();
	}

	// Pointing h to the List contained in heads[] at
	// the index stored at the "top" of the av_heads stack
	List* h = heads + av_heads[head_track];
	h->index = av_heads[head_track];
	head_n++;

	// Mark the index that was given out as being
	// in use. Good for clarity and debugging
	av_heads[head_track] = -1;

	// Deincrement head_track so that it is the index
	// of av_heads[] that had the index of the next
	// available List
	head_track--;

    return h;
}

// Returns the number of items in the List
int List_count(List* pList) {
	return pList->number;
}

// Sets current to the head of the List and
// returns that Node's item
void* List_first(List* pList) {
	if (pList->number <= 0) {
		pList->current = NULL;
		return NULL;
	}

	if (pList->after) {
		pList->after = 0;
		pList->before = 0;
	}
	if (pList->before) {
		pList->before = 0;
		pList->after = 0;
	}
	pList->current = pList->head;
	return pList->current->item;
}

// Sets current to the tail of the List and
// returns that Node's item
void* List_last(List* pList) {
	if (pList->number <= 0) {
		pList->current = NULL;
		return NULL;
	}
	if (pList->after) {
		pList->after = 0;
		pList->before = 0;
	}
	if (pList->before) {
		pList->before = 0;
		pList->before = 0;
	}
	pList->current = pList->tail;
	return pList->current->item;
}

// Iterates current one spot in the List
void* List_next(List* pList) {
	if (pList->number <= 0 || pList->after) {
		return NULL;
	}
	if (pList->before) {
		pList->current = pList->head;
		pList->before = 0;
		pList->after = 0;
	}
	if (pList->current == pList->tail) {
		pList->current = pList->current->next;
		pList->after = 1;
		pList->before = 0;
		return NULL;
	} else {
		pList->current = pList->current->next;
		if (pList->current) {
			return pList->current->item;
		}
		else {
			pList->after = 1;
			pList->before = 0;
			return NULL;
		}
	}
}

// Moves current back one node in the List
void* List_prev(List* pList) {
	if (pList->number <= 0 || pList->before) {
		return NULL;
	}
	if (pList->after) {
		pList->current = pList->tail;
		pList->after = 0;
		pList->before = 0;
		return pList->current->item;
	}
	if (pList->current == pList->head) {
		pList->current = pList->current->prev;
		pList->before = 1;
		pList->after = 0;
		return NULL;
	} else {
		pList->current = pList->current->prev;
		pList->before = 0;
		pList->after = 0;
		return pList->current->item;
	}
}

// Returns item at current, if current
// is beyond, before, or otherwise weird
// returns NULL
void* List_curr(List* pList) {
	if (pList->current) {
		return pList->current->item;
	}
	return NULL;
}

// Adds an item to the list right after current.
// Then sets current to that item
// Returns -1 if there was an issue, 0 otherwise
int List_add(List* pList, void* pItem) {
	if (node_n + 1 > LIST_MAX_NUM_NODES) {
		return -1;
	}
	// Makes add point to a Node in nodes[] at 
	// index returned from top of av_nodes
	Node* add = nodes + av_nodes[node_track];
	add->index = av_nodes[node_track];
	av_nodes[node_track] = -1;
	node_track--;
	node_n++;

	add->item = pItem;

	// In case of empty list
	if (pList->number <= 0) {
		pList->head = add;
		pList->tail = add;
		pList->current = add;
		pList->number = 1;
		pList->before = 0;
		pList->after = 0;
	}

	// In case of current being beyond list
	else if (pList->after) {
		pList->tail->next = add;
		add->prev = pList->tail;
		pList->tail = pList->tail->next;
		pList->current = pList->tail;
		pList->number++;
		pList->after = 0;
		pList->before = 0;
	}
	// In case of current being before list
	else if (pList->before) {
		add->next = pList->head;
		pList->head->prev = add;
		pList->head = pList->head->prev;
		pList->current = pList->head;
		pList->number++;
		pList->before = 0;
		pList->after = 0;
	} else {
		add->next = pList->current->next;
		add->prev = pList->current;
		if (pList->current->next) {
			pList->current->next->prev = add;
		}
		pList->current->next = add;
		if (pList->tail == pList->current) {
			pList->tail = add;
		}
		pList->current = add;
		pList->number++;
	}

	return 0;
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert(List* pList, void* pItem) {
	if (node_n + 1 > LIST_MAX_NUM_NODES) {
		return -1;
	}
	// Makes add point to a Node in nodes[] at 
	// index returned from top of av_nodes
	Node* add = nodes + av_nodes[node_track];
	add->index = av_nodes[node_track];
	av_nodes[node_track] = -1;
	node_track--;
	node_n++;

	add->item = pItem;

	// In case of empty list
	if (pList->number <= 0) {
		pList->head = add;
		pList->tail = add;
		pList->current = add;
		pList->number = 1;
	}
	// In case of current being beyond list
	else if (pList->after) {
		pList->tail->next = add;
		add->prev = pList->tail;
		pList->tail = pList->tail->next;
		pList->current = pList->tail;
		pList->number++;
		pList->after = 0;
		pList->before = 0;
	}
	// In case of current being before list
	else if (pList->before) {
		add->next = pList->head;
		pList->head->prev = add;
		pList->head = pList->head->prev;
		pList->current = pList->head;
		pList->number++;
		pList->before = 0;
		pList->after = 0;
	} else {
		pList->current->prev->next = add;
		add->prev = pList->current->prev;
		add->next = pList->current;
		pList->current->prev = add;
		pList->current = add;
		pList->number++;
		pList->before = 0;
		pList->after = 0;
	}
	return 0;
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
	if (node_n + 1 > LIST_MAX_NUM_NODES) {
		return -1;
	}
	// Makes add point to a Node in nodes[] at 
	// index returned from top of av_nodes
	Node* add = nodes + av_nodes[node_track];
	add->index = av_nodes[node_track];
	av_nodes[node_track] = -1;
	node_track--;
	node_n++;

	add->item = pItem;

	// In case of empty list
	if (pList->number <= 0) {
		pList->head = add;
		pList->tail = add;
		pList->current = add;
		pList->number = 1;
		pList->before = 0;
		pList->after = 0;
	}
	else {
		pList->tail->next = add;
		add->prev = pList->tail;
		pList->tail = pList->tail->next;
		pList->current = pList->tail;
		pList->number++;
		pList->before = 0;
		pList->after = 0;
	}

	return 0;

}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
	if (node_n + 1 > LIST_MAX_NUM_NODES) {
		return -1;
	}
	// Makes add point to a Node in nodes[] at 
	// index returned from top of av_nodes
	Node* add = nodes + av_nodes[node_track];
	add->index = av_nodes[node_track];
	av_nodes[node_track] = -1;
	node_track--;
	node_n++;

	add->item = pItem;

	// In case of empty list
	if (pList->number <= 0) {
		pList->head = add;
		pList->tail = add;
		pList->current = add;
		pList->number = 1;
		pList->before = 0;
		pList->after = 0;
	}
	else {
		add->next = pList->head;
		pList->head->prev = add;
		pList->head = pList->head->prev;
		pList->current = pList->head;
		pList->number++;
		pList->before = 0;
		pList->after = 0;
	}

	return 0;

}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
	// If current is not within the list or it has no items
	// or there isn't any possible nodes to free returns NULL
	if (pList->before || pList->after || pList->number <= 0 || node_n <= 0) {
		return NULL;
	}

	// thing that will be returned
	void* thing = pList->current->item;

	// Checking whether there are nodes on either side
	// of item to be removed and connecting them if they are there
	// Also checking if current is head or tail in which case
	// head or tail need to be reassigned
	if (pList->current->prev) {
		pList->current->prev->next = pList->current->next;
	}
	if (pList->current->next) {
		pList->current->next->prev = pList->current->prev;
	}
	if (pList->current == pList->head) {
		pList->head = pList->current->next;
	}
	if (pList->current == pList->tail) {
		pList->tail = pList->current->prev;
		pList->after = 1;
	}

	// Node's index in nodes[] is stored
	node_track++;
	node_n--;
	av_nodes[node_track] = pList->current->index;

	Node* temp = pList->current;
	pList->current = pList->current->next;
	if (pList->number - 1 <= 0) {
		pList->after = 1;
		pList->before = 1;
	}

	pList->number--;

	temp->item = NULL;
	temp->prev = NULL;
	temp->next = NULL;
	temp->index = -1;

	return thing;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2) {
	if (pList1->number <= 0 || pList2->number <= 0) {
		return;
	}

	pList1->tail->next = pList2->head;
	pList2->head->prev = pList1->tail;
	pList1->tail = pList2->tail;
	pList1->number = pList1->number + pList2->number;

	// Making list2 a head ready to be used
	head_track++;

	av_heads[head_track] = pList2->index;

	pList2->head = NULL;
	pList2->tail = NULL;
	pList2->current = NULL;
	pList2->number = 0;
	pList2->after = 1;
	pList2->before = 1;
	pList2->index = -1;

	head_n--;
	
	return;

}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
void List_free(List* pList, FREE_FN pItemFreeFn) {
	if (pList->number <= 0 || head_n <= 0 || 
			head_track >= LIST_MAX_NUM_HEADS - 1) {
		return;
	}

	pList->current = pList->tail;
	pList->after = 0;
	pList->before = 0;
	
	Node* rem;

	while(pList->number > 0) {
		rem = pList->current;
		
		if (pItemFreeFn) {
			(*pItemFreeFn)(rem->item);
		}
		List_remove(pList);
		if (pList->after) {
			pList->current = pList->tail;
			pList->after = 0;
			pList->before = 0;
		}
		else {
			pList->current = pList->tail;
		}
	}

	head_track++;
	av_heads[head_track] = pList->index;
	head_n--;

	pList->head = NULL;
	pList->tail = NULL;
	pList->current = NULL;
	pList->number = 0;
	pList->before = 1;
	pList->after = 1;
	pList->index = -1;

	return;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
	if (pList->number <= 0) {
		return NULL;
	}
	if (pList->after) {
		pList->after = 0;
		pList->before = 0;
	}
	if (pList->before) {
		pList->before = 0;
		pList->after = 0;
	}
	pList->current = pList->tail;

	void* thing = List_remove(pList);
	
	// current now equals the new tail
	if (pList->number <= 0) {
		pList->current = NULL;
		pList->after = 1;
		pList->before = 1;
	}
	else {
		pList->current = pList->tail;
		pList->after = 0;
		pList->before = 0;
	}
	return thing;
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
	if (pList->number <= 0) {
		return NULL;
	}
	if (pList->after) {
		pList->current = pList->head;
		pList->after = 0;
		pList->before = 0;
	}
	else if (pList->before) {
		pList->current = pList->head;
		pList->before = 0;
		pList->after = 0;
	}
	while (pList->current != NULL) {
		if ((*pComparator)(pList->current->item, pComparisonArg)) {
			return pList->current->item;
		}
		pList->current = pList->current->next;
	}
	pList->after = 1;
	pList->before = 0;
	return NULL;
}

