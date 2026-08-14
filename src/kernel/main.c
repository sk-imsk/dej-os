static void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void kentry(void)
{
    outb(0x3F8, 'C');

    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}
