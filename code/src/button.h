/*
 * button.h
 */

#ifndef SRC_BUTTON_H_
#define SRC_BUTTON_H_

#include <stdint.h>

/* GPIO init */
void init_blue_button();
void init_red_button();
uint32_t poll_blue_button();
uint32_t poll_red_button();

/* Sélection lignes EXTI */
void select_PC13_as_EXTI13();
void select_PB8_as_EXTI8();
void select_PB3_as_EXTI3();
void select_PB4_as_EXTI4();
void select_PB5_as_EXTI5();
void select_PB6_as_EXTI6();

/* Configuration évènements EXTI */
void setup_IRQ_on_EXTI13_events();
void setup_IRQ_on_EXTI8_events();
void setup_IRQ_on_EXTI3_events();
void setup_IRQ_on_EXTI4_events();
void setup_IRQ_on_EXTI5_events();
void setup_IRQ_on_EXTI6_events();

/* Activation NVIC */
void enable_EXTI15_10_IRQ_simple();
void enable_EXTI9_5_IRQ_simple();
void enable_EXTI15_10_IRQ();
void enable_EXTI9_5_IRQ();
void enable_EXTI3_IRQ();
void enable_EXTI4_IRQ();

/* Initialisation complète par switch */
void init_IRQ_PC13();
void init_IRQ_PB8();
void init_IRQ_PB3();      /* SW1 : toggle pause/resume  */
void init_IRQ_PB4();      /* SW2 : chanson suivante      */
void init_IRQ_PB5_PB6();  /* SW3 : tempo | SW4 : mode   */

/* Handlers IRQ */
void EXTI9_5_Handler();   /* PB8 + PB5 + PB6 */
void EXTI4_Handler();     /* PB4              */
void EXTI3_Handler();     /* PB3              */

#endif /* SRC_BUTTON_H_ */
