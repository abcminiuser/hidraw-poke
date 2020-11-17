#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/hidraw.h>
#include <getopt.h>

// hidraw-poke --write feature --id 2 --device /dev/hidraw0 012345

enum Operation
{
    OP_NONE,
    OP_READ,
    OP_WRITE,
};

enum ReportType
{
    REPORT_INPUT,
    REPORT_OUTPUT,
    REPORT_FEATURE,
};

int main(int argc, char** argv)
{
    static const struct option long_options[] =
        {
            {"device",    required_argument, 0,  'd' },
            {"read",      required_argument, 0,  'r' },
            {"write",     required_argument, 0,  'w' },
            {"id",        required_argument, 0,  'i' },
            {"length",    required_argument, 0,  'l' },
            {0,           0,                 0,  0 }
        };

    static const char* usage_text =
        "\n"
        "Linux HIDRAW Poker\n"
        "\n"
        "Usage: %s --device dev [--id report_id] (--read type --length bytes) | (--write type data)\n"
        "\n"
        "    --device dev        HIDRAW device to use\n"
        "    --read type         Read a report, where type is feature, input or output\n"
        "    --write type        Write a report, where type is feature, input or output\n"
        "    --id report_id      Report ID to read or write\n"
        "    --length bytes      Length of report to read (ignored when writing)\n"
        "    data                Bytes to write\n";

    struct
    {
        char*   path;
        int     operation;
        int     type;
        int     id;
        int     length;
    } context =
        {
            .path       = NULL,
            .operation  = OP_NONE,
            .type       = REPORT_INPUT,
            .id         = 0,
            .length     = -1,
        };

    int option_index;
    int option_char;
    while ((option_char = getopt_long(argc, argv, "d:r:w:i:l:", long_options, &option_index)) != -1)
    {
        switch (option_char)
        {
            case 'd':
            {
                free(context.path);
                context.path = strdup(optarg);
                break;
            }

            case 'r':
            {
                context.operation = OP_READ;

                if (strcasecmp(optarg, "input") == 0)
                    context.type = REPORT_INPUT;
                else if (strcasecmp(optarg, "write") == 0)
                    context.type = REPORT_OUTPUT;
                else
                    context.type = REPORT_FEATURE;

                break;
            }

            case 'w':
            {
                context.operation = OP_WRITE;

                if (strcasecmp(optarg, "input") == 0)
                    context.type = REPORT_INPUT;
                else if (strcasecmp(optarg, "write") == 0)
                    context.type = REPORT_OUTPUT;
                else
                    context.type = REPORT_FEATURE;

                break;
            }

            case 'i':
            {
                context.id = atoi(optarg);
                break;
            }

            case 'l':
            {
                context.length = atoi(optarg);
                break;
            }

            default:
            {
                fprintf(stderr, usage_text, argv[0]);
                exit(1);
            }
        }
    }

    if (! context.path)
    {
        fprintf(stderr, "Error: No device specified.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }

    int hid_handle = open(context.path, O_RDWR);
    if (hid_handle < 0)
    {
        perror("Failed to open device.");
        exit(1);
    }

    return 0;
}
