/*
 * player.c
 *
 * Coeur du lecteur de musique.
 * Gere la lecture des melodies, le menu interactif et les actions des switches.
 *
 * Deux modes de fonctionnement :
 *   - Mode AUTO   : SW4 (PB6) lance/arrete la chanson courante sans menu
 *   - Mode MANUEL : PB8 ouvre le menu interactif sur GTKTerm
 *
 * Flux de lecture :
 *   player_init() → charge la 1ere note → TIM5 compte sa duree
 *   → TIM5_Handler() → player_next_note() → charge la note suivante → ...
 */

#include <stdio.h>
#include <stdint.h>
#include "sys/devices.h"   /* Registres TIM2, TIM5                         */
#include "note.h"          /* note_t, note_compute_arr()                   */
#include "player.h"
#include "timer.h"         /* init_TIM5_compteur()                         */
#include "irq.h"           /* enable_IRQ_by_number()                       */
#include "tempo.h"         /* tempo_set_bpm(), tempo_duration_ms()         */
#include "volume.h"        /* player_set_duty_cycle()                      */
#include "led.h"           /* led_from_note(), set_led_color()             */
#include "melodie.h"       /* Catalogue des melodies                       */

/* ================================================================
   CATALOGUE DES MELODIES
   ================================================================
   Mettre a jour cette table pour ajouter ou retirer des melodies.
   Chaque entree : { "Nom affiche dans le menu", tableau_notes, &taille }
   ================================================================ */

typedef struct {
    const char     *nom;          /* Nom affiche dans le menu interactif   */
    const note_t   *notes;        /* Pointeur sur le tableau de notes      */
    const uint32_t *taille_ptr;   /* Pointeur sur la taille du tableau     */
    uint16_t        bpm_conseille;/* Tempo recommande pour cette melodie   */
} chanson_t;

static const chanson_t catalogue[] = {
    /* index 0 */ { "Frere Jacques",        melodie_frere_jacques,    &melodie_frere_jacques_size,    120 },
    /* index 1 */ { "Au Clair de la Lune",  melodie_au_clair,         &melodie_au_clair_size,          80 },
    /* index 2 */ { "Happy Birthday",       melodie_happy_birthday,   &melodie_happy_birthday_size,    90 },
    /* index 3 */ { "Nokia Ringtone",       melodie_nokia,            &melodie_nokia_size,            120 },
    /* index 4 */ { "Twinkle Twinkle",      melodie_twinkle,          &melodie_twinkle_size,          120 },
    /* index 5 */ { "Berceuse de Brahms",   melodie_brahms,           &melodie_brahms_size,            80 },
};

/* Nombre de melodies dans le catalogue (calcule automatiquement) */
#define NB_CHANSONS  (sizeof(catalogue) / sizeof(chanson_t))

/* ================================================================
   ETAT GLOBAL
   ================================================================ */

typedef enum { MODE_ONCE, MODE_REPEAT } play_mode_t;

static play_mode_t  g_mode       = MODE_REPEAT; /* Mode de lecture courant     */
uint8_t             g_demander   = 0;            /* 1 = afficher le menu        */
uint8_t             g_playing    = 0;            /* 1 = lecture en cours        */

static uint32_t     g_song_index = 0;            /* Index chanson courante      */

/* Table des BPM disponibles, cyclee par SW3 */
static const uint16_t g_bpm_table[] = { 60, 80, 90, 120, 160 };
#define NB_BPM  (sizeof(g_bpm_table) / sizeof(uint16_t))
static uint8_t g_bpm_index = 2;   /* Demarre a 90 BPM (valeur mediane)    */

/* ================================================================
   VARIABLES GLOBALES DU PLAYER
   (accessibles depuis timer.c via player.h)
   ================================================================ */

volatile uint32_t i              = 0;             /* Index note courante        */
volatile uint32_t taille_melodie = 0;             /* Nombre total de notes      */
note_t            note_curr      = {SILENCE, DUR_QUARTER}; /* Note en cours    */
const note_t     *melodie        = 0;             /* Pointeur sur le tableau    */

volatile uint32_t cycle_val      = 2;             /* Diviseur duty cycle PWM    */

static volatile uint8_t g_repeat = 1;             /* 1 = boucle, 0 = une fois  */
static volatile uint8_t g_done   = 0;             /* 1 = melodie terminee      */

/* ================================================================
   UTILITAIRE INTERNE
   ================================================================ */

/*
 * note_duration_us
 * Convertit une valeur musicale (dur_t) en microsecondes.
 * Multiplie par 1000 car TIM5 tourne a 1 MHz (1 tick = 1 µs).
 */
static uint32_t note_duration_us(dur_t d)
{
    return (uint32_t)tempo_duration_ms(d) * 1000;
}

/* ================================================================
   LECTURE UART
   ================================================================ */

/*
 * read_int
 * Lit un entier saisi par l'utilisateur sur GTKTerm (UART).
 * Ignore les caracteres de fin de ligne et espaces en debut de saisie.
 */
static int read_int(void)
{
    int val = 0;
    int c;
    do { c = getchar(); } while (c == '\n' || c == '\r' || c == ' ');
    val = c - '0';
    while (1) {
        c = getchar();
        if (c == '\n' || c == '\r') break;
        if (c >= '0' && c <= '9') val = val * 10 + (c - '0');
    }
    return val;
}

/* ================================================================
   MENU INTERACTIF (mode MANUEL, declenche par PB8)
   ================================================================ */

/*
 * demander_et_lancer
 * Affiche un menu sur GTKTerm pour choisir :
 *   1. La chanson a jouer (avec son tempo recommande affiche)
 *   2. Le tempo (BPM)
 *   3. Le mode : une fois ou en boucle
 * Puis initialise et demarre le player.
 */
void demander_et_lancer(void)
{
    uint32_t choix;
    uint16_t bpm;

    printf("\r\n");
    printf("========================================\r\n");
    printf("        ** MUSIC PLAYER STM32 **        \r\n");
    printf("========================================\r\n");

    /* --- Choix de la chanson --- */
    printf("\r\nQuelle chanson voulez-vous jouer ?\r\n");
    for (uint32_t k = 0; k < NB_CHANSONS; k++)
        printf("  %lu - %-24s  (conseille : %u BPM)\r\n",
               k + 1, catalogue[k].nom, catalogue[k].bpm_conseille);
    printf("\r\n> ");
    choix = read_int();
    if (choix < 1 || choix > NB_CHANSONS) {
        printf("Choix invalide, Frere Jacques par defaut.\r\n");
        choix = 1;
    }
    g_song_index = choix - 1;

    /* --- Choix du tempo --- */
    printf("\r\nQuel tempo voulez-vous ?\r\n");
    printf("  1 - Tres lent (  60 BPM)\r\n");
    printf("  2 - Lent      (  80 BPM)\r\n");
    printf("  3 - Modere    (  90 BPM)\r\n");
    printf("  4 - Rapide    ( 120 BPM)\r\n");
    printf("  5 - Tres vite ( 160 BPM)\r\n");
    printf("\r\n  [Appuyer ENTREE pour utiliser le tempo conseille : %u BPM]\r\n",
           catalogue[g_song_index].bpm_conseille);
    printf("\r\n> ");

    switch (read_int()) {
        case 1:  bpm =  60; g_bpm_index = 0; break;
        case 2:  bpm =  80; g_bpm_index = 1; break;
        case 3:  bpm =  90; g_bpm_index = 2; break;
        case 4:  bpm = 120; g_bpm_index = 3; break;
        case 5:  bpm = 160; g_bpm_index = 4; break;
        default:
            /* Entree invalide ou vide → tempo conseille de la chanson */
            bpm = catalogue[g_song_index].bpm_conseille;
            /* Trouver l'index correspondant dans g_bpm_table */
            for (uint8_t k = 0; k < NB_BPM; k++) {
                if (g_bpm_table[k] == bpm) { g_bpm_index = k; break; }
            }
            printf("Tempo conseille utilise : %u BPM\r\n", bpm);
            break;
    }

    /* --- Choix du mode --- */
    printf("\r\nMode de lecture :\r\n");
    printf("  1 - Une seule fois\r\n");
    printf("  2 - En boucle\r\n");
    printf("\r\n> ");
    g_mode = (read_int() == 1) ? MODE_ONCE : MODE_REPEAT;

    /* --- Lancement --- */
    const chanson_t *c = &catalogue[g_song_index];
    uint32_t taille = *(c->taille_ptr);

    printf("\r\n----------------------------------------\r\n");
    printf(">> %s  @  %u BPM  [%s]\r\n",
           c->nom, bpm,
           g_mode == MODE_ONCE ? "une fois" : "boucle");
    printf("----------------------------------------\r\n");

    tempo_set_bpm(bpm);
    player_stop();                /* Arrete proprement la lecture precedente */
    player_set_mode(g_mode == MODE_REPEAT ? 1 : 0);
    player_init(c->notes, taille);

    g_playing  = 1;
    g_demander = 0;
}

/* ================================================================
   CONTROLE DU MENU DEPUIS LA BOUCLE PRINCIPALE
   ================================================================ */

/*
 * player_check_done
 * Appelee dans la boucle principale de main().
 * En MODE_ONCE, detecte la fin de melodie et remet g_playing a 0.
 */
void player_check_done(void)
{
    if (g_mode == MODE_ONCE && g_playing && player_is_done()) {
        printf("\r\n[Fin] Chanson terminee.\r\n");
        g_playing  = 0;
        g_demander = 1;   /* Retour automatique au menu apres la fin */
    }
}

/*
 * player_needs_menu
 * Retourne 1 si le menu interactif doit etre affiche.
 * Mis a 1 par l'appui sur PB8 ou apres la fin d'une lecture en MODE_ONCE.
 */
uint8_t player_needs_menu(void)
{
    return g_demander;
}

/* ================================================================
   ACTIONS DES SWITCHES
   ================================================================ */

/*
 * player_next_song   (SW2 - PB4)
 * Passe a la chanson suivante dans le catalogue (ordre circulaire).
 * Redemarre immediatement avec le BPM et le mode courants.
 */
void player_next_song(void)
{
    if (!g_playing) return;

    g_song_index = (g_song_index + 1) % NB_CHANSONS;
    const chanson_t *c = &catalogue[g_song_index];
    uint32_t taille = *(c->taille_ptr);

    player_stop();
    player_init(c->notes, taille);
}

/*
 * player_next_tempo   (SW3 - PB5)
 * Avance d'un cran dans la table des BPM : 60 -> 80 -> 90 -> 120 -> 160 -> 60...
 * Le changement est immediat : la prochaine note joue au nouveau tempo.
 */
void player_next_tempo(void)
{
    g_bpm_index = (g_bpm_index + 1) % NB_BPM;
    tempo_set_bpm(g_bpm_table[g_bpm_index]);
}

/*
 * player_toggle_mode   (SW4 - PB6)
 * Mode automatique (sans menu) :
 *   - Si une lecture est en cours → stop + LED eteinte
 *   - Si rien ne joue → lance la chanson courante en boucle au BPM courant
 */
void player_toggle_mode(void)
{
    if (g_playing) {
        /* Arret de la lecture */
        player_stop();
        set_led_color(0b000);
        g_playing = 0;
    } else {
        /* Demarrage automatique avec le tempo conseille de la chanson courante */
        const chanson_t *c = &catalogue[g_song_index];
        uint32_t taille = *(c->taille_ptr);

        tempo_set_bpm(c->bpm_conseille);
        player_set_mode(1);        /* Mode boucle */
        g_mode = MODE_REPEAT;
        player_init(c->notes, taille);
        g_playing = 1;
    }
}

/* ================================================================
   CONTROLE DU BUZZER (PWM TIM2)
   ================================================================ */

/* Active la sortie PWM de TIM2 vers le buzzer (CCER.CC2E = 1) */
void buzzer_on_PWM(void)
{
    TIM2.CCER |= (1 << 4);
}

/* Desactive la sortie PWM (silence) */
void buzzer_off_PWM(void)
{
    TIM2.CCER &= ~(1 << 4);
}

/*
 * jouer_note
 * Configure TIM2 pour jouer une note a la frequence donnee :
 *   - ARR  = (FREQ_CNT / freq) - 1  → fixe la frequence du PWM
 *   - CCR2 = ARR / cycle_val        → fixe le duty cycle (volume)
 *   - TIM5.ARR = duree de la note en µs
 *   - EGR.UG force la prise en compte immediate de ARR dans TIM2
 */
void jouer_note(uint16_t freq_note)
{
    TIM2.ARR  = note_compute_arr(freq_note);
    if (cycle_val == 0) cycle_val = 2;
    TIM2.CCR2 = TIM2.ARR / cycle_val;
    TIM5.ARR  = note_duration_us(note_curr.dur);
    TIM2.EGR |= 1;                 /* UG : recharge ARR dans TIM2          */
}

/* ================================================================
   GESTION DU MODE
   ================================================================ */

/*
 * player_set_mode
 * repeat = 1 : lecture en boucle
 * repeat = 0 : lecture unique (s'arrete apres la derniere note)
 */
void player_set_mode(uint8_t repeat)
{
    g_repeat = repeat;
    g_done   = 0;
}

/* Retourne 1 si la melodie est arrivee a sa fin (MODE_ONCE uniquement) */
uint8_t player_is_done(void)
{
    return g_done;
}

/* ================================================================
   INITIALISATION DE LA LECTURE
   ================================================================ */

/*
 * player_init
 * Initialise le player avec une nouvelle melodie et demarre la lecture.
 * Etapes :
 *   1. Charge le tableau de notes et remet l'index a 0
 *   2. Initialise TIM5 (compteur de duree)
 *   3. Active l'IRQ TIM5 dans le NVIC (IRQ 50)
 *   4. Joue la premiere note (ou attend le silence)
 */
void player_init(const note_t *mel, uint32_t taille)
{
    melodie        = mel;
    taille_melodie = taille;
    i              = 0;
    g_done         = 0;

    if (taille == 0) return;

    note_curr = melodie[0];

    init_TIM5_compteur();         /* Demarre TIM5 a 1 MHz avec UIE activee */
    enable_IRQ_by_number(50);     /* Active TIM5_IRQ dans le NVIC           */

    if (note_curr.freq != SILENCE) {
        jouer_note(note_curr.freq);
        buzzer_on_PWM();
    } else {
        /* Silence : TIM5 compte la duree mais le buzzer reste eteint       */
        buzzer_off_PWM();
        TIM5.ARR  = note_duration_us(note_curr.dur);
        TIM5.EGR |= 1;
    }
}

/*
 * player_set_duty_cycle
 * Regle le duty cycle du PWM (= volume sonore).
 * diviseur = 2  → 50% (fort), diviseur = 10 → 10% (faible)
 * Appele par reglage_volume() dans volume.c.
 */
void player_set_duty_cycle(uint32_t diviseur)
{
    if (diviseur == 0) diviseur = 2;
    cycle_val = diviseur;
    if (note_curr.freq != SILENCE)
        TIM2.CCR2 = TIM2.ARR / cycle_val;
}

/* ================================================================
   PAUSE / RESUME / STOP
   ================================================================ */

/*
 * player_pause   (SW1 ON - PB3)
 * Gele la lecture : eteint le buzzer et arrete TIM5.
 * La position dans la melodie est conservee.
 */
void player_pause(void)
{
    buzzer_off_PWM();
    TIM5.CR1 &= ~1;               /* CEN = 0 : arrete le compteur TIM5     */
}

/*
 * player_resume   (SW1 OFF - PB3)
 * Reprend la lecture la ou elle s'etait arretee.
 */
void player_resume(void)
{
    TIM5.CR1 |= 1;                /* CEN = 1 : relance TIM5                */
    if (note_curr.freq != SILENCE)
        buzzer_on_PWM();
}

/*
 * player_stop
 * Arrete completement la lecture et remet l'index a 0.
 */
void player_stop(void)
{
    buzzer_off_PWM();
    TIM5.CR1 &= ~1;               /* CEN = 0 : arrete TIM5                 */
    i = 0;
}

/* ================================================================
   AVANCEMENT NOTE PAR NOTE   (appele par TIM5_Handler dans timer.c)
   ================================================================ */

/*
 * player_next_note
 * Appelee a chaque fin de note (IRQ TIM5).
 * Charge la note suivante, met a jour TIM2 et TIM5.
 *
 * En fin de melodie :
 *   - MODE_REPEAT : repart du debut (i = 0)
 *   - MODE_ONCE   : arrete TIM5, leve le flag g_done
 */
void player_next_note(void)
{
    i++;

    if (i >= taille_melodie) {
        if (g_repeat) {
            i = 0;                 /* Retour au debut : boucle              */
        } else {
            i      = 0;
            g_done = 1;            /* Signale la fin a player_check_done    */
            buzzer_off_PWM();
            TIM5.CR1 &= ~1;        /* Arrete TIM5                           */
            return;
        }
    }

    /* Charger la note suivante */
    note_curr = melodie[i];
    led_from_note(note_curr.freq); /* Met a jour la LED tricolore           */

    if (note_curr.freq == SILENCE) {
        /* Silence : eteindre le buzzer, laisser TIM5 compter la duree      */
        buzzer_off_PWM();
        TIM5.ARR  = note_duration_us(note_curr.dur);
        TIM5.EGR |= 1;
    } else {
        /* Note sonore : configure TIM2 et active le buzzer                 */
        jouer_note(note_curr.freq);
        buzzer_on_PWM();
    }
}
