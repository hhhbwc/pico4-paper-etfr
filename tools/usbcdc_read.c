/*
 * usbcdc_read.c - minimal USB CDC-ACM bulk reader for PICO 4 (arm64)
 * Reads raw data from the Paper eye/face tracker CDC device.
 *
 * Build (WSL NDK):
 *   clang --target=aarch64-linux-android29 -O2 -static -o usbcdc_read usbcdc_read.c
 *
 * Usage: usbcdc_read <busnum-devnum e.g. 1-3> [dur_ms] [ep_in_hex e.g. 82]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/usbdevice_fs.h>
#include <sys/ioctl.h>
#include <sys/time.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <busdev e.g. 1-3> [dur_ms] [ep_in_hex]\n", argv[0]);
        return 1;
    }
    int bus = 1, dev = 3;
    sscanf(argv[1], "%d-%d", &bus, &dev);
    int dur = argc > 2 ? atoi(argv[2]) : 5000;
    unsigned int ep = argc > 3 ? (unsigned int)strtol(argv[3], NULL, 16) : 0x82;

    char path[64];
    snprintf(path, sizeof(path), "/dev/bus/usb/%03d/%03d", bus, dev);
    int fd = open(path, O_RDWR);
    if (fd < 0) { perror("open"); return 1; }
    printf("opened %s, ep IN 0x%02x\n", path, ep);

    struct usbdevfs_ioctl ctl;
    ctl.ifno = 0; ctl.ioctl_code = USBDEVFS_RESET; ctl.data = NULL;
    ioctl(fd, USBDEVFS_IOCTL, &ctl);
    if (ioctl(fd, USBDEVFS_SETCONFIGURATION, 1) < 0) perror("setconfig");

    int ifs[2] = {0, 1};
    for (int i = 0; i < 2; i++) {
        if (ioctl(fd, USBDEVFS_CLAIMINTERFACE, &ifs[i]) < 0)
            fprintf(stderr, "claim if%d: %s\n", ifs[i], strerror(errno));
        else fprintf(stderr, "claimed if%d\n", ifs[i]);
    }

    unsigned char buf[4096];
    struct usbdevfs_bulktransfer bulk;
    struct timeval start, now;
    gettimeofday(&start, NULL);
    long total = 0;
    int got = 0;
    while (1) {
        gettimeofday(&now, NULL);
        long el = (now.tv_sec-start.tv_sec)*1000 + (now.tv_usec-start.tv_usec)/1000;
        if (el > dur) break;
        memset(&bulk, 0, sizeof(bulk));
        bulk.ep = ep;                // bulk IN endpoint
        bulk.len = sizeof(buf);
        bulk.data = buf;
        bulk.timeout = 500;          // ms
        int n = ioctl(fd, USBDEVFS_BULK, &bulk);
        if (n > 0) {
            got++;
            total += n;
            if (got <= 8) {
                printf("--- %d bytes ---\n", n);
                for (int i = 0; i < n && i < 80; i++) printf("%02x ", (unsigned char)buf[i]);
                printf("\n");
                for (int i = 0; i < n && i < 80; i++) {
                    unsigned char c = buf[i];
                    printf("%c", (c>=32&&c<127)?c:'.');
                }
                printf("\n");
            }
        } else if (n < 0 && errno != ETIMEDOUT) {
            perror("bulk");
            break;
        }
    }
    printf("\n=== total reads=%d bytes=%ld ===\n", got, total);
    close(fd);
    return 0;
}
