#include "coolbuild.h"

#include <string.h>

const char* cc(void) {
    const char* result = getenv("CC");
    if (result) {
        return result;
    }
    return "cc";
}

#define PROJECT_NAME "memalloc"
#define CFLAGS "-Wall", "-Wextra", "-Wpedantic", "-ggdb3"
#define LIB_CFLAGS CFLAGS
#define MAIN_CFLAGS CFLAGS
#define TEST_CFLAGS CFLAGS, "-I", "tau"

#define TAU_INC "-Itau"

const char* const lib_sources[] = {
        "memalloc",
};

const char* const main_sources[] = {
        "main",
};

const char* const test_sources[] = {
        "test_main",
};

#define COMPILE_OBJECTS(sources, flags, objects) \
    do { \
        *(objects) = malloc(sizeof(const char*) * (sizeof(sources) / sizeof(const char*) + 1)); \
        size_t i = 0; \
        FOREACH_ARRAY(const char*, source, sources, { \
            const char* object = PATH("build", "obj", CONCAT(source, ".o")); \
            CMD(cc(), "-c", flags, CONCAT(source, ".c"), "-o", object); \
            (*(objects))[i] = object; \
            i++; \
        }); \
        (*(objects))[i] = NULL; \
    } while (false)

int main(int argc, char** argv) {
    REBUILD_SELF(argc, argv);

    MKDIRS("build", "obj");

    const char** lib_objects;
    COMPILE_OBJECTS(lib_sources, LIB_CFLAGS, &lib_objects);

    char** build_lib =
            collect_args("vvvp", "ar", "rcs", "build/lib" PROJECT_NAME ".a", lib_objects);
    echo_cmd(build_lib);
    coolbuild_exec(build_lib);

    const char** main_objects;
    COMPILE_OBJECTS(main_sources, MAIN_CFLAGS, &main_objects);

    char** build_exe = collect_args("vpvvvv", cc(), main_objects, "-Lbuild", "-lmemalloc", "-o",
            PATH("build", PROJECT_NAME));
    echo_cmd(build_exe);
    coolbuild_exec(build_exe);

    const char** test_objects;
    COMPILE_OBJECTS(test_sources, TEST_CFLAGS, &test_objects);

    char** build_test_exe = collect_args("vpvvvv", cc(), test_objects, "-Lbuild", "-lmemalloc",
            "-o", PATH("build", "test_" PROJECT_NAME));
    echo_cmd(build_test_exe);
    coolbuild_exec(build_test_exe);

    if (argc > 1) {
        if (strcmp(argv[1], "run") == 0) {
            CMD(PATH("build", PROJECT_NAME));
        } else if (strcmp(argv[1], "test") == 0) {
            CMD(PATH("build", "test_" PROJECT_NAME));
        } else {
            fprintf(stderr, "Unknown command: %s\n", argv[1]);
            return 1;
        }
    }
}
