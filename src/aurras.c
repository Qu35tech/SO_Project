/* 
* Aurras Client File
*/

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // decide what to do based on argc
    if (argc==1) {
        // usage information
        printf(">> ./aurras status\n");
        printf(">> ./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...\n");
    } else if (argc==2) {
        // status
        printf(">> status\n");
    } else {
        printf(">> make request\n");
    }
    return 0;
}