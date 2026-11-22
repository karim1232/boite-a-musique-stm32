/*
 * melodie.h
 *
 * Catalogue des melodies du projet.
 *
 * Chaque melodie est definie dans melodie.c comme un tableau de note_t.
 * La taille de chaque tableau est calculee automatiquement dans melodie.c
 * via sizeof et exportee comme constante uint32_t.
 *
 * Tempos recommandes :
 *   1. Frere Jacques        → 120 BPM  (allant,  4/4)
 *   2. Au Clair de la Lune  →  80 BPM  (lent,    4/4)
 *   3. Happy Birthday       →  90 BPM  (modere,  3/4 simule)
 *   4. Nokia Ringtone       → 120 BPM  (Nokia 3310 original, 4/4)
 *   5. Twinkle Twinkle      → 120 BPM  (vif,     4/4)
 *
 * Pour ajouter une melodie :
 *   1. Declarer le tableau et sa taille ici (extern)
 *   2. Definir le tableau dans melodie.c
 *   3. Ajouter une entree dans catalogue[] de player.c
 */

#ifndef SRC_MELODIE_H_
#define SRC_MELODIE_H_

#include <stdint.h>
#include "note.h"    /* note_t, SILENCE, frequences (DO4, RE4...) */
#include "tempo.h"   /* dur_t (DUR_QUARTER, DUR_HALF...)           */

/* ================================================================
   DECLARATIONS DES MELODIES
   ================================================================ */

/* 1. Frere Jacques — Do majeur, 4/4, 120 BPM */
extern const note_t   melodie_frere_jacques[];
extern const uint32_t melodie_frere_jacques_size;

/* 2. Au Clair de la Lune — Do majeur, 4/4, 80 BPM */
extern const note_t   melodie_au_clair[];
extern const uint32_t melodie_au_clair_size;

/* 3. Happy Birthday — Sol majeur, 3/4, 90 BPM */
extern const note_t   melodie_happy_birthday[];
extern const uint32_t melodie_happy_birthday_size;

/* 4. Nokia Ringtone — La majeur, 4/4, 120 BPM */
extern const note_t   melodie_nokia[];
extern const uint32_t melodie_nokia_size;

/* 5. Twinkle Twinkle Little Star — Do majeur, 4/4, 120 BPM */
extern const note_t   melodie_twinkle[];
extern const uint32_t melodie_twinkle_size;



/* 6. Berceuse de Brahms Op. 49 No. 4 — Sol majeur, 3/4, 65 BPM */
extern const note_t   melodie_brahms[];
extern const uint32_t melodie_brahms_size;

#endif /* SRC_MELODIE_H_ */
