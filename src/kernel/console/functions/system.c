#include "../console.h"
#include "../../../libs/print/print.h"
#include "../../../libs/graphics/colors/stdclrs.h"
#include "../../../libs/string/string.h"
#include "../../../libs/memory/main.h"

FHDR(cmd_meminfo)
{
    (void)s; // unused parameter

    char buf[128];

    print("", GFX_WHITE);
    print("Memory Information:", GFX_CYAN);

    str_copy(buf, "Free:  ");
    str_append_uint(buf, (u32)(mem_get_free() / 1024));
    str_append(buf, " KB");
    print(buf, GFX_GREEN);

    str_copy(buf, "Used:  ");
    str_append_uint(buf, (u32)(mem_get_used() / 1024));
    str_append(buf, " KB");
    print(buf, GFX_YELLOW);

    size_t total = mem_get_free() + mem_get_used();
    str_copy(buf, "Total: ");
    str_append_uint(buf, (u32)(total / 1024));
    str_append(buf, " KB");
    print(buf, GFX_WHITE);

    print("", GFX_WHITE);
}
