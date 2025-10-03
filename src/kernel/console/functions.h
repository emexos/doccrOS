#pragma once

// function header because of FHDR it should be possible to use it in syscalls in future
#define FHDR(name) void name(const char* s);

// text.c
FHDR(cmd_echo);
FHDR(cmd_clear);
FHDR(cmd_help);

// system.c
FHDR(cmd_meminfo);
