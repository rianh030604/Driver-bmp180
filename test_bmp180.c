
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/bmp180"
#define BMP180_IOCTL_MAGIC 'b'
#define BMP180_IOCTL_READ_TEMP _IOR(BMP180_IOCTL_MAGIC, 1, int)

int main() {
    int fd, data;

    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    if (ioctl(fd, BMP180_IOCTL_READ_TEMP, &data) < 0) {
        perror("IOCTL read failed");
        close(fd);
        return -1;
    }

    printf("Temperature raw data: %d\n", data);
    close(fd);
    return 0;
}
