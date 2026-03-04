#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void strinit(char *str, size_t size);

size_t strlen(const char *str);

void strcpy(char *dest, const char *src);

void strncpy(char *dest, const char *src, size_t max);

int32_t strcmp(const char* s1, const char* s2);

char* strtok(char* str, const char delimiter);

int atoi(const char *str);

uint32_t htoi(const char *str);

#endif