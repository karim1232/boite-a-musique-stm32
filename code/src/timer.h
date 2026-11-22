/*
 * timer.h
 *
 * Interface des timers matériels TIM2 et TIM5.
 *
 * TIM2 : génère le signal PWM pour le buzzer
 *         Fréquence  = note musicale jouée (réglée via ARR)
 *         Duty cycle = volume sonore       (réglé via CCR2)
 *         Sortie     : PB9 (AF1 = TIM2_CH2)
 *
 * TIM5 : mesure la durée de chaque note et génère une IRQ à la fin
 *         Résolution : 1 µs (base de temps à 1 MHz)
 *         IRQ n°50   : TIM5_Handler() → player_next_note()
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

/*
 * init_TIM2_PWM
 *
 * Configure TIM2 en mode PWM mode 1 sur le canal CH2 (PB9).
 * Le prescaler est réglé pour une fréquence de comptage de 1 MHz.
 * ARR et CCR2 sont mis à jour dynamiquement par jouer_note() dans player.c.
 * La sortie physique est activée / désactivée par buzzer_on/off_PWM().
 */
void init_TIM2_PWM();

/*
 * init_TIM5_compteur
 *
 * Configure TIM5 en compteur simple à 1 MHz avec IRQ sur overflow.
 * À chaque fin de note (TIM5 atteint ARR), TIM5_Handler() est déclenché.
 * ARR est rechargé avec la durée de la note suivante par player_next_note().
 */
void init_TIM5_compteur();

/*
 * TIM5_Handler   (IRQ 50 – déclenché automatiquement par le NVIC)
 *
 * Exécuté à la fin de chaque note (overflow de TIM5).
 * Étapes :
 *   1. Efface le flag UIF dans TIM5.SR
 *   2. Met à jour le volume via reglage_volume() (lecture ADC)
 *   3. Charge la note suivante via player_next_note()
 */
void TIM5_Handler();

#endif /* SRC_TIMER_H_ */
