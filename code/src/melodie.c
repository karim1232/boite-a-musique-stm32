/*
 * melodie.c
 *
 * Catalogue des melodies du lecteur de musique STM32.
 *
 * Chaque melodie est un tableau de note_t : { frequence_Hz, valeur_musicale }.
 * Les durees (DUR_QUARTER, DUR_HALF...) sont converties en µs a l'execution
 * par tempo_duration_ms(dur) * 1000 selon le BPM courant.
 *
 * Tempos recommandes par melodie :
 *   1. Frere Jacques        → 120 BPM  (allant,  4/4)
 *   2. Au Clair de la Lune  →  80 BPM  (lent,    4/4)
 *   3. Happy Birthday       →  90 BPM  (modere,  3/4 simule)
 *   4. Nokia Ringtone       → 120 BPM  (original Nokia 3310, 4/4)
 *   5. Twinkle Twinkle      → 120 BPM  (vif,     4/4)
 */

#include "melodie.h"

/* ============================================================
   1. FRERE JACQUES
   Tonalite : Do majeur  |  Mesure : 4/4  |  Tempo : 120 BPM
   ============================================================
   Rythme : noires (DUR_QUARTER) sur les notes simples,
            blanches (DUR_HALF) sur les tenues de fin de phrase,
            croches  (DUR_EIGHTH) sur le passage rapide "sonnez les matines".

   Structure :
     Frere Jacques (x2)        : DO RE MI DO
     Dormez-vous   (x2)        : MI FA SOL.
     Sonnez les matines (x2)   : SOL LA SOL FA  MI DO
     Din din don (x2)          : DO SOL DO.
   ============================================================ */

const note_t melodie_frere_jacques[] = {

    /* Frere Jacques (x2) */
    {DO4, DUR_QUARTER}, {RE4, DUR_QUARTER}, {MI4, DUR_QUARTER}, {DO4, DUR_QUARTER},
    {DO4, DUR_QUARTER}, {RE4, DUR_QUARTER}, {MI4, DUR_QUARTER}, {DO4, DUR_QUARTER},

    /* Dormez-vous (x2) — SOL tenu = blanche */
    {MI4, DUR_QUARTER}, {FA4, DUR_QUARTER}, {SOL4, DUR_HALF},
    {MI4, DUR_QUARTER}, {FA4, DUR_QUARTER}, {SOL4, DUR_HALF},

    /* Sonnez les matines (x2) — croches rapides SOL LA SOL FA puis MI DO */
    {SOL4, DUR_EIGHTH}, {LA4, DUR_EIGHTH}, {SOL4, DUR_EIGHTH}, {FA4, DUR_EIGHTH},
    {MI4,  DUR_QUARTER}, {DO4, DUR_QUARTER},
    {SOL4, DUR_EIGHTH}, {LA4, DUR_EIGHTH}, {SOL4, DUR_EIGHTH}, {FA4, DUR_EIGHTH},
    {MI4,  DUR_QUARTER}, {DO4, DUR_QUARTER},

    /* Din din don (x2) — DO final tenu = blanche */
    {DO4, DUR_QUARTER}, {SOL4, DUR_QUARTER}, {DO4, DUR_HALF},
    {DO4, DUR_QUARTER}, {SOL4, DUR_QUARTER}, {DO4, DUR_HALF},
};
const uint32_t melodie_frere_jacques_size =
    sizeof(melodie_frere_jacques) / sizeof(note_t);


/* ============================================================
   2. AU CLAIR DE LA LUNE
   Tonalite : Do majeur  |  Mesure : 4/4  |  Tempo : 80 BPM
   ============================================================
   Notes en blanches (DUR_HALF) et rondes (DUR_WHOLE) qui donnent
   le caractere lent et melancolique de cette comptine.

   Structure (4 phrases de 3 mesures) :
     Phrase 1 : DO DO DO RE | MI.(ronde) RE.(ronde) | DO MI RE RE DO(ronde)
     Phrase 2 : repetition de la phrase 1
     Phrase 3 : RE RE RE RE | LA3.(ronde) LA3.(ronde) | RE DO SI3 LA3 SOL3(ronde)
     Phrase 4 : reprise finale de la phrase 1
   ============================================================ */

const note_t melodie_au_clair[] = {

    /* --- Phrase 1 --- */
    {DO4,     DUR_HALF},    /* Au    */
    {DO4,     DUR_HALF},    /* clair */
    {DO4,     DUR_HALF},    /* de    */
    {RE4,     DUR_HALF},    /* la    */
    {MI4,     DUR_WHOLE},   /* lu-   (ronde) */
    {RE4,     DUR_WHOLE},   /* ne    (ronde) */
    {DO4,     DUR_HALF},    /* mon   */
    {MI4,     DUR_HALF},    /* a-    */
    {RE4,     DUR_HALF},    /* mi    */
    {RE4,     DUR_HALF},    /* Pier- */
    {DO4,     DUR_WHOLE},   /* rot   (ronde finale) */
    {SILENCE, DUR_WHOLE},   /* silence entre les phrases */

    /* --- Phrase 2 (repetition) --- */
    {DO4,     DUR_HALF},
    {DO4,     DUR_HALF},
    {DO4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {MI4,     DUR_WHOLE},
    {RE4,     DUR_WHOLE},
    {DO4,     DUR_HALF},
    {MI4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {DO4,     DUR_WHOLE},
    {SILENCE, DUR_WHOLE},

    /* --- Phrase 3 : pont grave (octave 3) --- */
    {RE4,     DUR_HALF},    /* RE repete 4 fois = 2 mesures */
    {RE4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {LA3,     DUR_WHOLE},   /* descente vers LA3 (ronde) */
    {LA3,     DUR_WHOLE},
    {RE4,     DUR_HALF},
    {DO4,     DUR_HALF},
    {SI3,     DUR_HALF},
    {LA3,     DUR_HALF},
    {SOL3,    DUR_WHOLE},   /* SOL3 grave final */

    /* --- Phrase 4 : finale --- */
    {DO4,     DUR_HALF},
    {DO4,     DUR_HALF},
    {DO4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {MI4,     DUR_WHOLE},
    {RE4,     DUR_WHOLE},
    {DO4,     DUR_HALF},
    {MI4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {RE4,     DUR_HALF},
    {DO4,     DUR_WHOLE},
    {SILENCE, DUR_WHOLE},
};
const uint32_t melodie_au_clair_size =
    sizeof(melodie_au_clair) / sizeof(note_t);


/* ============================================================
   3. HAPPY BIRTHDAY
   Tonalite : Sol majeur  |  Mesure : 3/4  |  Tempo : 90 BPM
   ============================================================
   La mesure a 3 temps est simulee en combinant noires (DUR_QUARTER)
   et croches (DUR_EIGHTH) separees de courts silences.
   Cela reproduit la levee rythmique caracteristique du 3/4.

   Patron par vers :
     "Hap- py   Birth- day   to    you"
      croche noire  noire  noire  noire  blanche + silence

   Structure : 4 vers de 6 notes
     Vers 1 : SOL SOL LA SOL DO5 SI4.
     Vers 2 : SOL SOL LA SOL RE5 DO5.
     Vers 3 : SOL SOL SOL5 MI5 DO5 SI4 LA4.
     Vers 4 : FA5 FA5 MI5 DO5 RE5 DO5.
   ============================================================ */

const note_t melodie_happy_birthday[] = {

    /* "Happy Birthday to you" — vers 1 */
    {SOL4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {SOL4, DUR_EIGHTH},  {SILENCE, DUR_EIGHTH},
    {LA4,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {SOL4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {DO5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {SI4,  DUR_HALF},    {SILENCE, DUR_HALF},

    /* "Happy Birthday to you" — vers 2 (monte vers RE5) */
    {SOL4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {SOL4, DUR_EIGHTH},  {SILENCE, DUR_EIGHTH},
    {LA4,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {SOL4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {RE5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {DO5,  DUR_HALF},    {SILENCE, DUR_HALF},

    /* "Happy Birthday dear [name]" — vers 3 (point culminant SOL5) */
    {SOL4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {SOL4, DUR_EIGHTH},  {SILENCE, DUR_EIGHTH},
    {SOL5, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {MI5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {DO5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {SI4,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {LA4,  DUR_HALF},    {SILENCE, DUR_HALF},

    /* "Happy Birthday to you" — vers 4 finale */
    {FA5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {FA5,  DUR_EIGHTH},  {SILENCE, DUR_EIGHTH},
    {MI5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {DO5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {RE5,  DUR_QUARTER}, {SILENCE, DUR_EIGHTH},
    {DO5,  DUR_HALF},    {SILENCE, DUR_HALF},
};
const uint32_t melodie_happy_birthday_size =
    sizeof(melodie_happy_birthday) / sizeof(note_t);


/* ============================================================
   4. NOKIA RINGTONE
   Tonalite : La majeur  |  Mesure : 4/4  |  Tempo : 120 BPM
   ============================================================
   Sonnerie originale du Nokia 3310 (1998, basee sur la Grande Valse
   de Komitas). Les silences DUR_EIGHTH entre les notes reproduisent
   le staccato caracteristique du piezo de l'original.

   Sequence melodique :
     MI5 RE5  | F#4(blanche) G#4(blanche)  |
     C#5 SI4  | RE4(blanche) MI4(blanche)  |
     SI4 LA4  | C#4(blanche) MI4           |
     LA4 (ronde finale)
   ============================================================ */

const note_t melodie_nokia[] = {

    {MI5, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},    /* MI5              */
    {RE5, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},    /* RE5              */
    {FS4, DUR_HALF},    {SILENCE, DUR_QUARTER},   /* F#4  (blanche)   */
    {GS4, DUR_HALF},    {SILENCE, DUR_QUARTER},   /* G#4  (blanche)   */

    {CS5, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},    /* C#5              */
    {SI4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},    /* SI4              */
    {RE4, DUR_HALF},    {SILENCE, DUR_QUARTER},   /* RE4  (blanche)   */
    {MI4, DUR_HALF},    {SILENCE, DUR_QUARTER},   /* MI4  (blanche)   */

    {SI4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},    /* SI4              */
    {LA4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},    /* LA4              */
    {CS4, DUR_HALF},    {SILENCE, DUR_QUARTER},   /* C#4  (blanche)   */

    {MI4, DUR_QUARTER}, {SILENCE, DUR_EIGHTH},    /* MI4              */
    {LA4, DUR_WHOLE},   {SILENCE, DUR_QUARTER},   /* LA4  (ronde)     */
};
const uint32_t melodie_nokia_size =
    sizeof(melodie_nokia) / sizeof(note_t);


/* ============================================================
   5. TWINKLE TWINKLE LITTLE STAR
   Tonalite : Do majeur  |  Mesure : 4/4  |  Tempo : 120 BPM
   ============================================================
   Melodie simple et reguliere : uniquement des noires (DUR_QUARTER)
   et des blanches (DUR_HALF) sur les fins de phrase.
   La DO finale est une ronde (DUR_WHOLE) pour bien marquer la fin.

   Structure :
     Partie A  : "Twinkle twinkle little star"
                  DO DO SOL SOL  LA LA SOL(blanche)
                  FA FA  MI  MI  RE RE DO(blanche)
     Partie B  : "Up above the world so high" (pont repete 2x)
                  SOL SOL FA FA  MI MI RE(blanche)
     Partie A' : reprise integrale de A
                  ...   DO(ronde) finale
   ============================================================ */

const note_t melodie_twinkle[] = {

    /* --- Partie A --- */

    /* "Twinkle twinkle little star" */
    {DO4,  DUR_QUARTER}, {DO4,  DUR_QUARTER},   /* Twin- kle  */
    {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},   /* twin- kle  */
    {LA4,  DUR_QUARTER}, {LA4,  DUR_QUARTER},   /* lit-  tle  */
    {SOL4, DUR_HALF},                            /* star       */

    /* "How I wonder what you are" */
    {FA4,  DUR_QUARTER}, {FA4,  DUR_QUARTER},   /* How   I    */
    {MI4,  DUR_QUARTER}, {MI4,  DUR_QUARTER},   /* won-  der  */
    {RE4,  DUR_QUARTER}, {RE4,  DUR_QUARTER},   /* what  you  */
    {DO4,  DUR_HALF},                            /* are        */

    /* --- Partie B (pont x2) --- */

    /* "Up above the world so high" */
    {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},   /* Up    a-   */
    {FA4,  DUR_QUARTER}, {FA4,  DUR_QUARTER},   /* bove  the  */
    {MI4,  DUR_QUARTER}, {MI4,  DUR_QUARTER},   /* world so   */
    {RE4,  DUR_HALF},                            /* high       */

    /* "Like a diamond in the sky" */
    {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},   /* Like  a    */
    {FA4,  DUR_QUARTER}, {FA4,  DUR_QUARTER},   /* dia-  mond */
    {MI4,  DUR_QUARTER}, {MI4,  DUR_QUARTER},   /* in    the  */
    {RE4,  DUR_HALF},                            /* sky        */

    /* --- Partie A' (reprise finale) --- */

    /* "Twinkle twinkle little star" */
    {DO4,  DUR_QUARTER}, {DO4,  DUR_QUARTER},
    {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},
    {LA4,  DUR_QUARTER}, {LA4,  DUR_QUARTER},
    {SOL4, DUR_HALF},

    /* "How I wonder what you are" — DO ronde finale */
    {FA4,  DUR_QUARTER}, {FA4,  DUR_QUARTER},
    {MI4,  DUR_QUARTER}, {MI4,  DUR_QUARTER},
    {RE4,  DUR_QUARTER}, {RE4,  DUR_QUARTER},
    {DO4,  DUR_WHOLE},                           /* ronde finale */
};
const uint32_t melodie_twinkle_size =
    sizeof(melodie_twinkle) / sizeof(note_t);


/* ============================================================
   6. BERCEUSE DE BRAHMS  (Op. 49 No. 4)
   Tonalite : Sol majeur  |  Mesure : 3/4  |  Tempo : 65 BPM
   ============================================================
   Transcription complete en 2 couplets de la berceuse originale.
   Tonalite Sol majeur (notes : SOL LA SI DO RE MI FA# SOL).

   Rythme en 3/4 : chaque mesure = 3 noires.
   - Noires  (DUR_QUARTER) : notes courtes et syllabes
   - Blanches (DUR_HALF)   : notes tenues (fin de phrase)
   - Silences (DUR_QUARTER): espace respiratoire entre phrases
   - Ronde   (DUR_WHOLE)   : fin de chaque couplet

   La melodie debute par une anacrouse de 2 noires (levee avant
   la 1ere mesure complete), ce qui donne le balancement en 3/4.

   Structure :
     Couplet A (x2) :  "Guten Abend, gute Nacht"
       Phrase 1 : [SOL SOL] | SI RE5 SI  | DO5.
       Phrase 2 : [SOL SOL] | SI MI5 SI  | LA4.
       Phrase 3 : [LA4 LA4] | DO5 LA4 F#4| SOL4.    (x2)
       Fin A    : SOL4 (ronde)

     Couplet B :  "Morgen frueh, wenn Gott will"
       Phrase 4 : [RE5 RE5] | F#5 RE5 SI | DO5.
       Phrase 5 : [RE5 RE5] | F#5 RE5 SI | LA4.
       Phrase 6 : [RE5 DO5] | SI LA4 SOL4| F#4.     (descente)
       Phrase 7 : [SOL4 LA4]| SI DO5 RE5 | SOL4.    (finale)

   FS4 = FA#4 = 370 Hz  (defini dans melodie.h)
   FS5 = FA#5 = 740 Hz  (defini dans melodie.h)
   ============================================================ */

const note_t melodie_brahms[] = {

    /* ===== COUPLET A ===== */

    /* --- Phrase 1 : "Guten Abend, gute Nacht" --- */
    /* Anacrouse */ {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},
    /* M1        */ {SI4,  DUR_QUARTER}, {RE5,  DUR_QUARTER}, {SI4, DUR_QUARTER},
    /* M2        */ {DO5,  DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Phrase 2 : "mit Rosen bedacht" --- */
    /* Anacrouse */ {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},
    /* M3        */ {SI4,  DUR_QUARTER}, {MI5,  DUR_QUARTER}, {SI4, DUR_QUARTER},
    /* M4        */ {LA4,  DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Phrase 3 : "mit Naglein besteckt" --- */
    /* Anacrouse */ {LA4,  DUR_QUARTER}, {LA4,  DUR_QUARTER},
    /* M5        */ {DO5,  DUR_QUARTER}, {LA4,  DUR_QUARTER}, {FS4, DUR_QUARTER},
    /* M6        */ {SOL4, DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Reprise Phrase 1 --- */
    /* Anacrouse */ {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},
    /* M7        */ {SI4,  DUR_QUARTER}, {RE5,  DUR_QUARTER}, {SI4, DUR_QUARTER},
    /* M8        */ {DO5,  DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Reprise Phrase 2 --- */
    /* Anacrouse */ {SOL4, DUR_QUARTER}, {SOL4, DUR_QUARTER},
    /* M9        */ {SI4,  DUR_QUARTER}, {MI5,  DUR_QUARTER}, {SI4, DUR_QUARTER},
    /* M10       */ {LA4,  DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Reprise Phrase 3 + fin couplet A --- */
    /* Anacrouse */ {LA4,  DUR_QUARTER}, {LA4,  DUR_QUARTER},
    /* M11       */ {DO5,  DUR_QUARTER}, {LA4,  DUR_QUARTER}, {FS4, DUR_QUARTER},
    /* M12 fin A */ {SOL4, DUR_WHOLE},   /* ronde : fin du couplet A        */

    /* ===== COUPLET B ===== */

    /* --- Phrase 4 : "Morgen frueh, wenn Gott will" --- */
    /* Anacrouse */ {RE5,  DUR_QUARTER}, {RE5,  DUR_QUARTER},
    /* M13       */ {FS5,  DUR_QUARTER}, {RE5,  DUR_QUARTER}, {SI4, DUR_QUARTER},
    /* M14       */ {DO5,  DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Phrase 5 : "wirst du wieder geweckt" --- */
    /* Anacrouse */ {RE5,  DUR_QUARTER}, {RE5,  DUR_QUARTER},
    /* M15       */ {FS5,  DUR_QUARTER}, {RE5,  DUR_QUARTER}, {SI4, DUR_QUARTER},
    /* M16       */ {LA4,  DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Phrase 6 : descente douce --- */
    /* Anacrouse */ {RE5,  DUR_QUARTER}, {DO5,  DUR_QUARTER},
    /* M17       */ {SI4,  DUR_QUARTER}, {LA4,  DUR_QUARTER}, {SOL4, DUR_QUARTER},
    /* M18       */ {FS4,  DUR_HALF},    {SILENCE, DUR_QUARTER},

    /* --- Phrase 7 : finale ascendante --- */
    /* Anacrouse */ {SOL4, DUR_QUARTER}, {LA4,  DUR_QUARTER},
    /* M19       */ {SI4,  DUR_QUARTER}, {DO5,  DUR_QUARTER}, {RE5, DUR_QUARTER},
    /* M20 fin   */ {SOL4, DUR_WHOLE},   /* ronde finale                    */
};
const uint32_t melodie_brahms_size =
    sizeof(melodie_brahms) / sizeof(note_t);
