#include "init.h"
#include <stdio.h>
#include <stdlib.h>

/* Déclarations préalables des fonctions de routine (qu'on codera à l'étape suivante) */
void *coder_routine(void *arg);
void *monitor_routine(void *arg);

/* Fonction pour libérer la mémoire et détruire les mutexes */
void cleanup_env(t_env *env)
{
    int i;

    i = 0;
    while (i < env->config.nb_coders)
    {
        // On détruit le verrou de chaque dongle
        pthread_mutex_destroy(&env->dongles[i].mutex);
        i++;
    }
    // On libère la mémoire allouée sur la heap
    free(env->coders);
    free(env->dongles);
}

int main(int argc, char **argv)
{
    t_env       env;
    int         i;
    pthread_t   monitor_thread;

    /* 1. Parsing des arguments */
    if (!parse_args(argc, argv, &env.config))
        return (1); // Erreur de parsing, on quitte avec un code d'erreur

    /* 2. Initialisation de l'environnement (mallocs et mutexes) */
    if (!init_env(&env))
        return (1); // Erreur d'allocation, on quitte

    /* 3. Création des threads pour chaque codeur */
    i = 0;
    while (i < env.config.nb_coders)
    {
        pthread_create(&env.coders[i].thread_id, NULL, &coder_routine, &env.coders[i]);
        i++;
    }

    /* 4. Création du thread de surveillance (Monitor) */
    pthread_create(&monitor_thread, NULL, &monitor_routine, &env);

    /* 5. Attente de la fin de la simulation (Join) */
    pthread_join(monitor_thread, NULL);
    i = 0;
    while (i < env.config.nb_coders)
    {
        pthread_join(env.coders[i].thread_id, NULL);
        i++;
    }

    /* 6. Nettoyage de la mémoire avant de quitter */
    cleanup_env(&env);

    return (0); // Fin parfaite
}