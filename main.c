#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sender.h"
#include "receiver.h"

#define DEFAULT_TICK_MS 50
#define MIN_TICK_MS 10
#define MAX_TICK_MS 5000

static void print_usage(const char *progname) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, " %s send <ip> <port> [-t tick_ms]\n", progname);
    fprintf(stderr, " %s listen <port> \n", progname);
}

static int parse_long(const char *text, long min, long max, long *result)
{
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || value < min || value > max) {
        return 1;
    }

    *result = value;
    return 0;
}

static int parse_port(const char *text, int *port)
{
    long value;

    if (parse_long(text, 1, 65535, &value) != 0) {
        return 1;
    }

    *port = (int)value;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "send") == 0) {
        if (argc != 4 && argc != 6) {
            print_usage(argv[0]);
            return 1;
        }

        const char *ip = argv[2];
        int port;
        int tick_ms = DEFAULT_TICK_MS;

        if (parse_port(argv[3], &port) != 0) {
            fprintf(stderr, "Invalid port: %s\n", argv[3]);
            return 1;
        }

        if (argc == 6) {
            long value;

            if (strcmp(argv[4], "-t") != 0) {
                print_usage(argv[0]);
                return 1;
            }

            if (parse_long(argv[5], MIN_TICK_MS, MAX_TICK_MS, &value) != 0) {
                fprintf(stderr, "Invalid tick length: %s\n", argv[5]);
                fprintf(stderr, "Tick length must be %d-%d ms\n", MIN_TICK_MS, MAX_TICK_MS);
                return 1;
            }

            tick_ms = (int)value;
        }

        return run_sender(ip, port, tick_ms);
    }

    if (strcmp(argv[1], "listen") == 0) {
        if (argc != 3) {
            print_usage(argv[0]);
            return 1;
        }

        int port;

        if (parse_port(argv[2], &port) != 0) {
            fprintf(stderr, "Invalid port: %s\n", argv[2]);
            return 1;
        }

        return run_receiver(port);
    }

    print_usage(argv[0]);
    return 1;
}
