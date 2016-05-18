#ifndef USER_H
#define USER_H

typedef struct user {
	char username[100];
	char password[100];
	char workspace[512];
	int permission;
} user;

typedef struct users {
	user users[100];
	int length;
} users;

void load_users(const char* filename);

#endif