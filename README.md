# CuoreTerm
a very simple terminal for amd64 limine

## Example

```c
#define CUORETERM_IMPL // enable cuoreterm source code instead of just includes (in other source files do not do this
// #define CUORETERM_DATASEG_BACKBUFFER // use a backbuffer from .bss (useful if you do not want to provide your own malloc,free) (unrecommended)

#include "Cuoreterm.h"
#include "kfont.h" // font we provide you (iso10_f14_psf)

#include "limine.h" // from https://codeberg.org/Limine/limine-protocol/ (is not included here)

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

struct terminal fb_term;

// you will need to provide both of these functions (only if you arent using CUORETERM_DATASEG_BACKBUFFER)
// should be a heap allocator (also make sure there is enough space on the heap for the backbuffer)
void* malloc(size_t size);
void free(void* user_ptr);

void _start(void) {
    struct limine_framebuffer *fb = fb_req.response->framebuffers[0];

    cuoreterm_init(
         &fb_term,
         (void *)fb->address,
         (uint32_t) fb->pitch,
         (uint32_t)fb->width,
         (uint32_t)fb->height,
         (uint32_t)fb->bpp,
         (uint8_t) fb->red_mask_shift, (uint8_t) fb->green_mask_shift, (uint8_t) fb->blue_mask_shift,
         iso10_f14_psf, // font we provide for you in kfont.h but can be any psf1 font
         8, // font width
         14 // font height
         #ifdef CUORETERM_USE_DYNBUF
             ,
             &malloc,
             &free
         #endif
    );

    // clear the screen (not required after init, just used here for demonstration purposes)
    cuoreterm_clear(&fb_term);

    // print hello world in green
    char msg[] = "\x1b[#00FF00mhello world :3\x1b[0m";
    cuoreterm_write(&fb_term, msg, sizeof(msg) - 1);

    // optionally change font (PSF1 font)
    // cuoreterm_set_font(&fb_term, your_own_cute_font, font_width, font_height);
    // cuoreterm_write(&fb_term, "new font :3", 11);

    for (;;)
        __asm__("hlt");
}
```

### All code in this repo is licensed under the Mozilla Public License Version 2.0