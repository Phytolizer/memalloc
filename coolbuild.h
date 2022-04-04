#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH_SEPARATOR "/"
#define MKDIR(path, mode) mkdir(path, mode)

#define PATH_SEPARATOR_LENGTH (sizeof PATH_SEPARATOR - 1)

#define FOREACH_VARGS(type, arg, begin, body) \
    do { \
        va_list args; \
        va_start(args, begin); \
        for (type arg = va_arg(args, type); arg != NULL; arg = va_arg(args, type)) { \
            body; \
        } \
        va_end(args); \
    } while (false)

#define FOREACH_ARRAY(type, item, items, body) \
    do { \
        for (size_t i_ = 0; i_ < sizeof(items) / sizeof(type); i_++) { \
            type item = (items)[i_]; \
            body; \
        } \
    } while (false)

#define FOREACH_FILE_IN_DIR(file, dirpath, body) \
    do { \
        struct dirent* dp = NULL; \
        DIR* dir = opendir(dirpath); \
        while ((dp = readdir(dir)) != NULL) { \
            const char* file = dp->d_name; \
            body; \
        } \
    } while (false)

#define REBUILD_SELF(argc, argv) \
    do { \
        const char* source_path = __FILE__; \
        const char* binary_path = argv[0]; \
        struct stat statbuf; \
        if (stat(source_path, &statbuf) < 0) { \
            fprintf(stderr, "Could not stat %s: %s\n", source_path, strerror(errno)); \
        } \
        int source_mtime = statbuf.st_mtime; \
        if (stat(binary_path, &statbuf) < 0) { \
            fprintf(stderr, "Could not stat %s: %s\n", binary_path, strerror(errno)); \
        } \
        int binary_mtime = statbuf.st_mtime; \
        if (source_mtime > binary_mtime) { \
            rename(binary_path, CONCAT(binary_path, ".old")); \
            CMD("cc", source_path, "-o", binary_path); \
            echo_cmd(argv); \
            execv(argv[0], argv); \
        } \
    } while (false)

const char* vconcat_sep_impl(const char* sep, va_list args) {
    size_t length = 0;
    int64_t seps = -1;
    size_t sep_len = strlen(sep);

    va_list args2;
    va_copy(args2, args);
    for (const char* arg = va_arg(args2, const char*); arg != NULL;
            arg = va_arg(args2, const char*)) {
        if (arg[0] != '\0') {
            length += strlen(arg);
            seps += 1;
        }
    }
    va_end(args2);

    char* result = malloc(length + (seps * sep_len) + 1);

    length = 0;
    for (const char* arg = va_arg(args, const char*); arg != NULL;
            arg = va_arg(args, const char*)) {
        if (arg[0] != '\0') {
            size_t n = strlen(arg);
            memcpy(result + length, arg, n);
            length += n;
            if (seps > 0) {
                memcpy(result + length, sep, sep_len);
                length += sep_len;
                seps -= 1;
            }
        }
    }

    return result;
}

const char* concat_sep_impl(const char* sep, ...) {
    va_list args;
    va_start(args, sep);
    const char* result = vconcat_sep_impl(sep, args);
    va_end(args);

    return result;
}

#define CONCAT_SEP(sep, ...) concat_sep_impl(sep, __VA_ARGS__, NULL)

#define PATH(...) CONCAT_SEP(PATH_SEPARATOR, __VA_ARGS__)

void mkdirs_impl(int ignore, ...) {
    size_t length = 0;
    int64_t seps = -1;

    FOREACH_VARGS(const char*, arg, ignore, {
        length += strlen(arg);
        seps += 1;
    });

    char* result = malloc(length + (seps * PATH_SEPARATOR_LENGTH) + 1);

    length = 0;
    FOREACH_VARGS(const char*, arg, ignore, {
        size_t n = strlen(arg);
        memcpy(result + length, arg, n);
        length += n;

        result[length] = '\0';
        printf("[INFO] mkdir %s\n", result);
        if (MKDIR(result, 0755) < 0 && errno != EEXIST) {
            fprintf(stderr, "[ERROR] Could not create directory '%s': %s\n", result,
                    strerror(errno));
            exit(1);
        }

        if (seps > 0) {
            memcpy(result + length, PATH_SEPARATOR, PATH_SEPARATOR_LENGTH);
            length += PATH_SEPARATOR_LENGTH;
            seps -= 1;
        }
    });
}

#define MKDIRS(...) mkdirs_impl(69, __VA_ARGS__, NULL)

const char* concat_impl(int ignore, ...) {
    size_t length = 0;

    FOREACH_VARGS(const char*, arg, ignore, { length += strlen(arg); });

    char* result = malloc(length + 1);

    length = 0;
    FOREACH_VARGS(const char*, arg, ignore, {
        size_t n = strlen(arg);
        memcpy(result + length, arg, n);
        length += n;
    });
    result[length] = '\0';

    return result;
}

#define CONCAT(...) concat_impl(69, __VA_ARGS__, NULL)

void coolbuild_exec(char** argv) {
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "[ERROR] Could not fork: %s\n", strerror(errno));
        exit(1);
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "[ERROR] Could not execute %s: %s\n", argv[0], strerror(errno));
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "[ERROR] Subcommand reported non-zero status %d\n", status);
        exit(1);
    }
}

void echo_cmd(char** argv) {
    printf("[INFO]");
    for (size_t i = 0; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    printf("\n");
}

void cmd_impl(int ignore, ...) {
    size_t argc = 0;

    FOREACH_VARGS(const char*, arg, ignore, {
        if (arg[0] != '\0') {
            argc += 1;
        }
    });
    assert(argc >= 1);

    char** argv = malloc(sizeof(char*) * (argc + 1));

    argc = 0;
    FOREACH_VARGS(char*, arg, ignore, {
        if (arg[0] != '\0') {
            argv[argc] = arg;
            argc += 1;
        }
    });
    argv[argc] = NULL;

    va_list args;
    va_start(args, ignore);
    const char* command_message = vconcat_sep_impl(" ", args);
    va_end(args);
    printf("[INFO] %s\n", command_message);

    coolbuild_exec(argv);
}

#define CMD(...) cmd_impl(69, __VA_ARGS__, NULL)

char** collect_args(char* fmt, ...) {
    size_t length = 0;
    va_list args;
    va_start(args, fmt);
    size_t fmtlen = strlen(fmt);
    for (size_t i = 0; i < fmtlen; i++) {
        switch (fmt[i]) {
            case 'v':
                (void)va_arg(args, char*);
                length += 1;
                break;
            case 'p': {
                char** argv = va_arg(args, char**);
                while (*argv != NULL) {
                    length += 1;
                    argv++;
                }
            } break;
            default:
                assert(false && "illegal format character");
        }
    }
    va_end(args);

    char** result = malloc(sizeof(char*) * (length + 1));
    va_start(args, fmt);
    size_t j = 0;
    for (size_t i = 0; i < fmtlen; i++) {
        switch (fmt[i]) {
            case 'v':
                result[j] = va_arg(args, char*);
                j++;
                break;
            case 'p': {
                char** temp = va_arg(args, char**);
                while (*temp != NULL) {
                    result[j] = *temp;
                    j++;
                    temp++;
                }
            } break;
        }
    }
    va_end(args);
    result[j] = NULL;

    return result;
}
