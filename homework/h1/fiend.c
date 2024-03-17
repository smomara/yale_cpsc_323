// fiend.c - depth-first directory traversal utility

// TODO:
// - fix -o and -a
// - fix -exec
// - fix symbolic link loop detection
// - fix combining multiple options not working
//   - ex) ./fiend -print -name *.md

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
#define TABLE_SIZE 1024

typedef struct node {
    char *name;
    int depth;
    struct node *next;
} Node;

typedef struct entry {
    ino_t inode;
    dev_t device;
    struct entry *next;
} Entry;

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
Entry *visitedTable[TABLE_SIZE] = {NULL};

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

// Execute command for -exec action
void execCommand(char *path, char *command) {
    char *fullCmd = strdup(command);
    char *marker;

    while ((marker = strstr(fullCmd, "{}"))) {
        *marker = '\0';
        char *temp = malloc(strlen(fullCmd) + strlen(path) + strlen(marker+2) + 1);
        sprintf(temp, "%s%s%s", fullCmd, path, marker+2);
        free(fullCmd);
        fullCmd = temp;
    }

    fflush(stdout);
    if (system(fullCmd) != 0) {
        errMsg("exec command failed");
    }

    free(fullCmd);
}

// Callback for -exec action
void execAction(char *path, char *command) {
    execCommand(path, command);
}

void execActionWrapper(char *path) {
    execAction(path, ((Expr *)path)->arg);
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

// Parse -name test
Expr *parseNameTest(char **argv, int *index) {
    if (argv[*index+1] == NULL || argv[*index+1][0] == '-') {
        fatalError("missing argument to -name");
    }

    Expr *expr = malloc(sizeof(Expr));
    expr->test = nameTest;
    expr->arg = argv[*index+1];
    (*index)++;

    return expr;
}

// Parse -newer test
Expr *parseNewerTest(char **argv, int *index) {
    if (argv[*index+1] == NULL || argv[*index+1][0] == '-') {
        fatalError("missing argument to -name");
    }

    Expr *expr = malloc(sizeof(Expr));
    expr->test = newerTest;
    expr->arg = argv[*index+1];
    (*index)++;

    return expr;
}

// Parse -print action
Expr *parsePrintAction() {
    Expr *expr = malloc(sizeof(Expr));
    expr->action = printFile;
    return expr;
}

// Parse -exec action
Expr *parseExecAction(char **argv, int *index) {
    if (argv[*index+1] == NULL) {
        fatalError("missing argument to -exec");
    }

    int len = strlen(argv[*index+1]);
    if (len < 3 || strcmp(argv[*index+1]+len-2, "\\;") != 0) {
        fatalError("invalid -exec command");
    }

    Expr *expr = malloc(sizeof(Expr));
    expr->action = execActionWrapper;
    expr->arg = strndup(argv[*index+1], len-2);
    (*index)++;

    return expr;
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
            Expr *expr = NULL;

            if (strcmp(arg, "-name") == 0) {
                expr = parseNameTest(argv, index);
            } else if (strcmp(arg, "-newer") == 0) {
                expr = parseNewerTest(argv, index);
            } else if (strcmp(arg, "-print") == 0) {
                expr = parsePrintAction();
            } else if (strncmp(arg, "-exec", 5) == 0) {
                expr = parseExecAction(argv, index);
            } else if (strcmp(arg, "-depth") == 0) {
                errMsg("-depth should come before tests, actions, or operators");
                break;
            } else if (strncmp(arg, "-maxdepth", 9) == 0) {
                errMsg("-maxdepth should come before tests, actions, or operators");
                break;
            } else {
                break;
            }

            expr->operator = defaultOp;
            expr->next = NULL;

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

// Hash function for inode
unsigned int hash(ino_t inode) {
    return inode % TABLE_SIZE;
}

// Check if inode has already been visited
int isVisited(ino_t inode, dev_t device) {
    unsigned int index = hash(inode);
    Entry *entry = visitedTable[index];

    while (entry != NULL) {
        if (entry->inode == inode && entry->device == device) {
            return 1;
        }
        entry = entry->next;
    }

    entry = malloc(sizeof(Entry));
    entry->inode = inode;
    entry->device = device;
    entry->next = visitedTable[index];
    visitedTable[index] = entry;

    return 0;
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

// Free expression linked list
void freeExpr(Expr *expr) {
    while (expr != NULL) {
        Expr *next = expr->next;
        if (expr->action == &execActionWrapper) {
            free(expr->arg);
        }
        free(expr);
        expr = next;
    }
}

// Free visited hashtable entries
void freeVisited() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *entry = visitedTable[i];
        while (entry != NULL) {
            Entry *next = entry->next;
            free(entry);
            entry = next;
        }
    }
}

// Evaluate expression for given file path
int evalExpr(char *path, Expr *expr) {
    while (expr != NULL) {
        int result;

        if (expr->action != NULL) {
            if (expr->action == &execActionWrapper) {
                execCommand(path, expr->arg);
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

// Traverse directory tree using a stack
void traverseDir(char *path, Expr *expr) {
    struct stat fileStat;
    if (stat(path, &fileStat) != 0) {
        if (errno == ENOENT) {
            fprintf(stderr, "fiend: '%s': No such file or directory\n", path);
        } else {
            fprintf(stderr, "fiend: '%s': Cannot access file or directory\n", path);
        }
        return;
    }

    Node *stack = NULL;
    pushStack(&stack, path, 0);

    while (stack != NULL) {
        char *currPath = stack->name;
        int currDepth = stack->depth;
        popStack(&stack);

        if (followLinks) {
            if (stat(currPath, &fileStat) != 0) {
                errMsg("cannot stat file");
                continue;
            }
        } else {
            if (lstat(currPath, &fileStat) != 0) {
                errMsg("cannot stat file");
                continue;
            }
        }

        if (isVisited(fileStat.st_ino, fileStat.st_dev)) {
            errMsg("symbolic link loop detected");
            continue;
        }

        // Perform pre-order actions for directories
        if (!postOrder && S_ISDIR(fileStat.st_mode)) {
            if (expr == NULL || evalExpr(currPath, expr)) {
                printFile(currPath);
            }
        }

        if (S_ISDIR(fileStat.st_mode)) {
            if (currDepth < maxDepth) {
                DIR *dir = opendir(currPath);
                if (dir == NULL) {
                    errMsg("cannot open directory");
                } else {
                    struct dirent *entry;
                    while ((entry = readdir(dir)) != NULL) {
                        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                            continue;
                        }

                        char fullPath[MAX_PATH];
                        snprintf(fullPath, sizeof(fullPath), "%s/%s", currPath, entry->d_name);
                        pushStack(&stack, fullPath, currDepth + 1);
                    }

                    closedir(dir);
                }
            }
        } else {
            // Perform actions for regular files
            if (expr == NULL || evalExpr(currPath, expr)) {
                printFile(currPath);
            }
        }

        // Perform post-order actions for directories
        if (postOrder && S_ISDIR(fileStat.st_mode)) {
            if (expr == NULL || evalExpr(currPath, expr)) {
                printFile(currPath);
            }
        }

        free(currPath);
    }
}

// Main function - process command line arguments and initiate traversal
int main(int argc, char **argv) {

    char *rootDirs[argc];
    int numRoots = 0;

    int i = 1;
    while (i < argc && (strcmp(argv[i], "-L") == 0 || strcmp(argv[i], "-P") == 0)) {
        followLinks = (strcmp(argv[i++], "-L") == 0);
    }

    while (i < argc && argv[i][0] != '-') {
        rootDirs[numRoots++] = argv[i++];
    }

    printf("numRoots: %i\n", numRoots);

    if (numRoots == 0) {
        rootDirs[numRoots++] = ".";
    }

    while (i < argc) {
        char *arg = argv[i];
        printf("arg: %s\n", arg);

        if (strcmp(arg, "-depth") == 0) {
            postOrder = 1;
            printf("postOrder: %d\n", postOrder);
        } else if (strncmp(arg, "-maxdepth", 9) == 0) {
            if (argv[i+1] == NULL || argv[i+1][0] == '-') {
                fatalError("missing argument to -maxdepth");
            }

            char *depthStr = argv[i+1];
            char *endPtr;
            errno = 0;

            printf("depthStr: %s\n", depthStr);

            long depth = strtol(depthStr, &endPtr, 10);
            if (errno != 0 || *endPtr != '\0' || depth < 0 || depth > INT_MAX) {
                fatalError("invalid -maxdepth argument");
            }

            maxDepth = (int) depth;
            
            i++;
        } else {
            break;
        }

        i++;
    }

    Expr *expr = parseExpression(argv, &i);

    for (int j = 0; j < numRoots; j++) {
        traverseDir(rootDirs[j], expr);
    }

    freeExpr(expr);
    freeVisited();

    return 0;
}