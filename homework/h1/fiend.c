#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#include <time.h>

typedef struct {
    int follow_symlinks;    // 0 for -P, 1 for -L
    bool depth_first;       // true for postorder traversal
    int max_depth;          // max depth for traversal
    char *name_pattern;     // pattern for file names
    char *newer_than;       // compare file modification times
    bool print;             // true to print file names;
    char *exec_cmd;         // command to execute on each file
    bool exec;              // true if exec command is provided
} Options;

// Function prototypes
void parse_expression(int argc, char *argv[], int start, Options *opts);
void process_directory(const char *path, Options *opts, int depth);
bool evaluate_file(const char *path, const Options *opts);

int main(int argc, char *argv[]) {
    Options opts = {
        false,  // follow_symlinks
        false,  // depth_first
        -1,     // max_depth
        NULL,   // name_pattern
        NULL,   // newer_than
        false,  // print
        NULL,   // exec_cmd
        false   // exec
    };
    int i = 1;

    // parse -P and -L options
    for (; i < argc && (strcmp(argv[i], "-P") == 0 || strcmp(argv[i], "-L") == 0); i++) {
        opts.follow_symlinks = strcmp(argv[i], "-L") == 0;
    }

    // identify filenames and start of the expression
    int expr_start = i;
    for (; expr_start < argc && argv[expr_start][0] != '-'; expr_start++) {
        // filenames are processed after determining the start of the expression
    }

    if (expr_start == i) {
        // no filenames specified, use current directory
        parse_expression(argc, argv, expr_start, &opts);
        process_directory(".", &opts, 0);
    } else {
        // parse the expression starting from its identified position
        parse_expression(argc, argv, expr_start, &opts);

        // DEBUG print options
        printf("OPTIONS\n");
        printf("----------------------\n");
        printf("follow_symlinks: %d\n", opts.follow_symlinks);
        printf("depth_first: %d\n", opts.depth_first);
        printf("max_depth: %d\n", opts.max_depth);
        printf("name_patern: %s\n", opts.name_pattern);
        printf("newer_than: %s\n", opts.newer_than);
        printf("print: %d\n", opts.print);
        printf("exec_cmd: %s\n", opts.exec_cmd);
        printf("exec: %d\n", opts.exec);
        printf("----------------------\n\n");

        // process each directory specified
        for (; i < expr_start; i++) {
            size_t len = strlen(argv[i]);
            while (argv[i][len-1] == '/')
                argv[i][--len] = '\0';
            process_directory(argv[i], &opts, 0);
        }
    }

    return 0;
}

void parse_expression(int argc, char *argv[], int start, Options *opts) {
    for (int i = start; i < argc; i++) {
        if (strcmp(argv[i], "-depth") == 0) {
            opts->depth_first = true;
        } else if (strcmp(argv[i], "-maxdepth") == 0 && i + 1 < argc) {
            opts->max_depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-name") == 0 && i + 1 < argc) {
            opts->name_pattern = argv[++i];
        } else if (strcmp(argv[i], "-newer") == 0 && i + 1 < argc) {
            opts->newer_than = argv[++i];
        } else if (strcmp(argv[i], "-print") == 0) {
            opts->print = true;
        } else if (strcmp(argv[i], "-exec") == 0 && i + 1 < argc) {
            opts->exec_cmd = argv[++i];
            opts->exec = true;
            if (argv[i][strlen(argv[i]) - 1] == ';') {
                argv[i][strlen(argv[i]) - 1] = '\0'; // Remove the trailing ';' for exec command
            } else {
                fprintf(stderr, "Error: -exec command must end with ';'\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void process_directory(const char *path, Options *opts, int depth) {
    if (opts->max_depth != -1 && depth > opts->max_depth) {
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];
    struct stat statbuf;

    if (opts->depth_first) {
        // postorder traversal
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            if (opts->follow_symlinks)
                stat(full_path, &statbuf);
            else
                lstat(full_path, &statbuf);

            if (S_ISDIR(statbuf.st_mode)) {
                process_directory(full_path, opts, depth + 1);
            }
        }

        // Rewind and evaluate each entry, including files this time
        rewinddir(dir);
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            if (opts->follow_symlinks)
                stat(full_path, &statbuf);
            else
                lstat(full_path, &statbuf);

            if (!S_ISDIR(statbuf.st_mode)) { // Evaluate files now
                evaluate_file(full_path, opts);
            }
        }
    } else {
        // Pre-order traversal: Handle each entry as it comes
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            if (opts->follow_symlinks)
                stat(full_path, &statbuf);
            else
                lstat(full_path, &statbuf);

            bool isDir = S_ISDIR(statbuf.st_mode);

            // Evaluate before diving into directories
            if (evaluate_file(full_path, opts) || isDir) {
                process_directory(full_path, opts, depth + 1);
            }
        }
    }

    closedir(dir);
}

bool evaluate_file(const char *path, const Options *opts) {
    struct stat file_stat, ref_stat;
    bool eval_result = true;

    if (opts->name_pattern && fnmatch(opts->name_pattern, strrchr(path, '/') + 1, 0) != 0) {
        eval_result = false;
    }

    if (opts->newer_than && stat(opts->newer_than, &ref_stat) == 0) {
        stat(path, &file_stat);
        if (difftime(file_stat.st_mtime, ref_stat.st_mtime) <= 0) {
            eval_result = false;
        }
    }

    if (opts->print && eval_result) {
        printf("%s\n", path);
    }

    if (opts->exec && eval_result) {
        char cmd[PATH_MAX + 256];
        snprintf(cmd, sizeof(cmd), "%s %s", opts->exec_cmd, path);
        size_t cmd_len = strlen(cmd);
        if (cmd[cmd_len - 1] == ';') {
            cmd[cmd_len - 1] = '\0';  // Remove the semicolon for execution
        }
        if (system(cmd) != 0) {
            eval_result = false;
        }
    }

    return eval_result;
}