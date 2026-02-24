#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


size_t strlen(const char *str) {
    
    size_t len=0;
    while (str[len]) len++;
    return len;
}


void strcpy(char *dest, const char *src) {
    size_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void strncpy(char *dest, const char *src, size_t max) {
    size_t i = 0;
    while (i < max - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int32_t strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    // Restituisce la differenza tra i primi due caratteri diversi incontrati
    // Se sono uguali, la differenza sarà 0.
    // Se s1 è minore, il risultato sarà negativo.
    // Se s1 è maggiore, il risultato sarà positivo.
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strtok(char* str, const char delimiter) {
    static char* last_pos = NULL;

    
    if (str != NULL) {
        last_pos = str;
    }

    if (last_pos == NULL || *last_pos == '\0') {
        return NULL;
    }

    char* token_start = last_pos;

    // Cerchiamo il delimitatore
    while (*last_pos != '\0') {
        if (*last_pos == delimiter) {
            *last_pos = '\0'; 
            last_pos++;       
            return token_start;
        }
        last_pos++;
    }

    // Se arriviamo qui, abbiamo trovato l'ultimo pezzo della stringa
    last_pos = NULL; 
    return token_start;
}

#endif