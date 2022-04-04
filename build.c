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
#define CFLAGS "-Wall", "-Wextra", "-Wpedantic", "-ggdb"

#define TAU_INC "-Itau"

const char* const main_sources[] = {
        "main",
        "memalloc",
};

const char* const test_sources[] = {
        "test_main",
};

int main(int argc, char** argv) {
    REBUILD_SELF(argc, argv);

    const char** main_objects =
            malloc(sizeof(const char*) * (sizeof(main_sources) / sizeof(const char*) + 1));

    MKDIRS("build", "obj");
    size_t i = 0;
    FOREACH_ARRAY(const char*, source, main_sources, {
        const char* object = PATH("build", "obj", CONCAT(source, ".o"));
        CMD(cc(), "-c", CFLAGS, CONCAT(source, ".c"), "-o", object);
        main_objects[i] = object;
        i++;
    });
    main_objects[i] = NULL;

    char** build_exe = collect_args("vpvv", cc(), main_objects, "-o", PATH("build", PROJECT_NAME));
    echo_cmd(build_exe);
    coolbuild_exec(build_exe);

    i = 0;
    const char** test_objects =
            malloc(sizeof(const char*) * (sizeof(test_sources) / sizeof(const char*) + 1));
    FOREACH_ARRAY(const char*, source, test_sources, {
        const char* object = PATH("build", "obj", CONCAT(source, ".o"));
        CMD(cc(), "-c", CFLAGS, TAU_INC, CONCAT(source, ".c"), "-o", object);
        test_objects[i] = object;
        i++;
    });
    test_objects[i] = NULL;
    build_exe = collect_args("vpvv", cc(), test_objects, "-o", PATH("build", "test_" PROJECT_NAME));
    echo_cmd(build_exe);
    coolbuild_exec(build_exe);

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
