#include "../console.h"
#include "../../../libs/print/print.h"
#include "../../../libs/graphics/colors/stdclrs.h"
#include "../../../libs/string/string.h"

FHDR(cmd_echo)
{
    if (*s == '\0') {
        print("", GFX_WHITE);
        return;
    }

    print(s, GFX_WHITE);
}

FHDR(cmd_clear)
{
    u32 color = GFX_BG;

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
        print("Available commands:", GFX_CYAN);
        print("  echo       - echo [text]", GFX_WHITE);
        print("  clear      - clear screen", GFX_WHITE);
        print("  help       - displays this list", GFX_WHITE);
        print("  meminfo    - displays memory information", GFX_WHITE);
        print("", GFX_WHITE);
        print("Type 'help <command>' for a detail of the command", GFX_WHITE);
    } else {
        // show specific command help
        const char *p = s;
        while (*p == ' ') p++;

        console_cmd_t *cmd = console_find_cmd(p);
        if (cmd) {
            char buf[128];

            print("", GFX_WHITE);
            str_copy(buf, "Command: ");
            str_append(buf, cmd->name);
            print(buf, GFX_CYAN);

            str_copy(buf, "Description: ");
            str_append(buf, cmd->description);
            print(buf, GFX_WHITE);

            str_copy(buf, "Usage: ");
            str_append(buf, cmd->usage);
            print(buf, GFX_YELLOW);
            print("", GFX_WHITE);
        } else {
            print("Command not found", GFX_RED);
        }
    }
}
