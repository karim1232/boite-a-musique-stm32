# Music box on STM32 Nucleo F446RE

Microcontrollers course project, IESE3 at Polytech Grenoble (2025/2026), group 3 "Tidjallarim": Allan Bamas, Tidjane Clautrier and Karim Glelah.

The Nucleo F446RE board, with the course's daughter board, plays melodies on a buzzer using PWM. The RGB LED changes color with every note and a potentiometer sets the volume. The code is in C and works directly on the registers (Polytech framework, no HAL).

Documents (in French): [report](docs/Rapport_BoiteAMusique.pdf), [slides](docs/Diapos_BoiteAMusique.pdf), [session 1 report](docs/CR_seance1.pdf).

## Repository layout

```
.
├── code/                   STM32CubeIDE project (Polytech framework Makefile)
│   └── src/                C sources (details below)
└── docs/
    ├── Rapport_BoiteAMusique.pdf
    ├── Diapos_BoiteAMusique.pdf
    ├── CR_seance1.pdf      session 1 report
    └── reference/          excerpts from the ST documentation (RM0390, STM32F446 datasheet)
```

## Build and flash

In STM32CubeIDE, use *File → Import → Existing Projects into Workspace* and pick `code/`. You can also run `make` in `code/`; the binary ends up in `bin/prog.elf`.

## Hardware

| Component         | Pin            | Role                                 |
|-------------------|----------------|--------------------------------------|
| Buzzer            | PB9            | PWM output (TIM2 CH2 / AF1)          |
| RGB LED           | PA8, PA9, PA10 | red, green, blue                     |
| Potentiometer     | ADC            | volume, read again on every note     |
| Red button PB8    | PB8            | stop and open the menu               |
| Switch SW1        | PB3            | pause / resume                       |
| Switch SW2        | PB4            | next song                            |
| Switch SW3        | PB5            | change the tempo (BPM)               |
| Switch SW4        | PB6            | play / stop                          |

## Getting started

At power-up the board plays nothing and no menu shows up on GTKTerm.

| Action                 | Effect                                                              |
|------------------------|---------------------------------------------------------------------|
| SW4 (PB6)              | plays Frère Jacques in a loop at its suggested tempo                |
| SW4 a second time      | stops playback                                                      |
| PB8 (red button)       | stops playback and opens the menu on GTKTerm                        |

## During playback

| Control           | Action                                                    |
|-------------------|-----------------------------------------------------------|
| SW1 ON (PB3)      | pause: the music stops at its current position            |
| SW1 OFF (PB3)     | resume from where it stopped                              |
| SW2 (PB4)         | skip to the next song right away                          |
| SW3 (PB5)         | cycle the tempo: 60, 80, 90, 120, 160, then 60 BPM        |
| SW4 (PB6)         | stop playback                                             |
| Potentiometer     | adjust the volume while playing                           |
| PB8               | stop and go back to the menu on GTKTerm                   |

## Melodies

| #  | Title                  | Key        | Time   | Suggested tempo |
|----|------------------------|------------|--------|-----------------|
| 1  | Frère Jacques          | C major    | 4/4    | 120 BPM         |
| 2  | Au Clair de la Lune    | C major    | 4/4    |  80 BPM         |
| 3  | Happy Birthday         | G major    | 3/4    |  90 BPM         |
| 4  | Nokia Ringtone         | A major    | 4/4    | 120 BPM         |
| 5  | Twinkle Twinkle        | C major    | 4/4    | 120 BPM         |
| 6  | Brahms' Lullaby        | G major    | 3/4    |  80 BPM         |

## Serial menu (GTKTerm / UART)

The menu only opens with PB8. Each song is listed with its suggested tempo, and if an invalid tempo is typed, the suggested one is used. The menu text itself is in French:

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

## Code organization

```
code/src/
├── main.c          entry point: hardware init + main loop
├── player.c/.h     player core: playback, menu, switch actions
├── timer.c/.h      TIM2 setup (PWM) and TIM5 (note duration)
├── button.c/.h     GPIO, EXTI, NVIC and button/switch IRQ handlers
├── note.c/.h       note_t type, frequencies, timer ARR computation
├── tempo.c/.h      BPM handling and conversion of note values to ms
├── melodie.c/.h    the 6 melodies (arrays of note_t)
├── led.c/.h        RGB LED on PA8/PA9/PA10
└── volume.c/.h     potentiometer ADC reading → PWM duty cycle
```

## How it works

### Sound (TIM2 and buzzer)

TIM2 runs in PWM mode on channel CH2, output on PB9. The signal frequency sets the note:

```
ARR = (1 000 000 / note_frequency) - 1
```

and the duty cycle sets the volume:

```
CCR2 = ARR / cycle_val
```

`cycle_val` is read again from the potentiometer (through the ADC) at the end of each note.

### Rhythm (TIM5 and interrupt)

TIM5 counts at 1 MHz, so one tick is 1 µs. Its ARR is loaded with the duration of the current note:

```
TIM5.ARR = tempo_duration_ms(dur) * 1000
```

When TIM5 overflows, the `TIM5_Handler()` interrupt calls `player_next_note()`, which moves on to the next note.

### LED color per note

All 7 possible RGB combinations are used, across every note of the 6 melodies (octaves 3, 4 and 5, natural notes and sharps).

| Note(s)              | Color                |
|----------------------|----------------------|
| C (all octaves)      | Red                  |
| D (all octaves)      | Green                |
| E (all octaves)      | Blue                 |
| F (all octaves)      | Yellow (R + G)       |
| G (all octaves)      | Magenta (R + B)      |
| A (all octaves)      | Cyan (G + B)         |
| B (all octaves)      | White (R + G + B)    |
| CS4, CS5             | Yellow               |
| FS4, FS5             | Magenta              |
| GS4                  | Cyan                 |
| SILENCE              | Off                  |

### Button interrupts

| Pin    | EXTI    | IRQ | Handler           |
|--------|---------|-----|-------------------|
| PB3    | EXTI3   |  9  | `EXTI3_Handler`   |
| PB4    | EXTI4   | 10  | `EXTI4_Handler`   |
| PB5    | EXTI9_5 | 23  | `EXTI9_5_Handler` |
| PB6    | EXTI9_5 | 23  | `EXTI9_5_Handler` |
| PB8    | EXTI9_5 | 23  | `EXTI9_5_Handler` |

PB5, PB6 and PB8 share the same handler, which reads `EXTI.PR` to find which line triggered the interrupt.

## Adding a melody

1. Write the note array in `melodie.c` (note names follow the French convention: DO = C, MI = E, SOL = G):
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

2. Declare it in `melodie.h`:
```c
extern const note_t   melodie_ma_chanson[];
extern const uint32_t melodie_ma_chanson_size;
```

3. Add it to the `catalogue[]` array in `player.c`, with its suggested tempo:
```c
{ "Ma Chanson", melodie_ma_chanson, &melodie_ma_chanson_size, 120 },
```

## GTKTerm settings

| Setting   | Value                             |
|-----------|-----------------------------------|
| Port      | /dev/ttyACM0 (or COM on Windows)  |
| Baud rate | 9600                              |
