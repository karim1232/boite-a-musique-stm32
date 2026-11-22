/*
 * tempo.c
 *
 * Gestion du tempo et conversion des valeurs musicales en durées en ms.
 *
 * Seul g_bpm est global : il est modifiable à tout moment (SW3 ou menu)
 * et la prochaine note jouée reflétera automatiquement le nouveau tempo.
 */

#include "tempo.h"
#include <stdint.h>

/* BPM courant – initialisé à 60, modifié par tempo_set_bpm() */
volatile uint16_t g_bpm = 60;

/* ================================================================
   GESTION DU BPM
   ================================================================ */

/*
 * tempo_set_bpm
 * Met à jour g_bpm. Le nouveau tempo prendra effet dès la prochaine note.
 * Protection contre la valeur 0 qui provoquerait une division par zéro.
 */
void tempo_set_bpm(uint16_t bpm)
{
    if (bpm == 0) bpm = 1;
    g_bpm = bpm;
}

/* ================================================================
   DURÉES MUSICALES
   ================================================================ */

/*
 * tempo_quarter_ms  (statique : usage interne uniquement)
 *
 * Calcule la durée d'une noire en millisecondes selon le BPM courant.
 *   Formule : 60 000 ms / BPM
 *   Exemples :
 *     60  BPM → 1000 ms  (1 seconde par noire)
 *     90  BPM →  667 ms
 *     120 BPM →  500 ms
 *     160 BPM →  375 ms
 */
static uint16_t tempo_quarter_ms(void)
{
    return (uint16_t)(60000 / g_bpm);
}

/*
 * tempo_duration_ms
 * Convertit une valeur musicale (dur_t) en durée en millisecondes.
 * Toutes les durées sont exprimées en multiple ou fraction de la noire.
 *
 * Paramètre : d → valeur musicale (DUR_WHOLE, DUR_HALF, ...)
 * Retour    : durée en ms selon le BPM courant
 */
uint16_t tempo_duration_ms(dur_t d)
{
    uint32_t q = tempo_quarter_ms();   /* Durée d'une noire en ms */

    switch (d) {
        case DUR_WHOLE:      return q * 4;   /* Ronde         : 4 × noire */
        case DUR_HALF:       return q * 2;   /* Blanche       : 2 × noire */
        case DUR_QUARTER:    return q;        /* Noire         : 1 × noire */
        case DUR_EIGHTH:     return q / 2;   /* Croche        : ½ noire   */
        case DUR_SIXTEENTH:  return q / 4;   /* Double croche : ¼ noire   */
        default:             return q;        /* Valeur par défaut : noire */
    }
}
