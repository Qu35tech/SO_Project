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
#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
    // for each filter
    // information needed : Name; Location; Limit; Current;
    char* filters_name[NUM_FILTERS];
    char* filters_location[NUM_FILTERS];
    int filters_current[NUM_FILTERS];
    int filters_limit[NUM_FILTERS];

    // processing arguments
    // argv[1] -> config_filename
    // argv[2] -> filters_folder

    
    // parse configs
    int fd = open(argv[1], O_RDONLY);
    char* buf = malloc(sizeof(char)*BUF_SIZE);
    ssize_t fsize = read(fd, buf, BUF_SIZE);

    for(int i=0;i<NUM_FILTERS;i++) {
        char* filter = strsep(&buf, "\n");

        // name
        char* name = strsep(&filter, " ");
        filters_name[i] = strdup(name);

        // location
        char* location = strsep(&filter, " ");
        filters_location[i] = strdup(location);

        // limit
        char* limit = strsep(&filter, " ");
        filters_limit[i] = atoi(limit);

        // current
        filters_current[i] = 0;
    }
    free(buf);
    close(fd);

    // check parser
    //printf("%s %s %d/%d\n", filters_name[4], filters_location[4], filters_current[4], filters_limit[4]);

    //fill path on filters' array
    char* filters_folder = argv[2];
    strcat(filters_folder, "/");
    for(int i=0; i<NUM_FILTERS; i++) {
        char aux[128]; strcpy(aux, filters_folder);
        strcat(aux, filters_location[i]);
        filters_location[i] = strdup(aux);
    }



    // free alocated memory
    for(int i=0;i<NUM_FILTERS;i++) free(filters_location[i]);
    for(int i=0;i<NUM_FILTERS;i++) free(filters_name[i]);

    return 0;
}