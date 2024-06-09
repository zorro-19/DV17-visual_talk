#include "system/includes.h"
#include "generic/log.h"

extern version_t sys_version_begin[], sys_version_end[];


const char *sdk_version(void)
{
    return "AC57 SDK on branch [tmp/dv17_verify] tag [AC57N_SDK_V3.0.3_2023-08-14] ";
}


static int app_version_check()
{
    char *version;

    printf("================= SDK Version %s ===============\n", sdk_version());
    for (version = sys_version_begin; version < sys_version_end;) {
        version += 4;
        printf("%s\n", version);
        version += strlen(version) + 1;
    }
    puts("======================================\n");


    return 0;
}
early_initcall(app_version_check);
