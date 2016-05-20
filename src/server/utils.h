#ifndef UTILS_H
#define UTILS_H

int create_socket(int port);
int create_named_socket(const char* path);
void perm(int perm, char* str_perm);

#endif