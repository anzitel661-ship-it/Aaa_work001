#include <stdio.h>

int main() {
    char input[256];

    printf("Hello, World!\n");
    printf("Please enter something: ");
    fgets(input, sizeof(input), stdin);
    printf("You entered: %s", input);

    return 0;
}