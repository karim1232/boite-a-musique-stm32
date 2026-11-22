/*
 * note.h
 *
 * Définition du type note_t et des constantes de fréquences musicales.
 *
 * Une note est représentée par deux informations :
 *   - freq : fréquence en Hz (ex: DO4 = 262 Hz), 0 = silence
 *   - dur  : valeur musicale relative (noire, croche...) définie dans tempo.h
 *
 * La durée réelle en µs est calculée à l'exécution par :
 *   tempo_duration_ms(dur) * 1000
 * Ce mécanisme permet de changer le tempo (BPM) sans modifier les mélodies.
 *
 * Fréquences de référence (accordage standard LA4 = 440 Hz) :
 *   Octave 3 : 130 Hz (DO3) à 247 Hz (SI3)
 *   Octave 4 : 262 Hz (DO4) à 494 Hz (SI4)   ← octave principale
 *   Octave 5 : 523 Hz (DO5) à 988 Hz (SI5)
 */

#ifndef SRC_NOTE_H_
#define SRC_NOTE_H_

#include <stdint.h>
#include "tempo.h"   /* Pour le type dur_t */

/* ================================================================
   TYPE NOTE
   ================================================================ */

typedef struct {
    uint16_t freq;   /* Fréquence en Hz. SILENCE (0) = pas de son          */
    dur_t    dur;    /* Durée relative : DUR_WHOLE, DUR_HALF, DUR_QUARTER,  */
                     /*                  DUR_EIGHTH, DUR_SIXTEENTH           */
} note_t;

/* ================================================================
   FRÉQUENCE DE BASE DU TIMER
   ================================================================
   FREQ_CNT = 1 000 000 Hz = fréquence de comptage après prescaler.
   Formule : ARR = (FREQ_CNT / freq_note) - 1
   Exemple  : DO4 (262 Hz) → ARR = (1 000 000 / 262) - 1 = 3816
   ================================================================ */
#define FREQ_CNT 1000000

/* ================================================================
   FRÉQUENCES DES NOTES PAR OCTAVE (en Hz)
   ================================================================ */

/* --- Octave 3 --- */
#define DO3   130
#define RE3   147
#define MI3   165
#define FA3   175
#define SOL3  196
#define LA3   220
#define SI3   247

/* --- Octave 4 (octave centrale, utilisée par la LED tricolore) --- */
#define DO4   262
#define RE4   294
#define MI4   330
#define FA4   349
#define SOL4  392
#define LA4   440   /* LA de référence = 440 Hz */
#define SI4   494

/* --- Octave 5 --- */
#define DO5   523
#define RE5   587
#define MI5   659
#define FA5   698
#define SOL5  784
#define LA5   880
#define SI5   988

/* Silence : fréquence nulle → buzzer off pendant la durée de la note */
#define SILENCE 0

/* ================================================================
   NOTES DIEZES
   Utilisees uniquement par Nokia Ringtone.
   ================================================================ */
#define CS4  277    /* C# / Reb  octave 4  (entre DO4 et RE4)  */
#define DS4  311    /* D# / Mib  octave 4  (entre RE4 et MI4)  */
#define FS4  370    /* F# / Solb octave 4  (entre FA4 et SOL4) */
#define GS4  415    /* G# / Lab  octave 4  (entre SOL4 et LA4) */
#define CS5  554    /* C# octave 5         (entre DO5 et RE5)  */
#define FS5  740    /* F# octave 5         (entre FA5 et SOL5) - Berceuse de Brahms */

/* ================================================================
   FONCTION
   ================================================================ */

/*
 * note_compute_arr
 * Calcule la valeur à écrire dans TIM2.ARR pour obtenir la fréquence
 * souhaitée sur la sortie PWM.
 * Formule : ARR = (FREQ_CNT / freq_note) - 1
 * Retourne 0 si freq_note = 0 (silence).
 */
uint32_t note_compute_arr(uint16_t freq_note);

#endif /* SRC_NOTE_H_ */
