#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "sender.h"
#include "decoder.h"

#define DEFAULT_TICK_MS 50
#define DOT_TICKS 1
#define DASH_TICKS 3
#define LETTER_GAP_TICKS 6
#define WORD_GAP_TICKS 24

typedef enum {
    INPUT_NONE,
    INPUT_DOT,
    INPUT_DASH
} InputEvent;

static struct termios oldt;
static int old_flags = -1;

static void restore_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    if (old_flags >= 0) {
        fcntl(STDIN_FILENO, F_SETFL, old_flags);
    }
}

static int setup_terminal(void)
{
    struct termios newt;

    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        perror("tcgetattr");
        return 1;
    }

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
        perror("tcsetattr");
        return 1;
    }

    old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (old_flags < 0) {
        perror("fcntl get");
        return 1;
    }

    if (fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK) < 0) {
        perror("fcntl set");
        return 1;
    }

    atexit(restore_terminal);
    return 0;
}

static InputEvent read_input_sample(int *quit)
{
    unsigned char ch;
    InputEvent event = INPUT_NONE;

    *quit = 0;

    while (1) {
        ssize_t n = read(STDIN_FILENO, &ch, 1);

        if (n != 1) {
            break;
        }

        if (ch == 'q') {
            *quit = 1;
        } else if (ch == '\033') {
            unsigned char seq1;
            unsigned char seq2;

            if (read(STDIN_FILENO, &seq1, 1) == 1 &&
                read(STDIN_FILENO, &seq2, 1) == 1 &&
                seq1 == '[') {

                if (seq2 == 'A') {
                    event = INPUT_DOT;
                } else if (seq2 == 'B') {
                    event = INPUT_DASH;
                }
            }
        }
    }

    return event;
}

static int send_bit(int sockfd, struct sockaddr_in *target, char bit)
{
    ssize_t n = sendto(
        sockfd,
        &bit,
        1,
        0,
        (struct sockaddr *)target,
        sizeof(*target)
    );

    if (n < 0) {
        perror("sendto");
        return 1;
    }

    return 0;
}

static void print_local_symbol(InputEvent input, int idle_ticks, int has_symbol)
{
    if (has_symbol && idle_ticks >= WORD_GAP_TICKS) {
        printf(" / ");
    } else if (has_symbol && idle_ticks >= LETTER_GAP_TICKS) {
        putchar(' ');
    }

    if (input == INPUT_DOT) {
        putchar('.');
    } else if (input == INPUT_DASH) {
        putchar('-');
    }

    fflush(stdout);
}

int run_sender(const char *ip, int port, int tick_ms)
{
    int sockfd;
    struct sockaddr_in target;
    useconds_t tick_us;
    int pending_mark_ticks = 0;
    int idle_ticks = 0;
    int has_symbol = 0;

    if (tick_ms <= 0) {
        tick_ms = DEFAULT_TICK_MS;
    }

    tick_us = (useconds_t)tick_ms * 1000;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&target, 0, sizeof(target));

    target.sin_family = AF_INET;
    target.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &target.sin_addr) != 1) {
        fprintf(stderr, "Invalid IP\n");
        close(sockfd);
        return 1;
    }

    if (setup_terminal() != 0) {
        close(sockfd);
        return 1;
    }

    printf("Sender mode\n");
    printf("%dms tick: up = dot, down = dash, q = quit\n", tick_ms);
    decoder_print_morse_table();
    printf("Local keying: ");
    fflush(stdout);

    while (1) {
        int quit = 0;
        InputEvent input = read_input_sample(&quit);

        if (quit) {
            break;
        }

        if (input == INPUT_DOT) {
            print_local_symbol(input, idle_ticks, has_symbol);
            has_symbol = 1;
            pending_mark_ticks += DOT_TICKS;
            idle_ticks = 0;
        } else if (input == INPUT_DASH) {
            print_local_symbol(input, idle_ticks, has_symbol);
            has_symbol = 1;
            pending_mark_ticks += DASH_TICKS;
            idle_ticks = 0;
        }

        char bit = pending_mark_ticks > 0 ? '1' : '0';

        if (pending_mark_ticks > 0) {
            pending_mark_ticks--;
        } else {
            idle_ticks++;
        }

        send_bit(sockfd, &target, bit);

        usleep(tick_us);
    }

    close(sockfd);
    return 0;
}
