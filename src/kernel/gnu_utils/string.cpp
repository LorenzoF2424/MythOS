#include "string.h"


void strinit(char *str, size_t size) {

    for (size_t i=0; i<size-1;i++)
        str[i]='\0';

}

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

    while (*last_pos != '\0') {
        if (*last_pos == delimiter) {
            *last_pos = '\0'; 
            last_pos++;       
            return token_start;
        }
        last_pos++;
    }

    last_pos = NULL; 
    return token_start;
}


int atoi(const char *str) {
    int res = 0;
    int8_t sign = 1;
    size_t i = 0;

    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r') {
        i++;
    }

    if (str[i] == '-' || str[i] == '+') {
        if (str[i] == '-') {
            sign = -1;
        }
        i++;
    }

    
    while (str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    return sign * res;
}

uint32_t htoi(const char *str) {
    uint32_t res = 0;
    int i = 0;

    while (str[i] == ' ' || str[i] == '\t') {
        i++;
    }

    if (str[i] == '0' && (str[i+1] == 'x' || str[i+1] == 'X')) {
        i += 2;
    }

    while (str[i] != '\0') {
        uint8_t val = 0;

        if (str[i] >= '0' && str[i] <= '9') {
            val = str[i] - '0';
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            val = str[i] - 'A' + 10; 
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            val = str[i] - 'a' + 10; 
        } else {
            break; 
        }

        
        res = (res << 4) | val;
        i++;
    }

    return res;
}






















