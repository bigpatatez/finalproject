

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"




/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {

    dplist_t * theList = *list;
    int size = dpl_size(theList);
    for(int i=0;i< size;i++)
    {
        dpl_remove_at_index(theList,0,free_element);
    }
    free(theList);
    *list = NULL;

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;
    list_node = malloc(sizeof(dplist_node_t));

    if(insert_copy)
    {
        list_node->element = list->element_copy(element);
    }
    else
    {
        list_node->element = element;
    }

    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;

}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {

    if(list == NULL){return NULL;}
    else if (list ->head == NULL){return list;}

    dplist_node_t * node = dpl_get_reference_at_index(list, index);
    if(dpl_size(list)==1)
    {
        list->head = NULL;
    }
    else if(index <= 0)
    {
        dplist_node_t * next = node->next;
        list->head = next;
        next->prev = NULL;
    }
    else if(index >= dpl_size(list)-1)
    {
        dplist_node_t * prev = node->prev;
        prev->next = NULL;
    }
    else{
        dplist_node_t * prev = node->prev;
        dplist_node_t * next = node->next;
        prev->next = next;
        next->prev = prev;
    }

    if(free_element)
    {
        list->element_free(&(node->element));
        free(node);
    }
    else{ free(node);}

    return list;
}

int dpl_size(dplist_t *list) {
    int counter = 0;
    if(list == NULL){return -1;}
    else if(list->head == NULL)
    {
        return counter;
    }
    else
    {
        dplist_node_t * next_node = list -> head;
        while(next_node != NULL)
        {
            counter ++;
            next_node = next_node->next;
        }
        return counter;
    }
}

void *dpl_get_element_at_index(dplist_t *list, int index) {

    dplist_node_t * ref = dpl_get_reference_at_index(list,index);
    return ref->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    if(list == NULL||list->head == NULL){return -1;}
    else
    {
        for (int  i= 0 ; i< dpl_size(list);i++)
        {
            if(list->element_compare(dpl_get_element_at_index(list,i),element)==0)
            {
                return i;
            }
        }
    }
    return -1;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    if(list == NULL ||list->head == NULL){return NULL;}
    else
    {
        int size = dpl_size(list);
        if(index<= 0)
        {
            return list->head;
        }
        else if(index >= size)
        {
            dplist_node_t * el = list ->head;
            while(el->next != NULL)
            {
                el = el->next;
            }
            return el ;
        }
        else
        {
            int counter = 0 ;
            dplist_node_t * ref = list ->head;
            while(counter<index)
            {
                ref = ref->next;
                counter++;
            }
            return ref;
        }
    }
}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {

    if(list == NULL){return NULL;}
    else if(list->head == NULL||reference==NULL){return NULL;}
    dplist_node_t* node = list->head;
    while(node != NULL)
    {
        if(node == reference)
        {
            return node->element;
        }
        node = node->next;
    }
    return NULL;
}


