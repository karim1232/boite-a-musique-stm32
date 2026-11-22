/*
 * tempo.h
 *
 * Gestion du tempo musical (BPM) et conversion des valeurs musicales en ms.
 *
 * Le BPM (Beats Per Minute) définit la vitesse de lecture.
 * Une "noire" (DUR_QUARTER) dure : 60 000 ms / BPM
 *
 * Exemple à 120 BPM :
 *   Noire         = 60 000 / 120 = 500 ms
 *   Blanche       = 500 × 2      = 1000 ms
 *   Ronde         = 500 × 4      = 2000 ms
 *   Croche        = 500 / 2      = 250 ms
 *   Double croche = 500 / 4      = 125 ms
 *
 * Le BPM peut être changé en temps réel (SW3) sans modifier les mélodies,
 * car les durées sont calculées à la volée par tempo_duration_ms().
 */

#ifndef SRC_TEMPO_H_
#define SRC_TEMPO_H_

#include <stdint.h>

/* ================================================================
   VALEURS MUSICALES
   ================================================================ */

typedef enum {
    DUR_WHOLE,        /* Ronde         : 4 × noire */
    DUR_HALF,         /* Blanche       : 2 × noire */
    DUR_QUARTER,      /* Noire         : 1 × noire */
    DUR_EIGHTH,       /* Croche        : ½ noire   */
    DUR_SIXTEENTH     /* Double croche : ¼ noire   */
} dur_t;

/* ================================================================
   BPM GLOBAL
   ================================================================ */

/* Variable globale accessible depuis player.c pour la lecture du BPM courant */
extern volatile uint16_t g_bpm;

/* ================================================================
   FONCTIONS
   ================================================================ */

/*
 * tempo_set_bpm
 * Définit le tempo en BPM. Valeur minimale : 1 (protection division par zéro).
 * Appelé par demander_et_lancer() (menu) et player_next_tempo() (SW3).
 */
void tempo_set_bpm(uint16_t bpm);

/*
 * tempo_duration_ms
 * Convertit une valeur musicale (dur_t) en durée en millisecondes,
 * selon le BPM courant (g_bpm).
 * Utilisé par player.c pour charger TIM5.ARR (× 1000 pour µs).
 */
uint16_t tempo_duration_ms(dur_t d);

#endif /* SRC_TEMPO_H_ */
