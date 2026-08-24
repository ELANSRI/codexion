#include "init.h"
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

/* --- PARTIE 1 : UTILITAIRES DE TEMPS ET D'AFFICHAGE --- */

/* Renvoie le temps actuel en millisecondes */
long get_time_in_ms(void)
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/* Pause précise (usleep standard est trop imprécis) */
void precise_sleep(long time_in_ms)
{
    long start;
    
    start = get_time_in_ms();
    while ((get_time_in_ms() - start) < time_in_ms)
        usleep(500); // Petite pause pour ne pas faire fondre le processeur
}

/* Affichage protégé par un mutex pour éviter les lignes mélangées */
void print_status(t_coder *coder, const char *status)
{
    long current_time;

    // On verrouille l'écran pour être le seul à écrire
    pthread_mutex_lock(&coder->config->write_mutex); 
    
    current_time = get_time_in_ms() - coder->config->start_time;
    printf("%ld %d %s\n", current_time, coder->id, status);
    
    // On libère l'écran
    pthread_mutex_unlock(&coder->config->write_mutex);
}

/* --- PARTIE 2 : LE CYCLE DE VIE DU CODEUR --- */

/* Déclarations des fonctions d'arbitrage (on les codera à la prochaine étape) */
void acquire_dongles(t_coder *coder);
void release_dongles(t_coder *coder);

void *coder_routine(void *arg)
{
    t_coder *coder;

    coder = (t_coder *)arg;
    
    // Boucle infinie : le codeur répète son cycle jusqu'à la fin de la simulation
    while (1)
    {
        /* 1. Prise des dongles selon les règles FIFO ou EDF */
        acquire_dongles(coder);

        /* 2. Phase de compilation */
        print_status(coder, "is compiling");
        coder->last_compile_start = get_time_in_ms(); // Réinitialise le timer de burnout
        precise_sleep(coder->config->time_to_compile);
        
        /* 3. Libération des dongles */
        release_dongles(coder);
        coder->compile_count++;

        // Si le nombre requis de compilations est atteint, on arrête ce thread
        if (coder->config->nb_compiles_required != -1 && 
            coder->compile_count >= coder->config->nb_compiles_required)
            break;

        /* 4. Phase de Debugging */
        print_status(coder, "is debugging");
        precise_sleep(coder->config->time_to_debug);

        /* 5. Phase de Refactoring */
        print_status(coder, "is refactoring");
        precise_sleep(coder->config->time_to_refactor);
        
        // Et on recommence (il tentera immédiatement de reprendre des dongles)
    }
    return (NULL);
}