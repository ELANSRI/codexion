#include "init.h"
#include <stdlib.h>

int init_env(t_env *env)
{
    int i;

    // 1. Allocation mémoire
    env->coders = malloc(sizeof(t_coder) * env->config.nb_coders);
    env->dongles = malloc(sizeof(t_dongle) * env->config.nb_coders);
    if (!env->coders || !env->dongles)
        return (0);

    // 2. Initialisation des mutexes (les dongles)
    i = 0;
    while (i < env->config.nb_coders)
    {
        pthread_mutex_init(&env->dongles[i].mutex, NULL);
        env->dongles[i].available_at = 0;
        i++;
    }

    // 3. Configuration des codeurs et liaison circulaire
    i = 0;
    while (i < env->config.nb_coders)
    {
        env->coders[i].id = i + 1;
        env->coders[i].compile_count = 0;
        env->coders[i].config = &env->config;
        
        // Le dongle de gauche est celui avec le même index
        env->coders[i].left_dongle = &env->dongles[i];
        
        // Le dongle de droite est le suivant (modulo pour boucler)
        env->coders[i].right_dongle = &env->dongles[(i + 1) % env->config.nb_coders];
        i++;
    }
    return (1);
}