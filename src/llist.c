/*
 * sim-llist.c
 *
 * Link List functions
 *
 * Copyright (c) 2016 Terence Kelleher. All rights reserved.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/llist.h"

void
insert_llist(struct snode *entry, struct snode *list )
{	
	int limit = 100;
	
	// Scan to the first NULL entry (end of list)
	while ( list->next )
	{
		list = list->next;
		if ( limit-- == 0 )
		{
			fprintf(stderr, "insert_llist: Limit exceeded\n" );
			exit ( -1 );
		}
	}
	list->next = entry;
}

struct snode *
get_next_llist(struct snode *entry )
{
	return ( entry->next );
}