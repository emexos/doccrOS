#include "console.h"
#include "../../libs/print/print.h"
#include "../../libs/graphics/colors/stdclrs.h"
#include "../../libs/graphics/graphics.h"

void shell_clear_screen(u32 color)
{
    clear(color);
}

void shell_print_prompt(void)
{

    string("doccrOS:~> ", GFX_GREEN);
}

void shell_redraw_input(void)
{
    // for future use: redraw current input line
    // useful when implementing line editing features
}
