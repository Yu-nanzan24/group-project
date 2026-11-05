
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void handle_client(int fd){
    char buf[1024], response[1024];
    ssize_t n;

     while ((n = read_line(fd, buf, sizeof(buf))) > 0) {
        printf("client -> %s", buf);
        fflush(stdout);

        /* 行末の改行を取り除く */
        buf[n - 1] = '\0';

        char *text = NULL;

        /* UPPER: 小文字を大文字に変換 */
        if (strncmp(buf, "UPPER ", 6) == 0) {
            text = buf + 6;

            if (*text == '\0') {
                snprintf(response, sizeof(response),
                         "ERROR invalid-argument\n");
            } else {
                for (int i = 0; text[i] != '\0'; i++) {
                    text[i] = (char)toupper((unsigned char)text[i]);
                }
                snprintf(response, sizeof(response), "OK %s\n", text);
            }

        /* LOWER: 大文字を小文字に変換 */
        } else if (strncmp(buf, "LOWER ", 6) == 0) {
            text = buf + 6;

            if (*text == '\0') {
                snprintf(response, sizeof(response),
                         "ERROR invalid-argument\n");
            } else {
                for (int i = 0; text[i] != '\0'; i++) {
                    text[i] = (char)tolower((unsigned char)text[i]);
                }
                snprintf(response, sizeof(response), "OK %s\n", text);
            }

        /* LENGTH: 文字列のバイト数を返す */
        } else if (strncmp(buf, "LENGTH ", 7) == 0) {
            text = buf + 7;

            if (*text == '\0') {
                snprintf(response, sizeof(response),
                         "ERROR invalid-argument\n");
            } else {
                snprintf(response, sizeof(response), "OK %zu\n",
                         strlen(text));
            }

        /* コマンド名だけで引数がない場合 */
        } else if (strcmp(buf, "UPPER") == 0 ||
                   strcmp(buf, "LOWER") == 0 ||
                   strcmp(buf, "LENGTH") == 0) {
            snprintf(response, sizeof(response),
                     "ERROR invalid-argument\n");

        /* 未対応のコマンド */
        } else {
            snprintf(response, sizeof(response),
                     "ERROR unknown-command\n");
        }

        /* 処理結果を返す */
        if (write_all(fd, response, strlen(response)) < 0) {
            return;
        }
    }

    if (n < 0) {
        perror("read_line");
    }
}

int main(void)
{
    int listen_fd = -1;
    struct sockaddr_in addr = {0};

    signal(SIGPIPE, SIG_IGN);
    /*終了した子プロセスを回収する*/
    signal(SIGCHLD, SIG_IGN);

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
    for (;;) {
        int fd = accept(listen_fd, NULL, NULL);
        if (fd < 0) {
            if (errno == EINTR) continue;
            perror("accept"); break;
    }
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); close(fd); continue; }
    if (pid == 0) {
        close(listen_fd); handle_client(fd);
        close(fd); _exit(0);
        }
    close(fd);
    }
    /* STEP 5: 文字列を受信してコマンドを処理する */

    /* STEP 6: ソケットを閉じる */
done:
    close(listen_fd);

    return EXIT_FAILURE;
}