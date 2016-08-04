#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main(int argc, char** argv)
{
    FILE *stream;
    char *line = NULL;
    char *key, *value;
    size_t len = 0;
    ssize_t linelen;
    stream = fopen("input.cfg", "r");
    if (stream == NULL) exit(EXIT_FAILURE);
    while ((linelen = getline(&line, &len, stream)) != -1) {
        if (strchr(line,'[') != NULL) continue;
        else printf("Line of length %zu :\n", linelen);
        //printf("%s", line);
        key = strtok (line," =");
        printf("Key: %s\n",key);
        value = strtok (NULL, " =");
        printf("Value: %s\n",value);
    }
    free(line);
    fclose(stream);
    exit(EXIT_SUCCESS);
}
