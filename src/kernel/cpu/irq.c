#include "irq.h"
#include "idt.h"

static irq_handler_t irq_handlers[16];

static inline void outb(u16 port, u8 val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void pic_remap(void)
{
    // ICW1 - Initialize
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // ICW2 - Remap IRQs
    outb(PIC1_DATA, 0x20); // Master PIC vector offset (32)
    outb(PIC2_DATA, 0x28); // Slave PIC vector offset (40)

    // ICW3 - Setup cascading
    outb(PIC1_DATA, 0x04); // Tell Master PIC there's a slave at IRQ2
    outb(PIC2_DATA, 0x02); // Tell Slave PIC its cascade identity

    // ICW4 - Environment info
    outb(PIC1_DATA, 0x01); // 8086 mode
    outb(PIC2_DATA, 0x01);

    // Mask all IRQs initially
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void irq_install(void)
{
    // Initialisiere Handler Array
    for (int i = 0; i < 16; i++) {
        irq_handlers[i] = NULL;
    }

    // Remap PIC
    pic_remap();

    // Setze IDT Gates für alle IRQs
    idt_set_gate(32, (u64)irq0, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(33, (u64)irq1, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(34, (u64)irq2, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(35, (u64)irq3, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(36, (u64)irq4, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(37, (u64)irq5, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(38, (u64)irq6, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(39, (u64)irq7, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(40, (u64)irq8, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(41, (u64)irq9, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(42, (u64)irq10, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(43, (u64)irq11, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(44, (u64)irq12, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(45, (u64)irq13, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(46, (u64)irq14, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_gate(47, (u64)irq15, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);

    // Aktiviere alle IRQs
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);

    // Aktiviere Interrupts
    //__asm__ volatile("sti");
}

void irq_register_handler(u8 irq, irq_handler_t handler)
{
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_unregister_handler(u8 irq)
{
    if (irq < 16) {
        irq_handlers[irq] = NULL;
    }
}

void irq_ack(u8 irq)
{
    // Send EOI to PICs
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void irq_handler(cpu_state_t* state)
{
    u8 irq = state->int_no - 32;

    // Rufe Handler auf falls registriert
    if (irq < 16 && irq_handlers[irq] != NULL) {
        irq_handlers[irq](state);
    }

    // Send EOI
    irq_ack(irq);
}
