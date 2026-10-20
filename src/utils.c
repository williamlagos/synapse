#include "utils.h"

#define MAX_BUFFER 256

uv_work_t req[FIB_UNTIL];

static const char* resolve_impulse_sensor(void)
{
    if (access("./libimpulse.so", R_OK) == 0) {
        return "./libimpulse.so";
    }
    if (access("./libimpulse.dylib", R_OK) == 0) {
        return "./libimpulse.dylib";
    }
    if (access("/usr/local/lib/libimpulse.so", R_OK) == 0) {
        return "/usr/local/lib/libimpulse.so";
    }
    return "/usr/lib/libimpulse.so";
}

char** load_buffer(const char* filename, int* cnt)
{
    int count = 0;
    char buffer[MAX_BUFFER];
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        *cnt = 0;
        return NULL;
    }
    char **lines = (char**) calloc(MAX_BUFFER, sizeof(char*));
    for (count = 0; count < MAX_BUFFER && fgets(buffer, sizeof(buffer), f); count++) {
        lines[count] = strdup(buffer);
    }
    fclose(f);
    *cnt = count;
    return lines;
}

static void split_cmd_and_args(const char* value, char** out_cmd, char** out_args)
{
    while (*value == ' ' || *value == '\t') {
        value++;
    }
    const char* space = strchr(value, ' ');
    if (space == NULL) {
        *out_cmd = strdup(value);
        *out_args = strdup("");
    } else {
        size_t cmd_len = (size_t) (space - value);
        *out_cmd = (char*) malloc(cmd_len + 1);
        memcpy(*out_cmd, value, cmd_len);
        (*out_cmd)[cmd_len] = '\0';
        while (*space == ' ') {
            space++;
        }
        *out_args = strdup(space);
    }
}

void load_config(const char* filename, context_t* ctx_array)
{
    int max = 0;
    char **lines = load_buffer(filename, &max);
    if (lines == NULL) {
        fprintf(stderr, "Failed to open configuration file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int ctx_idx = -1;
    config_t* r = NULL;

    for (int l = 0; l < max; l++) {
        char* line = lines[l];
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0 || line[0] == '#') {
            free(lines[l]);
            continue;
        }

        char* open_br = strchr(line, '[');
        char* close_br = strchr(line, ']');
        if (open_br != NULL && close_br != NULL && close_br > open_br) {
            ctx_idx++;
            r = &(ctx_array[ctx_idx].config);
            memset(r, 0, sizeof(config_t));
            size_t sec_len = (size_t) (close_br - open_br - 1);
            if (sec_len >= sizeof(r->name)) {
                sec_len = sizeof(r->name) - 1;
            }
            memcpy(r->name, open_br + 1, sec_len);
            r->name[sec_len] = '\0';
            free(lines[l]);
            continue;
        }

        if (r == NULL) {
            free(lines[l]);
            continue;
        }

        char* eq = strchr(line, '=');
        if (eq == NULL) {
            free(lines[l]);
            continue;
        }
        *eq = '\0';
        const char* key = line;
        const char* value = eq + 1;

        if (strcmp(key, "command") == 0) {
            split_cmd_and_args(value, &r->command, &r->command_args);
        } else if (strcmp(key, "sensors") == 0) {
            int count = 0;
            char* dup = strdup(value);
            char* v = strtok(dup, ",");
            while (v != NULL && count < MAX_SENSORS) {
                r->sensors[count++] = strdup(v);
                v = strtok(NULL, ",");
            }
            free(dup);
            r->n_sensors = count;
        } else if (strcmp(key, "state") == 0) {
            split_cmd_and_args(value, &r->state, &r->state_args);
            r->sensors[0] = strdup(resolve_impulse_sensor());
            r->n_sensors = 1;
        }

        free(lines[l]);
    }

    free(lines);
    active_contexts = ctx_idx + 1;
}

void start_fib(uv_work_t *work_req)
{
    int n = *(int *) work_req->data;
    if (random() % 2) uv_sleep(1);
    else uv_sleep(3);
    int fib = 0;
    int a = 0;
    int b = 1;
    for (int i = 0; i < n; i++) {
        fib = a + b;
        a = b;
        b = fib;
    }
    fprintf(stderr, "%dth fibonacci is %d\n", n, fib);
}

void stop_fib(uv_work_t *work_req, int status)
{
    (void) status;
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *) work_req->data);
}

void fibonacci_cycle(uv_loop_t* l)
{
    int* data = calloc(FIB_UNTIL, sizeof(int));
    for (int i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        req[i].data = (void *) &data[i];
        uv_queue_work(l, &req[i], start_fib, stop_fib);
    }
}
