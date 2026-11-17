#include "msl/runinit.h"
#pragma use_frame on
void RunInit(struct runinit_table* table) {
    init_function* current;
    init_function function;

    if (!table) {
        return;
    }
    current = &table->functions[0];
    function = *current;
    while (function) {
        function();
        current++;
        function = *current;
    }
}
#pragma use_frame off
