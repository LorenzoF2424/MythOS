#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


size_t strlen(const char *str) {
    
    size_t len=0;
    while (str[len]) len++;
    return len;
}


void strcpy(char *dest, char *src) {


    for (size_t i=0;i<strlen(src);i++)
        dest[i]=src[i];

}