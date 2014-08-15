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
/* constants */
#define FRAMES_PER_SECOND 30
#define SCREEN_W          640
#define SCREEN_H          480
#define PI                3.141592654
#define TOTAL_POINTS      5

#define INIT_BALL_SPEED   4

/* global variables */
typedef struct ball_obj {
  SDL_Surface *img;        /* store our bitmap                       */
  SDL_Rect pos;            /* store our screen position              */
  float x;                 /* "real" screen position                 */
  float y;                 /* "real" vertical position               */
  float dx;                /* trajectory along x-axis                */
  float dy;                /* trajectory along y-axis                */
  int theta;               /* angle of trajectory                    */
} Ball;

typedef struct disp_obj {
  SDL_Surface *img;        /* store our bitmap                       */
  SDL_Rect pos;            /* store our screen position              */
} DispObj;

static int ball_speed;     /* how many pixels ball moves             */
static int paddle_speed;   /* how many pixels paddles move           */
static int num_bounces;    /* number of volleys                      */
static int left_score;     /* left player's score                    */
static int right_score;    /* right player's score                   */

Ball    ball;              /* keep your eye on it                    */
DispObj scores[6];         /* the five score numbers                 */
DispObj left_paddle;       /* left player                            */
DispObj right_paddle;      /* right player                           */
DispObj centerline;        /* split the court                        */

DispObj paused_text;       /* PAUSED                                 */
DispObj gameover_text;     /* GAME OVER                              */
DispObj gmanew_text;       /* game menu (new)                        */ 
DispObj gmaabout_text;     /* game menu (about)                      */ 
DispObj gmaquit_text;      /* game menu (quit)                       */ 
DispObj about_text;        /* the about screen                       */ 
DispObj gmnew_text;        /* game menu (new)                        */ 
DispObj gmresume_text;     /* game menu (resume)                     */ 
DispObj gmquit_text;       /* game menu (quit)                       */ 
DispObj go_text;           /* GO!                                    */
DispObj resume_text;       /* RESUMING IN...                         */
DispObj starting_text;     /* STARTING IN...                         */

/* function prototypes */
void EMJ_Quit(void);
void loadGraphics(void);
void setStartPos(void);
void UpdateScreen(SDL_Surface *s, int p, Uint32 c);
void ResumingScreen(SDL_Surface *s, Uint32 c, int r);
void WaitFrame(void);
void move_ball(SDL_Surface *s,Ball *b, DispObj *l, DispObj *r);
void new_ball(SDL_Surface *s, Ball *b, int p, int nb);
int GameMenu(SDL_Surface *s, Uint32 c);
void AboutMenu(SDL_Surface *s, Uint32 c);
void DisplayAbout(SDL_Surface *s, Uint32 c);
void GameOver(SDL_Surface *s, Uint32 c);

