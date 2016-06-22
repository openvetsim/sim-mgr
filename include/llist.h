/*
 * llist.h
 *
 * Support for Linked Lists
 *
 * Copyright (C) 2016 Terence Kelleher. All rights reserved.
 *
 * The "node" presents an entry that is added to a structure to support single linked lists
 * 
 */

#ifndef _LLIST_H
#define _LLIST_H

struct snode
{
	struct snode *next;
};

void insert_llist(struct snode *entry, struct snode *list );
struct snode *get_next_llist(struct snode *entry );

#endif // _LLIST_H