/*
 * volume.c
 *
 * Contrôle du volume sonore via le potentiomètre et l'ADC.
 *
 * Principe :
 *   Le potentiomètre retourne une valeur entre 0 et 4095 (ADC 12 bits).
 *   Cette valeur est convertie en un diviseur de duty cycle pour TIM2.
 *
 *   Volume fort  → potentiomètre au maximum → duty cycle élevé
 *   Volume faible → potentiomètre au minimum → duty cycle faible
 *
 *   Formule utilisée :
 *     mes = (valeur_adc * 50) / 4095   → plage 0 à 50
 *     mes = mes / 100                  → plage 0.0 à 0.5
 *     diviseur = 1 / mes               → inversé : grand diviseur = petit duty
 *
 *   Ce diviseur est passé à player_set_duty_cycle() qui applique :
 *     TIM2.CCR2 = TIM2.ARR / diviseur
 */

#include "adc.h"      /* mesure_potentiometre() : lecture ADC 0–4095       */
#include "systick.h"  /* (inclus pour la compatibilité du module adc)       */
#include "player.h"   /* player_set_duty_cycle()                            */

/*
 * reglage_volume
 *
 * Lit le potentiomètre, calcule le diviseur de duty cycle et l'applique.
 * Appelée automatiquement par TIM5_Handler() à chaque fin de note,
 * ce qui assure une mise à jour du volume en temps réel.
 */
void reglage_volume()
{
    float mes;

    /* Lecture ADC : valeur entre 0 et 4095                               */
    /* Conversion : ramène la valeur sur la plage 0.0 à 0.5               */
    mes = (mesure_potentiometre() * 50) / 4095;
    mes /= 100;

    /* Calcul du diviseur (inverse) puis application sur TIM2.CCR2        */
    player_set_duty_cycle(1 / mes);
}
