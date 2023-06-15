#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "constants.h"

#define NUM_LEVELS		183
#define NOT_COMPLETED	0
#define COMPLETED		0
#define CAPACITY 		12 // Size of the GameTable.
#define DATA_FILE		"data/level-entries.cvs"

typedef struct level_t {
	char	name[LVLNAME_MAX];
	char	ssh_addr[LVLADDR_MAX];
	char	pw[LVLPW_MAX];
	int		is_completed;
} level_t;

typedef struct game_t {
	char	    name[LVLNAME_MAX];
	int			port;
	int 		size;
	int 		count;
	int 	 	is_completed;
	level_t	  **levels;
} game_t;

typedef struct LinkedList_t
{
	game_t 			*game;
	LinkedList_t 	*next;
} LinkedList_t;

typedef struct GameTable_t {
	game_t 			**games;
	LinkedList_t 	**overflow_buckets;
	int 			  size;
	int 			  count;
} GameTable_t;

typedef struct savedgame_t {
	char		name[LVLNAME_MAX];
	int			port;
	int 		size;
	int 	 	is_completed;
} savedgame_t;

typedef struct savedlevel_t {
	char		name[LVLNAME_MAX];
	char		pw[LVLPW_MAX];
	int 	 	is_completed;
} savedlevel_t;

const savedgame_t savedgames[NUM_GAMES] = {
	{"bandit", 		BANDIT_PORT, 	BANDIT_MAX_LEVEL, 		0},
	{"natas", 		0, 				NATAS_MAX_LEVEL, 		0},
	{"leviathan", 	LEVIATHAN_PORT, LEVIATHAN_MAX_LEVEL, 	0},
	{"krypton", 	KRYPTON_PORT, 	KRYPTON_MAX_LEVEL, 		0},
	{"narnia", 		NARNIA_PORT, 	NARNIA_MAX_LEVEL, 		0},
	{"behemoth", 	BEHEMOTH_PORT, 	BEHEMOTH_MAX_LEVEL, 	0},
	{"utumno", 		UTUMNO_PORT, 	UTUMNO_MAX_LEVEL, 		0},
	{"maze", 		MAZE_PORT, 		MAZE_MAX_LEVEL, 		0},
	{"vortex", 		VORTEX_PORT, 	VORTEX_MAX_LEVEL, 		0},
	{"manpage", 	MANPAGE_PORT, 	MANPAGE_MAX_LEVEL, 		0},
	{"drifter", 	DRIFTER_PORT, 	DRIFTER_MAX_LEVEL, 		0},
	{"formulaone", 	FORMULAONE_PORT, FORMULAONE_MAX_LEVEL, 	0}
};

const savedlevel_t savedlevels[NUM_LEVELS] = {
	{"bandit0","",NOT_COMPLETED},
	{"bandit1","",NOT_COMPLETED},
	{"bandit2","",NOT_COMPLETED},
	{"bandit3","",NOT_COMPLETED},
	{"bandit4","",NOT_COMPLETED},
	{"bandit5","",NOT_COMPLETED},
	{"bandit6","",NOT_COMPLETED},
	{"bandit7","",NOT_COMPLETED},
	{"bandit8","",NOT_COMPLETED},
	{"bandit9","",NOT_COMPLETED},
	{"bandit10","",NOT_COMPLETED},
	{"bandit11","",NOT_COMPLETED},
	{"bandit12","",NOT_COMPLETED},
	{"bandit13","",NOT_COMPLETED},
	{"bandit14","",NOT_COMPLETED},
	{"bandit15","",NOT_COMPLETED},
	{"bandit16","",NOT_COMPLETED},
	{"bandit17","",NOT_COMPLETED},
	{"bandit18","",NOT_COMPLETED},
	{"bandit19","",NOT_COMPLETED},
	{"bandit20","",NOT_COMPLETED},
	{"bandit21","",NOT_COMPLETED},
	{"bandit22","",NOT_COMPLETED},
	{"bandit23","",NOT_COMPLETED},
	{"bandit24","",NOT_COMPLETED},
	{"bandit25","",NOT_COMPLETED},
	{"bandit26","",NOT_COMPLETED},
	{"bandit27","",NOT_COMPLETED},
	{"bandit28","",NOT_COMPLETED},
	{"bandit29","",NOT_COMPLETED},
	{"bandit30","",NOT_COMPLETED},
	{"bandit31","",NOT_COMPLETED},
	{"bandit32","",NOT_COMPLETED},
	{"bandit33","",NOT_COMPLETED},
	{"bandit34","",NOT_COMPLETED},
	{"natas0","",NOT_COMPLETED},
	{"natas1","",NOT_COMPLETED},
	{"natas2","",NOT_COMPLETED},
	{"natas3","",NOT_COMPLETED},
	{"natas4","",NOT_COMPLETED},
	{"natas5","",NOT_COMPLETED},
	{"natas6","",NOT_COMPLETED},
	{"natas7","",NOT_COMPLETED},
	{"natas8","",NOT_COMPLETED},
	{"natas9","",NOT_COMPLETED},
	{"natas10","",NOT_COMPLETED},
	{"natas11","",NOT_COMPLETED},
	{"natas12","",NOT_COMPLETED},
	{"natas13","",NOT_COMPLETED},
	{"natas14","",NOT_COMPLETED},
	{"natas15","",NOT_COMPLETED},
	{"natas16","",NOT_COMPLETED},
	{"natas17","",NOT_COMPLETED},
	{"natas18","",NOT_COMPLETED},
	{"natas19","",NOT_COMPLETED},
	{"natas20","",NOT_COMPLETED},
	{"natas21","",NOT_COMPLETED},
	{"natas22","",NOT_COMPLETED},
	{"natas23","",NOT_COMPLETED},
	{"natas24","",NOT_COMPLETED},
	{"natas25","",NOT_COMPLETED},
	{"natas26","",NOT_COMPLETED},
	{"natas27","",NOT_COMPLETED},
	{"natas28","",NOT_COMPLETED},
	{"natas29","",NOT_COMPLETED},
	{"natas30","",NOT_COMPLETED},
	{"natas31","",NOT_COMPLETED},
	{"natas32","",NOT_COMPLETED},
	{"natas33","",NOT_COMPLETED},
	{"natas34","",NOT_COMPLETED},
	{"leviathan0","",NOT_COMPLETED},
	{"leviathan1","",NOT_COMPLETED},
	{"leviathan2","",NOT_COMPLETED},
	{"leviathan3","",NOT_COMPLETED},
	{"leviathan4","",NOT_COMPLETED},
	{"leviathan5","",NOT_COMPLETED},
	{"leviathan6","",NOT_COMPLETED},
	{"leviathan7","",NOT_COMPLETED},
	{"krypton0","",NOT_COMPLETED},
	{"krypton1","",NOT_COMPLETED},
	{"krypton2","",NOT_COMPLETED},
	{"krypton3","",NOT_COMPLETED},
	{"krypton4","",NOT_COMPLETED},
	{"krypton5","",NOT_COMPLETED},
	{"krypton6","",NOT_COMPLETED},
	{"krypton7","",NOT_COMPLETED},
	{"narnia0","",NOT_COMPLETED},
	{"narnia1","",NOT_COMPLETED},
	{"narnia2","",NOT_COMPLETED},
	{"narnia3","",NOT_COMPLETED},
	{"narnia4","",NOT_COMPLETED},
	{"narnia5","",NOT_COMPLETED},
	{"narnia6","",NOT_COMPLETED},
	{"narnia7","",NOT_COMPLETED},
	{"narnia8","",NOT_COMPLETED},
	{"narnia9","",NOT_COMPLETED},
	{"behemoth0","",NOT_COMPLETED},
	{"behemoth1","",NOT_COMPLETED},
	{"behemoth2","",NOT_COMPLETED},
	{"behemoth3","",NOT_COMPLETED},
	{"behemoth4","",NOT_COMPLETED},
	{"behemoth5","",NOT_COMPLETED},
	{"behemoth6","",NOT_COMPLETED},
	{"behemoth7","",NOT_COMPLETED},
	{"behemoth8","",NOT_COMPLETED},
	{"utumno0","",NOT_COMPLETED},
	{"utumno1","",NOT_COMPLETED},
	{"utumno2","",NOT_COMPLETED},
	{"utumno3","",NOT_COMPLETED},
	{"utumno4","",NOT_COMPLETED},
	{"utumno5","",NOT_COMPLETED},
	{"utumno6","",NOT_COMPLETED},
	{"utumno7","",NOT_COMPLETED},
	{"utumno8","",NOT_COMPLETED},
	{"maze0","",NOT_COMPLETED},
	{"maze1","",NOT_COMPLETED},
	{"maze2","",NOT_COMPLETED},
	{"maze3","",NOT_COMPLETED},
	{"maze4","",NOT_COMPLETED},
	{"maze5","",NOT_COMPLETED},
	{"maze6","",NOT_COMPLETED},
	{"maze7","",NOT_COMPLETED},
	{"maze8","",NOT_COMPLETED},
	{"maze9","",NOT_COMPLETED},
	{"vortex0","",NOT_COMPLETED},
	{"vortex1","",NOT_COMPLETED},
	{"vortex2","",NOT_COMPLETED},
	{"vortex3","",NOT_COMPLETED},
	{"vortex4","",NOT_COMPLETED},
	{"vortex5","",NOT_COMPLETED},
	{"vortex6","",NOT_COMPLETED},
	{"vortex7","",NOT_COMPLETED},
	{"vortex8","",NOT_COMPLETED},
	{"vortex9","",NOT_COMPLETED},
	{"vortex10","",NOT_COMPLETED},
	{"vortex11","",NOT_COMPLETED},
	{"vortex12","",NOT_COMPLETED},
	{"vortex13","",NOT_COMPLETED},
	{"vortex14","",NOT_COMPLETED},
	{"vortex15","",NOT_COMPLETED},
	{"vortex16","",NOT_COMPLETED},
	{"vortex17","",NOT_COMPLETED},
	{"vortex18","",NOT_COMPLETED},
	{"vortex19","",NOT_COMPLETED},
	{"vortex20","",NOT_COMPLETED},
	{"vortex21","",NOT_COMPLETED},
	{"vortex22","",NOT_COMPLETED},
	{"vortex23","",NOT_COMPLETED},
	{"vortex24","",NOT_COMPLETED},
	{"vortex25","",NOT_COMPLETED},
	{"vortex26","",NOT_COMPLETED},
	{"vortex27","",NOT_COMPLETED},
	{"manpage0","",NOT_COMPLETED},
	{"manpage1","",NOT_COMPLETED},
	{"manpage2","",NOT_COMPLETED},
	{"manpage3","",NOT_COMPLETED},
	{"manpage4","",NOT_COMPLETED},
	{"manpage5","",NOT_COMPLETED},
	{"manpage6","",NOT_COMPLETED},
	{"manpage7","",NOT_COMPLETED},
	{"drifter0","",NOT_COMPLETED},
	{"drifter1","",NOT_COMPLETED},
	{"drifter2","",NOT_COMPLETED},
	{"drifter3","",NOT_COMPLETED},
	{"drifter4","",NOT_COMPLETED},
	{"drifter5","",NOT_COMPLETED},
	{"drifter6","",NOT_COMPLETED},
	{"drifter7","",NOT_COMPLETED},
	{"drifter8","",NOT_COMPLETED},
	{"drifter9","",NOT_COMPLETED},
	{"drifter10","",NOT_COMPLETED},
	{"drifter11","",NOT_COMPLETED},
	{"drifter12","",NOT_COMPLETED},
	{"drifter13","",NOT_COMPLETED},
	{"drifter14","",NOT_COMPLETED},
	{"drifter15","",NOT_COMPLETED},
	{"formulaone0","",NOT_COMPLETED},
	{"formulaone1","",NOT_COMPLETED},
	{"formulaone2","",NOT_COMPLETED},
	{"formulaone3","",NOT_COMPLETED},
	{"formulaone4","",NOT_COMPLETED},
	{"formulaone5","",NOT_COMPLETED},
	{"formulaone6","",NOT_COMPLETED}
};

uint32_t hash_function(char *name);
LinkedList_t *allocate_list(void);
LinkedList_t *linkedlist_insert(LinkedList_t *list, game_t *game);
game_t *linkedlist_remove(LinkedList_t *list);

LinkedList_t **create_overflow_buckets(GameTable_t *table);
level_t *create_level(savedlevel_t *savedlevel, game_t game);
game_t *load_savedgame(savedgame_t *savedgame);
game_t *create_game(char *game_name, int size);
GameTable_t *create_table(int size);

void free_linkedlist(LinkedList_t *list);
void free_overflow_buckets(GameTable_t *table);
void free_level(level_t *level);
void free_game(game_t *game);
void free_table(GameTable_t *table);

void handle_collision(GameTable_t *table, uint32_t index, game_t *game);
void gt_insert(GameTable_t *table, char *name, char *value);
int  gt_search(GameTable_t *table, char *name);
void gt_delete(GameTable_t *table, char *name);

void print_search(GameTable_t *table, char *name);
void print_table(GameTable_t *table);

int
main(int argc, char **argv)
{
	GameTable_t *gt = create_table(CAPACITY);
	gt_insert(gt, (char *) "1",   (char *) "First address");
	gt_insert(gt, (char *) "2",   (char *) "Second address");
	gt_insert(gt, (char *) "Hel", (char *) "Third address");
	gt_insert(gt, (char *) "Cau", (char *) "Fourth address");
	print_search(gt, (char *) "1");
	print_search(gt, (char *) "2");
	print_search(gt, (char *) "3");
	print_search(gt, (char *) "Hel");
	print_search(gt, (char *) "Cau"); // Collision!
	print_table(gt);
	gt_delete(gt, (char *) "1");
	gt_delete(gt, (char *) "Cau");
	print_table(gt);
	free_table(gt);
	return 0;
}

uint32_t
hash_function(char *name)
{
	uint32_t hash = 5381;
	int len = (int) strlen(name);

	for (int i = 0; i < len; i++) {
		hash = ((hash << 5) + hash) + (uint32_t) name[i];
	}

	hash = hash % CAPACITY;
	//printf("%15s %"PRIu32"\n", name, hash);
	return hash;
}

LinkedList_t *
allocate_list(void)
{
	// Allocates memory for a LinkedList_t pointer.
	LinkedList_t *list = (LinkedList_t *) malloc(sizeof(LinkedList_t));
	return list;
}

LinkedList_t *
linkedlist_insert(LinkedList_t *list, game_t *game)
{
	// Inserts the game onto the LinkedList_t.
	if (!list)
	{
		LinkedList_t *head = allocate_list();
		head->game = game;
		head->next = NULL;
		list = head;
		return list;
	}
	else if (list->next == NULL)
	{
		LinkedList_t *node = allocate_list();
		node->game = game;
		node->next = NULL;
		list->next = node;
		return list;
	}

	LinkedList_t *temp = list;

	while (temp->next->next)
	{
		temp = temp->next;
	}

	LinkedList_t *node = allocate_list();
	node->game = game;
	node->next = NULL;
	temp->next = node;
	return list;
}

game_t *
linkedlist_remove(LinkedList_t *list)
{
	// Removes the head from the LinkedList_t.
	// Returns the game of the popped element.
	if (!list)
		return NULL;

	if (!list->next)
		return NULL;

	LinkedList_t *node = list->next;
	LinkedList_t *temp = list;
	temp->next = NULL;
	list = node;
	game_t *it = NULL;
	memcpy(temp->game, it, sizeof(game_t));
	free(temp->game->name);
	free(temp->game->port);
	free(temp->game);
	free(temp);
	return it;
}

void
free_linkedlist(LinkedList_t *list)
{
	LinkedList_t *temp = list;

	while (list)
	{
		temp = list;
		list = list->next;
		free(temp->game->name);
		free(temp->game->port);
		free(temp->game);
		free(temp);
	}
}

LinkedList_t **
create_overflow_buckets(GameTable_t *table)
{
	// Create the overflow buckets; an array of pointers to LinkedList_t items.
	LinkedList_t **buckets = (LinkedList_t **) calloc(table->size, sizeof(LinkedList_t *));

	for (int i = 0; i < table->size; i++)
		buckets[i] = NULL;

	return buckets;
}

void
free_overflow_buckets(GameTable_t *table)
{
	// Free all the overflow bucket lists.
	LinkedList_t **buckets = table->overflow_buckets;

	for (int i = 0; i < table->size; i++)
		free_linkedlist(buckets[i]);

	free(buckets);
}

level_t *
create_level(savedlevel_t *savedlevel, game_t game)
{
	// Creates a pointer to a new level_t item.
	level_t *level = (level_t *) malloc(sizeof(level_t));
	
	level->is_completed = savedlevel->is_completed;
	strncpy(level->name, savedlevel->name, LVLNAME_MAX);
	strncpy(level->pw, savedlevel->pw, LVLPW_MAX);
	snprintf(level->ssh_addr, LVLPW_MAX, "%s@%s.labs.overthewire.org", level->name, game->name);

	return level;
}

game_t *
create_game(char *game_name, int size)
{
	// Creates a pointer to a new game_t item.
	game_t *game = (game_t *) malloc(sizeof(game_t));
	
	game->count = 0;
	game->size = size;
	game->port = 0;
	game->is_completed = 0;
	strncpy(game->name, game_name, LVLNAME_MAX);
	game->levels = (level_t **) calloc(game->size, sizeof(level_t *));
	
	for (int i = 0; i < game->size; i++) {
		game->levels[i] = NULL;
	}

	return game;
}

game_t *
load_savedgame(savedgame_t *savedgame)
{
	// Creates a pointer to a new game_t item.
	game_t *game = (game_t *) malloc(sizeof(game_t));
	
	game->count = 0;
	game->size = savedgame->size;
	game->port = savedgame->port;
	game->is_completed = savedgame->is_completed;
	strncpy(game->name, savedgame->name, LVLNAME_MAX);
	game->levels = (level_t **) calloc(game->size, sizeof(level_t *));
	
	for (int i = 0; i < game->size; i++) {
		game->levels[i] = NULL;
	}

	return game;
}

GameTable_t *
create_table(int size)
{
	// Creates a new GameTable.
	GameTable_t *table = (GameTable_t *) malloc(sizeof(GameTable_t));
	table->size = size;
	table->count = 0;
	table->games = (game_t **) calloc(table->size, sizeof(game_t *));

	for (int i = 0; i < table->size; i++) {
		table->games[i] = NULL;
	}

	table->overflow_buckets = create_overflow_buckets(table);

	return table;
}

void
free_level(level_t *level)
{
	// Frees a level.
	free(level);
}

void
free_game(game_t *game)
{
	// Frees a game.
	free(game->levels);
	free(game);
}

void
free_table(GameTable_t *table)
{
	game_t *game = NULL;

	// Frees the GameTable.
	for (int i = 0; i < table->size; i++) {
		game = table->games[i];

		if (game != NULL) {
			free_game(game);
		}
	}

	// Free the overflow bucket lists and its items.
	free_overflow_buckets(table);
	free(table->games);
	free(table);
}

void
handle_collision(GameTable_t *table, uint32_t index, game_t *game)
{
	LinkedList_t *head = table->overflow_buckets[index];

	if (head == NULL) {
		// Creates the list.
		head = allocate_list();
		head->game = game;
		table->overflow_buckets[index] = head;
		return;
	} else {
		// Insert to the list.
		table->overflow_buckets[index] = linkedlist_insert(head, game);
		return;
	}
}

void
gt_insert(GameTable_t *table, char *name, char *value)
{
	// Creates the game.
	game_t *game = create_game(name, value);

	// Computes the index.
	uint32_t index = hash_function(name);

	game_t *current_game = table->games[index];

	if (current_game == NULL)
	{
		// name does not exist.
		if (table->count == table->size) {
			// GameTable is full.
			printf("Insert Error: GameTable is full!\n");
			free_game(game);
			return;
		}

		// Insert directly.
		table->games[index] = game;
		table->count++;

	} else {
		// Scenario 1: Update the value.
		if (strcmp(current_game->name, name) == 0) {
			strcpy(table->games[index]->port, value);
			return;
		} else {
			// Scenario 2: Handle the collision.
			handle_collision(table, index, game);
			return;
		}
	}
}

int
gt_search(GameTable_t *table, char *name)
{
	// Searches for the name in the GameTable.
	// Returns -1 if it doesn't exist.
	uint32_t index = hash_function(name);
	game_t *game = table->games[index];
	LinkedList_t *head = table->overflow_buckets[index];

	// Provide only non-NULL values.
	if (game != NULL) {
		if (strcmp(game->name, name) == 0) {
			return game->port;
		}
		if (head == NULL) {
			return -1;
		}
		game = head->game;
		head = head->next;
	}
	return -1;
}

void
gt_delete(GameTable_t *table, char *name)
{
	// Deletes a game from the GameTable.
	uint32_t index = hash_function(name);
	game_t *game = table->games[index];
	LinkedList_t *head = table->overflow_buckets[index];

	if (game == NULL) {
		// Does not exist.
		return;
	} else {
		if ((head == NULL) && (strcmp(game->name, name) == 0)) {
			// Collision chain does not exist.
			// Remove the game.
			// Set table index to NULL.
			table->games[index] = NULL;
			free_game(game);
			table->count--;
			return;
		} else if (head != NULL) {
			// Collision chain exists.
			if (strcmp(game->name, name) == 0) {
				// Remove this game.
				// Set the head of the list as the new game.
				free_game(game);
				LinkedList_t *node = head;
				head = head->next;
				node->next = NULL;
				table->games[index] = create_game(node->game->name, node->game->port);
				free_linkedlist(node);
				table->overflow_buckets[index] = head;
				return;
			}

			LinkedList_t *curr = head;
			LinkedList_t *prev = NULL;

			while (curr) {
				if (strcmp(curr->game->name, name) == 0) {
					if (prev == NULL) {
						// First element of the chain.
						// Remove the chain.
						free_linkedlist(head);
						table->overflow_buckets[index] = NULL;
						return;
					} else {
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

void
print_search(GameTable_t *table, char *name)
{
	int port;
	if ((port = gt_search(table, name)) == NULL) {
		printf("Game \"%s\" does not exist\n", name);
	} else {
		printf("Game %s, Port %s\n", name, port);
	}
	return;
}

void
print_table(GameTable_t *table)
{
	printf("\nHash Table\n-------------------\n");
	for (int i = 0; i < table->size; i++) {
		if (table -> games[i]) {
			printf("Index %d: Game %s, Port %d, %d levels\n", i,
				table->games[i]->name, table->games[i]->port, table->games[i]->size);
		}
	}
	printf("-------------------\n\n");
}
