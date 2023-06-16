#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define NUM_GAMES				12
#define NUM_LEVELS				183

#define BANDIT_MAX		34
#define NATAS_MAX			34
#define LEVIATHAN_MAX		7
#define KRYPTON_MAX		7
#define NARNIA_MAX		9
#define BEHEMOTH_MAX		8
#define UTUMNO_MAX		8
#define MAZE_MAX			9
#define VORTEX_MAX		27
#define MANPAGE_MAX		7
#define DRIFTER_MAX		15
#define FORMULAONE_MAX	6
#define BANDIT_PORT				2220
#define LEVIATHAN_PORT			2223
#define KRYPTON_PORT			2231
#define NARNIA_PORT				2226
#define BEHEMOTH_PORT			2221
#define UTUMNO_PORT				2227
#define MAZE_PORT				2225
#define VORTEX_PORT				2228
#define MANPAGE_PORT			2224
#define DRIFTER_PORT			2230
#define FORMULAONE_PORT			2223

#define LVLNAME_MAX				24
#define LVLADDR_MAX				44
#define LVLPW_MAX				24
#define CAPACITY 				24 // Size of the GameTable.
#define GAME_DATA				"data/gamedata.cvs"
#define LEVEL_DATA				"data/leveldata.cvs"

typedef struct GameTable_t {
	game_t		**games;
	int 		  size;
} GameTable_t;

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
	int 	 	is_completed;
	level_t	  **levels;
} game_t;

typedef struct savedlevel_t {
	char		name[LVLNAME_MAX];
	char		pw[LVLPW_MAX];
	int 	 	is_completed;
} savedlevel_t;

const game_t savedgames[NUM_GAMES] = {
	{"bandit", 		BANDIT_PORT, 	BANDIT_MAX, 		0, NULL},
	{"natas", 		0, 				NATAS_MAX, 		0, NULL},
	{"leviathan", 	LEVIATHAN_PORT, LEVIATHAN_MAX, 	0, NULL},
	{"krypton", 	KRYPTON_PORT, 	KRYPTON_MAX, 		0, NULL},
	{"narnia", 		NARNIA_PORT, 	NARNIA_MAX, 		0, NULL},
	{"behemoth", 	BEHEMOTH_PORT, 	BEHEMOTH_MAX, 	0, NULL},
	{"utumno", 		UTUMNO_PORT, 	UTUMNO_MAX, 		0, NULL},
	{"maze", 		MAZE_PORT, 		MAZE_MAX, 		0, NULL},
	{"vortex", 		VORTEX_PORT, 	VORTEX_MAX, 		0, NULL},
	{"manpage", 	MANPAGE_PORT, 	MANPAGE_MAX, 		0, NULL},
	{"drifter", 	DRIFTER_PORT, 	DRIFTER_MAX, 		0, NULL},
	{"formulaone", 	FORMULAONE_PORT, FORMULAONE_MAX, 	0, NULL}
};

const savedlevel_t saved_bandit[BANDIT_MAX] = {
	{"bandit0","",0},
	{"bandit1","",0},
	{"bandit2","",0},
	{"bandit3","",0},
	{"bandit4","",0},
	{"bandit5","",0},
	{"bandit6","",0},
	{"bandit7","",0},
	{"bandit8","",0},
	{"bandit9","",0},
	{"bandit10","",0},
	{"bandit11","",0},
	{"bandit12","",0},
	{"bandit13","",0},
	{"bandit14","",0},
	{"bandit15","",0},
	{"bandit16","",0},
	{"bandit17","",0},
	{"bandit18","",0},
	{"bandit19","",0},
	{"bandit20","",0},
	{"bandit21","",0},
	{"bandit22","",0},
	{"bandit23","",0},
	{"bandit24","",0},
	{"bandit25","",0},
	{"bandit26","",0},
	{"bandit27","",0},
	{"bandit28","",0},
	{"bandit29","",0},
	{"bandit30","",0},
	{"bandit31","",0},
	{"bandit32","",0},
	{"bandit33","",0},
	{"bandit34","",0}
};

const savedlevel_t saved_natas[NATAS_MAX] = {
	{"natas0","",0},
	{"natas1","",0},
	{"natas2","",0},
	{"natas3","",0},
	{"natas4","",0},
	{"natas5","",0},
	{"natas6","",0},
	{"natas7","",0},
	{"natas8","",0},
	{"natas9","",0},
	{"natas10","",0},
	{"natas11","",0},
	{"natas12","",0},
	{"natas13","",0},
	{"natas14","",0},
	{"natas15","",0},
	{"natas16","",0},
	{"natas17","",0},
	{"natas18","",0},
	{"natas19","",0},
	{"natas20","",0},
	{"natas21","",0},
	{"natas22","",0},
	{"natas23","",0},
	{"natas24","",0},
	{"natas25","",0},
	{"natas26","",0},
	{"natas27","",0},
	{"natas28","",0},
	{"natas29","",0},
	{"natas30","",0},
	{"natas31","",0},
	{"natas32","",0},
	{"natas33","",0},
	{"natas34","",0}
};

const savedlevel_t saved_leviathan[LEVIATHAN_MAX] = {
	{"leviathan0","",0},
	{"leviathan1","",0},
	{"leviathan2","",0},
	{"leviathan3","",0},
	{"leviathan4","",0},
	{"leviathan5","",0},
	{"leviathan6","",0},
	{"leviathan7","",0}
};

const savedlevel_t saved_krypton[KRYPTON_MAX] = {
	{"krypton0","",0},
	{"krypton1","",0},
	{"krypton2","",0},
	{"krypton3","",0},
	{"krypton4","",0},
	{"krypton5","",0},
	{"krypton6","",0},
	{"krypton7","",0}
};

const savedlevel_t saved_narnia[NARNIA_MAX] = {
	{"narnia0","",0},
	{"narnia1","",0},
	{"narnia2","",0},
	{"narnia3","",0},
	{"narnia4","",0},
	{"narnia5","",0},
	{"narnia6","",0},
	{"narnia7","",0},
	{"narnia8","",0},
	{"narnia9","",0}
};

const savedlevel_t saved_behemoth[BEHEMOTH_MAX] = {
	{"behemoth0","",0},
	{"behemoth1","",0},
	{"behemoth2","",0},
	{"behemoth3","",0},
	{"behemoth4","",0},
	{"behemoth5","",0},
	{"behemoth6","",0},
	{"behemoth7","",0},
	{"behemoth8","",0}
};

const savedlevel_t saved_utumno[UTUMNO_MAX] = {
	{"utumno0","",0},
	{"utumno1","",0},
	{"utumno2","",0},
	{"utumno3","",0},
	{"utumno4","",0},
	{"utumno5","",0},
	{"utumno6","",0},
	{"utumno7","",0},
	{"utumno8","",0}
};

const savedlevel_t saved_maze[MAZE_MAX] = {
	{"maze0","",0},
	{"maze1","",0},
	{"maze2","",0},
	{"maze3","",0},
	{"maze4","",0},
	{"maze5","",0},
	{"maze6","",0},
	{"maze7","",0},
	{"maze8","",0},
	{"maze9","",0}
};

const savedlevel_t saved_vortex[VORTEX_MAX] = {
	{"vortex0","",0},
	{"vortex1","",0},
	{"vortex2","",0},
	{"vortex3","",0},
	{"vortex4","",0},
	{"vortex5","",0},
	{"vortex6","",0},
	{"vortex7","",0},
	{"vortex8","",0},
	{"vortex9","",0},
	{"vortex10","",0},
	{"vortex11","",0},
	{"vortex12","",0},
	{"vortex13","",0},
	{"vortex14","",0},
	{"vortex15","",0},
	{"vortex16","",0},
	{"vortex17","",0},
	{"vortex18","",0},
	{"vortex19","",0},
	{"vortex20","",0},
	{"vortex21","",0},
	{"vortex22","",0},
	{"vortex23","",0},
	{"vortex24","",0},
	{"vortex25","",0},
	{"vortex26","",0},
	{"vortex27","",0}
};

const savedlevel_t saved_manpage[MANPAGE_MAX] = {
	{"manpage0","",0},
	{"manpage1","",0},
	{"manpage2","",0},
	{"manpage3","",0},
	{"manpage4","",0},
	{"manpage5","",0},
	{"manpage6","",0},
	{"manpage7","",0}
};

const savedlevel_t saved_drifter[DRIFTER_MAX] = {
	{"drifter0","",0},
	{"drifter1","",0},
	{"drifter2","",0},
	{"drifter3","",0},
	{"drifter4","",0},
	{"drifter5","",0},
	{"drifter6","",0},
	{"drifter7","",0},
	{"drifter8","",0},
	{"drifter9","",0},
	{"drifter10","",0},
	{"drifter11","",0},
	{"drifter12","",0},
	{"drifter13","",0},
	{"drifter14","",0},
	{"drifter15","",0}
};

const savedlevel_t saved_formulaone[FORMULAONE_MAX] = {
	{"formulaone0","",0},
	{"formulaone1","",0},
	{"formulaone2","",0},
	{"formulaone3","",0},
	{"formulaone4","",0},
	{"formulaone5","",0},
	{"formulaone6","",0}
};

GameTable_t   *create_GameTable(int size);
void        *create_game(char *name, int size);
void       *create_level(char *name, game_t *game);

void load_savedgame(game_t *game, int gameidx);
void load_savedlevel(level_t *level, int levelidx, int gameidx);

void free_game(game_t *game);
void free_table(GameTable_t *table);

uint32_t hash_function(char *name);
int      GT_search(GameTable_t *table, char *name);
void     GT_delete(GameTable_t *table, char *name);
void 	*GT_insert_level(LinkedList_t *list, game_t *game);

void print_search(GameTable_t *table, char *name);
void print_table(GameTable_t *table);


int
main(int argc, char **argv)
{
	GameTable_t *gt = create_GameTable(NUM_GAMES);

	print_search(gt, (char *) "1");
	print_search(gt, (char *) "2");
	print_search(gt, (char *) "3");
	print_search(gt, (char *) "Hel");
	print_search(gt, (char *) "Cau"); // Collision!
	print_table(gt);

	GT_delete(gt, (char *) "1");
	GT_delete(gt, (char *) "Cau");

	print_table(gt);
	free_table(gt);
	return 0;
}

GameTable_t *
create_GameTable(int size)
{
	// Creates a new GameTable.
	GameTable_t *table = (GameTable_t *) malloc(sizeof(GameTable_t));
	table->size = size;
	table->games = (game_t **) calloc(table->size, sizeof(game_t *));

	for (int i = 0; i < table->size; i++) {
		load_savedgame(table, i);
	}

	return table;
}

void
load_savedgame(GameTable_t *table, int gameidx)
{
	// Creates a new game node and initialize it.
	game_t *game = (game_t *) malloc(sizeof(game_t));
	game->size = savedgames[gameidx].size;
	game->port = savedgames[gameidx].port;
	game->is_completed = savedgames[gameidx].is_completed;
	strncpy(game->name, savedgames[gameidx].name, LVLNAME_MAX);

	// Load levels belonging to this game node.
	game->levels = (level_t **) calloc(game->size, sizeof(level_t *));
	for (int l = 0; l < game->size; l++) {
		load_savedlevel(game, l, gameidx);
	}

	// Insert the game node into the GameTable.
	table->games[gameidx] = game;

	return;
}

void
load_savedlevel(game_t *game, int levelidx, int gameidx)
{
	// Create a new level node.
	level_t *level = (level_t *) malloc(sizeof(level_t));
	char *name = NULL;
	char *pw = NULL;

	switch (gameidx) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	case 11:
		break;
	default:
		fprintf(stderr, "invalid games array index");
		exit(EXIT_FAILURE);
	}

	strncpy(level->name, savedlevels->name, LVLNAME_MAX);
	strncpy(level->pw, savedlevel->pw, LVLPW_MAX);
	snprintf(level->ssh_addr, LVLADDR_MAX, "%s@%s.labs.overthewire.org", level->name, game->name);
	level->is_completed = savedlevel->is_completed;

	// Insert level into the game's levels array
	game->levels[levelidx] = level;
	
	return;
}

int
GT_search(GameTable_t *table, char *name)
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
GT_delete(GameTable_t *table, char *name)
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
create_list(void)
{
	// Allocates memory for a LinkedList_t pointer.
	LinkedList_t *list = (LinkedList_t *) malloc(sizeof(LinkedList_t));
	return list;
}

LinkedList_t *
GT_insert_level(LinkedList_t *list, game_t *game)
{
	// Inserts a level_t into a game_t item's levels array.
	if (!list) {
		LinkedList_t *head = create_list();
		head->game = game;
		head->next = NULL;
		list = head;
		return list;
	} else if (list->next == NULL) {
		LinkedList_t *node = create_list();
		node->game = game;
		node->next = NULL;
		list->next = node;
		return list;
	}

	LinkedList_t *temp = list;
	temp = temp->next;
	while (temp->next) {
		temp = temp->next;
	}

	LinkedList_t *node = create_list();
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

level_t *
create_level(char *name, game_t *game)
{
	// Creates a pointer to a new level_t item.
	level_t *level = (level_t *) malloc(sizeof(level_t));
	
	level->is_completed = 0;
	strncpy(level->name, name, LVLNAME_MAX);
	strncpy(level->pw, "", LVLPW_MAX);
	snprintf(level->ssh_addr, LVLADDR_MAX, "%s@%s.labs.overthewire.org", level->name, game->name);

	return level;
}

game_t *
create_game(char *name, int size)
{
	// Creates a pointer to a new game_t item.
	game_t *game = (game_t *) malloc(sizeof(game_t));
	
	game->count = 0;
	game->size  = size;
	game->port  = 0;
	game->is_completed = 0;
	strncpy(game->name, name, LVLNAME_MAX);
	game->levels = (level_t **) calloc(game->size, sizeof(level_t *));
	
	for (int i = 0; i < game->size; i++) {
		game->levels[i] = NULL;
	}

	return game;
}

void
free_game(game_t *game)
{
	// Frees the games.
	level_t *level = NULL;
	for (int i = 0; i < game->size; i++) {
		level = game->levels[i];
		if (level != NULL) {
			// Frees the level.
			free(level);
		}
	}

	free(game->levels);
	free(game);
}

void
free_table(GameTable_t *table)
{
	// Frees the GameTable.
	game_t *game = NULL;
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
		head = create_list();
		head->game = game;
		table->overflow_buckets[index] = head;
		return;
	} else {
		// Insert to the list.
		table->overflow_buckets[index] = GT_insert_level(head, game);
		return;
	}
}

void
print_search(GameTable_t *table, char *name)
{
	int port;
	if ((port = GT_search(table, name)) == NULL) {
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
