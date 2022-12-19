#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define list_append_ptr(list, ptr) list_append((list_node_t **)list, (list_node_t *)list_item_new_sizet(ptr))
#define list_append_str(list, str) list_append((list_node_t **)list, (list_node_t *)list_item_new_str(str))
#define list_remove_at_index(list, index) list_remove_at((list_node_t **)list, index)
#define list_reverse(list) lst_reverse((list_node_t **)list)

typedef struct list_node
{
    struct list_node *next;
    void *val;
} list_node_t;

typedef struct list_item
{
    list_node_t node;
} list_item_t;

list_node_t *list_get_tail(list_node_t **head)
{
    if (head == NULL)
    {
        return NULL;
    }
    list_node_t *current_node = *head;
    list_node_t *last_node = NULL;
    while (current_node)
    {
        last_node = current_node;
        current_node = current_node->next;
    }
    return last_node;
}

list_node_t *list_append(list_node_t **head, list_node_t *item)
{

    list_node_t *tail = list_get_tail(head);
    if (!tail)
    {
        *head = item;
    }
    else
    {
        tail->next = item;
    }
    item->next = NULL;
    return item;
}
list_node_t *list_pop(list_node_t **head)
{
    if (head == NULL)
    {
        return NULL;
    }
    list_node_t *current_head = *head;
    if (!current_head)
    {
        return NULL;
    }
    *head = (*head)->next;
    current_head->next = NULL;
    return current_head;
}

list_item_t *list_item_new_sizet(void *val)
{
    list_item_t *item = malloc(sizeof(list_item_t));
    if (!item)
    {
        return NULL;
    }
    item->node.val = val;
    return item;
}

list_item_t *list_item_new_str(char *val)
{
    list_item_t *item = malloc(sizeof(list_item_t));
    if (!item)
    {
        return NULL;
    }
    item->node.val = malloc(strlen(val) + 1);
    strcpy_s(item->node.val, strlen(val) + 1, val);
    return item;
}
int list_len(list_node_t **head)
{   
    if (*head == NULL || head == NULL)
    {
        return 0;
    }
    int counter = 1;
    list_node_t *cur_head = *head;
    while (cur_head)
    {
        cur_head = cur_head->next;
        counter++;
    }
    return counter;
}

void free_linked_list_node(list_node_t *cur_head)
{
    free(cur_head->val);
    cur_head->val = NULL;
    free(cur_head);
    cur_head = NULL;
}

list_node_t *list_remove_at(list_node_t **head, int index)
{
    if (head == NULL)
    {
        return NULL;
    }
    if (index < 0 || index >= list_len(head))
    {
        return NULL;
    }
    list_node_t *prev = NULL;
    if (index == 0)
    {

        prev = *head;
        *head = (*head)->next;
        return prev;
        // free_linked_list_node(prev);
    }

    int counter = 0;
    list_node_t *cur_head = *head;
    while (counter < index)
    {
        prev = cur_head;
        cur_head = cur_head->next;
        counter++;
    }

    prev->next = cur_head->next;
    return cur_head;
    // free_linked_list_node(cur_head);
}

void list_clear(list_node_t **head)
{
    if (head == NULL)
    {
        return;
    }

    list_node_t *cur_node = *head;
    list_node_t *next_node = NULL;
    while (cur_node)
    {
        next_node = cur_node->next;
        cur_node->val = NULL;
        free(cur_node->val);
        cur_node->next = NULL;
        free(cur_node);
        cur_node = next_node;
    }
    free(head);
    *head = NULL;
}
void lst_reverse(list_node_t **head)
{
    if (head == NULL)
    {
        return;
    }
    list_node_t *curr_node = *head;
    list_node_t *prev_node = NULL;
    list_node_t *next_node = NULL;
    while (curr_node)
    {
        next_node = curr_node->next;
        curr_node->next = prev_node;
        prev_node = curr_node;
        curr_node = next_node;
    }
    *head = prev_node;
}
