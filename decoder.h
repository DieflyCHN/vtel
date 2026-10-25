#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

#define DECODER_SYMBOL_CAP 16
#define DECODER_MORSE_CAP 4096
#define DECODER_TEXT_CAP 1024

typedef struct {
    int dash_threshold;
    int letter_gap_threshold;
    int word_gap_threshold;
    uint8_t initialized;
    uint8_t current_bit;
    int run_length;
    uint8_t in_idle;
    uint8_t gap_state;
    char current_symbol[DECODER_SYMBOL_CAP];
    int current_symbol_len;
    char morse_output[DECODER_MORSE_CAP];
    int morse_output_len;
    char text_output[DECODER_TEXT_CAP];
    int text_output_len;
    uint8_t section_display;
} Decoder;

void decoder_init(Decoder *d);
void decoder_feed_bit(Decoder *d, uint8_t bit);
void decoder_flush(Decoder *d);
void decoder_print_morse_table(void);

#endif
