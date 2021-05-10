/* 
* Aurras Server File
*/

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_FILTERS 5

int main(int argc, char* argv[]) {
    // processing arguments
    // argv[1] -> config_filename
    // argv[2] -> filters_folder

    char* config_filename = argv[1];
    char* filters[NUM_FILTERS];
    // parse configs

    // initializing filter array just for testing
    for(int i=0; i<NUM_FILTERS; i++) filters[i]="filter";

    // testing purposes
    printf("Reading configs from file : %s\n", config_filename);

    char* filters_folder = argv[2];
    strcat(filters_folder, "/");

    //fulfill path on filters' array
    for(int i=0; i<NUM_FILTERS; i++) {
        char aux[128]; strcpy(aux, filters_folder);
        strcat(aux, filters[i]);
        filters[i] = strdup(aux);
    }
    

    // just for testing purposes
    printf("Location of the first filter : %s\n", filters[4]);

    return 0;
}