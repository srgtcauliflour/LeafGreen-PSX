#include <assert.h>
#include "lg/resource.h"
int main(void){LGResourceEntry e[]={{10,LG_RES_MAP,100,20},{20,LG_RES_TEXTURE,200,30}};LGResourceIndex i={e,2};assert(lg_resource_find(&i,20)==&e[1]);assert(!lg_resource_find(&i,99));return 0;}
