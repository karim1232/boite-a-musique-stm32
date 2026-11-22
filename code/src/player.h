/*
 * player.h
 *
 * Interface du lecteur de musique.
 *
 * Ce module gère :
 *   - La lecture des mélodies note par note (piloté par TIM5)
 *   - Le menu interactif UART (mode MANUEL via PB8)
 *   - Les actions des switches SW2 / SW3 / SW4
 *   - Le contrôle de lecture : pause, resume, stop
 */

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#include <stdint.h>
#include "note.h"

/* ================================================================
   VARIABLES GLOBALES PARTAGÉES
   (déclarées dans player.c, utilisées par timer.c et button.c)
   ================================================================ */

extern volatile uint32_t  i;              /* Index de la note courante dans le tableau  */
extern volatile uint32_t  taille_melodie; /* Nombre total de notes de la mélodie        */
extern note_t             note_curr;      /* Note en cours de lecture                   */
extern const note_t      *melodie;        /* Pointeur sur le tableau de notes actif     */
extern volatile uint32_t  cycle_val;      /* Diviseur du duty cycle PWM (volume)        */

/* ----------------------------------------------------------------
   Flags d'état – partagés avec button.c
   ---------------------------------------------------------------- */
extern uint8_t g_demander; /* 1 = le menu interactif doit être affiché      */
extern uint8_t g_playing;  /* 1 = une mélodie est en cours de lecture       */

/* ================================================================
   CONTRÔLE DU BUZZER
   ================================================================ */

/* Active la sortie PWM de TIM2 vers le buzzer (CCER.CC2E = 1) */
void buzzer_on_PWM(void);

/* Coupe la sortie PWM → silence (CCER.CC2E = 0) */
void buzzer_off_PWM(void);

/* ================================================================
   LECTURE D'UNE NOTE
   ================================================================ */

/*
 * jouer_note
 * Charge TIM2 avec la fréquence et le duty cycle de la note,
 * et TIM5 avec la durée de la note en µs.
 * Paramètre : fréquence en Hz (ex : DO4 = 262)
 */
void jouer_note(uint16_t freq_note);

/* ================================================================
   INITIALISATION
   ================================================================ */

/*
 * player_init
 * Initialise le player avec un tableau de notes et lance la lecture.
 * Configure TIM5 et active son IRQ dans le NVIC.
 * Paramètres :
 *   mel    : pointeur sur le tableau de notes (melodie_xxx dans melodie.c)
 *   taille : nombre de notes dans le tableau
 */
void player_init(const note_t *mel, uint32_t taille);

/* ================================================================
   MODE DE LECTURE
   ================================================================ */

/*
 * player_set_mode
 * repeat = 1 : lecture en boucle (recommence au début en fin de mélodie)
 * repeat = 0 : lecture unique    (s'arrête après la dernière note)
 */
void player_set_mode(uint8_t repeat);

/*
 * player_is_done
 * Retourne 1 si la mélodie est terminée (en mode "une seule fois").
 * Remis à 0 automatiquement au prochain appel à player_init().
 */
uint8_t player_is_done(void);

/* ================================================================
   VOLUME
   ================================================================ */

/*
 * player_set_duty_cycle
 * Ajuste le volume en changeant le diviseur du duty cycle PWM.
 * diviseur = 2  → 50% duty → son fort
 * diviseur = 10 → 10% duty → son faible
 * Appelé par reglage_volume() dans volume.c.
 */
void player_set_duty_cycle(uint32_t diviseur);

/* ================================================================
   CONTRÔLE DE LA LECTURE
   ================================================================ */

/* Gèle la lecture : buzzer off + TIM5 arrêté (position conservée) */
void player_pause(void);

/* Reprend la lecture là où elle s'était arrêtée */
void player_resume(void);

/* Arrête complètement la lecture et remet l'index à 0 */
void player_stop(void);

/* ================================================================
   AVANCEMENT NOTE PAR NOTE
   ================================================================ */

/*
 * player_next_note
 * Appelée par TIM5_Handler() à chaque fin de note.
 * Charge la note suivante dans TIM2 et TIM5.
 * Gère la fin de mélodie (reboucle ou lève g_done).
 */
void player_next_note(void);

/* ================================================================
   ACTIONS DES SWITCHES
   ================================================================ */

/* SW2 (PB4) : passe à la chanson suivante dans le catalogue */
void player_next_song(void);

/* SW3 (PB5) : cycle le BPM : 60 → 90 → 120 → 160 → 60 BPM */
void player_next_tempo(void);

/*
 * SW4 (PB6) : toggle play/stop en mode automatique (sans menu)
 *   Si en lecture  → stop + LED éteinte
 *   Si à l'arrêt  → lance la chanson courante en boucle
 */
void player_toggle_mode(void);

/* ================================================================
   MENU INTERACTIF
   ================================================================ */

/*
 * demander_et_lancer
 * Affiche le menu sur GTKTerm (UART) et attend les choix de l'utilisateur.
 * Démarre ensuite la lecture avec les paramètres sélectionnés.
 * Appelée depuis main() quand player_needs_menu() retourne 1.
 */
void demander_et_lancer(void);

/*
 * player_check_done
 * Détecte la fin de mélodie en mode "une seule fois".
 * À appeler régulièrement dans la boucle principale de main().
 */
void player_check_done(void);

/*
 * player_needs_menu
 * Retourne 1 si le menu doit être affiché (PB8 appuyé).
 * Mis à 1 par EXTI9_5_Handler() dans button.c.
 */
uint8_t player_needs_menu(void);

#endif /* SRC_PLAYER_H_ */
