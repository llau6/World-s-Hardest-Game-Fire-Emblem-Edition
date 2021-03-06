#include "myLib.h"
#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include "splash.h"
#include "game.h"
#include "end.h"

#define SCREENHEIGHT 135
#define SCREENMIN 20
#define NUMOBJS 12

typedef struct {
    int row;
    int col;
    int rd;
    int cd;
    int size;
    u8 color;
} MOVOBJ;
  
enum {REDIDX, GREENIDX, BLUEIDX, MAGENTAIDX, CYANIDX, YELLOWIDX, WHITEIDX, BGIDX};
void delay(int n);
void checkCollisions(MOVOBJ *p, int pSize, MOVOBJ *e, int eSize, int *score, int *lives, int *died);

int main() {
	REG_DISPCNT = BG2_EN | MODE_4;
// Declaire moving objects and pointers
    MOVOBJ objs[NUMOBJS];
    MOVOBJ *cur;
    MOVOBJ player;
	MOVOBJ *pPlayer = &player;
// Declaire enemy sizes, velocities, and colors
	int sizes[] = {4, 8, 12, 16, 20, 24};
	int numsizes = sizeof(sizes)/sizeof(sizes[0]);
    int dels[] = {-3, -2, -1, 1, 2, 3};
    int numdels = sizeof(dels)/sizeof(dels[0]);
    u16 colors[] = {RED, ORANGE, YELLOW, GREEN, CYAN, PURPLE, WHITE};
    int numcolors = sizeof(colors)/sizeof(colors[0]);
	const u16 *hearts[] = {hearts0, hearts1, hearts2, hearts3, hearts4, hearts5}; 
//    volatile u16 bgcol4 = BGIDX << 8 | BGIDX;
// Declare States
	enum GBAState {
		START,
		OBJ,
		NEXT,
		RESPAWN,
		GAME,
		LOSE,
		WIN,
		OVER,
		CONT
	};
	enum GBAState state = START;
// Declare pointers and such
	int died;
	int *pDied = &died;
    int score;
	int *pScore = &score;
    int lives;
	int *pLives = &lives;       
	int isFinished = 0;
	int release = 0;
	int *pFinished = &isFinished;
    char buffer[41];   
// Game loop
	while(1) {
		if (KEY_DOWN_NOW(BUTTON_SELECT)) {
			state = START;
	    }  
		switch (state) {
	// Draw splash screen
		case START:
			if (!KEY_DOWN_NOW(BUTTON_START)) {
				release = 1;
			}
			if (release == 1) {
				DMA[3].src = splash_palette;
				DMA[3].dst = PALETTE;
				DMA[3].cnt = 256 | DMA_ON;
				drawImage4 (0, 0, 240, 160, splash);
				FlipPage();
				if (KEY_DOWN_NOW(BUTTON_START)) {
					state = NEXT;
					release = 0;
				}  
			}
			break;
		case OBJ:
		// Set game palette
			DMA[3].src = game_palette;
			DMA[3].dst = PALETTE;
			DMA[3].cnt = 256 | DMA_ON;
			for (int i = 0; i < numcolors; i++) {
        		PALETTE[253-i] = colors[i];
		    }
		    volatile u16 bgcol4 = PALETTE[130];
			DMA[3].src = &bgcol4;
			DMA[3].dst = videoBuffer;
			DMA[3].cnt = 19200 | DMA_ON | DMA_SOURCE_FIXED;
			FlipPage();
			break;
	// Initialization
		case NEXT:
			*pLives = 5;
			*pFinished = 0;
			*pScore = 0;
		// Initialize player attributes
			pPlayer->row = SCREENMIN + 5;
			pPlayer->col = 5;
			pPlayer->size = 4;
			pPlayer->color = 247;
			pPlayer->rd = 5;
			pPlayer->cd = 5;
		// Initialize enemy position, velocity, and color
			for (int i = 0; i < NUMOBJS; i++) {
				objs[i].row = rand()%150;
				objs[i].col = rand()%150 + 50;
				objs[i].rd = dels[rand()%numdels];
				objs[i].cd = dels[rand()%numdels];
				objs[i].size = sizes[i%numsizes];
				objs[i].color = 253-(i%(numcolors-1));
			}
		// Set game palette
			DMA[3].src = game_palette;
			DMA[3].dst = PALETTE;
			DMA[3].cnt = 256 | DMA_ON;
			for (int i = 0; i < numcolors; i++) {
        		PALETTE[253-i] = colors[i];
		    }
		// Draw initial bg, lives score
			drawImage4 (0, 0, 240, 160, gbaocean);
			drawImage4 (4, 45, 40, 7, hearts5);
		    sprintf(buffer, "Lives:");
		    drawString4(3, 5, buffer, REDIDX);
			FlipPage(); 
			state = GAME;
			break;
	// Lost a life
		case RESPAWN:
			pPlayer->row = SCREENMIN + 5;
			pPlayer->col = 5;
			drawImage4 (0, 0, 240, 160, gbaocean);
			sprintf(buffer, "Lives:");
			drawImage4 (4, 45, 40, 7, hearts[lives]);
			drawString4(3, 5, buffer, REDIDX);
			sprintf(buffer, "Score: %d", score);        
			drawString4(3, 170, buffer, REDIDX);
		    drawRect4(pPlayer->row, pPlayer->col, pPlayer->size, pPlayer->size, pPlayer->color);
			state = GAME;
			for (int i = 0; i < NUMOBJS; i++) {
				cur = &objs[i];
				objs[i].row = rand()%150;
				while (objs[i].row < SCREENMIN || objs[i].row > SCREENHEIGHT) {
					objs[i].row = rand()%150;
				}
				objs[i].col = rand()%150 + 50;
		        drawRect4(cur->row, cur->col, cur->size, cur->size, cur->color);
			}
			FlipPage();
			break;
	// Super fun awesome game time begins right here !!!
		case GAME:
			if (*pLives == 0) {
				state = LOSE;
		    }
			if (*pScore >= 600) {
				state = WIN;
			}
		// Button controls for player
		    if (KEY_DOWN_NOW(BUTTON_UP)) {
				pPlayer->row += -pPlayer->rd;
		    }        
		    if (KEY_DOWN_NOW(BUTTON_DOWN)) {
				pPlayer->row += pPlayer->rd;
		    }        
		    if (KEY_DOWN_NOW(BUTTON_LEFT)) {
				pPlayer->col += -pPlayer->cd;
		    }        
		    if (KEY_DOWN_NOW(BUTTON_RIGHT)) {
				pPlayer->col += pPlayer->cd;
			}
		// Enemy movement/boundaries
		    for (int i = 0; i < NUMOBJS; i++) {
		        cur = &objs[i];           
		        cur->row += cur->rd;
		        cur->col += cur->cd;
		        if (cur->row < SCREENMIN) {
		            cur->row = SCREENMIN;
		            cur->rd = -cur->rd;
		        }
		        if (cur->row > SCREENHEIGHT-cur->size) {
		            cur->row = SCREENHEIGHT-cur->size;
		            cur->rd = -cur->rd;
		        }
		        if (cur->col + cur->size < 0) {
		            cur->col = 240 - cur->size;
		        }
		        if (cur->col > 240) {
		            cur->col = 0;
		        }
		    }
		// Player sizing
			pPlayer->size = sizes[score / 100];
		// Player boundaries
		    if (pPlayer->row < SCREENMIN) {
		        pPlayer->row = SCREENMIN;
		    }
		    if (pPlayer->row > SCREENHEIGHT-pPlayer->size) {
		        pPlayer->row = SCREENHEIGHT-pPlayer->size;
		    }
	        if (pPlayer->col + pPlayer->size < 0) {
	            pPlayer->col = 240 - pPlayer->size;
	        }
	        if (pPlayer->col > 240) {
	            pPlayer->col = 0;
	        }
		// Draw background
			drawImage4 (0, 0, 240, 160, gbaocean);
		// Drawing squares
	        drawRect4(pPlayer->row, pPlayer->col, pPlayer->size, pPlayer->size, pPlayer->color);
		    for (int i = 0; i < NUMOBJS; i++) {
		        cur = &objs[i];
		        drawRect4(cur->row, cur->col, cur->size, cur->size, cur->color);
				checkCollisions(pPlayer, pPlayer->size, cur, cur->size, pScore, pLives, pDied);
		    }
			if (*pDied == 1) {
				state = RESPAWN;
				*pDied = 0;
			}
		// Draw lives and score
		    sprintf(buffer, "Lives:");
			drawImage4 (4, 45, 40, 7, hearts[lives]);
		    drawString4(3, 5, buffer, REDIDX);
		    sprintf(buffer, "Score: %d", score);        
		    drawString4(3, 170, buffer, REDIDX);
		    delay(3);		    
			waitForVblank();
		    FlipPage();
			break;
		case WIN:
			DMA[3].src = end_palette;
			DMA[3].dst = PALETTE;
			DMA[3].cnt = 256 | DMA_ON;
			drawImage4 (0, 0, 240, 160, win);
			FlipPage();
			if (KEY_DOWN_NOW(BUTTON_START)) {
				state = CONT;
		    }  
			break;
	// Lose, hearts animation
		case LOSE:
			// Loop through animation once
			if (isFinished == 0) {
				for (int i = 0; i < HEARTSANI_FRAMES; i++) {
					drawImage4 (0, 0, 240, 160, gbaocean);
					sprintf(buffer, "Lives:");
					drawString4(3, 5, buffer, REDIDX);
					sprintf(buffer, "Score: %d", score);        
					drawString4(3, 170, buffer, REDIDX);
				    drawRect4(pPlayer->row, pPlayer->col, pPlayer->size, pPlayer->size, pPlayer->color);
					for (int i = 0; i < NUMOBJS; i++) {
						cur = &objs[i];
						drawRect4(cur->row, cur->col, cur->size, cur->size, cur->color);
					}
					drawImage4 (4, 45, 40, 7, *(heartsAni_frames + i));
					if (i < 12) {
						delay(5);
					} else {
						delay(20);
					}
					FlipPage();
				}
				*pFinished = 1;
				state = OVER;
			}
			break;
		case OVER:
			DMA[3].src = end_palette;
			DMA[3].dst = PALETTE;
			DMA[3].cnt = 256 | DMA_ON;
			drawImage4 (0, 0, 240, 160, over);
			FlipPage();
			if (KEY_DOWN_NOW(BUTTON_START)) {
				state = CONT;
		    }  
			break;
		case CONT:			
			if (!KEY_DOWN_NOW(BUTTON_START)) {
				release = 1;
			}
			if (release == 1) {
				drawImage4 (0, 0, 240, 160, cont);
				FlipPage();
				if (KEY_DOWN_NOW(BUTTON_START)) {
					state = START;
					release = 0;
				}  
			}
			break;
		} // End of switch statements
    } // End of while loop
	return 0;
}

void delay(int n) {
    int i = 0;
    volatile int x=0;
    for(i=0; i<n*10000; i++) {
        x++;
    }
}

void checkCollisions(MOVOBJ *p, int pSize, MOVOBJ *e, int eSize, int *score, int *lives, int *died) {
	if ((((p->row < e->row + eSize) && (p->row + pSize > e->row))
		|| ((e->row + eSize > p->row) && (e->row < p->row + pSize)))
		&& (((p->col + pSize > e->col) && (p->col < e->col + eSize))
		|| ((e->col < p->col + pSize) && (e->col + eSize > p->col)))) {
	// Player is bigger than or equal to enemy	
		if (p->size >= e->size) {
			*score += 10;
			e->row = 0;
			e->col = 0;
	// Enemy is bigger, take life
		} else if (*lives > 0) {
			*lives -= 1;
			if (*lives != 0) {
				*died = 1;
			}
		}
	}
}

















