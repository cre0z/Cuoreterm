# CuoreTerm
a very simple terminal for amd64 limine

## Examples

It's recommended to provide CuoreTerm with heap allocation functions with the following macros:

```c
#define CUORETERM_MALLOC(bytes) kmalloc(bytes)
#define CUORETERM_FREE(addr) kfree(addr)
```

If you do not provide these macros however, CuoreTerm will allocate a backbuffer on the stack, assuming a resolution of 1280x720.

```c
#define CUORETERM_MALLOC(bytes) kmalloc(bytes) // OPTIONAL
#define CUORETERM_FREE(addr) kfree(addr) // OPTIONAL
#define CUORETERM_IMPL // enable cuoreterm source code instead of just includes (in other source files do not do this, so should only do this once preferably in your entry)
#include "Cuoreterm.h"
#include "kfont.h" // font we provide you (iso10_f14_psf)

#include "limine.h" // from https://codeberg.org/Limine/limine-protocol/ (is not included here)

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

struct terminal fb_term;

void _start(void) {
    struct limine_framebuffer *fb = fb_req.response->framebuffers[0];

    cuoreterm_init(
         &fb_term,
         (void *)fb->address,
         (uint32_t)fb->width,
         (uint32_t)fb->height,
         (uint32_t)fb->bpp,
         iso10_f14_psf, // font we provide for you in kfont.h but can be any psf1 font
         8, // font width
         14 // font height
    );

    // optionally clear the screen
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