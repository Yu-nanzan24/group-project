#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/* TCPは1行ずつ通信する */
static ssize_t read_line(int fd, char *buf, size_t capacity)
{
    size_t used = 0;
    while (used + 1 < capacity) {
        ssize_t n = read(fd, buf + used, 1);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) {
            if (used == 0) return 0;
            errno = EPROTO;
            return -1;
        }
        if (buf[used++] == '\n') {
            buf[used] = '\0';
            return (ssize_t)used;
        }
    }
    errno = EMSGSIZE;
    return -1;
}

/* データをすべて送信する */
static int write_all(int fd, const char *buf, size_t length)
{
    size_t sent = 0;
    while (sent < length) {
        ssize_t n = write(fd, buf + sent, length - sent);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("write");
            return -1;
        }
        if (n == 0) {
            errno = EIO;
            perror("write");
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

int main(void)
{
    int listen_fd = -1, client_fd = -1, status = EXIT_FAILURE;
    struct sockaddr_in addr = {0};
    char buf[1024];
    ssize_t n;

    signal(SIGPIPE, SIG_IGN);

    /* STEP 1: ソケットを作成 */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        goto done;
    }

    int yes = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   &yes, sizeof(yes)) < 0) {
        perror("setsockopt");
        goto done;
    }

    /* STEP 2: ポート5000を設定 */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        goto done;
    }

    /* STEP 3: 接続を待つ */
    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        goto done;
    }

    puts("LISTEN 0.0.0.0:5000");
    fflush(stdout);

    /* STEP 4: クライアントを受け付ける */
    client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        goto done;
    }

    printf("connected: listen_fd=%d client_fd=%d\n",
           listen_fd, client_fd);
    fflush(stdout);

    /* STEP 5: 文字列を受信して大文字に変換する */
    status = EXIT_SUCCESS;
    while ((n = read_line(client_fd, buf, sizeof(buf))) > 0) {
        printf("client -> %s", buf);
        fflush(stdout);

        /* 小文字を大文字に変換 */
        for (int i = 0; buf[i] != '\0'; i++) {
            buf[i] = (char)toupper((unsigned char)buf[i]);
        }

        /* 変換した文字列を返す */
        if (write_all(client_fd, buf, (size_t)n) < 0) break;
    }

    if (n < 0) {
        perror("read_line");
        status = EXIT_FAILURE;
    }

    puts("connection closed; restart server for the next client");

done:
    /* STEP 6: ソケットを閉じる */
    if (client_fd >= 0) close(client_fd);
    if (listen_fd >= 0) close(listen_fd);

    return status;
}
