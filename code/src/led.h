/*
 * led.h
 *
 * Interface de la LED tricolore sur PA8 (rouge), PA9 (vert), PA10 (bleu).
 *
 * Les 7 combinaisons RGB possibles sont utilisees et reparties
 * sur toutes les notes jouees dans le catalogue de melodies :
 *
 *   0b001 Rouge    → DO (toutes octaves), SI3
 *   0b010 Vert     → RE (toutes octaves)
 *   0b011 Jaune    → FA (toutes octaves), CS4, CS5
 *   0b100 Bleu     → MI (toutes octaves)
 *   0b101 Magenta  → SOL (toutes octaves), FS4
 *   0b110 Cyan     → LA (toutes octaves), DS4, GS4
 *   0b111 Blanc    → SI (toutes octaves)
 *   0b000 Eteinte  → SILENCE
 */

#ifndef SRC_LED_H_
#define SRC_LED_H_

#include <stdint.h>

/*
 * init_led_tri
 * Configure PA8, PA9, PA10 en sortie push-pull haute vitesse.
 * A appeler une seule fois au demarrage dans main().
 */
void init_led_tri();

/*
 * set_led_color
 * Applique un masque 3 bits (bits 2=Bleu, 1=Vert, 0=Rouge) sur la LED.
 * Exemple : set_led_color(0b110) → Cyan (vert + bleu)
 */
void set_led_color(uint8_t mask);

/*
 * led_from_note
 * Choisit automatiquement la couleur selon la frequence de la note.
 * Couvre toutes les notes des 5 melodies du catalogue (octaves 3, 4, 5,
 * notes naturelles et diezes).
 * Appelee par player_next_note() a chaque changement de note.
 */
void led_from_note(uint16_t note);

#endif /* SRC_LED_H_ */
