#include "codexion.h"

static int is_valid_number(const char *str)
{
    int i;

    if (!str || str[0] == '\0')
        return (0);
    i = 0;
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

int parse_args(int argc, char **argv, t_config *config)
{
    int i;

    if (argc != 9)
    {
        fprintf(stderr, "Error: Exactly 8 arguments required.\n");
        return (0);
    }
    i = 1;
    while (i <= 7)
    {
        if (!is_valid_number(argv[i]))
        {
            fprintf(stderr, "Error: Arguments 1-7 must be positive integers.\n");
            return (0);
        }
        i++;
    }
    config->nb_coders = atoi(argv[1]);
    config->time_to_burnout = atoi(argv[2]);
    config->time_to_compile = atoi(argv[3]);
    config->time_to_debug = atoi(argv[4]);
    config->time_to_refactor = atoi(argv[5]);
    config->nb_compiles_required = atoi(argv[6]);
    config->dongle_cooldown = atoi(argv[7]);

    if (config->nb_coders < 1)
    {
        fprintf(stderr, "Error: There must be at least 1 coder.\n");
        return (0);
    }
    if (strcmp(argv[8], "fifo") == 0)
        config->scheduler = FIFO;
    else if (strcmp(argv[8], "edf") == 0)
        config->scheduler = EDF;
    else
    {
        fprintf(stderr, "Error: Scheduler must be 'fifo' or 'edf'.\n");
        return (0);
    }
    return (1);
}