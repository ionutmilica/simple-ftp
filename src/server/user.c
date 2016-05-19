#include "user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates a new user manager struct and populate it with users from a db
 *
 * @param filename const char*
 * @return user_manager*
 */
user_manager* user_manager_new(const char* filename) {
	FILE* db = fopen(filename, "r");

	if (db == NULL) {
		printf("Cannot open database!\n");
		exit(EXIT_FAILURE);
	}

	user_manager* mgr = malloc(sizeof(user_manager));
	mgr->length = 0;

	while (mgr->length < MAX_USERS && fscanf(db, "%s %s %d %s", mgr->users[mgr->length].name, mgr->users[mgr->length].pass,
              &mgr->users[mgr->length].perm, mgr->users[mgr->length].path) == 4) {
		mgr->length++;
	}

	fclose(db);
	return mgr;
}

/**
 * Display all the users loaded from the database
 *
 * @param mgr user_manager*
 * @return void
 */
void user_manager_display(user_manager* mgr)
{
	int i;
	for (i = 0; i < mgr->length; i++) {
		printf("User: %s, Pass: %s, Perm: %d, Path: %s\n", mgr->users[i].name, mgr->users[i].pass, mgr->users[i].perm,  mgr->users[i].path);
	}
}

/**
 * Dispose the user manager object
 *
 * @param mgr user_manager*
 * @return void
 */
void user_manager_destroy(user_manager* mgr)
{
	if (mgr != NULL) {
		free(mgr);
	}
}

/**
 * Check if user exists in the database
 *
 * @param mgr user_manager*
 * @param name const char*
 * @return void
 */
int user_manager_find(user_manager* mgr, const char* name)
{
	int i = 0;
	for (i = 0; i < mgr->length; i++) {
		if (strcmp(mgr->users[i].name, name) == 0) {
			return 1;
		}
	}

	return 0;
}

/**
 * Check if user with password exist in the database
 *
 * @param mgr user_manager*
 * @param name const char*
 * @return void
 */
int user_manager_find_with_pass(user_manager* mgr, const char* name, const char* pass)
{
	int i = 0;
	for (i = 0; i < mgr->length; i++) {
		if (!strcmp(mgr->users[i].name, name) && !strcmp(mgr->users[i].pass, pass)) {
			return 1;
		}
	}

	return 0;
}
