#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "queue.h"
#include "parts.h"
#include "header.h"


BeybladeConfig configbeyblade() {
    keypad_init_pins();
	keypad_init_timer();
    
	int state = 0; // 0=start 1=blade 2=disc 3=tip 4=confirm
    int firstsel = 0;
	int sel_blade = -1;
	int sel_disc = -1;
	int sel_tip = -1;
    BeybladeConfig player_bey = { sel_blade, sel_disc, sel_tip };
	int saved = 0;

	printf(MENU_START);
    
    // for (;;) {
    //     printf(MENU_START);
    //     sleep_ms(1000);
    // }

	for (;;) {
		uint16_t keyevent = key_pop();

		// ignore empty queue and release events
		if (!(keyevent & 0x100)) continue;
		char key = (char)(keyevent & 0xFF);

		switch (state) {

			// 0: starting screen
			case 0:
				printf(MENU_BLADE);
				state = 1;
				break;

            // 1: pick a blade
			case 1:
				if (key >= '1' && key <= '5') {
					sel_blade = key - '1';
				} else if (key == '#' && sel_blade >= 0) {
					printf(MENU_DISC);
					state = 2;
				}
				break;

            // 2: pick a disc / ratchet
			case 2:
				if (key >= '1' && key <= '5') {
					sel_disc = key - '1';
				} else if (key == '#' && sel_disc >= 0) {
					printf(MENU_TIP);
					state = 3;
				}
				break;

            // 3: pick a tip
			case 3:
				if (key >= '1' && key <= '5') {
					sel_tip = key - '1';
				} else if (key == '#' && sel_tip >= 0) {
					printf(MENU_CONFIRM);
					state = 4;
                    firstsel = 1;
				}
				break;

            // 4: confirm / change / save
			case 4:
				if (key == '#') {
					int total = blades[sel_blade].cost
						+ discs[sel_disc].cost
						+ tips[sel_tip].cost;

					if (total <= COST_CAP) {
						printf("Saved beyblade: %s + %s + %s (cost %d).\n",
								blades[sel_blade].name,
								discs[sel_disc].name,
								tips[sel_tip].name,
								total);

						player_bey.blade = sel_blade;
						player_bey.disc = sel_disc;
						player_bey.tip = sel_tip;
						saved = 1;
						break; // leave switch
					} else {
                    // cost cap exceeded
						printf("Error: cost %d > %d. "
								"Confirm: # to confirm, 1=change blade, "
								"2=change disc, 3=change tip.\n",
								total, COST_CAP);
					}
				} else if (key == '1') {
					printf(MENU_BLADE);
					state = 1;
				} else if (key == '2') {
					printf(MENU_DISC);
					state = 2;
				} else if (key == '3') {
					printf(MENU_TIP);
					state = 3;
				}
				break;
		}

		if (saved) return player_bey ; // leave selection loop once saved
	}
}