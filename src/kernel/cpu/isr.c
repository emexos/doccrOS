#include "isr.h"
#include "idt.h"
#include "../../libs/print/print.h"
#include "../../libs/graphics/colors/stdclrs.h"
#include "../../libs/string/string.h"

static isr_handler_t isr_handlers[32];

static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

void isr_install(void)
{
    // Initialisiere Handler Array
    for (int i = 0; i < 32; i++) {
        isr_handlers[i] = NULL;
    }

    // Setze IDT Gates für alle Exceptions
    idt_set_gate(0, (u64)isr0, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(1, (u64)isr1, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(2, (u64)isr2, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(3, (u64)isr3, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(4, (u64)isr4, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(5, (u64)isr5, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(6, (u64)isr6, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(7, (u64)isr7, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(8, (u64)isr8, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(9, (u64)isr9, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(10, (u64)isr10, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(11, (u64)isr11, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(12, (u64)isr12, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(13, (u64)isr13, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(14, (u64)isr14, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(15, (u64)isr15, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(16, (u64)isr16, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(17, (u64)isr17, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(18, (u64)isr18, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(19, (u64)isr19, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(20, (u64)isr20, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(21, (u64)isr21, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(22, (u64)isr22, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(23, (u64)isr23, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(24, (u64)isr24, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(25, (u64)isr25, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(26, (u64)isr26, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(27, (u64)isr27, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(28, (u64)isr28, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(29, (u64)isr29, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(30, (u64)isr30, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(31, (u64)isr31, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
}

void isr_register_handler(u8 num, isr_handler_t handler)
{
    if (num < 32) {
        isr_handlers[num] = handler;
    }
}

void isr_unregister_handler(u8 num)
{
    if (num < 32) {
        isr_handlers[num] = NULL;
    }
}

void isr_handler(cpu_state_t* state)
{
    // Rufe registrierten Handler auf
    if (state->int_no < 32 && isr_handlers[state->int_no] != NULL) {
        isr_handlers[state->int_no](state);
    } else {
        // Keine Handler - zeige Fehler
        print("", GFX_WHITE);
        print("!!! KERNEL PANIC !!!", GFX_RED);

        char buf[128];
        str_copy(buf, "Exception: ");
        str_append(buf, exception_messages[state->int_no]);
        print(buf, GFX_RED);

        str_copy(buf, "INT: ");
        str_append_uint(buf, (u32)state->int_no);
        str_append(buf, " ERR: ");
        str_append_uint(buf, (u32)state->err_code);
        print(buf, GFX_YELLOW);

        str_copy(buf, "RIP: 0x");
        str_append_uint(buf, (u32)(state->rip >> 32));
        str_append_uint(buf, (u32)(state->rip & 0xFFFFFFFF));
        print(buf, GFX_YELLOW);

        str_copy(buf, "RSP: 0x");
        str_append_uint(buf, (u32)(state->rsp >> 32));
        str_append_uint(buf, (u32)(state->rsp & 0xFFFFFFFF));
        print(buf, GFX_YELLOW);

        print("System halted.", GFX_WHITE);

        // Halte System an
        while(1) {
            __asm__ volatile("cli; hlt");
        }
    }
}
