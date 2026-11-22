/*
 * note.c
 *
 * Calcul de la valeur ARR du timer pour jouer une note à la bonne fréquence.
 *
 * Principe :
 *   TIM2 compte de 0 à ARR à une fréquence de FREQ_CNT = 1 000 000 Hz.
 *   La fréquence du signal PWM en sortie est donc :
 *       f_pwm = FREQ_CNT / (ARR + 1)
 *   On en déduit :
 *       ARR = (FREQ_CNT / f_note) - 1
 */

#include "note.h"

/*
 * note_compute_arr
 *
 * Calcule la valeur ARR à charger dans TIM2 pour générer un PWM
 * à la fréquence de la note demandée.
 *
 * Paramètre : freq_note → fréquence en Hz (ex : LA4 = 440)
 * Retour    : valeur ARR à écrire dans TIM2.ARR
 *             0 si freq_note = 0 (cas SILENCE, buzzer géré par buzzer_off_PWM)
 *
 * Exemple :
 *   DO4 = 262 Hz → ARR = (1 000 000 / 262) - 1 = 3816
 *   LA4 = 440 Hz → ARR = (1 000 000 / 440) - 1 = 2272
 */
uint32_t note_compute_arr(uint16_t freq_note)
{
    if (freq_note == 0)
        return 0;   /* Silence : ARR non pertinent, buzzer sera coupé */

    return (FREQ_CNT / freq_note) - 1;
}
