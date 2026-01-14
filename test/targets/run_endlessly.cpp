//
// Created by yax on 15/01/26.
//


[[noreturn]] int main(int argc, char *argv[]) {
    volatile int i = 1;
    while (true) i = 42;
}
