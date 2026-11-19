#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "queue.h"
#include "parts.h"
#include "header.h"
#include "lcd.h"

// we dont need to check if its a valid index thats dealt with in input
// File scope, so the pointers stay valid after the function returns.
// 6 buffers for the 6 stats (ATK, DEF, STA, BUR, SS, Cost).
static char statbuf[7][24];
static const char* labels[8];
static int state; // 0=start 1=blade menu 2=disc menu 3=tip menu 4=summary 5=blade stats 6=disc stats 7=tip stats

void keypad_init_pins();
void keypad_init_timer();

static void draw_centered(int cell_x, int cell_y, int cell_w, int cell_h, uint16_t fg, uint16_t bg, const char *s) {
	int tw = (int)strlen(s) * 8;   // 8px per char at size 16
	int x = cell_x + (cell_w - tw) / 2;
	int y = cell_y;
	if (x < cell_x) x = cell_x;    // clamp if text is wider than cell
	if (y < cell_y) y = cell_y;
	LCD_DrawString(x, y, fg, bg, (char *)s, 16, 1);
}

static void vert_divider(int x) {
    for (int i = 70; i < 300; i = i + 20)
    {
        LCD_DrawChar(x, i, WHITE, BLACK, '|', 16, 1);
    }
}

static void horiz_divider(int y) {
    LCD_DrawString(0, y, WHITE, BLACK, "------------------------------", 16, 1);
}

static void clear_menu() {
	LCD_Clear(BLACK);
    vert_divider(120);
    
	for (int i = 60; i < 300; i = i + 60)
	{
        horiz_divider(i);
	}
}

static void top_text(const char* line1, const char* line2, const char* line3) {
    LCD_DrawString(5,10, CYAN, BLACK, line1, 16, 1);
    LCD_DrawString(5,25, CYAN, BLACK, line2, 16, 1);
    LCD_DrawString(5,40, CYAN, BLACK, line3, 16, 1);
}

static void print_into_boxes_generic(const char *labels[]) {
    clear_menu();
    const char* firstline = "";
    int count = 0;;
    switch (state) {
        case 1: count = BLADE_COUNT;
        case 5: firstline = " Pick a blade!"; break;
        case 2: count = DISC_COUNT;
        case 6: firstline = " Pick a disc!"; break;
        case 3: count = TIP_COUNT;
        case 7: firstline = " Pick a tip!"; break;
    }
    if (strcmp(firstline, "")) {
        top_text(firstline, " 1-5 to select", " * to confirm");
    }
    // Which list is currently being shown, and how many items it has.

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = 2 * i + j;
            if (!labels[idx]) continue;

            // Draw the item name.
            char buf[32];
            snprintf(buf, sizeof(buf), "%s", labels[idx]);
            draw_centered(125 * j, 90 + 60 * i,
                          120, 60, CYAN, BLACK, buf);

            // Draw the 1-based selection number next to valid slots only.
            if (count > 0 && idx < count) {
                char num[2] = { (char)('1' + idx), '\0' };
                draw_centered(125 * j, 73 + 60 * i,
                              20, 20, CYAN, BLACK, num);
            }
        }
    }
}


static void print_stats(int i) {
    const Part *p = NULL;

    switch (state) {
        case 1: case 5: p = blades; break;
        case 2: case 6: p = discs;  break;
        case 3: case 7: p = tips;   break;
        default: return;
    }

    labels[0] = p[i].name;
    
    snprintf(statbuf[6], sizeof(statbuf[6]),  "#%u",  (unsigned)(i + 1));
    snprintf(statbuf[0], sizeof(statbuf[0]), "ATK %u",  (unsigned)p[i].atk);
    snprintf(statbuf[1], sizeof(statbuf[1]), "DEF %u",  (unsigned)p[i].def);
    snprintf(statbuf[2], sizeof(statbuf[2]), "STA %u",  (unsigned)p[i].sta);
    snprintf(statbuf[3], sizeof(statbuf[3]), "BUR %u",  (unsigned)p[i].bur);
    snprintf(statbuf[4], sizeof(statbuf[4]), "SS %u",   (unsigned)p[i].ss);
    snprintf(statbuf[5], sizeof(statbuf[5]), "Cost %u", (unsigned)p[i].cost);
    
    labels[1] = statbuf[6];
    labels[2] = statbuf[0];
    labels[3] = statbuf[1];
    labels[4] = statbuf[2];
    labels[5] = statbuf[3];
    labels[6] = statbuf[4];
    labels[7] = statbuf[5];

    print_into_boxes_generic(labels);
}

static void abbr(const char *src, char out[4]) {
    int n = 0;
    while (n < 3 && src[n] != '\0') {
        out[n] = (char)toupper((unsigned char)src[n]);
        n++;
    }
    out[n] = '\0';
}

static void print_build_summary(int b, int d, int t) {
    int cb = (int)blades[b].cost;
    int cd = (int)discs[d].cost;
    int ct = (int)tips[t].cost;
    int total = cb + cd + ct;
    char names[5][5];

    printf("Build:\n %s [%d]\n", blades[b].name, cb);
    printf(" %s [%d]\n", discs[d].name, cd);
    printf(" %s [%d]\n", tips[t].name, ct);
    printf("ATK %d DEF %d STA %d BUR %d SS %d\n",
            (int)(blades[b].atk + discs[d].atk + tips[t].atk),
            (int)(blades[b].def + discs[d].def + tips[t].def),
            (int)(blades[b].sta + discs[d].sta + tips[t].sta),
            (int)(blades[b].bur + discs[d].bur + tips[t].bur),
            (int)(blades[b].ss  + discs[d].ss  + tips[t].ss));
    printf("Total cost %d/%d\n", total, COST_CAP);
    if (total > COST_CAP) printf("Cost cap exceeded.\n");
    printf("# save, 1 change blade, 2 change disc, 3 change tip\n");

    LCD_Clear(BLACK);
    top_text(total > COST_CAP ? "Status: REDUCE COST" : "Status: VALID",
             "1/2/3 to swap Blade/Disc/Tip",
             total > COST_CAP ? "Cannot confirm: INVALID build" : "# to lock in");
    horiz_divider(100);

    // Header row
    abbr(blades[b].name, names[1]);
    abbr(discs[d].name, names[2]);
    abbr(tips[t].name, names[3]);
    strcpy(names[4], "TOT");
    strcpy(names[0], "STAT");

    for (int i = 44; i < 236; i = i + 48) vert_divider(i);
    for (int i = 0; i < 5; i++) {
        draw_centered(48 * i, 85, 48, 20, CYAN, BLACK, names[i]);
    }

    // Stat rows: ATK, DEF, STA, BUR, SS, Cost
    const char *stat_names[6] = {"ATK", "DEF", "STA", "BUR", "SS", "Cost"};
    char stat_cells[6][5][12];   // 6 rows, 5 columns, max 12 chars per cell

    for (int row = 0; row < 6; row++) {
        // Column 0: stat name
        snprintf(stat_cells[row][0], sizeof(stat_cells[row][0]), "%s", stat_names[row]);

        // Column 1: blade value
        int blade_val = (row == 0) ? blades[b].atk :
                        (row == 1) ? blades[b].def :
                        (row == 2) ? blades[b].sta :
                        (row == 3) ? blades[b].bur :
                        (row == 4) ? blades[b].ss  :
                                     blades[b].cost;
        snprintf(stat_cells[row][1], sizeof(stat_cells[row][1]), "%d", blade_val);

        // Column 2: disc value
        int disc_val = (row == 0) ? discs[d].atk :
                       (row == 1) ? discs[d].def :
                       (row == 2) ? discs[d].sta :
                       (row == 3) ? discs[d].bur :
                       (row == 4) ? discs[d].ss  :
                                    discs[d].cost;
        snprintf(stat_cells[row][2], sizeof(stat_cells[row][2]), "%d", disc_val);

        // Column 3: tip value
        int tip_val = (row == 0) ? tips[t].atk :
                      (row == 1) ? tips[t].def :
                      (row == 2) ? tips[t].sta :
                      (row == 3) ? tips[t].bur :
                      (row == 4) ? tips[t].ss  :
                                   tips[t].cost;
        snprintf(stat_cells[row][3], sizeof(stat_cells[row][3]), "%d", tip_val);

        // Column 4: total
        int total_val = blade_val + disc_val + tip_val;
        snprintf(stat_cells[row][4], sizeof(stat_cells[row][4]), "%d", total_val);
    }

    // Draw the stat rows
    for (int row = 0; row < 6; row++) {
        for (int col = 0; col < 5; col++) {
            draw_centered(48 * col, 115 + 30 * row, 48, 20, CYAN, BLACK, stat_cells[row][col]);
        }
    }
}
BeybladeConfig configbeyblade() {
	keypad_init_pins();
	keypad_init_timer();
    state = 0;

	int sel_blade = -1;
	int sel_disc = -1;
	int sel_tip = -1;
	int editing = 0; // 1=blade 2=disc 3=tip
	BeybladeConfig player_bey = { sel_blade, sel_disc, sel_tip };
	int saved = 0;

	// LCD_DrawString(90,150, CYAN, BLACK, MENU_START, 16, 1);
	LCD_DrawString(10,142, CYAN, BLACK, "BEYBLADE BUILDER", 16, 1);
	LCD_DrawString(10,158, CYAN, BLACK, "press any key to start", 16, 1);
	printf("%s", MENU_START);

	for (;;) {
		uint16_t keyevent = key_pop();

		if (!(keyevent & 0x100)) continue;
		char key = (char)(keyevent & 0xFF);

		switch (state) {
			case 0:
				printf("%s", MENU_BLADE);
				for (int k = 0; k < 8; k++) labels[k] = k < BLADE_COUNT ? blades[k].name : NULL;
				state = 1;
                clear_menu();
				print_into_boxes_generic(labels);
				break;

			case 1: // blade menu
				if (key >= '1' && key <= (char)(BLADE_COUNT + '0')) {
					sel_blade = key - '1';
					state = 5;
					print_stats(sel_blade);
				} else if (key == '*' && editing == 1) {
					editing = 0;
					state = 4;
					print_build_summary(sel_blade, sel_disc, sel_tip);
				}
				break;

			case 5: // blade stats
				if (key >= '1' && key <= (char)(BLADE_COUNT + '0')) {
					sel_blade = key - '1';
					print_stats(sel_blade);
				} else if (key == '#') {
					if (editing == 1) {
						editing = 0;
						state = 4;
						print_build_summary(sel_blade, sel_disc, sel_tip);
					} else {
						// LCD_DrawString(90,150, CYAN, BLACK, MENU_DISC, 16, 1);
						printf("%s", MENU_DISC);
						for (int k = 0; k < 8; k++) labels[k] = k < DISC_COUNT ? discs[k].name : NULL;
						state = 2;
						print_into_boxes_generic(labels);
					}
				} else if (key == '*') {
					if (editing == 1) {
						editing = 0;
						state = 4;
						print_build_summary(sel_blade, sel_disc, sel_tip);
					} else {
						// LCD_DrawString(90,150, CYAN, BLACK, MENU_BLADE, 16, 1);
						printf("%s", MENU_BLADE);
						for (int k = 0; k < 8; k++) labels[k] = k < BLADE_COUNT ? blades[k].name : NULL;
						state = 1;
						print_into_boxes_generic(labels);
					}
				}
				break;

			case 2: // disc menu
				if (key >= '1' && key <= (char)(DISC_COUNT + '0')) {
					sel_disc = key - '1';
					state = 6;
					print_stats(sel_disc);
				} else if (key == '*' && editing == 2) {
					editing = 0;
					state = 4;
					print_build_summary(sel_blade, sel_disc, sel_tip);
				}
				break;

			case 6: // disc stats
				if (key >= '1' && key <= (char)(DISC_COUNT + '0')) {
					sel_disc = key - '1';
					print_stats(sel_disc);
				} else if (key == '#') {
					if (editing == 2) {
						editing = 0;
						state = 4;
						print_build_summary(sel_blade, sel_disc, sel_tip);
					} else {
						// LCD_DrawString(90,150, CYAN, BLACK, MENU_TIP, 16, 1);
						printf("%s", MENU_TIP);
						for (int k = 0; k < 8; k++) labels[k] = k < TIP_COUNT ? tips[k].name : NULL;
						state = 3;
						print_into_boxes_generic(labels);
					}
				} else if (key == '*') {
					if (editing == 2) {
						editing = 0;
						state = 4;
						print_build_summary(sel_blade, sel_disc, sel_tip);
					} else {
						// LCD_DrawString(90,150, CYAN, BLACK, MENU_DISC, 16, 1);
						printf("%s", MENU_DISC);
						for (int k = 0; k < 8; k++) labels[k] = k < DISC_COUNT ? discs[k].name : NULL;
						state = 2;
						print_into_boxes_generic(labels);
					}
				}
				break;

			case 3: // tip menu
				if (key >= '1' && key <= (char)(TIP_COUNT + '0')) {
					sel_tip = key - '1';
					state = 7;
					print_stats(sel_tip);
				} else if (key == '*' && editing == 3) {
					editing = 0;
					state = 4;
					print_build_summary(sel_blade, sel_disc, sel_tip);
				}
				break;

			case 7: // tip stats
				if (key >= '1' && key <= (char)(TIP_COUNT + '0')) {
					sel_tip = key - '1';
					print_stats(sel_tip);
				} else if (key == '#') {
					editing = 0;
					state = 4;
					print_build_summary(sel_blade, sel_disc, sel_tip);
				} else if (key == '*') {
					if (editing == 3) {
						editing = 0;
						state = 4;
						print_build_summary(sel_blade, sel_disc, sel_tip);
					} else {
						// LCD_DrawString(90,150, CYAN, BLACK, MENU_TIP, 16, 1);
						printf("%s", MENU_TIP);
						for (int k = 0; k < 8; k++) labels[k] = k < TIP_COUNT ? tips[k].name : NULL;
						state = 3;
						print_into_boxes_generic(labels);
					}
				}
				break;

			case 4: // summary
				if (key == '#') {
					if (sel_blade < 0 || sel_disc < 0 || sel_tip < 0) { // IT IS IMPOSSIBLE TO GET HERE!! no need to make graphics. 
						printf("Incomplete build.\n");
						break;
					}

					int total = (int)blades[sel_blade].cost
						+ (int)discs[sel_disc].cost
						+ (int)tips[sel_tip].cost;

					if (total <= COST_CAP) {
						// LCD_DrawString(90,150, CYAN, BLACK, ("Saved beyblade: %s + %s + %s (cost %d).\n",
						//      blades[sel_blade].name,
						//      discs[sel_disc].name,
						//      tips[sel_tip].name,
						//      total), 16, 1);
						printf("Saved beyblade: %s + %s + %s (cost %d).\n",
								blades[sel_blade].name,
								discs[sel_disc].name,
								tips[sel_tip].name,
								total);
						player_bey.blade = sel_blade;
						player_bey.disc = sel_disc;
						player_bey.tip = sel_tip;
						saved = 1;
                        LCD_DrawString(20, 60, RED, BLACK, "GET READY!", 16, 0);
						break;
					} else {
						// LCD_DrawString(90,150, CYAN, BLACK, ("Error: cost %d > %d. "
						//      "Confirm: # to confirm, 1=change blade, "
						//      "2=change disc, 3=change tip.\n",
						//      total, COST_CAP), 16, 1);
						printf("Error: cost %d > %d. "
								"Confirm: # to confirm, 1=change blade, "
								"2=change disc, 3=change tip.\n",
								total, COST_CAP);
					}
				} else if (key == '1') {
					editing = 1;
					// LCD_DrawString(90,150, CYAN, BLACK, MENU_BLADE, 16, 1);
					printf("%s", MENU_BLADE);
					for (int k = 0; k < 8; k++) labels[k] = k < BLADE_COUNT ? blades[k].name : NULL;
					state = 1;
					print_into_boxes_generic(labels);
				} else if (key == '2') {
					editing = 2;
					// LCD_DrawString(90,150, CYAN, BLACK, MENU_DISC, 16, 1);
					printf("%s", MENU_DISC);
					for (int k = 0; k < 8; k++) labels[k] = k < DISC_COUNT ? discs[k].name : NULL;
					state = 2;
					print_into_boxes_generic(labels);
				} else if (key == '3') {
					editing = 3;
					// LCD_DrawString(90,150, CYAN, BLACK, MENU_TIP, 16, 1);
					printf("%s", MENU_TIP);
					for (int k = 0; k < 8; k++) labels[k] = k < TIP_COUNT ? tips[k].name : NULL;
					state = 3;
					print_into_boxes_generic(labels);
				}
				break;
		}

		if (saved) {
            return player_bey;
        }
	}
}
