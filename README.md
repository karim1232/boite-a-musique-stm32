# Boîte à musique STM32 — Nucleo F446RE

Projet Microcontrôleurs, IESE3, Polytech Grenoble (2025/2026), groupe 3 « Tidjallarim » : Allan Bamas, Tidjane Clautrier et Karim Glelah.

Lecteur de musique embarqué sur carte **STM32 Nucleo F446RE** avec la carte fille du TP.
Le buzzer joue des mélodies en PWM, la LED tricolore change de couleur selon la note,
et le volume est réglé par un potentiomètre. Le code est en C avec accès direct aux registres (framework Polytech, sans HAL).

📄 [Rapport](docs/Rapport_BoiteAMusique.pdf) · [Diapos](docs/Diapos_BoiteAMusique.pdf) · [Compte rendu séance 1](docs/CR_seance1.pdf)

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

Dans STM32CubeIDE : *File → Import → Existing Projects into Workspace*, puis choisir `code/`. On peut aussi lancer `make` dans `code/`. Le binaire produit est `bin/prog.elf`.

---

## Matériel requis

| Composant         | Broche         | Rôle                                 |
|-------------------|----------------|--------------------------------------|
| Buzzer            | PB9            | Sortie PWM (TIM2 CH2 / AF1)          |
| LED tricolore     | PA8, PA9, PA10 | Rouge, Vert, Bleu                    |
| Potentiomètre     | ADC            | Contrôle du volume en temps réel     |
| Bouton rouge PB8  | PB8            | Stop + ouvre le menu interactif      |
| Switch SW1        | PB3            | Toggle pause / resume                |
| Switch SW2        | PB4            | Chanson suivante                     |
| Switch SW3        | PB5            | Cycle du tempo (BPM)                 |
| Switch SW4        | PB6            | Play / Stop automatique              |

---

## Démarrage rapide

Au démarrage la carte est **silencieuse** : aucun menu n'apparaît sur GTKTerm.

| Action                 | Effet                                                               |
|------------------------|---------------------------------------------------------------------|
| **SW4** (PB6)          | Lance la 1ère chanson (Frère Jacques) en boucle au tempo conseillé |
| **SW4** à nouveau      | Arrête la lecture                                                   |
| **PB8** (bouton rouge) | Arrête la lecture et ouvre le menu interactif sur GTKTerm           |

---

## Contrôles en cours de lecture

| Contrôle          | Action                                                    |
|-------------------|-----------------------------------------------------------|
| **SW1** ON (PB3)  | Pause — musique gelée, position conservée                 |
| **SW1** OFF (PB3) | Resume — reprend là où ça s'était arrêté                  |
| **SW2** (PB4)     | Passe immédiatement à la chanson suivante                 |
| **SW3** (PB5)     | Cycle le tempo : 60 → 80 → 90 → 120 → 160 → 60 BPM       |
| **SW4** (PB6)     | Stop la lecture                                           |
| **Potentiomètre** | Règle le volume en temps réel                             |
| **PB8**           | Stop + retour au menu interactif sur GTKTerm              |

---

## Catalogue des mélodies

| #  | Titre                  | Tonalité   | Mesure | Tempo conseillé |
|----|------------------------|------------|--------|-----------------|
| 1  | Frère Jacques          | Do majeur  | 4/4    | 120 BPM         |
| 2  | Au Clair de la Lune    | Do majeur  | 4/4    |  80 BPM         |
| 3  | Happy Birthday         | Sol majeur | 3/4    |  90 BPM         |
| 4  | Nokia Ringtone         | La majeur  | 4/4    | 120 BPM         |
| 5  | Twinkle Twinkle        | Do majeur  | 4/4    | 120 BPM         |
| 6  | Berceuse de Brahms     | Sol majeur | 3/4    |  80 BPM         |

---

## Menu interactif (GTKTerm / UART)

Déclenché uniquement par **PB8**. Le menu s'affiche sur le terminal série.  
Le tempo conseillé de chaque chanson est affiché à côté de son nom.  
Si une entrée invalide est saisie pour le tempo, le tempo conseillé est appliqué automatiquement.

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

---

## Architecture du code

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

---

## Fonctionnement technique

### Génération du son (TIM2 + buzzer)

TIM2 est configuré en mode PWM sur le canal CH2 (sortie sur PB9).  
La **fréquence** du signal correspond à la note jouée :

```
ARR = (1 000 000 / fréquence_note) - 1
```

Le **duty cycle** correspond au volume :

```
CCR2 = ARR / cycle_val
```

`cycle_val` est mis à jour par le potentiomètre via l'ADC à chaque fin de note.

### Rythme des notes (TIM5 + IRQ)

TIM5 est configuré en compteur simple à 1 MHz (1 tick = 1 µs).  
Sa valeur ARR est chargée avec la durée de la note en cours :

```
TIM5.ARR = tempo_duration_ms(dur) * 1000
```

À chaque overflow de TIM5 → IRQ `TIM5_Handler()` → `player_next_note()`.

### Correspondance note → couleur LED

Toutes les 7 combinaisons RGB possibles sont utilisées, réparties sur toutes
les notes des 6 mélodies (octaves 3, 4 et 5, notes naturelles et dièses).

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

### Mapping IRQ des boutons

| Broche | EXTI    | IRQ | Handler           |
|--------|---------|-----|-------------------|
| PB3    | EXTI3   |  9  | `EXTI3_Handler`   |
| PB4    | EXTI4   | 10  | `EXTI4_Handler`   |
| PB5    | EXTI9_5 | 23  | `EXTI9_5_Handler` |
| PB6    | EXTI9_5 | 23  | `EXTI9_5_Handler` |
| PB8    | EXTI9_5 | 23  | `EXTI9_5_Handler` |

PB5, PB6 et PB8 partagent le même handler. Le flag `EXTI.PR` est lu pour
identifier quelle ligne a déclenché l'interruption.

---

## Ajouter une mélodie

**1.** Définir le tableau dans `melodie.c` :
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

**2.** Déclarer dans `melodie.h` :
```c
extern const note_t   melodie_ma_chanson[];
extern const uint32_t melodie_ma_chanson_size;
```

**3.** Ajouter dans le tableau `catalogue[]` de `player.c` avec son tempo conseillé :
```c
{ "Ma Chanson", melodie_ma_chanson, &melodie_ma_chanson_size, 120 },
```

---

## Configuration GTKTerm

| Paramètre | Valeur                            |
|-----------|-----------------------------------|
| Port      | /dev/ttyACM0 (ou COM sur Windows) |
| Baud rate | 9600                           |