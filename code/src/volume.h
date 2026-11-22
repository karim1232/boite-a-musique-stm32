/*
 * volume.h
 *
 * Interface du module de contrôle du volume sonore.
 * Le volume est réglé par un potentiomètre analogique lu via l'ADC.
 */

#ifndef SRC_VOLUME_H_
#define SRC_VOLUME_H_

/*
 * reglage_volume
 * Lit la position du potentiomètre via l'ADC et ajuste le duty cycle
 * du PWM de TIM2 pour modifier le volume du buzzer.
 * Appelée par TIM5_Handler() à chaque changement de note.
 */
void reglage_volume();

#endif /* SRC_VOLUME_H_ */
