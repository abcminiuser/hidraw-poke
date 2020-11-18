#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/hidraw.h>
#include <getopt.h>

#ifndef HIDIOCSFEATURE
    #define HIDIOCSFEATURE(len)     _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
    #define HIDIOCGFEATURE(len)     _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif
#ifndef HIDIOCSINPUT
    #define HIDIOCSINPUT(len)       _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x09, len)
    #define HIDIOCGINPUT(len)       _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x0A, len)
#endif
#ifndef HIDIOCSOUTPUT
    #define HIDIOCSOUTPUT(len)      _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x0B, len)
    #define HIDIOCGOUTPUT(len)      _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x0C, len)
#endif

#define HID_REPORT_MAX_SIZE         (16 * 1024)

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


static int report_type_from_string(const char* text)
{
    if (strcasecmp(optarg, "input") == 0)
        return REPORT_INPUT;
    else if (strcasecmp(optarg, "output") == 0)
        return REPORT_OUTPUT;
    else
        return REPORT_FEATURE;
}

static const char* report_type_to_string(int type)
{
    switch (type)
    {
        case REPORT_FEATURE:
            return "feature";
        case REPORT_INPUT:
            return "input";
        case REPORT_OUTPUT:
            return "output";
        default:
            return "?";
    }
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
                context->type = report_type_from_string(optarg);
                break;
            }

            case 'w':
            {
                context->operation = OP_WRITE;
                context->type = report_type_from_string(optarg);
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

    if (! context.path)
    {
        fprintf(stderr, "Error: No device specified.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }

    if (context.operation == OP_READ && context.length <= 0)
    {
        fprintf(stderr, "Error: Read operation requires an explicit length.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }
    else if (context.operation == OP_WRITE)
    {
        context.length = argc - data_arg_index;
    }

    if (context.length <= 0)
    {
        fprintf(stderr, "Error: Invalid length.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }
    else if (context.length > HID_REPORT_MAX_SIZE)
    {
        fprintf(stderr, "Error: Length too large.\n");

        fprintf(stderr, usage_text, argv[0]);
        exit(1);
    }

    const int fd = open(context.path, O_RDWR);
    if (fd < 0)
    {
        perror("Error: Failed to open device");
        exit(1);
    }

    switch (context.operation)
    {
        case OP_READ:
        {
            fprintf(stderr, "Reading %d byte %s report from report ID %d.\n",
                context.length,
                report_type_to_string(context.type),
                context.id);

            uint8_t* buffer = calloc(context.length + 1, 1);
            int      result = -1;

            buffer[0] = context.id;

            switch (context.type)
            {
                case REPORT_FEATURE:
                    result = ioctl(fd, HIDIOCGFEATURE(context.length), buffer);
                    if (result < 0)
                        perror("Error: HIDIOCGFEATURE failed");

                    break;

                case REPORT_INPUT:
                    result = ioctl(fd, HIDIOCGINPUT(context.length), buffer);
                    if (result < 0)
                        perror("Error: HIDIOCGINPUT failed");

                    break;

                case REPORT_OUTPUT:
                    result = ioctl(fd, HIDIOCGOUTPUT(context.length), buffer);
                    if (result < 0)
                        perror("Error: HIDIOCGOUTPUT failed");

                    break;
            }

            if (result < 0)
                exit(1);

            for (int i = 0; i < result; i++)
                printf("%02X ", (uint8_t)buffer[i]);
            printf("\n");

            break;
        }

        case OP_WRITE:
        {
            fprintf(stderr, "Writing %d byte %s report from report ID %d.\n",
                context.length,
                report_type_to_string(context.type),
                context.id);

            uint8_t* buffer = calloc(context.length + 1, 1);
            int      result = -1;

            memset(buffer, 0, context.length + 1);
            buffer[0] = context.id;

            for (int i = 0; i < context.length; i++)
                buffer[1 + i] = strtol(argv[data_arg_index + i], NULL, 16);

            switch (context.type)
            {
                case REPORT_FEATURE:
                    result = ioctl(fd, HIDIOCSFEATURE(context.length), buffer);
                    if (result < 0)
                        perror("Error: HIDIOCSFEATURE failed");

                    break;

                case REPORT_INPUT:
                    result = ioctl(fd, HIDIOCSINPUT(context.length), buffer);
                    if (result < 0)
                        perror("Error: HIDIOCSINPUT failed");

                    break;

                case REPORT_OUTPUT:
                    result = ioctl(fd, HIDIOCSOUTPUT(context.length), buffer);
                    if (result < 0)
                        perror("Error: HIDIOCSOUTPUT failed");

                    break;
            }

            if (result < 0)
                exit(1);

            break;
        }
    }

    return 0;
}
