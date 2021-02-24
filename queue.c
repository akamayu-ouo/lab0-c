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
static list_ele_t *new_element(char *s, list_ele_t *next)
{
    if (!s)
        return NULL;
    list_ele_t *new_ele = malloc(sizeof(list_ele_t));
    if (!new_ele)
        return NULL;
    size_t len = strlen(s);
    new_ele->value = malloc((len + 1) * sizeof(char));
    if (!new_ele->value) {
        free(new_ele);
        return NULL;
    }
    new_ele->next = next;
    strncpy(new_ele->value, s, len);
    new_ele->value[len] = 0;
    return new_ele;
}

/*
 * Deallocate a pointer to a list element.
 * Assume the input is not NULL.
 * Return the next element
 */
static list_ele_t *del_element(list_ele_t *e)
{
    if (e->value)
        free(e->value);
    list_ele_t *next = e->next;
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
        while (q->head) {
            q->head = del_element(q->head);
        }
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
    if (!q)
        return false;
    list_ele_t *newh = new_element(s, q->head);
    if (!newh)
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
    list_ele_t *newt = new_element(s, NULL);
    if (!newt)
        return false;
    q->tail->next = newt;
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
    list_ele_t *head = q->head;
    if (sp) {
        sp[0] = 0;
        if (head->value) {
            strncpy(sp, head->value, bufsize - 1);
            sp[bufsize - 1] = 0;
        }
    }
    q->head = del_element(head);
    q->size--;
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
static int cmp_elem(const list_ele_t *a, const list_ele_t *b)
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
 * Split a non-empty closed-ranged linked list in half.
 * Output via changing the content of r1 and r2
 */
static void split(const range_t *r, range_t *r1, range_t *r2)
{
    if (r->head == r->tail) {
        *r1 = *r;
        r2->tail = r2->head = NULL;
        return;
    }
    list_ele_t *cut = r->head;
    for (list_ele_t *probe = r->head->next; probe && probe->next;) {
        probe = probe->next->next;
        cut = cut->next;
    }
    r1->head = r->head;
    r1->tail = cut;
    r2->head = cut->next;
    r2->tail = r->tail;
    r1->tail->next = r2->tail->next = NULL;
}

/*
 * Merge two closed-ranged linked lists, r1 and r2.
 * Output via changing the content of rr
 */
static void merge(const range_t *r1, const range_t *r2, range_t *rr)
{
    list_ele_t lead = {.value = NULL, .next = NULL};
    list_ele_t *tail = &lead;
    list_ele_t *h1 = r1->head;
    list_ele_t *h2 = r2->head;
    while (h1 && h2) {
        list_ele_t **source = (cmp_elem(h1, h2) < 0) ? (&h1) : (&h2);
        tail->next = (*source);
        tail = tail->next;
        (*source) = (*source)->next;
    }
    if (!h1) {
        tail->next = h2;
        rr->tail = r2->tail;
    } else {
        tail->next = h1;
        rr->tail = r1->tail;
    }
    rr->head = lead.next;
}

/*
 * User merge sort to sort a linked list in a closed range.
 */
static void merge_sort(range_t *r)
{
    {
        list_ele_t *head = r->head;
        list_ele_t *tail = r->tail;
        if (head == tail) {
            return;
        } else if (head->next == tail) {
            if (cmp_elem(tail, head) < 0) {
                tail->next = head;
                head->next = NULL;
                r->head = tail;
                r->tail = head;
            }
            return;
        }
    }
    range_t r1 = {.head = NULL, .tail = NULL};
    range_t r2 = {.head = NULL, .tail = NULL};
    split(r, &r1, &r2);
    merge_sort(&r1);
    merge_sort(&r2);
    merge(&r1, &r2, r);
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
    range_t range = {.head = q->head, .tail = q->tail};
    merge_sort(&range);
    q->head = range.head;
    q->tail = range.tail;
}
