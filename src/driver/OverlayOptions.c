#define _MSL_SIZE_T_TYPE unsigned int
#include "driver/OverlayOptions.h"
#include "driver/ParserFace.h"
#include <string.h>
#include "driver/Projects.h"

char overlay_group_name_5DD8D4[24] = "(unnamed overlay group)";
char overlay_name_5DD8EC[20] = "(unnamed overlay)";
char overlay_name_buffer_634C58[24];

int fn_00412168(void) {
    if (!overlay_name_buffer_634C58[0]) {
        memcpy(overlay_name_buffer_634C58, overlay_group_name_5DD8D4, 24);
    }
    AddOverlayGroup(
        overlay_name_buffer_634C58,
        &overlay_group_address_625CC8,
        &parseopts.currentOverlayGroup,
        &parseopts.currentOverlay
    );
    overlay_name_buffer_634C58[0] = 0;
    return TRUE;
}

int fn_004121AC(void) {
    if (!overlay_name_buffer_634C58[0]) {
        memcpy(overlay_name_buffer_634C58, overlay_name_5DD8EC, 18);
    }
    AddOverlay(
        parseopts.currentOverlayGroup, overlay_name_buffer_634C58, &parseopts.currentOverlay
    );
    overlay_name_buffer_634C58[0] = 0;
    return TRUE;
}
