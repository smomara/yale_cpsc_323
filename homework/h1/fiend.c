// fiend.c - depth-first directory traversal utility

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <fnmatch.h>

#define MAX_PATH 4096

typedef struct node {
    char *name;
    int depth;
    struct node *next;
} Node;

typedef enum {AND, OR} Op;

typedef struct expr {
    char *arg;
    void (*action)(char*);
    int (*test)(char*, char*);
    Op operator;
    struct expr *next;
} Expr;

// Global variables
int followLinks = 0;
int maxDepth = INT_MAX;
int postOrder = 0;

// Print error message to stderr and continue execution
void errMsg(char *msg) {
    fprintf(stderr, "fiend: %s\n", msg);
}

// Print error message to stderr and exit with error
void fatalError(char *msg) {
    errMsg(msg);
    exit(1);
}

// Callback for -print action
void printFile(char *path) {
    printf("%s\n", path);
}

// Callback for -exec action
void execAction(char *cmd) {
    if (system(cmd) != 0) {
        errMsg("exec command failed");
    }
}

// Return 1 if file name matches specified pattern, 0 otherwise
int nameTest(char *path, char *pattern) {
    char *name = strrchr(path, '/');
    if (name == NULL) name = path;
    else name++;

    return fnmatch(pattern, name, 0) == 0;
}

// Return 1 if file is newer than specified reference file, 0 otherwise
int newerTest(char *path, char *refFile) {
    struct stat fileStat, refStat;

    if (stat(path, &fileStat) != 0) {
        errMsg("cannot stat file");
        return 0;
    }

    if (stat(refFile, &refStat) != 0) {
        fatalError("cannot stat reference file");
    }

    return (fileStat.st_mtim.tv_sec > refStat.st_mtim.tv_sec) ||
           (fileStat.st_mtim.tv_sec == refStat.st_mtim.tv_sec && 
            fileStat.st_mtim.tv_nsec > refStat.st_mtim.tv_nsec);
}

// Parse command line expression into Expr linked list
Expr *parseExpression(char **argv, int *index) {
    Expr *head = NULL, *current = NULL;
    Op defaultOp = AND;

    while (argv[*index] != NULL) {
        char *arg = argv[*index];

        if (strcmp(arg, "-o") == 0) {
            defaultOp = OR;
        } else if (strcmp(arg, "-a") == 0) {
            defaultOp = AND;
        } else {
            Expr *expr = malloc(sizeof(Expr));
            expr->arg = NULL;
            expr->action = NULL;
            expr->test = NULL;
            expr->operator = defaultOp;
            expr->next = NULL;

            if (strcmp(arg, "-print") == 0) {
                expr->action = printFile;
            } else if (strncmp(arg, "-exec", 5) == 0) {
                int len = strlen(arg);
                if (len < 7 || strcmp(arg+len-2, "\\;") != 0) {
                    fatalError("invalid -exec command");
                }
                arg[len-2] = '\0';
                expr->action = execAction;
                expr->arg = arg+6;
            } else if (strncmp(arg, "-name", 5) == 0) {
                if (argv[*index+1] != NULL && argv[*index+1][0] != '-') {
                    expr->test = nameTest;
                    expr->arg = argv[*index+1];
                    (*index)++;
                }
            } else if (strncmp(arg, "-newer", 6) == 0) {
                if (argv[*index+1] != NULL && argv[*index+1][0] != '-') {
                    expr->test = newerTest;
                    expr->arg = argv[*index+1];
                    (*index)++;
                }
            }

            if (head == NULL) {
                head = expr;
            } else {
                current->next = expr;
            }
            current = expr;
        }

        (*index)++;
    }

    return head;
}

// Push path onto stack
void pushStack(Node **stack, char *path, int depth) {
    Node *node = malloc(sizeof(Node));
    node->name = strdup(path);
    node->depth = depth;
    node->next = *stack;
    *stack = node;
}

// Pop path from stack
char *popStack(Node **stack) {
    if (*stack != NULL) {
        Node *node = *stack;
        *stack = node->next;
        char *path = node->name;
        free(node);
    }
    return NULL;
}

// Check if file/directory has already been visited
int isVisited(char *path, ino_t inode) {
    static Node *visited = NULL;
    Node *current = visited;

    while (current != NULL) {
        struct stat fileStat;

        if (stat(current->name, &fileStat) == 0 && fileStat.st_ino == inode) {
            errMsg("symbolic link loop deteced");
            return 1;
        }
        current = current->next;
    }

    Node *node = malloc(sizeof(Node));
    node->name = strdup(path);
    node->next = visited;
    visited = node;

    return 0;
}

// Evaluate expression for given file path
int evalExpr(char *path, Expr *expr) {
    while (expr != NULL) {
        int result;
        if (expr->action != NULL) {
            if (expr->action == execAction) {
                char *fullCmd = strdup(expr->arg);
                char *marker;
                while ((marker = strstr(fullCmd, "{}")) != NULL) {
                    *marker = '\0';
                    char *temp = malloc(strlen(fullCmd) + strlen(path) + strlen(marker+2) + 1);
                    sprintf(temp, "%s%s%s", fullCmd, path, marker+2);
                    free(fullCmd);
                    fullCmd = temp;
                }
                fflush(stdout);
                expr->action(fullCmd);
                free(fullCmd);
            } else {
                expr->action(path);
            }
            result = 1;
        } else if (expr->test != NULL) {
            result = expr->test(path, expr->arg);
        } else {
            result = 1;
        }

        if ((!result && expr->operator == AND) || (result && expr->operator == OR)) {
            return result;
        }
        expr = expr->next;
    }
    return 1;
}

// Recursively traverse directory tree
void traverseDir(char *path, int depth, Expr *expr) {
    struct stat fileStat;
    if (followLinks) {
        if (stat(path, &fileStat) != 0) {
            errMsg("cannot stat file");
            return;
        }
    } else {
        if (lstat(path, &fileStat) != 0) {
            errMsg("cannot stat file");
            return;
        }
    }

    // Perform pre-order actions for directories
    if (!postOrder && S_ISDIR(fileStat.st_mode)) {
        if (expr == NULL || evalExpr(path, expr)) {
            printFile(path);
        }
    }

    if (S_ISDIR(fileStat.st_mode)) {
        if (depth < maxDepth) {
            DIR *dir = opendir(path);
            if (dir == NULL) {
                errMsg("cannot open directory");
                return;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }

                char fullPath[MAX_PATH];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

                if (!followLinks && S_ISLNK(fileStat.st_mode)) {
                    continue;
                }

                if (followLinks && isVisited(fullPath, fileStat.st_ino)) {
                    continue;
                }

                traverseDir(fullPath, depth + 1, expr);
            }

            closedir(dir);
        }
    } else {
        // Perform actions for regular files
        if (expr == NULL || evalExpr(path, expr)) {
            printFile(path);
        }
    }

    // Perform post-order actions for directories
    if (postOrder && S_ISDIR(fileStat.st_mode)) {
        if (expr == NULL || evalExpr(path, expr)) {
            printFile(path);
        }
    }
}

// Main function - process command line arguments and initiate traversal
int main(int argc, char **argv) {

    char *rootDirs[argc];
    int numRoots = 0;

    int i = 1;
    while (i < argc && (strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "-P") == 0)) {
        followLinks = strcmp(argv[i++], "-P");
    }

    while (i < argc && argv[i][0] != '-') {
        rootDirs[numRoots++] = argv[i++];
    }

    if (numRoots == 0) {
        rootDirs[numRoots++] = ".";
    }

    while (i < argc) {
        char *arg = argv[i];

        if (strcmp(arg, "-depth") == 0) {
            postOrder = 1;
        } else if (strncmp(arg, "-maxdepth", 9) == 0) {
            if (argv[i+1] != NULL && argv[i+1][0] != '-') {
                char *depthStr = argv[i+1];
                char *endPtr;
                errno = 0;
                unsigned long depth = strtoul(depthStr, &endPtr, 10);
                if (errno != 0 || *endPtr != '\0' || depth > INT_MAX) {
                    fatalError("invalid max depth");
                }
                maxDepth = (int) depth;
                i++;
            }
        } else {
            break;
        }

        i++;
    }

    Expr *expr = parseExpression(argv, &i);

    for (int j = 0; j < numRoots; j++) {
        traverseDir(rootDirs[j], 0, expr);
    }

    return 0;
}