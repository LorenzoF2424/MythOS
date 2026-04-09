#include "gnu_utils/string.h"


void strinit(char *str, size_t size) {

    for (size_t i=0; i<size-1;i++)
        str[i]='\0';

}

size_t strlen(const char *str) {
    
    size_t len=0;
    while (str[len]) len++;
    return len;
}

void strinit(char *str) {

    size_t size = strlen(str);
    for (size_t i=0; i<size-1;i++)
        str[i]='\0';

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

char* strtok(char* str, const char* delim) {
    static char* last_pos = nullptr;

    // Start from the new string if provided
    if (str != nullptr) {
        last_pos = str;
    }

    // Early return: nothing left to parse
    if (last_pos == nullptr) return nullptr;

    // 1. Skip leading delimiters (The Standard C behavior)
    while (*last_pos != '\0') {
        bool is_delim = false;
        
        // Check if the current character matches any of the delimiters
        for (int i = 0; delim[i] != '\0'; i++) {
            if (*last_pos == delim[i]) {
                is_delim = true;
                break;
            }
        }
        
        // Early break: we found a valid character that is NOT a delimiter!
        if (!is_delim) break; 
        
        last_pos++;
    }

    // Early return: reached the end while skipping delimiters
    if (*last_pos == '\0') {
        last_pos = nullptr;
        return nullptr;
    }

    // 2. Find the end of the current token
    char* token_start = last_pos;
    while (*last_pos != '\0') {
        bool is_delim = false;
        
        for (int i = 0; delim[i] != '\0'; i++) {
            if (*last_pos == delim[i]) {
                is_delim = true;
                break;
            }
        }

        if (is_delim) {
            // Terminate the token and advance the pointer for the next call
            *last_pos = '\0'; 
            last_pos++;       
            return token_start;
        }
        last_pos++;
    }

    // 3. Reached the end of the entire string (last token)
    last_pos = nullptr;
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


extern "C" void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if (d < s) {
        // Copy forward
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copy backward to handle overlap safely
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}


extern "C" void* memset(void* dest, int c, size_t n) {
    // Early return: safety check to prevent writing to null pointers
    if (dest == nullptr || n == 0) return dest;

    uint8_t* ptr = (uint8_t*)dest;
    uint8_t value = (uint8_t)c;

    for (size_t i = 0; i < n; i++) {
        ptr[i] = value;
    }

    return dest;
}
















