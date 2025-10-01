

//NULL standart
#define NULL ((void*)0)

// Limine Requests
__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Halt
static void hcf(void)
{
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// Simple delay function
static void delay(uint32_t count)
{
    for (volatile uint32_t i = 0; i < count * 10000000; i++)
    {
        __asm__ volatile ("nop");
    }
}
