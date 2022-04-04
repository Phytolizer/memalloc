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

const char* const sources[] = {
        "main",
        "memalloc",
};

int main(int argc, char** argv) {
    REBUILD_SELF(argc, argv);

    const char** objects =
            malloc(sizeof(const char*) * (sizeof(sources) / sizeof(const char*) + 1));

    MKDIRS("build", "obj");
    size_t i = 0;
    FOREACH_ARRAY(const char*, source, sources, {
        const char* object = PATH("build", "obj", CONCAT(source, ".o"));
        CMD(cc(), "-c", CFLAGS, CONCAT(source, ".c"), "-o", object);
        objects[i] = object;
        i++;
    });
    objects[i] = NULL;

    char** build_exe = collect_args("vpvv", cc(), objects, "-o", PATH("build", PROJECT_NAME));
    echo_cmd(build_exe);
    coolbuild_exec(build_exe);

    if (argc > 1 && strcmp(argv[1], "run") == 0) {
        CMD(PATH("build", PROJECT_NAME));
    }
}
