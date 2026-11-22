#include "button.h"
#include "sys/devices.h"
#include "irq.h"
#include "player.h"
#include "led.h"
#include "melodie.h"
#include <stdio.h>

/* ============================================================
   Init GPIO en entrée sans pull
   ============================================================ */

void init_blue_button() {
  /* PC13 : input, no pull */
  enable_GPIOC();
  GPIOC.MODER = GPIOC.MODER & ~(3<<26);
  GPIOC.PUPDR = GPIOC.PUPDR & ~(3<<26);
}

void init_red_button() {
  /* PB8 : input, no pull */
  enable_GPIOB();
  GPIOB.MODER = GPIOB.MODER & ~(3<<16);
  GPIOB.PUPDR = GPIOB.PUPDR & ~(3<<16);
}

uint32_t poll_blue_button() {
  return ((GPIOC.IDR >> 13) & 1);
}

uint32_t poll_red_button() {
  return ((GPIOB.IDR >> 8) & 1);
}

/* ============================================================
   Sélection des lignes EXTI
   ============================================================ */

void select_PC13_as_EXTI13() {
  enable_SYSCFG();
  SYSCFG.EXTICR4 = (SYSCFG.EXTICR4 & ~(0xf << 4)) | (2 << 4);
}

void select_PB8_as_EXTI8() {
  enable_SYSCFG();
  SYSCFG.EXTICR3 = (SYSCFG.EXTICR3 & ~(0xf << 0)) | (1 << 0);
}

void select_PB3_as_EXTI3() {
  /* EXTICR1 bits [15:12] = 0001 (port B) */
  enable_SYSCFG();
  SYSCFG.EXTICR1 = (SYSCFG.EXTICR1 & ~(0xf << 12)) | (1 << 12);
}

void select_PB4_as_EXTI4() {
  /* EXTICR2 bits [3:0] = 0001 (port B) */
  enable_SYSCFG();
  SYSCFG.EXTICR2 = (SYSCFG.EXTICR2 & ~(0xf << 0)) | (1 << 0);
}

void select_PB5_as_EXTI5() {
  /* EXTICR2 bits [7:4] = 0001 (port B) */
  enable_SYSCFG();
  SYSCFG.EXTICR2 = (SYSCFG.EXTICR2 & ~(0xf << 4)) | (1 << 4);
}

void select_PB6_as_EXTI6() {
  /* EXTICR2 bits [11:8] = 0001 (port B) */
  enable_SYSCFG();
  SYSCFG.EXTICR2 = (SYSCFG.EXTICR2 & ~(0xf << 8)) | (1 << 8);
}

/* ============================================================
   Configuration des évènements EXTI
   ============================================================ */

void setup_IRQ_on_EXTI13_events() {
  EXTI.IMR  |= (1<<13);
  EXTI.FTSR |= (1<<13);
  EXTI.RTSR |= (1<<13);
  EXTI.PR    = (1<<13);
}

void setup_IRQ_on_EXTI8_events() {
  /* PB8 : front montant uniquement (bouton poussoir) */
  EXTI.IMR  |= (1<<8);
  EXTI.RTSR |= (1<<8);
  EXTI.PR    = (1<<8);
}

void setup_IRQ_on_EXTI3_events() {
  /* PB3 toggle : les deux fronts */
  EXTI.IMR  |= (1<<3);
  EXTI.RTSR |= (1<<3);
  EXTI.FTSR |= (1<<3);
  EXTI.PR    = (1<<3);
}

void setup_IRQ_on_EXTI4_events() {
  /* PB4 : front montant (appui switch) */
  EXTI.IMR  |= (1<<4);
  EXTI.RTSR |= (1<<4);
  EXTI.FTSR |= (1<<4);
  EXTI.PR    = (1<<4);
}

void setup_IRQ_on_EXTI5_events() {
  /* PB5 : front montant (appui switch) */
  EXTI.IMR  |= (1<<5);
  EXTI.RTSR |= (1<<5);
  EXTI.FTSR |= (1<<5);
  EXTI.PR    = (1<<5);
}

void setup_IRQ_on_EXTI6_events() {
  /* PB6 : front montant (appui switch) */
  EXTI.IMR  |= (1<<6);
  EXTI.RTSR |= (1<<6);
  EXTI.FTSR |= (1<<6);
  EXTI.PR    = (1<<6);
}

/* ============================================================
   Activation NVIC
   ============================================================ */

void enable_EXTI15_10_IRQ_simple() { NVIC.ISER[1] = (1<<8);  }
void enable_EXTI9_5_IRQ_simple()   { NVIC.ISER[0] = (1<<23); }

void enable_EXTI15_10_IRQ() { enable_IRQ_by_number(40); }
void enable_EXTI9_5_IRQ()   { enable_IRQ_by_number(23); }
void enable_EXTI3_IRQ()     { enable_IRQ_by_number(9);  }
void enable_EXTI4_IRQ()     { enable_IRQ_by_number(10); }

/* ============================================================
   Fonctions d'initialisation complètes
   ============================================================ */

void init_IRQ_PC13() {
  init_blue_button();
  select_PC13_as_EXTI13();
  setup_IRQ_on_EXTI13_events();
  enable_EXTI15_10_IRQ();
}

void init_IRQ_PB8() {
  init_red_button();
  select_PB8_as_EXTI8();
  setup_IRQ_on_EXTI8_events();
  enable_EXTI9_5_IRQ();
}

void init_IRQ_PB3() {
  /* SW1 : toggle pause/resume */
  enable_GPIOB();
  GPIOB.MODER = GPIOB.MODER & ~(3 << 6);
  GPIOB.PUPDR = GPIOB.PUPDR & ~(3 << 6);
  select_PB3_as_EXTI3();
  setup_IRQ_on_EXTI3_events();
  enable_EXTI3_IRQ();
}

void init_IRQ_PB4() {
  /* SW2 : chanson suivante */
  enable_GPIOB();
  GPIOB.MODER = GPIOB.MODER & ~(3 << 8);
  GPIOB.PUPDR = GPIOB.PUPDR & ~(3 << 8);
  select_PB4_as_EXTI4();
  setup_IRQ_on_EXTI4_events();
  enable_EXTI4_IRQ();
}

void init_IRQ_PB5_PB6() {
  /* SW3 (tempo) et SW4 (mode) partagent EXTI9_5 */
  enable_GPIOB();
  GPIOB.MODER = GPIOB.MODER & ~(3 << 10); /* PB5 input */
  GPIOB.PUPDR = GPIOB.PUPDR & ~(3 << 10);
  GPIOB.MODER = GPIOB.MODER & ~(3 << 12); /* PB6 input */
  GPIOB.PUPDR = GPIOB.PUPDR & ~(3 << 12);
  select_PB5_as_EXTI5();
  select_PB6_as_EXTI6();
  setup_IRQ_on_EXTI5_events();
  setup_IRQ_on_EXTI6_events();
  /* EXTI9_5 déjà activé par init_IRQ_PB8, pas besoin de le refaire */
}

/* ============================================================
   HANDLERS IRQ
   ============================================================ */

/* ----------------------------------------------------------
   EXTI9_5 : PB8 (stop) + PB5 (tempo) + PB6 (mode)
   ---------------------------------------------------------- */
void EXTI9_5_Handler()
{
    /* --- PB8 : stop + retour menu --- */
    if (EXTI.PR & (1 << 8)) {
        EXTI.PR |= (1 << 8);
        if (g_playing) {
            player_stop();
            set_led_color(0b000);
            g_playing  = 0;
            printf("\r\n[PB8] Arret. Nouveau choix...\r\n");
        }
        g_demander = 1;
    }

    /* --- PB5 (SW3) : cycle tempo 60->90->120->160->60 BPM --- */
    if (EXTI.PR & (1 << 5)) {
        EXTI.PR |= (1 << 5);
        player_next_tempo();
    }

    /* --- PB6 (SW4) : bascule mode boucle <-> une fois --- */
    if (EXTI.PR & (1 << 6)) {
        EXTI.PR |= (1 << 6);
        player_toggle_mode();

    }
}

/* ----------------------------------------------------------
   EXTI4 : PB4 (SW2) -> chanson suivante
   ---------------------------------------------------------- */
void EXTI4_Handler()
{
    if (EXTI.PR & (1 << 4)) {
        EXTI.PR |= (1 << 4);
        player_next_song();
    }
}

/* ----------------------------------------------------------
   EXTI3 : PB3 (SW1) -> toggle pause / resume
   ---------------------------------------------------------- */
void EXTI3_Handler()
{
    if (EXTI.PR & (1 << 3)) {
        EXTI.PR |= (1 << 3);

        if (!g_playing) return;

        uint32_t sw_on = (GPIOB.IDR >> 3) & 1;
        if (sw_on) {
            player_pause();
            printf("\r\n[SW1] Pause\r\n");
        } else {
            player_resume();
            printf("\r\n[SW1] Resume\r\n");
        }
    }
}
