
int main() __attribute__((constructor())){
    __asm__ volatile ("int $3");
}
