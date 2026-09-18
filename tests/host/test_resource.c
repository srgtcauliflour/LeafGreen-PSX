#include <assert.h>
#include <string.h>
#include "lg/resource.h"
int main(void){
    LGResourceEntry e[]={{10,LG_RES_MAP,100,20},{20,LG_RES_TEXTURE,200,30}};
    LGResourceIndex i={e,2};
    assert(lg_resource_find(&i,20)==&e[1]);
    assert(!lg_resource_find(&i,99));

    uint8_t buffer[256];
    for (size_t n = 0; n < sizeof buffer; ++n) buffer[n] = (uint8_t)n;
    size_t size = 0;
    const uint8_t *bytes = lg_resource_bytes(&i, 10, LG_RES_MAP, buffer, sizeof buffer, &size);
    assert(bytes == buffer + 100 && size == 20);
    assert(memcmp(bytes, buffer + 100, 20) == 0);

    /* Wrong kind, missing id, and invalid arguments are all rejected. */
    assert(!lg_resource_bytes(&i, 10, LG_RES_TEXTURE, buffer, sizeof buffer, &size));
    assert(!lg_resource_bytes(&i, 99, LG_RES_MAP, buffer, sizeof buffer, &size));
    assert(!lg_resource_bytes(&i, 10, LG_RES_MAP, 0, sizeof buffer, &size));
    assert(!lg_resource_bytes(&i, 10, LG_RES_MAP, buffer, sizeof buffer, 0));

    /* An entry that fits exactly to the last byte succeeds; one byte over
       the buffer, or an offset alone past the buffer, is rejected -- an
       overflowing or malformed entry is never trusted, not even
       partially. */
    LGResourceEntry exact_fit[] = {{1, LG_RES_SCRIPT, 236, 20}}; /* 236+20 == 256 */
    LGResourceIndex i_exact = {exact_fit, 1};
    assert(lg_resource_bytes(&i_exact, 1, LG_RES_SCRIPT, buffer, sizeof buffer, &size));
    assert(size == 20);

    LGResourceEntry too_big[] = {{1, LG_RES_SCRIPT, 237, 20}}; /* 237+20 == 257 */
    LGResourceIndex i_too_big = {too_big, 1};
    assert(!lg_resource_bytes(&i_too_big, 1, LG_RES_SCRIPT, buffer, sizeof buffer, &size));

    LGResourceEntry past_end[] = {{1, LG_RES_SCRIPT, 300, 1}};
    LGResourceIndex i_past_end = {past_end, 1};
    assert(!lg_resource_bytes(&i_past_end, 1, LG_RES_SCRIPT, buffer, sizeof buffer, &size));

    return 0;
}
