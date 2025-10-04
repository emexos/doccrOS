#include "panic.h"
#include "proc/scheduler.h"
#include "proc/process.h"
#include "../libs/print/print.h"
#include "../libs/graphics/colors/stdclrs.h"
#include "../libs/string/string.h"
#include "../libs/graphics/graphics.h"
#include "../../shared/theme/doccr.h"
#include "cpu/timer.h"

static int safe_mode = 0;
static int userspace_crash_count = 0;
static const int MAX_CRASH_RETRIES = 3;

void kernel_panic(const char* message, panic_reason_t reason)
{
    // Deaktiviere Interrupts
    __asm__ volatile("cli");

    // Deaktiviere Scheduler
    scheduler_disable();

    // Clear Screen
    clear(0xFF8B0000);  // Dunkelrot
    draw_rect(40, 40, fb_width - 80, fb_height - 80, 0xFF8B0000);

    reset_cursor();

    cursor_x = 60;
    cursor_y = 60;

    // Zeige Panic Screen
    print("", GFX_WHITE);
    print("=========================================\n", GFX_WHITE);
    cursor_x = 60;
    print("!!!   KERNEL PANIC - SYSTEM HALTED   !!!\n", GFX_RED);
    cursor_x = 60;
    print("=========================================\n", GFX_WHITE);
    print("", GFX_WHITE);

    char buf[128];

    cursor_x = 60;

    // Zeige Reason
    str_copy(buf, "Reason: ");
    switch (reason) {
        case PANIC_OUT_OF_MEMORY:
            cursor_x = 124;
            str_append(buf, "Out of Memory\n");
            break;
        case PANIC_KERNEL_FAULT:
            cursor_x = 124;
            str_append(buf, "Kernel Fault\n");
            break;
        case PANIC_USERSPACE_FAULT:
            cursor_x = 124;
            str_append(buf, "Userspace Fault\n");
            break;
        case PANIC_SCHEDULER_ERROR:
            cursor_x = 124;
            str_append(buf, "Scheduler Error\n");
            break;
        case PANIC_HARDWARE_ERROR:
            cursor_x = 124;
            str_append(buf, "Hardware Error\n");
            break;
        default:
            cursor_x = 124;
            str_append(buf, "General Fault\n");
            break;
    }
    print(buf, GFX_YELLOW);

    // Zeige Message
    print("\n", GFX_WHITE);
    cursor_x = 60;
    print("Message:", GFX_CYAN);
    cursor_x = 132;
    print(message, GFX_WHITE);
    print("\n", GFX_WHITE);

    // Zeige System Info
    cursor_x = 60;
    str_copy(buf, "Active Processes: ");
    str_append_uint(buf, (u32)scheduler_get_active_processes());
    cursor_x = 60;
    print(buf, GFX_GRAY_50);
    print("\n", GFX_WHITE);

    cursor_x = 60;
    str_copy(buf, "Context Switches: ");
    str_append_uint(buf, (u32)scheduler_get_context_switches());
    cursor_x = 60;
    print(buf, GFX_GRAY_50);
    print("\n", GFX_WHITE);

    cursor_x = 60;
    str_copy(buf, "Uptime: ");
    str_append_uint(buf, (u32)timer_get_seconds());
    str_append(buf, " seconds");
    cursor_x = 60;
    print(buf, GFX_GRAY_50);

    cursor_x = 60;
    print("\n\n", GFX_WHITE);
    cursor_x = 60;
    print("System halted. Please restart your computer.", GFX_RED);

    // Halt forever
    while (1) {
        __asm__ volatile("hlt");
    }
}

void kernel_panic_cpu(const char* message, cpu_state_t* state)
{
    __asm__ volatile("cli");
    scheduler_disable();

    clear(0xFF8B0000);

    print("", GFX_WHITE);
    print("=========================================", GFX_WHITE);
    print("!!!   CPU EXCEPTION - KERNEL PANIC   !!!", GFX_RED);
    print("=========================================", GFX_WHITE);
    print("", GFX_WHITE);

    print(message, GFX_WHITE);
    print("", GFX_WHITE);

    char buf[128];

    // CPU State
    print("CPU State:", GFX_CYAN);

    str_copy(buf, "RAX: 0x");
    str_append_uint(buf, (u32)(state->rax >> 32));
    str_append_uint(buf, (u32)(state->rax & 0xFFFFFFFF));
    print(buf, GFX_YELLOW);

    str_copy(buf, "RBX: 0x");
    str_append_uint(buf, (u32)(state->rbx >> 32));
    str_append_uint(buf, (u32)(state->rbx & 0xFFFFFFFF));
    print(buf, GFX_YELLOW);

    str_copy(buf, "RIP: 0x");
    str_append_uint(buf, (u32)(state->rip >> 32));
    str_append_uint(buf, (u32)(state->rip & 0xFFFFFFFF));
    print(buf, GFX_RED);

    str_copy(buf, "RSP: 0x");
    str_append_uint(buf, (u32)(state->rsp >> 32));
    str_append_uint(buf, (u32)(state->rsp & 0xFFFFFFFF));
    print(buf, GFX_YELLOW);

    str_copy(buf, "INT: ");
    str_append_uint(buf, (u32)state->int_no);
    str_append(buf, "  ERR: ");
    str_append_uint(buf, (u32)state->err_code);
    print(buf, GFX_YELLOW);

    print("", GFX_WHITE);
    print("System halted. Please restart.", GFX_RED);

    while (1) {
        __asm__ volatile("hlt");
    }
}

void userspace_crash_handler(cpu_state_t* state)
{
    process_t* current = scheduler_get_current();

    if (current == NULL) {
        kernel_panic("Userspace crash but no current process", PANIC_USERSPACE_FAULT);
        return;
    }

    // Protokolliere Crash
    userspace_crash_count++;

    // Zeige Fehler
    clear(CONSOLESCREEN_COLOR);
    reset_cursor();

    print("", GFX_WHITE);
    print("!!! USERSPACE APPLICATION CRASHED !!!", GFX_RED);
    print("", GFX_WHITE);

    char buf[128];
    str_copy(buf, "Process: ");
    str_append(buf, current->name);
    str_append(buf, " (PID: ");
    str_append_uint(buf, (u32)current->pid);
    str_append(buf, ")");
    print(buf, GFX_YELLOW);

    str_copy(buf, "Exception: ");
    str_append_uint(buf, (u32)state->int_no);
    print(buf, GFX_YELLOW);

    print("", GFX_WHITE);

    // Versuche Recovery
    if (userspace_crash_count < MAX_CRASH_RETRIES) {
        print("Attempting automatic recovery...", GFX_CYAN);
        userspace_try_recovery();
    } else {
        print("Too many crashes. Entering safe mode...", GFX_RED);
        print("System will restart in 10 seconds...", GFX_YELLOW);

        system_enter_safe_mode();

        // Warte 10 Sekunden
        timer_wait(10000);

        // ACPI Reboot würde hier kommen
        // Für jetzt: einfach halt
        print("Please restart your computer manually.", GFX_RED);
        while (1) {
            __asm__ volatile("hlt");
        }
    }
}

void userspace_try_recovery(void)
{
    process_t* current = scheduler_get_current();

    if (current == NULL) {
        return;
    }

    print("Terminating crashed process...", GFX_YELLOW);

    // Beende den fehlerhaften Prozess
    process_exit(-1);

    // Scheduler wird neuen Prozess wählen
    scheduler_yield();

    print("Recovery successful. System stable.", GFX_GREEN);

    // Reset crash counter nach erfolgreicher Recovery
    userspace_crash_count = 0;
}

void system_enter_safe_mode(void)
{
    safe_mode = 1;

    // Deaktiviere alle nicht-essentiellen Services
    print("Entering safe mode...", GFX_YELLOW);

    // Stoppe alle User-Prozesse
    // (Vereinfachte Version)
    scheduler_disable();

    print("Safe mode active. Only kernel services running.", GFX_GREEN);
}

int system_is_safe_mode(void)
{
    return safe_mode;
}
