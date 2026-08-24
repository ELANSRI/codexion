#ifndef INIT_H
# define INIT_H

# include <pthread.h>
# include "parsing.h"

typedef struct s_dongle {
    pthread_mutex_t mutex;
    long            available_at; // Pour gérer le cooldown
} t_dongle;
    
typedef struct s_coder {
    int             id;
    int             compile_count;
    long            last_compile_start;
    pthread_t       thread_id;
    t_dongle        *left_dongle;
    t_dongle        *right_dongle;
    t_config        *config;
} t_coder;

typedef struct s_env {
    t_config        config;
    t_dongle        *dongles;
    t_coder         *coders;
} t_env;

int init_env(t_env *env);

#endif