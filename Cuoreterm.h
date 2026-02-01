#ifndef CUORETERM_H
#define CUORETERM_H

// defining the following variables before including this header will have the following effects 
// you will also have to provide your own implementations of these functions
// malloc, free, write_to_whatever(for debug_write)

// CUORETERM_DATASEG_BACKBUFFER 
// removes the following arguments from cuoreterm_init and uses a backbuffer in .bss instead of dynamicly allocating it
//    void *(*malloc_fn)(size_t)
//    void (*free_fn)(void *)

// CUORETERM_DEBUG
// adds the following argument to cuoreterm_init and uses it to print debug messages to the
//    void (*debug_write)(const char *msg, size_t len)

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CUORETERM_DATASEG_BACKBUFFER
#define CUORETERM_FALLBACK_RESOLUTION (1280 * 720 * 4)
static uint8_t c_backbuffer[CUORETERM_FALLBACK_RESOLUTION]; // .bss section im pretty sure
#endif

struct terminal {
    void *fb_addr;
    uint32_t fb_width, fb_height, fb_pitch;
    uint8_t  fb_bpp; // bytes per pixel
    
    uint8_t *fb_buf;
    uint32_t fb_buf_pitch;

    uint32_t fgcol;
    uint32_t cursor_x, cursor_y;
    uint8_t r_offset, g_offset, b_offset, a_offset;

    const uint8_t *font_data;
    uint32_t font_width, font_height;

    uint32_t cols, rows;
};

void cuoreterm_init(
    struct terminal *term,
    void *fb_addr,
    uint32_t fb_pitch,
    uint32_t fb_width,
    uint32_t fb_height,
    uint32_t fb_bpp_bits,
    uint8_t r_shift, uint8_t g_shift, uint8_t b_shift,
    const uint8_t *font,
    uint32_t font_w,
    uint32_t font_h
    #ifndef CUORETERM_DATASEG_BACKBUFFER
        , void *(*malloc_fn)(size_t)
        , void (*free_fn)(void *)
    #endif
        
    #ifdef CUORETERM_DEBUG
       , void (*debug_write)(const char *msg, size_t len)
    #endif
);

void cuoreterm_write(void *ctx, const char *msg, uint64_t len);
void cuoreterm_draw_char(struct terminal *term, char c, uint32_t fg);
void cuoreterm_set_font(struct terminal *term, const uint8_t *font, uint32_t font_w, uint32_t font_h);
void cuoreterm_clear(struct terminal *term);

#ifdef __cplusplus
}
#endif

#ifdef CUORETERM_IMPL

static inline void *h_memset(void *dst, uint8_t v, uint32_t n) {
    uint8_t *p = (uint8_t *)dst;
    if (!n) return dst;

    uint64_t vv = (uint64_t)v;
    uint64_t val64 =
        vv | (vv << 8) | (vv << 16) | (vv << 24) |
        (vv << 32) | (vv << 40) | (vv << 48) | (vv << 56);

    // 8 byte aligned
    while (((uintptr_t)p & 7) && n) {
        *p++ = v;
        n--;
    }

    uint64_t *p64 = (uint64_t *)p;
    while (n >= 8) {
        *p64++ = val64;
        n -= 8;
    }

    p = (uint8_t *)p64;
    while (n--) {
        *p++ = v;
    }

    return dst;
}

static inline void *h_memcpy(void *dst, const void *src, uint32_t n) {
    if (!n || dst == src) return dst;

    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    while (((uintptr_t)d & 7) && n) {
        *d++ = *s++;
        n--;
    }

    uint64_t *d64 = (uint64_t *)d;
    const uint64_t *s64 = (const uint64_t *)s;

    while (n >= 8) {
        *d64++ = *s64++;
        n -= 8;
    }

    d = (uint8_t *)d64;
    s = (const uint8_t *)s64;

    while (n--) {
        *d++ = *s++;
    }

    return dst;
}

static inline void *h_memmove(void *dst, const void *src, uint32_t n) {
    if (!n || dst == src) return dst;

    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    if (d < s || d >= s + n) {
        while (((uintptr_t)d & 7) && n) {
            *d++ = *s++;
            n--;
        }

        uint64_t *d64 = (uint64_t *)d;
        const uint64_t *s64 = (const uint64_t *)s;

        while (n >= 8) {
            *d64++ = *s64++;
            n -= 8;
        }

        d = (uint8_t *)d64;
        s = (const uint8_t *)s64;

        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;

        while (((uintptr_t)d & 7) && n) {
            *--d = *--s;
            n--;
        }

        uint64_t *d64 = (uint64_t *)d;
        const uint64_t *s64 = (const uint64_t *)s;

        while (n >= 8) {
            *--d64 = *--s64;
            n -= 8;
        }

        d = (uint8_t *)d64;
        s = (const uint8_t *)s64;

        while (n--) {
            *--d = *--s;
        }
    }

    return dst;
}


static inline void swap_buffer(struct terminal *term) {
    uint8_t *src = term->fb_buf;
    uint8_t *dst = term->fb_addr;

    for (uint32_t y = 0; y < term->fb_height; y++) {
        h_memcpy(dst, src, term->fb_width * term->fb_bpp);
        src += term->fb_width * term->fb_bpp;
        dst += term->fb_pitch;
    }
}

void cuoreterm_init(
    struct terminal *term,
    void *fb_addr,
    uint32_t fb_pitch,
    uint32_t fb_width,
    uint32_t fb_height,
    uint32_t fb_bpp_bits,
    uint8_t r_shift, uint8_t g_shift, uint8_t b_shift,
    const uint8_t *font,
    uint32_t font_w,
    uint32_t font_h
    #ifndef CUORETERM_DATASEG_BACKBUFFER
       , void *(*malloc_fn)(size_t)
       , void (*free_fn)(void *)
    #endif

    #ifdef CUORETERM_DEBUG
       , void (*debug_write)(const char *msg, size_t len)
    #endif
) 
{
    term->fb_addr = fb_addr;

    #ifdef CUORETERM_DATASEG_BACKBUFFER
        term->fb_width  = 1280;
        term->fb_height = 720;
        term->fb_bpp    = 4;
        term->fb_buf    = c_backbuffer;
    #else
        term->fb_bpp = (uint8_t)(fb_bpp_bits >> 3);
        term->fb_width  = fb_width;
        term->fb_height = fb_height;
        term->fb_buf = malloc_fn(fb_width * fb_height * term->fb_bpp);
    #endif
    
    cuoreterm_set_font(term, font, font_w, font_h); // sets term->cols, term->rows, term->font_*    

    if (!term->fb_buf) { // if malloc failed (returns NULL)
        #ifdef CUORETERM_DEBUG
        debug_write("malloc returned NULL\n", 21);
        #endif
        return;
    }

    term->r_offset = r_shift >> 3;
    term->g_offset = g_shift >> 3;
    term->b_offset = b_shift >> 3;

    term->a_offset = 0;
    if (term->fb_bpp == 4)
        term->a_offset = 3;

    term->fb_pitch = fb_pitch;
    term->fb_buf_pitch = term->fb_width * term->fb_bpp;
    term->fgcol = 0xFFFFFF;
    term->cursor_x = term->cursor_y = 0;

    cuoreterm_clear(term);
}

static void term_scroll(struct terminal *term) {
    uint32_t row_bytes = term->font_height * term->fb_buf_pitch;
    uint8_t *fb = term->fb_buf;

    h_memmove(fb, fb + row_bytes,
              (term->fb_height * term->fb_pitch) - row_bytes);

    h_memset(fb + (term->fb_height * term->fb_pitch) - row_bytes,
             0, row_bytes);

    if (term->cursor_y) term->cursor_y--;
}

void cuoreterm_draw_char(struct terminal *term, char c, uint32_t fg) {
    if (c == '\n') {
        term->cursor_x = 0;
        if (++term->cursor_y >= term->rows) term_scroll(term);
        return;
    }

    uint32_t px = term->cursor_x * term->font_width;
    uint32_t py = term->cursor_y * term->font_height;
    const uint8_t *glyph = term->font_data + 4 + ((uint8_t)c * term->font_height);

    uint16_t fg16 = (((fg >> 19) & 0x1F) << 11) |
                    (((fg >> 10) & 0x3F) << 5) |
                    ((fg >> 3) & 0x1F);

    uint8_t fg_gray = (uint8_t)(
        (((fg >> 16) & 0xFF) +
        ((fg >> 8) & 0xFF) +
        (fg & 0xFF)) / 3
    );


    for (uint32_t r = 0; r < term->font_height; r++) {
        uint8_t bits = glyph[r];
        uint8_t *row = term->fb_buf + (py + r) * term->fb_buf_pitch + px * term->fb_bpp;

        for (int col = 0; col < 8; col++) {
            if (!(bits & (1 << (7 - col)))) continue;

            uint8_t *pixel = row + col * term->fb_bpp;

            switch (term->fb_bpp) {
                case 4:
                case 3:
                    pixel[term->r_offset] = (fg >> 16) & 0xFF;
                    pixel[term->g_offset] = (fg >> 8) & 0xFF;
                    pixel[term->b_offset] = fg & 0xFF;
                    break;
                case 2:
                    *(uint16_t*)pixel = fg16;
                    break;
                case 1:
                    *pixel = fg_gray;
                    break;
            }
        }
    }

    if (++term->cursor_x >= term->cols) {
        term->cursor_x = 0;
        if (++term->cursor_y >= term->rows) term_scroll(term);
    }
}

static uint8_t hex_lut[256] = {
    ['0']=0, ['1']=1, ['2']=2, ['3']=3, ['4']=4,
    ['5']=5, ['6']=6, ['7']=7, ['8']=8, ['9']=9,
    ['a']=10,['b']=11,['c']=12,['d']=13,['e']=14,['f']=15,
    ['A']=10,['B']=11,['C']=12,['D']=13,['E']=14,['F']=15,
};

static inline uint32_t hex_digit(char c) {
    return hex_lut[(uint8_t)c];
}

static inline uint32_t parse_hex_color(const char *s) {
    return (hex_lut[(uint8_t)s[0]] << 20) |
           (hex_lut[(uint8_t)s[1]] << 16) |
           (hex_lut[(uint8_t)s[2]] << 12) |
           (hex_lut[(uint8_t)s[3]] << 8)  |
           (hex_lut[(uint8_t)s[4]] << 4)  |
            hex_lut[(uint8_t)s[5]];
}

static inline void handle_hex_ansi(struct terminal *term, char **p_c) {
    char *c = *p_c;
    if (*c++ != '[') return;

    if (c[0] == '0' && c[1] == 'm') {
        term->fgcol = 0xFFFFFF;
        *p_c = c + 2;
        return;
    }

    if (*c == '#') {
        term->fgcol = parse_hex_color(c + 1);
        c += 7;
    }

    if (*c == 'm') c++;
    *p_c = c;
}

void cuoreterm_write(void *ctx, const char *msg, uint64_t len) {
    struct terminal *term = (struct terminal *)ctx;
    char *p_c = (char *)msg;
    char *end = p_c + len;

    while (p_c < end) {
        char c = *p_c++;

        if (c == '\n') {
            term->cursor_x = 0;
            term->cursor_y++;
            if (term->cursor_y >= term->rows) term_scroll(term);
        }
        else if (c == '\b') {
            if (term->cursor_x == 0) {
                if (term->cursor_y == 0) continue;
                term->cursor_y--;
                term->cursor_x = term->cols - 1;
            } else {
                term->cursor_x--;
            }

            uint32_t px = term->cursor_x * term->font_width;
            uint32_t py = term->cursor_y * term->font_height;

            uint8_t *start = (uint8_t *)term->fb_buf + py * term->fb_pitch + px * term->fb_bpp;
            for (uint32_t r = 0; r < term->font_height; r++) {
                h_memset(start + r * term->fb_pitch, 0x00, term->font_width * term->fb_bpp);
            }
        }
        else if (c == '\x1b') {
            if (p_c < end && *p_c == '[')
                handle_hex_ansi(term, &p_c);
        }
        else {
            cuoreterm_draw_char(term, c, term->fgcol);
        }
    }

    swap_buffer(term);
}

void cuoreterm_set_font(struct terminal *term,
                        const uint8_t *font,
                        uint32_t font_w,
                        uint32_t font_h) {
    term->font_data = font;
    term->font_width = font_w;
    term->font_height = font_h;
    term->cols = term->fb_width / font_w;
    term->rows = term->fb_height / font_h;
}

void cuoreterm_clear(struct terminal *term) {
    h_memset(term->fb_buf, 0, term->fb_buf_pitch * term->fb_height);
    term->cursor_x = term->cursor_y = 0;
    swap_buffer(term);
}

#endif
#endif