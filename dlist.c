#ifdef __QUECTEL_BCM_AT_CMD__//victor.chen


#ifndef LIST_H
#define LIST_H
#define offset_of(type, member) \
    ((unsigned long)(&(((type*)0)->member)))
 
#define container_of(ptr, type, member) \
    ((type*)((unsigned long)(ptr) - offset_of(type, member)))
/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static   void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a mynew entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static   void __list_add(struct list_head *mynew,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = mynew;
	mynew->next = next;
	mynew->prev = prev;
	prev->next = mynew;
}

/**
 * list_add - add a mynew entry
 * @mynew: mynew entry to be added
 * @head: list head to add it after
 *
 * Insert a mynew entry after the specified head.
 * This is good for implementing stacks.
 */
static   void list_add(struct list_head *mynew, struct list_head *head)
{
	__list_add(mynew, head, head->next);
}


/**
 * list_add_tail - add a mynew entry
 * @mynew: mynew entry to be added
 * @head: list head to add it before
 *
 * Insert a mynew entry before the specified head.
 * This is useful for implementing queues.
 */
static   void list_add_tail(struct list_head *mynew, struct list_head *head)
{
	__list_add(mynew, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static   void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static   void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

/**
 * list_replace - replace old entry by mynew one
 * @old : the element to be replaced
 * @mynew : the mynew element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static   void list_replace(struct list_head *old,
				struct list_head *mynew)
{
	mynew->next = old->next;
	mynew->next->prev = mynew;
	mynew->prev = old->prev;
	mynew->prev->next = mynew;
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static   void list_move(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static   void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static   int list_is_last(const struct list_head *list,
				const struct list_head *head)
{
	return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static   int list_empty(const struct list_head *head)
{
	return head->next == head;
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static   int list_is_singular(const struct list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}



/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)
static   void prefetch(const void *x) {;}

#define list_for_each_entry(pos,pos_type, head, member)				\
	for (pos = list_entry((head)->next, pos_type, member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, pos_type, member))

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.

#define list_for_each_entry(pos, head, member,type)				\
	for (pos = list_entry((head)->next, type, member);	\
	     prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, type, member))
 */

/*
#define list_for_each_entry(pos, head, member,type) \ 
for (pos = list_entry((head)-> next,type,member),	\ 
prefetch(pos-> member.next);	 \ 
&pos-> member != (head); \ 
pos = list_entry(pos-> member.next, type, member),	\ 
prefetch(pos-> member.next)) 
*/



#endif



#endif
