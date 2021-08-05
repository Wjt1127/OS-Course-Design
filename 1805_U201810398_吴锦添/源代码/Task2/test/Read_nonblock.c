#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(char argc, char* argv[])
{
    int fd;
    char val2[] = { "wujintianU201810398xinan1805wujintianU201810398xinan1805wujintianU201810398xinan1805" };
    fd = open("/dev/FIFOdev", O_RDWR|O_NONBLOCK);
    if (fd < 0) {
        printf("can't open!\n");
    }
    for (int i = 0; i < 84; i++) {
        read(fd, val2 + i, 1);
        printf("%c\n", val2[i]);
    }

    return 0;
}

