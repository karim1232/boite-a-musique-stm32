/*
 * timer.c
 *
 * Gestion des timers matériels TIM2 et TIM5 du STM32F446.
 *
 * Vue d'ensemble
 * ──────────────
 * Ce fichier configure deux timers avec des rôles complémentaires :
 *
 *  ┌─────────┬──────────────┬────────────────────────────────────────────────┐
 *  │ Timer   │ Mode         │ Rôle                                           │
 *  ├─────────┼──────────────┼────────────────────────────────────────────────┤
 *  │ TIM2    │ PWM          │ Génère le signal sonore sur PB9 (buzzer)        │
 *  │         │ canal CH2    │ Fréquence PWM  = fréquence de la note jouée     │
 *  │         │              │ Duty cycle     = volume sonore (via ADC)        │
 *  ├─────────┼──────────────┼────────────────────────────────────────────────┤
 *  │ TIM5    │ Compteur     │ Mesure la durée de chaque note en µs            │
 *  │         │ + IRQ update │ Déclenche TIM5_Handler() à la fin de la note   │
 *  │         │              │ Le handler charge la note suivante              │
 *  └─────────┴──────────────┴────────────────────────────────────────────────┘
 *
 * Base de temps commune : FREQ_CNT = 1 000 000 Hz → 1 tick = 1 µs
 * Le prescaler est calculé à l'exécution depuis l'horloge APB1.
 *
 * Relation entre les deux timers :
 *
 *   player_init()
 *       │
 *       ├─► TIM2.ARR  = (1 000 000 / freq_note) - 1   → fréquence du son
 *       ├─► TIM2.CCR2 = ARR / cycle_val               → duty cycle / volume
 *       └─► TIM5.ARR  = tempo_duration_ms(dur) * 1000 → durée en µs
 *
 *   [TIM5 overflow après ARR µs]
 *       │
 *       └─► TIM5_Handler()
 *               ├─► reglage_volume()     → met à jour CCR2 via ADC
 *               └─► player_next_note()  → charge TIM2 + TIM5 pour la note suivante
 */

#include "timer.h"
#include "sys/devices.h"   /* Registres TIM2, TIM5                           */
#include "sys/clock.h"     /* get_APB1TIMCLK() : fréquence horloge APB1      */
#include "note.h"          /* FREQ_CNT = 1 000 000                            */
#include "player.h"        /* player_next_note()                              */
#include "volume.h"        /* reglage_volume()                                */

/* ================================================================
   init_TIM2_PWM
   ================================================================

   Objectif :
     Configurer TIM2 en mode PWM mode 1 sur le canal 2 (CH2).
     La sortie CH2 est connectée à PB9 (configuré en AF1 dans main.c).

   Fonctionnement du PWM :
     TIM2 compte de 0 jusqu'à ARR en continu.
     ┌──────────────────────────────────────────────┐
     │ CNT : 0 ──────────► CCR2 ────────────► ARR  │
     │        HIGH (buzzer ON)   LOW (buzzer OFF)   │
     └──────────────────────────────────────────────┘
     - Fréquence du signal = FREQ_CNT / (ARR + 1)
     - Duty cycle          = CCR2 / (ARR + 1) × 100 %

   ARR et CCR2 sont mis à jour par jouer_note() dans player.c
   à chaque changement de note.

   Registres utilisés :
   ┌─────────────┬─────┬──────────────────────────────────────────────┐
   │ Registre    │ Bit │ Signification                                │
   ├─────────────┼─────┼──────────────────────────────────────────────┤
   │ PSC         │ -   │ Prescaler : divise APB1 pour obtenir 1 MHz   │
   │ CCMR1[14:12]│ OC2M│ 110 = PWM mode 1 (HIGH si CNT < CCR2)        │
   │ CCMR1[11]   │OC2PE│ 1 = preload CCR2 actif (mis à jour au UEV)   │
   │ CCMR1[9:8]  │ CC2S│ 00 = canal en sortie                         │
   │ CR1[7]      │ ARPE│ 1 = preload ARR (mis à jour au UEV)          │
   │ CR1[0]      │ CEN │ 1 = démarre le compteur                      │
   └─────────────┴─────┴──────────────────────────────────────────────┘

   Note : l'activation de la sortie physique (CCER.CC2E) est gérée
   séparément par buzzer_on_PWM() et buzzer_off_PWM() dans player.c,
   ce qui permet de couper le son sans arrêter le compteur.
   ================================================================ */
void init_TIM2_PWM()
{
    /* --- Prescaler : APB1 / 1 000 000 - 1 ---
       Sur STM32F446 à 180 MHz, APB1TIMCLK = 90 MHz
       PSC = (90 000 000 / 1 000 000) - 1 = 89
       → TIM2 compte à 1 MHz = 1 tick par µs                             */
    TIM2.PSC = (get_APB1TIMCLK() / FREQ_CNT) - 1;

    /* --- Configuration du canal CH2 (PWM mode 1) ---
       On efface d'abord les 16 bits bas de CCMR1 (canaux 1 et 2)
       puis on configure uniquement le canal 2                            */
    TIM2.CCMR1 &= ~0xffff;         /* Reset des bits [15:0]              */
    TIM2.CCMR1 |= (6 << 12);       /* OC2M[2:0] = 110 : PWM mode 1       */
                                    /*   sortie HIGH tant que CNT < CCR2  */
    TIM2.CCMR1 |= (1 << 11);       /* OC2PE = 1 : preload CCR2 activé    */
                                    /*   CCR2 est lu à l'UEV suivant      */
    TIM2.CCMR1 &= ~(3 << 8);       /* CC2S[1:0] = 00 : canal en sortie   */

    /* --- Preload de ARR ---
       Avec ARPE = 1, la nouvelle valeur de ARR n'est prise en compte
       qu'au prochain overflow (UEV), évitant les glitches sonores        */
    TIM2.CR1 |= (1 << 7);          /* ARPE = 1 : ARR bufferisé           */

    /* --- Démarrage du compteur TIM2 ---
       Le compteur tourne en permanence.
       Le son est activé/coupé via CCER.CC2E dans player.c               */
    TIM2.CR1 |= (1 << 0);          /* CEN = 1 : compteur actif           */
}

/* ================================================================
   init_TIM5_compteur
   ================================================================

   Objectif :
     Configurer TIM5 comme un compteur à 1 MHz qui génère une IRQ
     à chaque fois qu'il atteint la valeur ARR (fin de durée d'une note).

   Fonctionnement :
     ┌────────────────────────────────────────────────────────┐
     │  CNT : 0 ─────────────────────────────────► ARR       │
     │                                              │          │
     │                                        overflow        │
     │                                              │          │
     │                                    IRQ TIM5_Handler()  │
     │                                    → note suivante     │
     │                                    → nouveau ARR       │
     └────────────────────────────────────────────────────────┘

   ARR est rechargé dans player.c par jouer_note() :
     TIM5.ARR  = tempo_duration_ms(dur) * 1000  (durée en µs)
     TIM5.EGR |= 1  (UG : force le chargement de ARR immédiatement)

   Registres utilisés :
   ┌─────────────┬─────┬──────────────────────────────────────────────┐
   │ Registre    │ Bit │ Signification                                │
   ├─────────────┼─────┼──────────────────────────────────────────────┤
   │ PSC         │ -   │ Prescaler : même valeur que TIM2             │
   │ CR1[7]      │ ARPE│ 1 = preload ARR                              │
   │ DIER[0]     │ UIE │ 1 = IRQ activée sur chaque overflow          │
   │ ARR         │ -   │ Valeur initiale temporaire (1000 µs = 1 ms)  │
   │ EGR[0]      │ UG  │ 1 = force le chargement de PSC et ARR       │
   │ CR1[0]      │ CEN │ 1 = démarre le compteur                     │
   └─────────────┴─────┴──────────────────────────────────────────────┘
   ================================================================ */
void init_TIM5_compteur()
{
    /* --- Prescaler identique à TIM2 : 1 tick = 1 µs ---              */
    TIM5.PSC = (get_APB1TIMCLK() / FREQ_CNT) - 1;

    /* --- Preload ARR ---
       Permet de mettre à jour ARR en cours de route sans glitch        */
    TIM5.CR1 |= (1 << 7);          /* ARPE = 1                           */

    /* --- Activation de l'interruption sur overflow ---
       DIER.UIE = 1 : une IRQ est générée chaque fois que CNT = ARR
       → déclenche TIM5_Handler() → appelle player_next_note()          */
    TIM5.DIER |= 1;                 /* UIE = 1                            */

    /* --- Valeur ARR de départ ---
       On place une valeur non nulle pour éviter qu'une IRQ soit
       déclenchée immédiatement après le démarrage.
       Elle sera remplacée par la durée de la 1ère note dans player_init() */
    TIM5.ARR = 1000;                /* 1000 µs = 1 ms (valeur temporaire) */

    /* --- Forcer le chargement immédiat de PSC et ARR ---
       Sans UG, PSC et ARR ne sont effectifs qu'au prochain overflow.
       Écrire UG = 1 force un Update Event qui :
         1. Remet le compteur à 0
         2. Charge PSC depuis son registre shadow
         3. Charge ARR depuis son registre shadow                        */
    TIM5.EGR |= 1;                  /* UG = 1 : Update Event forcé        */

    /* --- Démarrage de TIM5 ---                                        */
    TIM5.CR1 |= (1 << 0);          /* CEN = 1 : compteur actif           */
}

/* ================================================================
   TIM5_Handler   (IRQ numéro 50)
   ================================================================

   Déclenchement :
     Appelé automatiquement par le NVIC à chaque overflow de TIM5,
     c'est-à-dire à chaque fin de durée d'une note musicale.

   Séquence d'exécution :
     1. Vérifier que l'IRQ vient bien d'un overflow (UIF = 1 dans SR)
     2. Effacer le flag UIF immédiatement pour permettre la prochaine IRQ
     3. Lire le potentiomètre et ajuster le volume (duty cycle de TIM2)
     4. Passer à la note suivante (met à jour TIM2 et TIM5.ARR)

   Registres de statut :
   ┌──────────┬─────┬──────────────────────────────────────────────────┐
   │ Registre │ Bit │ Signification                                    │
   ├──────────┼─────┼──────────────────────────────────────────────────┤
   │ SR[0]    │ UIF │ Update Interrupt Flag                            │
   │          │     │ mis à 1 par le hardware à chaque overflow        │
   │          │     │ remis à 0 en écrivant 0 dessus (Write-Clear)     │
   └──────────┴─────┴──────────────────────────────────────────────────┘

   Important : UIF doit être effacé AVANT d'appeler player_next_note(),
   car cette fonction peut relancer TIM5 (via EGR.UG) et un nouveau
   flag UIF pourrait apparaître immédiatement après.
   ================================================================ */
void TIM5_Handler()
{
    /* --- Vérification du flag UIF ---
       On s'assure que l'IRQ est bien due à un overflow (UIF = 1)
       et non à une autre source d'interruption de TIM5               */
    if (TIM5.SR & 1) {
        TIM5.SR &= ~1;              /* Efface UIF (Write-Clear : mettre 0) */
    } else {
        return;                     /* Autre source → on ignore            */
    }

    /* --- Mise à jour du volume ---
       Lit la valeur du potentiomètre via l'ADC et met à jour
       TIM2.CCR2 (duty cycle) pour ajuster le volume sonore            */
    reglage_volume();

    /* --- Passage à la note suivante ---
       Incrémente l'index i, charge note_curr, met à jour :
         - TIM2.ARR  (fréquence de la nouvelle note)
         - TIM2.CCR2 (duty cycle)
         - TIM5.ARR  (durée de la nouvelle note)
       Si la mélodie est terminée, arrête TIM5 ou reboucle au début   */
    player_next_note();
}
