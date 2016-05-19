#ifndef USER_H
#define USER_H

#define MAX_USERS 100

typedef struct user {
	char name[100];
	char pass[100];
	char path[512];
	int perm;
} user;

typedef struct user_manager {
	user users[MAX_USERS];
	int length;
} user_manager;

user_manager* user_manager_new(const char* filename);
void user_manager_destroy(user_manager* mgr);
void user_manager_display(user_manager* mgr);
int user_manager_find(user_manager* mgr, const char* name);
int user_manager_find_with_pass(user_manager* mgr, const char* name, const char* pass);

#endif