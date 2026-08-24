#ifndef PARSING_H
# define PARSING_H

# include <stdlib.h> // Pour atoi
# include <string.h> // Pour strcmp
# include <stdio.h>  // Pour fprintf
#include <sys/_pthread/_pthread_mutex_t.h>

/* Enumération pour rendre le type de scheduler explicite et lisible */
typedef enum e_scheduler {
    FIFO,
    EDF
} t_scheduler;

/* Structure contenant toute la configuration de la simulation */
typedef struct s_config {
    int nb_coders;
    int time_to_burnout;
    int time_to_compile;
    int time_to_debug;
    int time_to_refactor;
    int nb_compiles_required;
    int dongle_cooldown;
    t_scheduler scheduler;
    long            start_time;
    pthread_mutex_t write_mutex;
} t_config;

int parse_args(int argc, char **argv, t_config *config);

#endif