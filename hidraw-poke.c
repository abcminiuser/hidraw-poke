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

enum operation
{
    OP_NONE,
    OP_READ,
    OP_WRITE,
};

enum report_type
{
    REPORT_INPUT,
    REPORT_OUTPUT,
    REPORT_FEATURE,
};

struct app_context
{
    char*   path;
    int     operation;
    int     type;
    int     id;
    int     length;
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

static const struct option long_options[] =
    {
        {"device",    required_argument, 0,  'd' },
        {"read",      required_argument, 0,  'r' },
        {"write",     required_argument, 0,  'w' },
        {"id",        required_argument, 0,  'i' },
        {"length",    required_argument, 0,  'l' },
        {0,           0,                 0,  0 }
    };

static const char* short_options =  "d:r:w:i:l:";


static int strtoop(const char* text)
{
    if (strcasecmp(optarg, "input") == 0)
        return REPORT_INPUT;
    else if (strcasecmp(optarg, "output") == 0)
        return REPORT_OUTPUT;
    else
        return REPORT_FEATURE;
}

static int parse_args(int argc, char** argv, struct app_context* context)
{
    int option_index;
    int option_char;

    while ((option_char = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (option_char)
        {
            case 'd':
            {
                free(context->path);
                context->path = strdup(optarg);
                break;
            }

            case 'r':
            {
                context->operation = OP_READ;
                context->type = strtoop(optarg);
                break;
            }

            case 'w':
            {
                context->operation = OP_WRITE;
                context->type = strtoop(optarg);
                break;
            }

            case 'i':
            {
                context->id = atoi(optarg);
                break;
            }

            case 'l':
            {
                context->length = atoi(optarg);
                break;
            }

            case '?':
            {
                return -1;
            }
        }
    }

    return optind;
}

int main(int argc, char** argv)
{
    struct app_context context =
        {
            .path       = NULL,
            .operation  = OP_NONE,
            .type       = REPORT_INPUT,
            .id         = 0,
            .length     = -1,
        };

    const int data_arg_index = parse_args(argc, argv, &context);

    if (data_arg_index < 0)
    {
        fprintf(stderr, usage_text, argv[0]);
        exit(0);
    }
    else if (! context.path)
    {
        fprintf(stderr, "Error: No device specified.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }
    else if (context.operation == OP_NONE)
    {
        fprintf(stderr, "Error: No read or write operation specified.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }
    else if (context.operation == OP_READ && context.length <= 0)
    {
        fprintf(stderr, "Error: Read operation requires an explicit length.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }

    const int hid_handle = open(context.path, O_RDWR);
    if (hid_handle < 0)
    {
        perror("Failed to open device.");
        exit(1);
    }

    return 0;
}
