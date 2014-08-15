/***************************************************************************
** +--------------------------------------------------------------------+ **
** |                                                                    | **
** | SPong - a simple SDL program developed on Linux                    | **
** | Copyright (C) 2001 Eric Johnson                                    | **
** |                                                                    | **
** | This program is free software; you can redistribute it and/or      | **
** | modify it under the terms of the GNU General Public License        | **
** | as published by the Free Software Foundation; either version 2     | **
** | of the License, or (at your option) any later version.             | **
** |                                                                    | **
** | This program is distributed in the hope that it will be useful,    | **
** | but WITHOUT ANY WARRANTY; without even the implied warranty of     | **
** | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      | **
** | GNU General Public License for more details.                       | **
** |                                                                    | **
** | You should have received a copy of the GNU General Public License  | **
** | along with this program; if not, write to the                      | **
** |   Free Software Foundation, Inc.                                   | **
** |   59 Temple Place - Suite 330                                      | **
** |   Boston, MA  02111-1307, USA.                                     | **
** |                                                                    | **
** +--------------------------------------------------------------------+ **
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL/SDL.h>
#include "spong.h"


int main(int argc, char **argv) {
	SDL_Surface *screen;
	SDL_Event event;
	int quit=0;
	int pause=0;
	int resuming=0;
	Uint8 *keys;
	Uint32 black;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr,"%s: could not initial SDL components: %s\n",
            argv[0],SDL_GetError());
		exit(1);
	}
	/* clean exit */
	atexit(EMJ_Quit);

	/* set randomness and initial ball dir */
	srand((unsigned int)time(NULL));

	/* let's ignore all events except keyboard/wm */
	SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN,SDL_IGNORE);
	SDL_EventState(SDL_JOYAXISMOTION,SDL_IGNORE);
	SDL_EventState(SDL_JOYBALLMOTION,SDL_IGNORE);
	SDL_EventState(SDL_JOYHATMOTION,SDL_IGNORE);
	SDL_EventState(SDL_JOYBUTTONUP,SDL_IGNORE);
	SDL_EventState(SDL_JOYBUTTONDOWN,SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE); /* can't resize win? */

	screen = SDL_SetVideoMode(SCREEN_W,SCREEN_H,8,
	  SDL_HWSURFACE|SDL_DOUBLEBUF);

	loadGraphics();
	setStartPos();

	black = SDL_MapRGB(screen->format,0x00,0x00,0x00);
	ball_speed = INIT_BALL_SPEED;
	paddle_speed = 5;
	left_score = right_score = 0;

	SDL_WM_SetCaption("SIMPLE PONG","PONG");
	SDL_ShowCursor(SDL_DISABLE); /* hide mouse */

	if (screen == NULL) {
		fprintf(stderr,"%s error: couldn't set video mode: %s\n",
            argv[0],SDL_GetError());
		exit(1);
	}
	
	/* starting paddle stuff */
	/* between 120 - 240 degrees */
	ball.theta = 120+(Uint16)(121.0*rand()/(RAND_MAX+1.0));
	ball.x = SCREEN_W / 2;
	ball.y = SCREEN_H / 2;
	ball.dx = cos(ball.theta*PI/180) * ball_speed;
	ball.dy = sin(ball.theta*PI/180) * ball_speed * -1; /* y-axis is inverted */

	AboutMenu(screen,black);

	ResumingScreen(screen,black,0);

	while (!quit) { /* main game loop */
		resuming=0;
		WaitFrame();
		SDL_PumpEvents(); /* fill event queue from devices */

		/* grab the next event if it matches our mask */
		if (SDL_PeepEvents(&event,1,SDL_GETEVENT,
		  SDL_KEYUPMASK|SDL_KEYDOWNMASK|SDL_QUITMASK) == 1) {
			switch (event.type) {
				case SDL_QUIT: quit=1; break;
				default:
					keys = SDL_GetKeyState(NULL);
					break;
			}
		}

		if (keys[SDLK_a] == SDL_PRESSED && !pause) {
			/* record previous server paddle position */
			if (left_paddle.pos.y - paddle_speed < 0) {
				left_paddle.pos.y = 0;
			} else {
				left_paddle.pos.y -= paddle_speed;
			}
		} else if (keys[SDLK_z] == SDL_PRESSED && !pause) {
			/* record previous server paddle position */
			if (left_paddle.pos.y+paddle_speed+left_paddle.pos.h > screen->h) {
				left_paddle.pos.y = screen->h - left_paddle.pos.h;
			} else {
				left_paddle.pos.y += paddle_speed;
			}

		}
		if (keys[SDLK_UP] == SDL_PRESSED && !pause) {
			/* record previous server paddle position */
			if (right_paddle.pos.y - paddle_speed < 0) {
				right_paddle.pos.y = 0;
			} else {
				right_paddle.pos.y -= paddle_speed;
			}
		} else if (keys[SDLK_DOWN] == SDL_PRESSED && !pause) {
			/* record previous server paddle position */
			if (right_paddle.pos.y+paddle_speed+right_paddle.pos.h > screen->h) {
				right_paddle.pos.y = screen->h - right_paddle.pos.h;
			} else {
				right_paddle.pos.y += paddle_speed;
			}

		}

		if (keys[SDLK_q] == SDL_PRESSED) {
			switch (GameMenu(screen,black)) {
				case 0: /* new game */
					setStartPos();
					ResumingScreen(screen,black,0);
					resuming=0; pause=0;
					break;
				case 1: /* resume */
					resuming=1; pause=0;
					break;
				case 2: /* quit */
					quit=1;
					break;
			}
		}

		/* move 'server' paddle */
		if (keys[SDLK_p] == SDL_PRESSED) {
			if (pause) {
				pause = 0;
				resuming=1;
			} else {
				pause = 1;
			}
		}

		/* move ball regardless of our/client activity */
		if (!pause && !resuming) {
			move_ball(screen,&ball,&left_paddle,&right_paddle);
		}

		if (resuming) {
			ResumingScreen(screen,black,1);
			resuming=0;
		}

		UpdateScreen(screen,pause,black);

		SDL_Flip(screen);

#ifdef PONG_DBUG
		printf("==> drawing coordinates:\n");
		printf("      ball.theta = %d\n",ball.theta);
		printf("      ball.pos.x = %d\n",ball.pos.x);
		printf("      ball.pos.x = %d\n",ball.pos.y);
		printf("    server.pos.x = %d\n",left_paddle.pos.x);
		printf("    server.pos.x = %d\n",left_paddle.pos.y);
		printf("    client.pos.x = %d\n",right_paddle.pos.x);
		printf("    client.pos.y = %d\n\n",right_paddle.pos.y);
#endif

		if (keys[SDLK_p] == SDL_PRESSED) {
			SDL_Delay(250);
		}
	}
	
	EMJ_Quit();

	return 0;
}

void GameOver(SDL_Surface *screen, Uint32 black) {
	SDL_Rect src;

	src.x = src.y = 0;
	gameover_text.pos.w = gameover_text.img->w;
	gameover_text.pos.h = gameover_text.img->h;
	gameover_text.pos.x = SCREEN_W / 2 - gameover_text.img->w / 2;
	gameover_text.pos.y = SCREEN_H / 2 - gameover_text.img->h / 2;

	SDL_FillRect(screen,NULL,black);
	src.w = gameover_text.img->w;
	src.h = gameover_text.img->h;
	SDL_BlitSurface(gameover_text.img,&src,screen,&gameover_text.pos);
	SDL_Flip(screen);
	sleep(2);

	return;
}

void DisplayAbout(SDL_Surface *screen, Uint32 black) {
	SDL_Rect src;
	SDL_Event event;
	int loop=1;

	src.x = src.y = 0;
	about_text.pos.w = about_text.img->w;
	about_text.pos.h = about_text.img->h;
	about_text.pos.x = SCREEN_W / 2 - about_text.img->w / 2;
	about_text.pos.y = SCREEN_H / 2 - about_text.img->h / 2;

	SDL_FillRect(screen,NULL,black);
	src.w = about_text.img->w;
	src.h = about_text.img->h;
	SDL_BlitSurface(about_text.img,&src,screen,&about_text.pos);
	SDL_Flip(screen);
	sleep(2);

	while (loop) {
		WaitFrame();
		SDL_PumpEvents(); /* fill event queue from devices */

		/* grab the next event if it matches our mask */
		if (SDL_PeepEvents(&event,1,SDL_GETEVENT,
		  SDL_KEYUPMASK|SDL_KEYDOWNMASK|SDL_QUITMASK) == 1) {
			switch (event.type) {
				case SDL_QUIT: exit(0); break;
				case SDL_KEYDOWN: loop=0; break;
				default:
					break;
			}
		}
	}

	return;
}

void AboutMenu(SDL_Surface *screen, Uint32 black) {
	SDL_Rect src;
	SDL_Event event;
	Uint8 *keys;
	int done=0;
	int option=0; /* 0=>new, 1=>about, 2=>quit */

	src.x = src.y = 0;
	gmanew_text.pos.w = gmanew_text.img->w;
	gmanew_text.pos.h = gmanew_text.img->h;
	gmanew_text.pos.x = SCREEN_W / 2 - gmanew_text.img->w / 2;
	gmanew_text.pos.y = SCREEN_H / 2 - gmanew_text.img->h / 2;

	gmaabout_text.pos.w = gmaabout_text.img->w;
	gmaabout_text.pos.h = gmaabout_text.img->h;
	gmaabout_text.pos.x = SCREEN_W / 2 - gmaabout_text.img->w / 2;
	gmaabout_text.pos.y = SCREEN_H / 2 - gmaabout_text.img->h / 2;

	gmaquit_text.pos.w = gmaquit_text.img->w;
	gmaquit_text.pos.h = gmaquit_text.img->h;
	gmaquit_text.pos.x = SCREEN_W / 2 - gmaquit_text.img->w / 2;
	gmaquit_text.pos.y = SCREEN_H / 2 - gmaquit_text.img->h / 2;

	/* blank out screen */
	SDL_FillRect(screen,NULL,black);
	src.w = gmanew_text.img->w;
	src.h = gmanew_text.img->h;
	SDL_BlitSurface(gmanew_text.img,&src,screen,&gmanew_text.pos);
	SDL_Flip(screen);

	while (!done) {
		WaitFrame();
		SDL_PumpEvents(); /* fill event queue from devices */

		/* grab the next event if it matches our mask */
		if (SDL_PeepEvents(&event,1,SDL_GETEVENT,
		  SDL_KEYUPMASK|SDL_KEYDOWNMASK|SDL_QUITMASK) == 1) {
			switch (event.type) {
				case SDL_QUIT: exit(0); break;
				default:
					keys = SDL_GetKeyState(NULL);
					break;
			}

			if (keys[SDLK_RETURN] == SDL_PRESSED) {
				/* do selected */
				switch (option) {
					case 0: /* new game */
						return;
					case 1: /* about */
						DisplayAbout(screen,black);
						break;
					case 2: /* quit */
						exit(0);
				}
			} else if (keys[SDLK_UP] == SDL_PRESSED) {
				/* if not at top, move ptr up */
				option--;
			} else if (keys[SDLK_DOWN] == SDL_PRESSED) {
				/* if not at bottom, move ptr down */
				option++;
			}

			if (option < 0) option=0;
			if (option > 2) option=2;
		
			/* blank out screen */
			SDL_FillRect(screen,NULL,black);

			switch (option) {
				case 0:
					src.w = gmanew_text.img->w;
					src.h = gmanew_text.img->h;
					SDL_BlitSurface(gmanew_text.img,&src,screen,&gmanew_text.pos);
					break;
				case 1:
					src.w = gmaabout_text.img->w;
					src.h = gmaabout_text.img->h;
					SDL_BlitSurface(gmaabout_text.img,&src,screen,&gmaabout_text.pos);
					break;
				case 2:
					src.w = gmaquit_text.img->w;
					src.h = gmaquit_text.img->h;
					SDL_BlitSurface(gmaquit_text.img,&src,screen,&gmaquit_text.pos);
					break;
			}

			SDL_Flip(screen);
		}
	}

	return;
}

int GameMenu(SDL_Surface *screen, Uint32 black) {
	SDL_Rect src;
	SDL_Event event;
	Uint8 *keys;
	int done=0;
	int option=0; /* 0=>new, 1=>resume, 2=>quit */

	src.x = src.y = 0;
	gmnew_text.pos.w = gmnew_text.img->w;
	gmnew_text.pos.h = gmnew_text.img->h;
	gmnew_text.pos.x = SCREEN_W / 2 - gmnew_text.img->w / 2;
	gmnew_text.pos.y = SCREEN_H / 2 - gmnew_text.img->h / 2;

	gmresume_text.pos.w = gmresume_text.img->w;
	gmresume_text.pos.h = gmresume_text.img->h;
	gmresume_text.pos.x = SCREEN_W / 2 - gmresume_text.img->w / 2;
	gmresume_text.pos.y = SCREEN_H / 2 - gmresume_text.img->h / 2;

	gmquit_text.pos.w = gmquit_text.img->w;
	gmquit_text.pos.h = gmquit_text.img->h;
	gmquit_text.pos.x = SCREEN_W / 2 - gmquit_text.img->w / 2;
	gmquit_text.pos.y = SCREEN_H / 2 - gmquit_text.img->h / 2;

	while (!done) {
		WaitFrame();
		SDL_PumpEvents(); /* fill event queue from devices */

		/* grab the next event if it matches our mask */
		if (SDL_PeepEvents(&event,1,SDL_GETEVENT,
		  SDL_KEYUPMASK|SDL_KEYDOWNMASK|SDL_QUITMASK) == 1) {
			switch (event.type) {
				case SDL_QUIT: exit(0); break;
				default:
					keys = SDL_GetKeyState(NULL);
					break;
			}

			if (keys[SDLK_RETURN] == SDL_PRESSED) {
				/* do selected */
				return option;
			} else if (keys[SDLK_UP] == SDL_PRESSED) {
				/* if not at top, move ptr up */
				option--;
			} else if (keys[SDLK_DOWN] == SDL_PRESSED) {
				/* if not at bottom, move ptr down */
				option++;
			}

			if (option < 0) option=0;
			if (option > 2) option=2;
		
			/* blank out screen */
			SDL_FillRect(screen,NULL,black);

			switch (option) {
				case 0:
					src.w = gmnew_text.img->w;
					src.h = gmnew_text.img->h;
					SDL_BlitSurface(gmnew_text.img,&src,screen,&gmnew_text.pos);
					break;
				case 1:
					src.w = gmresume_text.img->w;
					src.h = gmresume_text.img->h;
					SDL_BlitSurface(gmresume_text.img,&src,screen,&gmresume_text.pos);
					break;
				case 2:
					src.w = gmquit_text.img->w;
					src.h = gmquit_text.img->h;
					SDL_BlitSurface(gmquit_text.img,&src,screen,&gmquit_text.pos);
					break;
			}

			SDL_Flip(screen);
		}
	}

	return 0;
}

/* handle blitting calls */
void ResumingScreen(SDL_Surface *screen, Uint32 black, int resume) {
	int count;
	SDL_Rect src,dest;

	src.x = src.y = 0;

	SDL_FillRect(screen,NULL,black); /* clear the screen */

	src.w = centerline.img->w;
	src.h = centerline.img->h;
	SDL_BlitSurface(centerline.img,&src,screen,&centerline.pos);
	src.w = left_paddle.img->w;
	src.h = left_paddle.img->h;
	SDL_BlitSurface(left_paddle.img,&src,screen,&left_paddle.pos);
	src.w = right_paddle.img->w;
	src.h = right_paddle.img->h;
	SDL_BlitSurface(right_paddle.img,&src,screen,&right_paddle.pos);
	src.w = ball.img->w;
	src.h = ball.img->h;
	SDL_BlitSurface(ball.img,&src,screen,&ball.pos);

	/* left score */
	src.w = scores[left_score].img->w;
	src.h = scores[left_score].img->h;
	dest.x = (SCREEN_W / 4) - (scores[left_score].img->w / 2);
	dest.y = 10;
	dest.w = scores[left_score].img->w;
	dest.h = scores[left_score].img->h;
	SDL_BlitSurface(scores[left_score].img,&src,screen,&dest);

	/* right score */
	src.w = scores[right_score].img->w;
	src.h = scores[right_score].img->h;
	dest.x = (SCREEN_W / 2) + (SCREEN_W / 4) - (scores[right_score].img->w / 2);
	dest.y = 10;
	dest.w = scores[right_score].img->w;
	dest.h = scores[right_score].img->h;
	SDL_BlitSurface(scores[right_score].img,&src,screen,&dest);


	if (resume) {
		src.w = resume_text.img->w;
		src.h = resume_text.img->h;
		dest.x = (SCREEN_W / 2) - (resume_text.img->w / 2);
		dest.y = (SCREEN_H / 2) - (resume_text.img->h / 2);
		dest.w = resume_text.img->w;
		dest.h = resume_text.img->h;
		SDL_BlitSurface(resume_text.img,&src,screen,&dest);
		SDL_Flip(screen);
		sleep(1);
	} else {
		src.w = starting_text.img->w;
		src.h = starting_text.img->h;
		dest.x = (SCREEN_W / 2) - (starting_text.img->w / 2);
		dest.y = (SCREEN_H / 2) - (starting_text.img->h / 2);
		dest.w = starting_text.img->w;
		dest.h = starting_text.img->h;
		SDL_BlitSurface(starting_text.img,&src,screen,&dest);
		SDL_Flip(screen);
		sleep(1);
	}
	

	for (count=2;count >=0; count--) {
		SDL_FillRect(screen,NULL,black); /* clear the screen */

		/* fill in important stuff */
		src.w = centerline.img->w;
		src.h = centerline.img->h;
		SDL_BlitSurface(centerline.img,&src,screen,&centerline.pos);
	
		src.w = left_paddle.img->w;
		src.h = left_paddle.img->h;
		SDL_BlitSurface(left_paddle.img,&src,screen,&left_paddle.pos);
	
		src.w = right_paddle.img->w;
		src.h = right_paddle.img->h;
		SDL_BlitSurface(right_paddle.img,&src,screen,&right_paddle.pos);

		src.w = ball.img->w;
		src.h = ball.img->h;
		SDL_BlitSurface(ball.img,&src,screen,&ball.pos);

		/* left score */
		src.w = scores[left_score].img->w;
		src.h = scores[left_score].img->h;
		dest.x = (SCREEN_W / 4) - (scores[left_score].img->w / 2);
		dest.y = 10;
		dest.w = scores[left_score].img->w;
		dest.h = scores[left_score].img->h;
		SDL_BlitSurface(scores[left_score].img,&src,screen,&dest);

		/* right score */
		src.w = scores[right_score].img->w;
		src.h = scores[right_score].img->h;
		dest.x = (SCREEN_W / 2) + (SCREEN_W / 4) - (scores[right_score].img->w / 2);
		dest.y = 10;
		dest.w = scores[right_score].img->w;
		dest.h = scores[right_score].img->h;
		SDL_BlitSurface(scores[right_score].img,&src,screen,&dest);

		src.w = scores[count].img->w;
		src.h = scores[count].img->h;
		dest.x = (SCREEN_W / 2) - (scores[count].img->w / 2);
		dest.y = (SCREEN_H / 2) - (scores[count].img->h / 2);
		dest.w = scores[count].img->w;
		dest.h = scores[count].img->h;
		SDL_BlitSurface(scores[count].img,&src,screen,&dest);

		/* push screen to user and sleep for a second*/
		SDL_Flip(screen);

		sleep(1);
	}

	src.w = centerline.img->w;
	src.h = centerline.img->h;
	SDL_BlitSurface(centerline.img,&src,screen,&centerline.pos);
	src.w = left_paddle.img->w;
	src.h = left_paddle.img->h;
	SDL_BlitSurface(left_paddle.img,&src,screen,&left_paddle.pos);
	src.w = right_paddle.img->w;
	src.h = right_paddle.img->h;
	SDL_BlitSurface(right_paddle.img,&src,screen,&right_paddle.pos);
	src.w = ball.img->w;
	src.h = ball.img->h;
	SDL_BlitSurface(ball.img,&src,screen,&ball.pos);

	/* left score */
	src.w = scores[left_score].img->w;
	src.h = scores[left_score].img->h;
	dest.x = (SCREEN_W / 4) - (scores[left_score].img->w / 2);
	dest.y = 10;
	dest.w = scores[left_score].img->w;
	dest.h = scores[left_score].img->h;
	SDL_BlitSurface(scores[left_score].img,&src,screen,&dest);

	/* right score */
	src.w = scores[right_score].img->w;
	src.h = scores[right_score].img->h;
	dest.x = (SCREEN_W / 2) + (SCREEN_W / 4) - (scores[right_score].img->w / 2);
	dest.y = 10;
	dest.w = scores[right_score].img->w;
	dest.h = scores[right_score].img->h;
	SDL_BlitSurface(scores[right_score].img,&src,screen,&dest);

	src.w = go_text.img->w;
	src.h = go_text.img->h;
	dest.x = (SCREEN_W / 2) - (go_text.img->w / 2);
	dest.y = (SCREEN_H / 2) - (go_text.img->h / 2);
	dest.w = go_text.img->w;
	dest.h = go_text.img->h;
	SDL_BlitSurface(go_text.img,&src,screen,&dest);
	SDL_Flip(screen);
	sleep(1);

	return;
}

void new_ball(SDL_Surface *s, Ball *ball, int p, int nb) {
	/* ball went off screen, create new position/trajectory */
	/* new bounce from 30 - 150 degrees */
	ball->theta = 30+(Uint16)(121.0*rand()/(RAND_MAX+1.0));
	if (p) { /* hit right wall */
		ball->theta = 270 - ball->theta;
	} else { /* hit left wall */
		if (ball->theta >= 90) {
			ball->theta = ball->theta - 90;
		} else {
			ball->theta = 270 + ball->theta;
		}
	}
	ball->dx = cos(ball->theta*PI/180) * ball_speed;
	ball->dy = sin(ball->theta*PI/180) * ball_speed * -1;

	if (nb) {
		if (p) {
			ball->pos.x = s->w - (s->w / 4);
			ball->pos.y = s->h / 2;
		} else {
			ball->pos.x = s->w / 4;
			ball->pos.y = s->h / 2;
		}
		ball->x = ball->pos.x;
		ball->y = ball->pos.y;
	}
	return;
}

void move_ball(SDL_Surface *s,Ball *b, DispObj *l, DispObj *r) {
	float next_x=0.0,next_y=0.0;
	int hit_paddle=0;
	Uint32 black;

	black = SDL_MapRGB(s->format,0x00,0x00,0x00);

	/* determine next coords along trajectory */
	next_x = b->x + b->dx;
	next_y = b->y + b->dy;

#ifdef PONG_DBUG
	printf( "\nEntering move_ball()\n");
	printf( "  b->pos.x = %d\n",b->pos.x);
	printf( "  b->pos.y = %d\n",b->pos.y);
	printf( "      b->x = %.2f\n",b->x);
	printf( "      b->y = %.2f\n",b->y);
	printf( "    next_x = %.2f\n",next_x);
	printf( "    next_y = %.2f\n",next_y);
#endif

	/* left paddle check */
	if (next_x >= l->pos.x && next_x <= l->pos.x + l->pos.w) {
		if (next_y + b->pos.h >= l->pos.y && next_y <= l->pos.y + l->pos.h) {
			new_ball(s,b,0,0);
			hit_paddle=1;
			num_bounces++;
			if (num_bounces % 5 == 0) ball_speed++;
		}
	}
	/* right paddle check */
	if (!hit_paddle) {
		if (next_x + b->pos.w >= r->pos.x && next_x + b->pos.w <= r->pos.x + r->pos.w) {
			if (next_y + b->pos.h >= r->pos.y && next_y <= r->pos.y + r->pos.h) {
				new_ball(s,b,1,0);
				hit_paddle=1;
				num_bounces++;
				if (num_bounces % 5 == 0) ball_speed++;
			}
		}
	}
	if (!hit_paddle) {
		/* update x and/or theta */
		if (next_x + b->pos.w > s->w) { /* right wall */
			/* need new theta - actually a point for lefty */
			left_score++;
			if (left_score > TOTAL_POINTS) {
				GameOver(s,black);
				AboutMenu(s,black);
				setStartPos();
				ResumingScreen(s,black,0);
				return;
			}
			/*b->dx = b->dx * -1;*/
			ball_speed = INIT_BALL_SPEED; /* reset ball_speed */
			new_ball(s,b,1,1);
#ifdef PONG_DBUG
			printf("*** COLLISION on RIGHT wall *****\n");
#endif
		} else if (next_x <= 0) { /* left wall */
			/* need new theta - actually a point for righty */
			right_score++;
			if (right_score > TOTAL_POINTS) {
				GameOver(s,black);
				AboutMenu(s,black);
				setStartPos();
				ResumingScreen(s,black,0);
				return;
			}
			/*b->dx = b->dx * -1;*/
			ball_speed = INIT_BALL_SPEED; /* reset ball_speed */
			new_ball(s,b,0,1);
#ifdef PONG_DBUG
			printf("*** COLLISION on LEFT wall *****\n");
#endif
		}

		/* update y and/or theta */
		if (next_y + b->pos.h >= s->h) { /* bottom wall */
			/* need new theta - bounce off bottom */
			b->dy = b->dy * -1;
#ifdef PONG_DBUG
			printf("*** COLLISION on BOTTOM wall *****\n");
#endif
		} else if (next_y <= 0) { /* top wall */
			/* need new theta - bounce off top */
			b->dy = b->dy * -1;
#ifdef PONG_DBUG
			printf("*** COLLISION on TOP wall *****\n");
#endif
		}
	} /* end if (!hit_paddle)... */


	b->x = b->x + b->dx;
	b->y = b->y + b->dy;

	/* update new screen position */
	b->pos.x = (Uint16)b->x;
	b->pos.y = (Uint16)b->y;

#ifdef PONG_DBUG
	printf( "Leaving move_ball()\n");
	printf( "  b->pos.x = %d\n",b->pos.x);
	printf( "  b->pos.y = %d\n",b->pos.y);
	printf( "      b->x = %.2f\n",b->x);
	printf( "      b->y = %.2f\n",b->y);
	printf( "    next_x = %.2f\n",next_x);
	printf( "    next_y = %.2f\n\n",next_y);
#endif
	return;
}

/* handle blitting calls */
void UpdateScreen(SDL_Surface *screen, int paused, Uint32 black) {
	SDL_Rect src,dest;

	SDL_FillRect(screen,NULL,black);
	src.x = src.y = 0;

	/* left score */
	src.w = scores[left_score].img->w;
	src.h = scores[left_score].img->h;
	dest.x = (SCREEN_W / 4) - (scores[left_score].img->w / 2);
	dest.y = 10;
	dest.w = scores[left_score].img->w;
	dest.h = scores[left_score].img->h;
	SDL_BlitSurface(scores[left_score].img,&src,screen,&dest);

	/* right score */
	src.w = scores[right_score].img->w;
	src.h = scores[right_score].img->h;
	dest.x = (SCREEN_W / 2) + (SCREEN_W / 4) - (scores[right_score].img->w / 2);
	dest.y = 10;
	dest.w = scores[right_score].img->w;
	dest.h = scores[right_score].img->h;
	SDL_BlitSurface(scores[right_score].img,&src,screen,&dest);

	src.w = centerline.img->w;
	src.h = centerline.img->h;
	SDL_BlitSurface(centerline.img,&src,screen,&centerline.pos);
	
	src.x = src.y = 0;
	src.w = left_paddle.img->w;
	src.h = left_paddle.img->h;
	SDL_BlitSurface(left_paddle.img,&src,screen,&left_paddle.pos);
	
	src.x = src.y = 0;
	src.w = right_paddle.img->w;
	src.h = right_paddle.img->h;
	SDL_BlitSurface(right_paddle.img,&src,screen,&right_paddle.pos);

	src.x = src.y = 0;
	src.w = ball.img->w;
	src.h = ball.img->h;
	SDL_BlitSurface(ball.img,&src,screen,&ball.pos);

	if (paused) {
		src.w = paused_text.img->w;
		src.h = paused_text.img->h;
		dest.x = (SCREEN_W / 2) - (paused_text.img->w / 2);
		dest.y = (SCREEN_H / 2) - (paused_text.img->h / 2);
		dest.w = paused_text.img->w;
		dest.h = paused_text.img->h;
		SDL_BlitSurface(paused_text.img,&src,screen,&dest);
	}

	return;
}

/* reset starting position of all display objects */
void setStartPos(void) {

	ball_speed = INIT_BALL_SPEED;
	paddle_speed = 5;
	left_score = right_score = 0;

	ball.theta = 120+(Uint16)(121.0*rand()/(RAND_MAX+1.0));
	ball.x = SCREEN_W / 2;
	ball.y = SCREEN_H / 2;
	ball.dx = cos(ball.theta*PI/180) * ball_speed;
	ball.dy = sin(ball.theta*PI/180) * ball_speed * -1; /* y-axis is inverted */

	ball.pos.x = SCREEN_W / 2;
	ball.pos.y = SCREEN_H / 2;
	ball.pos.w = ball.img->w;
	ball.pos.h = ball.img->h;

	centerline.pos.x = (SCREEN_W / 2) - (centerline.img->w / 2);
	centerline.pos.y = 0;
	centerline.pos.w = centerline.img->w;
	centerline.pos.h = centerline.img->h;

	left_paddle.pos.x = 10;
	left_paddle.pos.y = (SCREEN_H / 2) - (left_paddle.img->h / 2);
	left_paddle.pos.w = left_paddle.img->w;
	left_paddle.pos.h = left_paddle.img->h;

	right_paddle.pos.x = SCREEN_W - 10 - right_paddle.img->w;
	right_paddle.pos.y = (SCREEN_H / 2) - (right_paddle.img->h / 2);
	right_paddle.pos.w = right_paddle.img->w;
	right_paddle.pos.h = right_paddle.img->h;

	return;
}

/* load all bitmap images */
void loadGraphics(void) {
	SDL_Surface *temp;

	temp = SDL_LoadBMP("img/starting_in.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load starting_in.bmp\n");
		exit(1);
	}
	starting_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/resuming_in.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load resuming_in.bmp\n");
		exit(1);
	}
	resume_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/paused.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load paused.bmp\n");
		exit(1);
	}
	paused_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/paddle.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load paddle.bmp\n");
		exit(1);
	}
	left_paddle.img = SDL_DisplayFormat(temp);
	right_paddle.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/go.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load go.bmp\n");
		exit(1);
	}
	go_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/game_over.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load game_over.bmp\n");
		exit(1);
	}
	gameover_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/about.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load about.bmp\n");
		exit(1);
	}
	about_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/gamemenu_about_new.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load gamemenu_about_new.bmp\n");
		exit(1);
	}
	gmanew_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/gamemenu_about_about.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load gamemenu_about_about.bmp\n");
		exit(1);
	}
	gmaabout_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/gamemenu_about_quit.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load gamemenu_about_quit.bmp\n");
		exit(1);
	}
	gmaquit_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/game_menu_new.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load game_menu_new.bmp\n");
		exit(1);
	}
	gmnew_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/game_menu_resume.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load game_menu_resume.bmp\n");
		exit(1);
	}
	gmresume_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/game_menu_quit.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load game_menu_quit.bmp\n");
		exit(1);
	}
	gmquit_text.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/centerline.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load centerline.bmp\n");
		exit(1);
	}
	centerline.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/ball.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load ball.bmp\n");
		exit(1);
	}
	ball.img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/0.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load 0.bmp\n");
		exit(1);
	}
	scores[0].img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/1.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load 1.bmp\n");
		exit(1);
	}
	scores[1].img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/2.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load 2.bmp\n");
		exit(1);
	}
	scores[2].img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/3.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load 3.bmp\n");
		exit(1);
	}
	scores[3].img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/4.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load 4.bmp\n");
		exit(1);
	}
	scores[4].img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	temp = SDL_LoadBMP("img/5.bmp");
	if (temp == NULL) {
		fprintf(stderr, "unable to load 5.bmp\n");
		exit(1);
	}
	scores[5].img = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);

	return;
}

/* pause a little */
void WaitFrame(void)
{
	static Uint32 next_tick = 0;
	Uint32 this_tick;

	/* Wait for next frame */
	this_tick = SDL_GetTicks();
	if (this_tick < next_tick) {
		SDL_Delay(next_tick - this_tick);
	}
	next_tick = this_tick + (1000/FRAMES_PER_SECOND);
}

void EMJ_Quit(void)
{
	SDL_Quit();
}

