/*
 * main.c
 *
 * Point d'entrée du lecteur de musique sur STM32 Nucleo F446.
 *
 * Rôle de ce fichier :
 *   - Initialiser tous les périphériques hardware (GPIO, TIM2, TIM5, ADC, LED)
 *   - Configurer les interruptions de tous les boutons et switches
 *   - Lancer la boucle principale qui orchestre le menu et la lecture
 *
 * La logique de lecture est entièrement dans player.c.
 * Les interruptions sont gérées dans button.c.
 * Les timers sont configurés dans timer.c.
 *
 * Contrôles disponibles :
 *   PB8  (bouton rouge) → stop + ouvre le menu interactif sur GTKTerm
 *   SW1  PB3            → toggle pause / resume
 *   SW2  PB4            → passe à la chanson suivante
 *   SW3  PB5            → cycle tempo : 60 / 90 / 120 / 160 BPM
 *   SW4  PB6            → play / stop en mode automatique (sans menu)
 */

#include <stdio.h>
#include "sys/cm4.h"       /* __WFI() : mise en veille entre interruptions    */
#include "sys/devices.h"   /* Registres GPIO, TIM2, U_ID                      */
#include "sys/init.h"      /* Initialisation système de base                  */
#include "sys/clock.h"     /* get_SYSCLK(), get_APB1TIMCLK()                  */
#include "timer.h"         /* init_TIM2_PWM(), enable_TIM2/5()                */
#include "button.h"        /* init_IRQ_PBx()                                  */
#include "player.h"        /* player_needs_menu(), demander_et_lancer()       */
#include "tempo.h"         /* (inclus via player.h - BPM)                     */
#include "melodie.h"       /* Catalogue des mélodies                          */
#include "adc.h"           /* configuration_potentiometre()                   */
#include "led.h"           /* init_led_tri()                                  */

int main()
{
    /* ----------------------------------------------------------------
       Affichage d'information système au démarrage (GTKTerm)
       \e[2J      : efface l'écran du terminal
       \e[1;1H    : replace le curseur en haut à gauche
       \e[?25l    : cache le curseur
       ---------------------------------------------------------------- */
    printf("\e[2J\e[1;1H\r\n*** Welcome to Nucleo F446 ! ***\r\n");
    printf("   %08lx-%08lx-%08lx\r\n", U_ID[0], U_ID[1], U_ID[2]);
    printf("SYSCLK     = %9lu Hz\r\n", get_SYSCLK());
    printf("APB1TIMCLK = %9lu Hz\r\n", get_APB1TIMCLK());
    printf("\r\n");
    printf("\e[?25l");

    /* ----------------------------------------------------------------
       Configuration de PB9 en mode alternatif TIM2_CH2 (buzzer PWM)
       PB9 est connecté au buzzer sur le shield.
       MODER bits [19:18] = 10 → mode alternatif
       AFRH  bits [7:4]   = 0001 → AF1 = TIM2_CH2
       ---------------------------------------------------------------- */
    enable_GPIOB();
    GPIOB.MODER &= ~(3 << 18);   /* Efface les bits de mode de PB9           */
    GPIOB.MODER |=  (2 << 18);   /* PB9 en mode alternatif                   */
    GPIOB.AFRH  &= ~(0xf << 4);  /* Efface la fonction alternative de PB9    */
    GPIOB.AFRH  |=  (1   << 4);  /* AF1 : TIM2_CH2                           */

    /* ----------------------------------------------------------------
       Initialisation des timers
       TIM2 : génère le PWM pour le buzzer (fréquence = note jouée)
       TIM5 : mesure la durée de chaque note (déclenche l'IRQ de fin)
       ---------------------------------------------------------------- */
    enable_TIM2();
    init_TIM2_PWM();
    enable_TIM5();               /* TIM5 sera démarré par player_init()       */

    /* ----------------------------------------------------------------
       Initialisation de l'ADC et de la LED tricolore
       Le potentiomètre règle le volume via le duty cycle de TIM2.
       La LED change de couleur selon la note jouée.
       ---------------------------------------------------------------- */
    configuration_potentiometre();
    init_led_tri();

    /* ----------------------------------------------------------------
       Configuration des boutons et switches (GPIO + EXTI + NVIC)
       L'ordre est important : init_IRQ_PB8 active EXTI9_5 dans le NVIC,
       ce qui est partagé par PB5 et PB6 (init_IRQ_PB5_PB6 ne le réactive pas).
       ---------------------------------------------------------------- */
    init_IRQ_PB8();      /* Bouton rouge PB8 : stop + menu interactif        */
    init_IRQ_PB3();      /* SW1 PB3 : toggle pause / resume                  */
    init_IRQ_PB4();      /* SW2 PB4 : chanson suivante                       */
    init_IRQ_PB5_PB6();  /* SW3 PB5 : tempo  |  SW4 PB6 : play/stop auto     */

    /* ----------------------------------------------------------------
       Boucle principale
       Le CPU se met en veille (__WFI) entre chaque interruption.
       À chaque réveil :
         - Si PB8 a été pressé (g_demander = 1) → affiche le menu UART
         - Si MODE_ONCE et mélodie terminée → remet g_playing à 0
       ---------------------------------------------------------------- */
    while (1) {

        /* Mode MANUEL : affiche le menu sur GTKTerm si PB8 a été pressé */
        if (player_needs_menu())
            demander_et_lancer();

        /* Détecte la fin de mélodie en mode "une seule fois" */
        player_check_done();

        /* Mise en veille : le CPU attend la prochaine interruption         */
        /* (TIM5, EXTI3, EXTI4 ou EXTI9_5) avant de continuer              */
        __WFI();
    }
}
