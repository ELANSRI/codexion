#include "parsing.h"

/* Fonction utilitaire pour vérifier si une chaîne ne contient que des chiffres */
static int is_valid_number(const char *str)
{
    int i;

    if (!str || str[0] == '\0')
        return (0);
    i = 0;
    while (str[i])
    {
        /* Si le caractère n'est pas un chiffre entre 0 et 9, c'est invalide */
        if (str[i] < '0' || str[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

/* Fonction principale de parsing */
int parse_args(int argc, char **argv, t_config *config)
{
    int i;

    /* 1. Vérification du nombre d'arguments (1 nom du programme + 8 arguments = 9) */
    if (argc != 9)
    {
        fprintf(stderr, "Error: Invalid number of arguments. 8 arguments required.\n");
        return (0);
    }

    /* 2. Vérification que les arguments 1 à 7 sont bien des entiers positifs */
    i = 1;
    while (i <= 7)
    {
        if (!is_valid_number(argv[i]))
        {
            fprintf(stderr, "Error: Arguments 1 to 7 must be positive integers.\n");
            return (0);
        }
        i++;
    }

    /* 3. Conversion des chaînes en entiers avec atoi */
    config->nb_coders = atoi(argv[1]);
    config->time_to_burnout = atoi(argv[2]);
    config->time_to_compile = atoi(argv[3]);
    config->time_to_debug = atoi(argv[4]);
    config->time_to_refactor = atoi(argv[5]);
    config->nb_compiles_required = atoi(argv[6]);
    config->dongle_cooldown = atoi(argv[7]);

    /* Sécurité logique : il faut au moins 1 codeur pour la simulation */
    if (config->nb_coders < 1)
    {
        fprintf(stderr, "Error: There must be at least 1 coder.\n");
        return (0);
    }

    /* 4. Parsing spécifique pour le scheduler (argument 8) */
    if (strcmp(argv[8], "fifo") == 0)
        config->scheduler = FIFO;
    else if (strcmp(argv[8], "edf") == 0)
        config->scheduler = EDF;
    else
    {
        fprintf(stderr, "Error: Scheduler must be exactly 'fifo' or 'edf'.\n");
        return (0);
    }

    /* Tout s'est bien passé, on retourne 1 (vrai) */
    return (1);
}