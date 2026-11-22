# Boîte à musique sur STM32 Nucleo F446RE

Projet de Microcontrôleurs en IESE3 à Polytech Grenoble (2025/2026), groupe 3 « Tidjallarim » : Allan Bamas, Tidjane Clautrier et Karim Glelah.

La carte Nucleo F446RE, avec la carte fille du TP, joue des mélodies sur un buzzer en PWM. La LED tricolore change de couleur à chaque note et un potentiomètre règle le volume. Le code est en C et travaille directement sur les registres (framework Polytech, sans HAL).

Documents : [rapport](docs/Rapport_BoiteAMusique.pdf), [diapos](docs/Diapos_BoiteAMusique.pdf), [compte rendu de la séance 1](docs/CR_seance1.pdf).

## Arborescence du dépôt

```
.
├── code/                   projet STM32CubeIDE (Makefile du framework Polytech)
│   └── src/                sources C (détail plus bas)
└── docs/
    ├── Rapport_BoiteAMusique.pdf
    ├── Diapos_BoiteAMusique.pdf
    ├── CR_seance1.pdf      compte rendu de la séance 1
    └── reference/          extraits de la documentation ST (RM0390, datasheet STM32F446)
```

## Compiler et flasher

Dans STM32CubeIDE, passer par *File → Import → Existing Projects into Workspace* et choisir `code/`. On peut aussi lancer `make` dans `code/` ; le binaire sort dans `bin/prog.elf`.

## Matériel

| Composant         | Broche         | Rôle                                 |
|-------------------|----------------|--------------------------------------|
| Buzzer            | PB9            | sortie PWM (TIM2 CH2 / AF1)          |
| LED tricolore     | PA8, PA9, PA10 | rouge, vert, bleu                    |
| Potentiomètre     | ADC            | volume, relu à chaque note           |
| Bouton rouge PB8  | PB8            | stop et ouverture du menu            |
| Switch SW1        | PB3            | pause / reprise                      |
| Switch SW2        | PB4            | chanson suivante                     |
| Switch SW3        | PB5            | change le tempo (BPM)                |
| Switch SW4        | PB6            | lecture / stop                       |

## Démarrage

À l'allumage la carte ne joue rien et aucun menu ne s'affiche sur GTKTerm.

| Action                 | Effet                                                               |
|------------------------|---------------------------------------------------------------------|
| SW4 (PB6)              | lance Frère Jacques en boucle, au tempo conseillé                   |
| SW4 une deuxième fois  | arrête la lecture                                                   |
| PB8 (bouton rouge)     | arrête la lecture et ouvre le menu sur GTKTerm                      |

## Pendant la lecture

| Contrôle          | Action                                                    |
|-------------------|-----------------------------------------------------------|
| SW1 ON (PB3)      | pause : la musique s'arrête à sa position actuelle        |
| SW1 OFF (PB3)     | reprise à l'endroit où elle s'était arrêtée               |
| SW2 (PB4)         | passe tout de suite à la chanson suivante                 |
| SW3 (PB5)         | fait tourner le tempo : 60, 80, 90, 120, 160 puis 60 BPM  |
| SW4 (PB6)         | arrête la lecture                                         |
| Potentiomètre     | règle le volume pendant la lecture                        |
| PB8               | arrête et revient au menu sur GTKTerm                     |

## Les mélodies

| #  | Titre                  | Tonalité   | Mesure | Tempo conseillé |
|----|------------------------|------------|--------|-----------------|
| 1  | Frère Jacques          | Do majeur  | 4/4    | 120 BPM         |
| 2  | Au Clair de la Lune    | Do majeur  | 4/4    |  80 BPM         |
| 3  | Happy Birthday         | Sol majeur | 3/4    |  90 BPM         |
| 4  | Nokia Ringtone         | La majeur  | 4/4    | 120 BPM         |
| 5  | Twinkle Twinkle        | Do majeur  | 4/4    | 120 BPM         |
| 6  | Berceuse de Brahms     | Sol majeur | 3/4    |  80 BPM         |

## Menu sur le terminal série (GTKTerm / UART)

Le menu ne s'ouvre qu'avec PB8. Chaque chanson y est affichée avec son tempo conseillé, et si on tape un tempo invalide, c'est ce tempo conseillé qui est utilisé.

```
========================================
        ** MUSIC PLAYER STM32 **
========================================

Quelle chanson voulez-vous jouer ?
  1 - Frere Jacques              (conseille : 120 BPM)
  2 - Au Clair de la Lune        (conseille :  80 BPM)
  3 - Happy Birthday             (conseille :  90 BPM)
  4 - Nokia Ringtone             (conseille : 120 BPM)
  5 - Twinkle Twinkle            (conseille : 120 BPM)
  6 - Berceuse de Brahms         (conseille :  80 BPM)

Quel tempo voulez-vous ?
  1 - Tres lent  ( 60 BPM)
  2 - Lent       ( 80 BPM)
  3 - Modere     ( 90 BPM)
  4 - Rapide     (120 BPM)
  5 - Tres vite  (160 BPM)

  [Appuyer ENTREE pour utiliser le tempo conseille]

Mode de lecture :
  1 - Une seule fois
  2 - En boucle
```

## Organisation du code

```
code/src/
├── main.c          Point d'entrée : init hardware + boucle principale
├── player.c/.h     Cœur du lecteur : lecture, menu, actions switches
├── timer.c/.h      Configuration TIM2 (PWM) et TIM5 (durée des notes)
├── button.c/.h     GPIO, EXTI, NVIC et handlers IRQ des boutons/switches
├── note.c/.h       Type note_t, fréquences, calcul ARR du timer
├── tempo.c/.h      Gestion du BPM et conversion valeurs musicales → ms
├── melodie.c/.h    Catalogue des 6 mélodies (tableaux de note_t)
├── led.c/.h        LED tricolore PA8/PA9/PA10
└── volume.c/.h     Lecture ADC potentiomètre → duty cycle PWM
```

## Comment ça marche

### Le son (TIM2 et buzzer)

TIM2 tourne en PWM sur le canal CH2, qui sort sur PB9. La fréquence du signal donne la note :

```
ARR = (1 000 000 / fréquence_note) - 1
```

et le rapport cyclique donne le volume :

```
CCR2 = ARR / cycle_val
```

`cycle_val` est relu sur le potentiomètre (via l'ADC) à la fin de chaque note.

### Le rythme (TIM5 et interruption)

TIM5 compte à 1 MHz, donc un tick vaut 1 µs. On charge dans son ARR la durée de la note en cours :

```
TIM5.ARR = tempo_duration_ms(dur) * 1000
```

Quand TIM5 déborde, l'interruption `TIM5_Handler()` appelle `player_next_note()`, qui passe à la note suivante.

### Couleur de la LED selon la note

Les 7 combinaisons RGB possibles sont toutes utilisées, sur toutes les notes des 6 mélodies (octaves 3, 4 et 5, notes naturelles et dièses).

| Note(s)              | Couleur              |
|----------------------|----------------------|
| DO  (toutes octaves) | Rouge                |
| RE  (toutes octaves) | Vert                 |
| MI  (toutes octaves) | Bleu                 |
| FA  (toutes octaves) | Jaune  (R + V)       |
| SOL (toutes octaves) | Magenta (R + B)      |
| LA  (toutes octaves) | Cyan   (V + B)       |
| SI  (toutes octaves) | Blanc  (R + V + B)   |
| CS4, CS5             | Jaune                |
| FS4, FS5             | Magenta              |
| GS4                  | Cyan                 |
| SILENCE              | Éteinte              |

### Interruptions des boutons

| Broche | EXTI    | IRQ | Handler           |
|--------|---------|-----|-------------------|
| PB3    | EXTI3   |  9  | `EXTI3_Handler`   |
| PB4    | EXTI4   | 10  | `EXTI4_Handler`   |
| PB5    | EXTI9_5 | 23  | `EXTI9_5_Handler` |
| PB6    | EXTI9_5 | 23  | `EXTI9_5_Handler` |
| PB8    | EXTI9_5 | 23  | `EXTI9_5_Handler` |

PB5, PB6 et PB8 tombent dans le même handler, qui lit `EXTI.PR` pour savoir quelle ligne a déclenché l'interruption.

## Ajouter une mélodie

1. Écrire le tableau de notes dans `melodie.c` :
```c
const note_t melodie_ma_chanson[] = {
    {DO4,     DUR_QUARTER},
    {MI4,     DUR_QUARTER},
    {SOL4,    DUR_HALF},
    {SILENCE, DUR_QUARTER},
};
const uint32_t melodie_ma_chanson_size =
    sizeof(melodie_ma_chanson) / sizeof(note_t);
```

2. Le déclarer dans `melodie.h` :
```c
extern const note_t   melodie_ma_chanson[];
extern const uint32_t melodie_ma_chanson_size;
```

3. L'ajouter au tableau `catalogue[]` de `player.c`, avec son tempo conseillé :
```c
{ "Ma Chanson", melodie_ma_chanson, &melodie_ma_chanson_size, 120 },
```

## Réglages de GTKTerm

| Paramètre | Valeur                            |
|-----------|-----------------------------------|
| Port      | /dev/ttyACM0 (ou COM sur Windows) |
| Baud rate | 9600                              |
