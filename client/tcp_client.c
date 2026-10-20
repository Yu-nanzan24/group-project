#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int write_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;

    while (sent < len) {
        ssize_t n = write(fd, buf + sent, len - sent);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (n == 0) {
            return -1;
        }

        sent += (size_t)n;
    }

    return 0;
}

int read_line(int fd, char *buf, size_t capacity) {
    size_t i = 0;

    while (i < capacity - 1) {
        char c;
        ssize_t n = read(fd, &c, 1);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (n == 0) {
            return i == 0 ? 0 : -1;
        }

        buf[i++] = c;

        if (c == '\n') {
            buf[i] = '\0';
            return (int)i;
        }
    }

    errno = EMSGSIZE;
    return -1;
}

int main(int argc, char *argv[]) {
    int fd;
    struct sockaddr_in server_addr;
    char line[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s IPv4 PORT\n", argv[0]);
        return 1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address\n");
        close(fd);
        return 1;
    }

    if (connect(fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    while (1) {
        printf("text > ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        if (strchr(line, '\n') == NULL) {
            fprintf(stderr, "Input is too long\n");
            break;
        }

        if (write_all(fd, line, strlen(line)) < 0) {
            perror("write");
            break;
        }

        int n = read_line(fd, response, sizeof(response));

        if (n <= 0) {
            if (n < 0) {
                perror("read");
            }
            break;
        }

        printf("converted > %s", response);
    }

    close(fd);
    return 0;
}
