#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <time.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

typedef enum e_scheduler {
    FIFO,
    EDF
}   t_scheduler;

/* Élément d'une requête dans le Tas (Priority Queue) */
typedef struct s_req {
    int     coder_id;
    long    priority_key; // Timestamp d'arrivée (FIFO) ou Deadline (EDF)
    long    arrival_time; // Pour départager les égalités en EDF
}   t_req;

/* Structure du Min-Heap pour l'arbitrage */
typedef struct s_heap {
    t_req   *data;
    int     size;
    int     capacity;
}   t_heap;

/* Configuration globale de la simulation */
typedef struct s_config {
    int             nb_coders;
    int             time_to_burnout;
    int             time_to_compile;
    int             time_to_debug;
    int             time_to_refactor;
    int             nb_compiles_required;
    int             dongle_cooldown;
    t_scheduler     scheduler;
    long            start_time;
    int             simulation_running;
    pthread_mutex_t write_mutex;
    pthread_mutex_t stop_mutex;
}   t_config;

/* Structure d'un Dongle */
typedef struct s_dongle {
    int     id;
    int     in_use;
    long    available_at; // Gère le cooldown
    t_heap  wait_queue;   // File de priorité pour ce dongle
}   t_dongle;

/* Structure d'un Codeur */
typedef struct s_coder {
    int             id;
    int             compile_count;
    long            last_compile_start;
    pthread_mutex_t state_mutex;
    pthread_t       thread_id;
    t_dongle        *left_dongle;
    t_dongle        *right_dongle;
    struct s_env    *env;
}   t_coder;

/* Environnement central */
typedef struct s_env {
    t_config        config;
    t_dongle        *dongles;
    t_coder         *coders;
    pthread_mutex_t table_mutex; // Verrou central pour l'arbitre
    pthread_cond_t  table_cond;  // Condition variable pour réveiller les codeurs
}   t_env;

/* --- Prototypes --- */
/* parsing.c */
int     parse_args(int argc, char **argv, t_config *config);

/* heap.c */
int     heap_init(t_heap *h, int capacity);
void    heap_free(t_heap *h);
int     heap_push(t_heap *h, t_req req, t_scheduler sched);
int     heap_peek(t_heap *h, t_req *out);
void    heap_remove_id(t_heap *h, int coder_id, t_scheduler sched);

/* init.c */
int     init_env(t_env *env);
void    cleanup_env(t_env *env);

/* utils.c */
long    get_time_in_ms(void);
void    precise_sleep(long time_in_ms);
void    print_status(t_coder *coder, const char *status);
int     is_simulation_running(t_config *config);
void    stop_simulation(t_config *config);

/* arbitration.c */
void    acquire_dongles(t_coder *coder);
void    release_dongles(t_coder *coder);

/* monitor.c */
void    *monitor_routine(void *arg);

/* routine.c */
void    *coder_routine(void *arg);

#endif