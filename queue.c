#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
 * Allocate and initialize an element with given string and
 * next element.
 * Return NULL if the string was NULL or an allocation failed.
 * Return a pointer to a initialized element if success.
 */
inline static list_ele_t *new_element(char *s, list_ele_t *next)
{
    list_ele_t *new_ele = NULL;
    if (!s || !(new_ele = malloc(sizeof(list_ele_t))) ||
        !(new_ele->value = strdup(s))) {
        free(new_ele);
        return NULL;
    }
    new_ele->next = next;
    return new_ele;
}

/*
 * Deallocate a pointer to a list element.
 * Assume the input is not NULL.
 * Return the next element
 */
inline static list_ele_t *del_element(list_ele_t *e)
{
    list_ele_t *next = e->next;
    free(e->value);
    free(e);
    return next;
}

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));
    if (q) {
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
    }
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    if (q) {
        while (q->head)
            q->head = del_element(q->head);
        free(q);
    }
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    list_ele_t *newh = NULL;
    if (!q || !(newh = new_element(s, q->head)))
        return false;
    if (0 == q->size)
        q->tail = newh;
    q->head = newh;
    q->size++;
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    if (!q)
        return false;
    if (0 == q->size)
        return q_insert_head(q, s);
    if (!(q->tail->next = new_element(s, NULL)))
        return false;
    q->tail = q->tail->next;
    q->size++;
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return true if successful.
 * Return false if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 * The space used by the list element and the string should be freed.
 */
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    if (0 == q_size(q))
        return false;
    if (sp) {
        strncpy(sp, q->head->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    q->head = del_element(q->head);
    q->size--;
    if (1 >= q->size)
        q->tail = q->head;
    return true;
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    return (q) ? q->size : 0;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    if (q_size(q) <= 1)
        return;
    list_ele_t *pre = NULL;
    list_ele_t *now = q->head;
    list_ele_t *nxt = now->next;
    while (nxt) {
        now->next = pre;
        pre = now;
        now = nxt;
        nxt = nxt->next;
    }
    now->next = pre;
    q->tail = q->head;
    q->head = now;
}

/*
 * List element compare function.
 */
inline static int cmp_elem(const list_ele_t *const a, const list_ele_t *const b)
{
    return strcmp(a->value, b->value);
}

/*
 * Continuous range
 */
typedef struct {
    list_ele_t *head;
    list_ele_t *tail;
} range_t;

/*
 * Find the middle "slit" of a linked list
 */
inline static range_t split(list_ele_t *const head, list_ele_t *const tail)
{
    list_ele_t *slow = head;
    for (list_ele_t *fast = head->next; fast != tail && fast != tail->next;) {
        fast = fast->next->next;
        slow = slow->next;
    }
    return (range_t){.head = slow, .tail = slow->next};
}

/*
 * Merge two closed-ranged linked lists, r1 and r2.
 * Output the merged range
 */
inline static range_t merge(range_t r1, range_t r2)
{
    list_ele_t *merged = NULL, **tail = NULL;
    for (tail = &merged; r1.head && r2.head; tail = &((*tail)->next)) {
        list_ele_t **source =
            (cmp_elem(r1.head, r2.head) < 0) ? (&r1.head) : (&r2.head);
        *tail = *source;
        *source = (*source)->next;
    }
    range_t *result = r1.head ? &r1 : &r2;
    *tail = result->head;
    result->head = merged;
    return *result;
}

/*
 * Use merge sort to sort the linked list.
 */
static range_t merge_sort(list_ele_t *head, list_ele_t *tail)
{
    if (head == tail)
        return (range_t){.head = head, .tail = tail};
    range_t slit = split(head, tail);
    slit.head->next = tail->next = NULL;
    return merge(merge_sort(head, slit.head), merge_sort(slit.tail, tail));
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(queue_t *q)
{
    if (q_size(q) <= 1)
        return;
    range_t range = merge_sort(q->head, q->tail);
    q->head = range.head;
    q->tail = range.tail;
}
