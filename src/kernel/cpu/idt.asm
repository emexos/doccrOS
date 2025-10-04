; IDT Assembly Functions
[BITS 64]

global idt_flush

; void idt_flush(u64 idt_ptr)
idt_flush:
    lidt [rdi]      ; Lade IDT von Adresse in RDI
    ret
