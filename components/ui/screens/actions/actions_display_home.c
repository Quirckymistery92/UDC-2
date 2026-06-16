#include "actions_display_home.h"
#include "actions_control.h"

void ui_int__act_display_home_tick(void) {
    ui_pub_actions_backg_update("bg_thunder_rail.jpg", objects.ds_home);
}