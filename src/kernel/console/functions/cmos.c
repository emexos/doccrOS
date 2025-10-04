#include "../console.h"
#include "../../../libs/print/print.h"
#include "../../../libs/graphics/colors/stdclrs.h"
#include "../../../libs/string/string.h"
#include "../../../libs/memory/main.h"
#include "../../../drivers/cmos/cmos.h"

FHDR(cmd_date)
{
    (void)s; // unused parameter

    print("\n", GFX_WHITE);
    GetCMOSDate();
    print("\n", GFX_WHITE);

}
