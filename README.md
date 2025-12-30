# CuoreTerm
a very simple terminal for amd64 limine

## Example (AMD64/Limine)
```c
#include "cuoreterm.h"
#include "limine.h" // from https://codeberg.org/Limine/limine-protocol/

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
         (uint32_t)fb->pitch,
         (uint32_t)fb->bpp
    );

    // print hello world
    char msg[] = "hello world :3";
    cuoreterm_write(&fb_term, msg, sizeof(msg));

    for (;;)
        __asm__("hlt");
}
```

### All code in this repo is licensed under the Mozilla Public License Version 2.0