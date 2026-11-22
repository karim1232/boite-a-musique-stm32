/*
 * led.c
 *
 * Controle de la LED tricolore sur PA8 (rouge), PA9 (vert), PA10 (bleu).
 * La couleur change a chaque note jouee pour donner un retour visuel
 * en temps reel sur la melodie en cours.
 *
 * Brochage :
 *   PA8  → composante Rouge  (bit 0 du masque)
 *   PA9  → composante Vert   (bit 1 du masque)
 *   PA10 → composante Bleu   (bit 2 du masque)
 *
 * Les 7 combinaisons RGB possibles sont toutes utilisees et
 * reparties sur toutes les notes jouees dans le catalogue :
 *
 * ┌──────────┬──────────────────────────────────────────────────────┐
 * │ Masque   │ Couleur affichee                                     │
 * ├──────────┼──────────────────────────────────────────────────────┤
 * │ 0b000    │ Eteinte   (SILENCE)                                  │
 * │ 0b001    │ Rouge     (DO4, DO5, SI3)                            │
 * │ 0b010    │ Vert      (RE4, RE5)                                 │
 * │ 0b011    │ Jaune     (FA4, FA5, CS4, CS5)                       │
 * │ 0b100    │ Bleu      (MI4, MI5)                                 │
 * │ 0b101    │ Magenta   (SOL4, SOL5, GS4)                          │
 * │ 0b110    │ Cyan      (LA4, LA5, FS4, SOL3, LA3)                 │
 * │ 0b111    │ Blanc     (SI4, SI5, DS4)                            │
 * └──────────┴──────────────────────────────────────────────────────┘
 *
 * Logique d'attribution :
 *   - Octave 4 (notes les plus jouees) : chaque note a une couleur unique
 *     parmi les 7 combinaisons possibles, du grave (rouge) a l'aigu (blanc)
 *   - Octave 5 : meme couleur que la note de meme nom en octave 4
 *   - Octave 3 : couleur de la note de meme nom (familles harmoniques)
 *   - Diezes   : couleur de la note la plus proche harmoniquement
 */

#include <stdint.h>
#include "note.h"          /* Constantes de frequences (DO4, RE4, etc.) */
#include "sys/devices.h"   /* Registre GPIOA.ODR                        */

/* ================================================================
   set_led_color
   ================================================================
   Applique un masque 3 bits sur les sorties PA8, PA9, PA10.
   Efface d'abord les 3 bits [10:8] de GPIOA.ODR, puis les
   remplace par le masque decale de 8 positions.

   Combinaisons de masque :
     0b000 = eteinte
     0b001 = rouge              (PA8 seul)
     0b010 = vert               (PA9 seul)
     0b011 = jaune  (rouge + vert)
     0b100 = bleu               (PA10 seul)
     0b101 = magenta (rouge + bleu)
     0b110 = cyan   (vert  + bleu)
     0b111 = blanc  (rouge + vert + bleu)
   ================================================================ */
void set_led_color(uint8_t mask)
{
    GPIOA.ODR &= ~(7 << 8);    /* Eteint PA8, PA9, PA10                  */
    GPIOA.ODR |=  (mask << 8); /* Applique la nouvelle couleur            */
}

/* ================================================================
   led_from_note
   ================================================================
   Assigne une couleur a chaque note jouee dans le catalogue.
   Les 7 combinaisons RGB sont toutes utilisees.

   Regles d'assignation :
     Octave 4  : couleur unique par note, gamme chromatique
                 DO(rouge) RE(vert) MI(bleu) FA(jaune)
                 SOL(magenta) LA(cyan) SI(blanc)
     Octave 5  : meme couleur que la note correspondante en octave 4
     Octave 3  : meme couleur que la note correspondante
     Diezes    : couleur de la note voisine la plus proche
   ================================================================ */
void led_from_note(uint16_t note)
{
    switch (note)
    {
        /* ============================================================
           OCTAVE 4 — 7 couleurs uniques sur les 7 notes naturelles
           Ordre : croissant de DO a SI = rouge → blanc
           ============================================================ */

        case DO4:
            set_led_color(0b001);   /* Rouge                             */
            break;

        case RE4:
            set_led_color(0b010);   /* Vert                              */
            break;

        case MI4:
            set_led_color(0b100);   /* Bleu                              */
            break;

        case FA4:
            set_led_color(0b011);   /* Jaune  (Rouge + Vert)             */
            break;

        case SOL4:
            set_led_color(0b101);   /* Magenta (Rouge + Bleu)            */
            break;

        case LA4:
            set_led_color(0b110);   /* Cyan   (Vert + Bleu)              */
            break;

        case SI4:
            set_led_color(0b111);   /* Blanc  (Rouge + Vert + Bleu)      */
            break;

        /* ============================================================
           OCTAVE 5 — meme couleur que la note de meme nom en octave 4
           Utilisees dans : Happy Birthday, Nokia Ringtone
           ============================================================ */

        case DO5:
            set_led_color(0b001);   /* Rouge   (famille DO)              */
            break;

        case RE5:
            set_led_color(0b010);   /* Vert    (famille RE)              */
            break;

        case MI5:
            set_led_color(0b100);   /* Bleu    (famille MI)              */
            break;

        case FA5:
            set_led_color(0b011);   /* Jaune   (famille FA)              */
            break;

        case SOL5:
            set_led_color(0b101);   /* Magenta (famille SOL)             */
            break;

        case LA5:
            set_led_color(0b110);   /* Cyan    (famille LA)              */
            break;

        case SI5:
            set_led_color(0b111);   /* Blanc   (famille SI)              */
            break;

        /* ============================================================
           OCTAVE 3 — notes graves de Au Clair de la Lune
           Meme couleur que la note de meme nom
           ============================================================ */

        case DO3:
            set_led_color(0b001);   /* Rouge   (famille DO)              */
            break;

        case RE3:
            set_led_color(0b010);   /* Vert    (famille RE)              */
            break;

        case MI3:
            set_led_color(0b100);   /* Bleu    (famille MI)              */
            break;

        case FA3:
            set_led_color(0b011);   /* Jaune   (famille FA)              */
            break;

        case SOL3:
            set_led_color(0b101);   /* Magenta (famille SOL)             */
            break;

        case LA3:
            set_led_color(0b110);   /* Cyan    (famille LA)              */
            break;

        case SI3:
            set_led_color(0b111);   /* Blanc   (famille SI)              */
            break;

        /* ============================================================
           NOTES DIEZES — utilisees par Nokia Ringtone
           Couleur de la note voisine la plus proche harmoniquement
           ============================================================ */

        case CS4:   /* C# entre DO4 et RE4 → Jaune (mi-chemin DO/RE)    */
            set_led_color(0b011);   /* Jaune                             */
            break;

        case DS4:   /* D# entre RE4 et MI4 → Cyan (mi-chemin RE/MI)     */
            set_led_color(0b110);   /* Cyan                              */
            break;

        case FS4:   /* F# entre FA4 et SOL4 → Magenta (proche SOL)      */
            set_led_color(0b101);   /* Magenta                           */
            break;

        case GS4:   /* G# entre SOL4 et LA4 → Cyan (proche LA)          */
            set_led_color(0b110);   /* Cyan                              */
            break;

        case CS5:   /* C#5 — meme famille que CS4                        */
            set_led_color(0b011);   /* Jaune                             */
            break;

        /* ============================================================
           SILENCE — LED eteinte pendant les pauses
           ============================================================ */

        case SILENCE:
            set_led_color(0b000);   /* Eteinte                           */
            break;

        /* ============================================================
           CAS PAR DEFAUT — note inconnue
           ============================================================ */

        default:
            set_led_color(0b111);   /* Blanc par defaut                  */
            break;
    }
}

/* ================================================================
   init_led_tri
   ================================================================
   Configure PA8, PA9, PA10 en sortie push-pull haute vitesse.

   Registres configures :
     MODER   : bits [21:16] → 01 (output) pour PA8, PA9, PA10
               PA8  = bits [17:16]
               PA9  = bits [19:18]
               PA10 = bits [21:20]
     OTYPER  : bits [10:8]  → 0 (push-pull, pas open-drain)
     OSPEEDR : bits [21:16] → 11 (high speed, jusqu'a 100 MHz)
   ================================================================ */
void init_led_tri()
{
    enable_GPIOA();

    /* Sortie push-pull sur PA8, PA9, PA10 */
    GPIOA.MODER   = (GPIOA.MODER   & ~(0x3F << 16))
                  | ((1<<16) | (1<<18) | (1<<20));  /* output mode       */
    GPIOA.OTYPER  =  GPIOA.OTYPER  & ~(0x7  <<  8); /* push-pull         */
    GPIOA.OSPEEDR =  GPIOA.OSPEEDR |  (0x3F << 16); /* haute vitesse     */
}
