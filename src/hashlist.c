#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "typedefs.h"

/*
bandit1 -> bandit2 ...
natas1 -> natas2 ...
narnia1 -> narnia2 ...

'b' + 'a' + 'n' = 305
'n' + 'a' + 't' = 323
'n' + 'a' + 'r' = 321

hash_table[0].key = 305
hash_table[0].next -> bandit1_node -> bandit2_node ...
hash_table[1].key = 323
hash_table[1].next -> natas1_node -> natas2_node ...
hash_table[2].key = 321
hash_table[2].next -> narnia1_node -> narnia2_node ...
...
*/

typedef struct Ht_item_t {
	unsigned int	key;
	int				size;
	int				count;
	level_t   	  **levels;
} Ht_item_t;

typedef struct HashTable_t {
	Ht_item_t  **items;
	int			 size;
	int			 count;
} HashTable_t;

unsigned int
hash_function(char *game_name)
{
	unsigned int i = 0;

	for (int j = 0; j < 3; j++) {
		i += game_name[j];
	}

	return i;
}

Ht_item_t *
create_item(unsigned int key, int size)
{
	// Creates a pointer to a new HashTable_t item.
	Ht_item_t *item = (Ht_item_t *) malloc(sizeof(Ht_item_t));
	item->key = key;
	item->size = size;
	item->count = 0;
	item->levels = (level_t **) calloc(item->size, sizeof(level_t *));
	return item;
}

HashTable_t *
create_table(int size)
{
    // Creates a new HashTable.
    HashTable_t *table = (HashTable_t *) malloc(sizeof(HashTable_t));
    table->size = size;
    table->count = 0;
    table->items = (Ht_item_t **) calloc(table->size, sizeof(Ht_item_t *));

    for (int i = 0; i < table->size; i++) {
        table->items[i] = NULL;
	}

    return table;
}

void
free_item(Ht_item_t *item)
{
	// Frees the item.
    for (int i = 0; i < item->size; i++)
    {
        level_t *level = item->levels[i];

        if (level != NULL)
            free(level);
    }

    free(item->levels);
    free(item);
	return;
}

void
free_table(HashTable_t *table)
{
    // Frees the table.
    for (int i = 0; i < table->size; i++)
    {
        Ht_item_t *item = table->items[i];

        if (item != NULL)
            free_item(item);
    }

    free(table->items);
    free(table);
	return;
}

void
print_table(HashTable_t *table)
{
    printf("\nHash Table\n-------------------\n");
	printf("Size = %d, Count = %d\n", table->size, table->count);
    for (int i = 0; i < table->size; i++) {
        if (table->items[i]) {
            printf("Table item %d:, Key = %u, Size = %d, Count = %d\n", i,
				table->items[i]->key, table->items[i]->size, table->items[i]->count);

       		printf("Levels: ");
    		for (int j = 0; j < table->items[i]->size; j++) {
        		if (table->items[i]->levels[j]) {
            		printf("%s", table->items[i]->levels[j]->game_name);
					if (j != table->items[i]->size - 1) {
						printf(", ");
					}
        		}
    		}

        }
    }
    printf("-------------------\n\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 50000 // Size of the HashTable.

unsigned long hash_function(char *str)
{
    unsigned long i = 0;

    for (int j = 0; str[j]; j++)
        i += str[j];

    return i % CAPACITY;
}

// Defines the HashTable item.
typedef struct Ht_item
{
    char *key;
    char *value;
} Ht_item;

// Defines the LinkedList.
typedef struct LinkedList
{
    Ht_item_t *item;
    Llist_t *next;
} LinkedList;

// Defines the HashTable.
typedef struct HashTable
{
    // Contains an array of pointers to items.
    Ht_item_t **items;
    Llist_t **overflow_buckets;
    int size;
    int count;
} HashTable;

Llist_t *allocate_list()
{
    // Allocates memory for a LinkedList pointer.
    Llist_t *list = (Llist_t *)malloc(sizeof(LinkedList));
    return list;
}

Llist_t *linkedlist_insert(Llist_t *list, Ht_item_t *item)
{
    // Inserts the item onto the LinkedList.
    if (!list)
    {
        Llist_t *head = allocate_list();
        head->item = item;
        head->next = NULL;
        list = head;
        return list;
    }
    else if (list->next == NULL)
    {
        Llist_t *node = allocate_list();
        node->item = item;
        node->next = NULL;
        list->next = node;
        return list;
    }

    Llist_t *temp = list;

    while (temp->next->next)
    {
        temp = temp->next;
    }

    Llist_t *node = allocate_list();
    node->item = item;
    node->next = NULL;
    temp->next = node;
    return list;
}

Ht_item_t *linkedlist_remove(Llist_t *list)
{
    // Removes the head from the LinkedList.
    // Returns the item of the popped element.
    if (!list)
        return NULL;

    if (!list->next)
        return NULL;

    Llist_t *node = list->next;
    Llist_t *temp = list;
    temp->next = NULL;
    list = node;
    Ht_item_t *it = NULL;
    memcpy(temp->item, it, sizeof(Ht_item));
    free(temp->item->key);
    free(temp->item->value);
    free(temp->item);
    free(temp);
    return it;
}

void free_linkedlist(Llist_t *list)
{
    Llist_t *temp = list;

    while (list)
    {
        temp = list;
        list = list->next;
        free(temp->item->key);
        free(temp->item->value);
        free(temp->item);
        free(temp);
    }
}

Llist_t **create_overflow_buckets(HashTable_t *table)
{
    // Create the overflow buckets; an array of LinkedLists.
    Llist_t **buckets = (Llist_t **)calloc(table->size, sizeof(Llist_t *));

    for (int i = 0; i < table->size; i++)
        buckets[i] = NULL;

    return buckets;
}

void free_overflow_buckets(HashTable_t *table)
{
    // Free all the overflow bucket lists.
    Llist_t **buckets = table->overflow_buckets;

    for (int i = 0; i < table->size; i++)
        free_linkedlist(buckets[i]);

    free(buckets);
}

Ht_item_t *create_item(char *key, char *value)
{
    // Creates a pointer to a new HashTable item.
    Ht_item_t *item = (Ht_item_t *)malloc(sizeof(Ht_item));
    item->key = (char *)malloc(strlen(key) + 1);
    item->value = (char *)malloc(strlen(value) + 1);
    strcpy(item->key, key);
    strcpy(item->value, value);
    return item;
}

HashTable_t *create_table(int size)
{
    // Creates a new HashTable.
    HashTable_t *table = (HashTable_t *)malloc(sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->items = (Ht_item_t **)calloc(table->size, sizeof(Ht_item_t *));

    for (int i = 0; i < table->size; i++)
        table->items[i] = NULL;

    table->overflow_buckets = create_overflow_buckets(table);

    return table;
}

void free_item(Ht_item_t *item)
{
    // Frees an item.
    free(item->key);
    free(item->value);
    free(item);
}

void free_table(HashTable_t *table)
{
    // Frees the table.
    for (int i = 0; i < table->size; i++)
    {
        Ht_item_t *item = table->items[i];

        if (item != NULL)
            free_item(item);
    }

    // Free the overflow bucket lists and its items.
    free_overflow_buckets(table);
    free(table->items);
    free(table);
}

void handle_collision(HashTable_t *table, unsigned long index, Ht_item_t *item)
{
    Llist_t *head = table->overflow_buckets[index];

    if (head == NULL)
    {
        // Creates the list.
        head = allocate_list();
        head->item = item;
        table->overflow_buckets[index] = head;
        return;
    }
    else
    {
        // Insert to the list.
        table->overflow_buckets[index] = linkedlist_insert(head, item);
        return;
    }
}

void ht_insert(HashTable_t *table, char *key, char *value)
{
    // Creates the item.
    Ht_item_t *item = create_item(key, value);

    // Computes the index.
    int index = hash_function(key);

    Ht_item_t *current_item = table->items[index];

    if (current_item == NULL)
    {
        // Key does not exist.
        if (table->count == table->size)
        {
            // HashTable is full.
            printf("Insert Error: Hash Table is full\n");
            free_item(item);
            return;
        }

        // Insert directly.
        table->items[index] = item;
        table->count++;
    }
    else
    {
        // Scenario 1: Update the value.
        if (strcmp(current_item->key, key) == 0)
        {
            strcpy(table->items[index]->value, value);
            return;
        }
        else
        {
            // Scenario 2: Handle the collision.
            handle_collision(table, index, item);
            return;
        }
    }
}

char *ht_search(HashTable_t *table, char *key)
{
    // Searches for the key in the HashTable.
    // Returns NULL if it doesn't exist.
    int index = hash_function(key);
    Ht_item_t *item = table->items[index];
    Llist_t *head = table->overflow_buckets[index];

    // Provide only non-NULL values.
    if (item != NULL)
    {
        if (strcmp(item->key, key) == 0)
            return item->value;

        if (head == NULL)
            return NULL;

        item = head->item;
        head = head->next;
    }

    return NULL;
}

void ht_delete(HashTable_t *table, char *key)
{
    // Deletes an item from the table.
    int index = hash_function(key);
    Ht_item_t *item = table->items[index];
    Llist_t *head = table->overflow_buckets[index];

    if (item == NULL)
    {
        // Does not exist.
        return;
    }
    else
    {
        if (head == NULL && strcmp(item->key, key) == 0)
        {
            // Collision chain does not exist.
            // Remove the item.
            // Set table index to NULL.
            table->items[index] = NULL;
            free_item(item);
            table->count--;
            return;
        }
        else if (head != NULL)
        {
            // Collision chain exists.
            if (strcmp(item->key, key) == 0)
            {
                // Remove this item.
                // Set the head of the list as the new item.
                free_item(item);
                Llist_t *node = head;
                head = head->next;
                node->next = NULL;
                table->items[index] = create_item(node->item->key, node->item->value);
                free_linkedlist(node);
                table->overflow_buckets[index] = head;
                return;
            }

            Llist_t *curr = head;
            Llist_t *prev = NULL;

            while (curr)
            {
                if (strcmp(curr->item->key, key) == 0)
                {
                    if (prev == NULL)
                    {
                        // First element of the chain.
                        // Remove the chain.
                        free_linkedlist(head);
                        table->overflow_buckets[index] = NULL;
                        return;
                    }
                    else
                    {
                        // This is somewhere in the chain.
                        prev->next = curr->next;
                        curr->next = NULL;
                        free_linkedlist(curr);
                        table->overflow_buckets[index] = head;
                        return;
                    }
                }

                curr = curr->next;
                prev = curr;
            }
        }
    }
}

void print_search(HashTable_t *table, char *key)
{
    char *val;

    if ((val = ht_search(table, key)) == NULL)
    {
        printf("Key:%s does not exist\n", key);
        return;
    }
    else
    {
        printf("Key:%s, Value:%s\n", key, val);
    }
}

void print_table(HashTable_t *table)
{
    printf("\nHash Table\n-------------------\n");

    for (int i = 0; i < table -> size; i++)
    {
        if (table -> items[i])
        {
            printf("Index:%d, Key:%s, Value:%s\n", i, table -> items[i] -> key, table -> items[i] -> value);
        }
    }

    printf("-------------------\n\n");
}

int main()
{
    HashTable_t *ht = create_table(CAPACITY);
    ht_insert(ht, (char *)"1", (char *)"First address");
    ht_insert(ht, (char *)"2", (char *)"Second address");
    ht_insert(ht, (char *)"Hel", (char *)"Third address");
    ht_insert(ht, (char *)"Cau", (char *)"Fourth address");
    print_search(ht, (char *)"1");
    print_search(ht, (char *)"2");
    print_search(ht, (char *)"3");
    print_search(ht, (char *)"Hel");
    print_search(ht, (char *)"Cau"); // Collision!
    print_table(ht);
    ht_delete(ht, (char *)"1");
    ht_delete(ht, (char *)"Cau");
    print_table(ht);
    free_table(ht);
    return 0;
}