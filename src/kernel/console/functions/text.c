#include "../console.h"
#include "../../../libs/print/print.h"
#include "../../../libs/graphics/colors/stdclrs.h"
#include "../../../libs/string/string.h"
#include "../../../../shared/theme/doccr.h"

FHDR(cmd_echo)
{
    if (*s == '\0') {
        print("\n", GFX_WHITE);
        return;
    }

    print(s, GFX_WHITE);
    print("\n", GFX_WHITE);
}

FHDR(cmd_clear)
{
    u32 color = CONSOLESCREEN_COLOR;

    // parse color argument if provided
    if (*s != '\0') {
        // simple color name parsing
        const char *p = s;
        while (*p == ' ') p++;

        if (*p == 'b' && *(p+1) == 'l' && *(p+2) == 'a' && *(p+3) == 'c' && *(p+4) == 'k') {
            color = GFX_BLACK;
        } else if (*p == 'w' && *(p+1) == 'h' && *(p+2) == 'i' && *(p+3) == 't' && *(p+4) == 'e') {
            color = GFX_WHITE;
        } else if (*p == 'r' && *(p+1) == 'e' && *(p+2) == 'd') {
            color = GFX_RED;
        } else if (*p == 'g' && *(p+1) == 'r' && *(p+2) == 'e' && *(p+3) == 'e' && *(p+4) == 'n') {
            color = GFX_GREEN;
        } else if (*p == 'b' && *(p+1) == 'l' && *(p+2) == 'u' && *(p+3) == 'e') {
            color = GFX_BLUE;
        } else if (*p == 'c' && *(p+1) == 'y' && *(p+2) == 'a' && *(p+3) == 'n') {
            color = GFX_CYAN;
        } else if (*p == 'y' && *(p+1) == 'e' && *(p+2) == 'l' && *(p+3) == 'l' && *(p+4) == 'o' && *(p+5) == 'w') {
            color = GFX_YELLOW;
        } else if (*p == 'p' && *(p+1) == 'u' && *(p+2) == 'r' && *(p+3) == 'p' && *(p+4) == 'l' && *(p+5) == 'e') {
            color = GFX_PURPLE;
        }
    }

    shell_clear_screen(color);
}

FHDR(cmd_help)
{
    if (*s == '\0') {
        // show all commands
        print("\nAvailable commands:\n", GFX_CYAN);
        print("  echo       - echo [text]\n", GFX_WHITE);
        print("  clear      - clear screen\n", GFX_WHITE);
        print("  help       - displays this list\n", GFX_WHITE);
        print("  meminfo    - displays memory information\n", GFX_WHITE);
        print("  dofetch    - shows doccrOS fetch (fastfetch)\n", GFX_WHITE);
        print("\n", GFX_WHITE);
        print("Type 'help <command>' for a detail of the command\n", GFX_WHITE);
    } else {
        // show specific command help
        const char *p = s;
        while (*p == ' ') p++;

        console_cmd_t *cmd = console_find_cmd(p);
        if (cmd) {
            char buf[128];

            print("", GFX_WHITE);
            str_copy(buf, "\nCommand: ");
            str_append(buf, cmd->name);
            print(buf, GFX_CYAN);

            str_copy(buf, "\nDescription: ");
            str_append(buf, cmd->description);
            print(buf, GFX_WHITE);

            str_copy(buf, "\nUsage: ");
            str_append(buf, cmd->usage);
            print(buf, GFX_YELLOW);
            print("\n", GFX_WHITE);
        } else {
            print("\nCommand not found\n", GFX_RED);
        }
    }
}
