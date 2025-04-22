// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// George Adamopoulos and Ezrah Machuca
// Last Modified: 4/15/2024

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"



// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

Arabic_t ArabicAlphabet[]={
alif,ayh,baa,daad,daal,dhaa,dhaal,faa,ghayh,haa,ha,jeem,kaaf,khaa,laam,meem,noon,qaaf,raa,saad,seen,sheen,ta,thaa,twe,waaw,yaa,zaa,space,dot,null
};
Arabic_t Hello[]={alif,baa,ha,raa,meem,null}; // hello
Arabic_t WeAreHonoredByYourPresence[]={alif,noon,waaw,ta,faa,raa,sheen,null}; // we are honored by your presence
int main0(void){ // main 0, demonstrate Arabic output
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(ST7735_WHITE);
  Arabic_SetCursor(0,15);
  Arabic_OutString(Hello);
  Arabic_SetCursor(0,31);
  Arabic_OutString(WeAreHonoredByYourPresence);
  Arabic_SetCursor(0,63);
  Arabic_OutString(ArabicAlphabet);
  while(1){
  }
}



typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_BLACKTAB);
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}


uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}

// Global Variables for Main 5
uint8_t FrameCounter = 0;
uint8_t ADCFlag = 0; 
uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm
uint32_t Time;
uint16_t Score = 0;
uint8_t  Hits  = 0;
bool Paused = false;
bool BossSpawned = false;
int8_t BossDir = 2;
static bool lastWasPaused = false;




/* Here we intiialize our languages like they did in Main 1*/
/* Also, we have our user defined data types of gamestates which tracks which point of the 
game we are at, such as menu or paused or over*/
typedef enum {ENGLISH = 0, SPANISH = 1} Lang_t;
Lang_t Language = ENGLISH;
typedef enum {MENU, RUNNING, PAUSED, GAMEOVER} GameState_t;
GameState_t GameState = MENU;
const char *ScoreTxt[]   = { "Score:",  "Puntos:" };   // These are all constant character arrays with english or spanish versions of what we output
const char *HitsTxt[]    = { "Hits:",  "Golpes:" };     // Index 0 is english and index 1 is spanish, as defined in the data type
const char *PausedTxt[]  = { "PAUSED",  "PAUSA"};
const char *GameOverTxt[] = { "GAME OVER", "FIN DEL JUEGO" };
const char *YouWinTxt[]   = { "YOU WIN!",  "¡GANASTE!"};
const char *FinalScoreTxt[]   = { "Total Score: ",  "Total Puntos: " };


// labels for the game defined here
#define MaxEnemies 5
#define MaxExplosions 5  // max enemies, max explosions, and max enemy bullets are all adjustable for list sizes so as to 
#define MaxEnemyBullets 5 // control how many are on the screen
#define BOSS_HITS 5 // health of the boss
#define Max_Hits 3 // health of the player
#define Enemy_Step_X 2 // how quickly the enemy goes horizontally
#define Enemy_minY 20 // the highest that the enemies in wave 1 can go 
#define Enemy_maxY 80 // lowest the enemies in wave 1 can go

// Enemy direction variables that are adjusted in the isr for how enemies move
// enemy Dx and Dy are direction/velocity in x and y respectively
// enemy ticker is used to move enemies at a rate of 15 Hz as compared to the ISR's 30 Hz
// so what we do is we update it and move it every 2
static int8_t  EnemyDx =  Enemy_Step_X; 
static int8_t  EnemyDy =  1;  
static uint8_t EnemyTicker = 0;   


// the universal sprite struct 
struct Sprite{
  int32_t x; // x-coord
  int32_t y; // y-coord
  const uint16_t *image; // pointer to image array
  uint8_t w; // width of image
  uint8_t h; // height of image
  uint8_t on; // on display or off display (1 or 0) 
  int32_t PrevX, PrevY;
  uint8_t health; // health of sprite
  uint8_t frame; // which sprite are they on so as to animate the sprite
  uint8_t erase; // whether or not the sprite needs to be erased from old position (including if it moves or dies)
};  
typedef struct Sprite Sprite_t; 

// initialize player  
Sprite_t player = {60, 140, PlayerShip0, 18, 8, 1, 60, 140, 3, 0, 0};
const uint16_t *playerSprites[] = {PlayerShip0, PlayerShip1, PlayerShip2};

// only one missile so no array
Sprite_t missile = {0, 0, missile0, 12, 12, 0, 0, 0, 1, 0, 0};

// variable length so i can change it later
Sprite_t enemies[MaxEnemies];
const uint16_t *enemySprites[] = {SmallEnemy10pointA, SmallEnemy10pointB};
// keep bunkers at 3 
Sprite_t bunkers[3];
const uint16_t *bunkerSprites[] = {Bunker0, Bunker1, Bunker2, Bunker3};

// limit amount of epxlosions
Sprite_t explosions[MaxExplosions];
uint8_t explosionCounter[MaxExplosions];

Sprite_t enemyBullets[MaxEnemyBullets];
Sprite_t boss = { 48, 20, AlienBossB, 16,8, 0, -100,-100, BOSS_HITS, 0,0 };


// enemy move function for the entire first wave
static void Enemies_GroupMove(void){
  int32_t minX = 127, maxX = 0, minY = 159, maxY = 0; // initialize max and mins
  // finds current box of ALL enemies TOGETHER
  for(uint8_t i=0;i<MaxEnemies;i++){ 
    if(!enemies[i].on) // check if enemy on
      continue; // if not, check next
    if(enemies[i].x < minX)
      minX = enemies[i].x;
    if(enemies[i].x+enemies[i].w > maxX)
      maxX = enemies[i].x+enemies[i].w;
    if(enemies[i].y < minY)
      minY = enemies[i].y;
    if(enemies[i].y > maxY) 
      maxY = enemies[i].y;
  }

  // First part checks if we are going go over the left side and the second part checks if we are going to over the right side
  // both check if they are going to go over on the NEXT step
  // if so, then reverse the directions
  // the EnemyDx thing is to check if we are moving left or right depending on the sign
  if((EnemyDx < 0 && minX + EnemyDx <= 0) || (EnemyDx > 0 && maxX + EnemyDx >= 127)){
    EnemyDx = -EnemyDx;
  }

  // same thing here but for roof and floor
  // EnemyDy is to check if we are going up or down depending on the sign
  if((EnemyDy < 0 && minY + EnemyDy <= Enemy_minY) || (EnemyDy > 0 && maxY + EnemyDy >= Enemy_maxY)){
    EnemyDy = -EnemyDy;
  }

  // goes through each enemy and moves them according to the enemy directions
  for(uint8_t i=0;i<MaxEnemies;i++){
    if(!enemies[i].on) // if sprite not on the LCD dont do anything bc it isnt beign printed
      continue;
    enemies[i].PrevX = enemies[i].x; // saves previous thing so that we can erase it 
    enemies[i].PrevY = enemies[i].y;
    enemies[i].x += EnemyDx; // move it according to what enemydx or dy is 
    enemies[i].y += EnemyDy;
    enemies[i].erase = 1; // because the sprite moved, set erase equal to 1 so we can erase it
  } 



}



// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){
  if((TIMG12->CPU_INT.IIDX) == 1){
    GPIOB->DOUTTGL31_0 = GREEN; // Debug toggle

     if(GameState != RUNNING){  // if the state is not running (i.e. paused or in menu or over) then just return
       ADCFlag = 1;
       return;
     }
    
    // 1) Sample slide potentiometer
    Data = ADCin();
    player.PrevX = player.x;  // before updating, save player's old position
    player.PrevY = player.y;
    player.x = (Data * (125 - player.w)) / 4095; // adjust based on the ADC
    player.erase = 1; // erase the old thing

    // 2) Handle shooting input
    if((Switch_In() & 0x02) && missile.on == 0){
      missile.x = player.x + (player.w / 2) - (missile.w / 2); // get to the ships center then output the misisles center not its left corner
      missile.y = player.y - missile.h;
      missile.PrevX = missile.x;
      missile.PrevY = missile.y;
      missile.on = 1;
      missile.erase = 1;
      Sound_Shoot();
    }
    
    // 3) Move missile
    if(missile.on){
      missile.PrevX = missile.x;
      missile.PrevY = missile.y;
      missile.y -= 4; // move them by 4 so they arent too slow
      missile.erase = 1; // erase to update sprite
      if(missile.y < 10){    // if it is beyond the top then just erase it 
        missile.on = 0;
        missile.erase = 1;
      }

      // Missile-enemy collision check
      for(uint8_t i = 0; i < MaxEnemies; i++){
        if(enemies[i].on && // is the enemy it hit on?
           missile.x + missile.w > enemies[i].x && // did the missile overlap w the enemies x position?
           missile.x < enemies[i].x + enemies[i].w && // is the left edge of the missile still to the left of the right of the enemy?
           missile.y < enemies[i].y + enemies[i].h && // is the top of the missile above the bottom edge of the enemy?
           missile.y + missile.h > enemies[i].y){  // did the missile overlap w the enemies y position?

          enemies[i].health--; // if so, lower enemy health
          if(enemies[i].health == 0){  
            enemies[i].on = 0;
            Score += 10; // increase players score and erase the sprite
            enemies[i].erase = 1;
            Sound_Explosion(); // kaboom

            for(uint8_t j = 0; j < MaxExplosions; j++){ // handles the visualization of the explosions
              if(explosions[j].on == 0){
                explosions[j].x = enemies[i].x;
                explosions[j].y = enemies[i].y;
                explosions[j].PrevX = explosions[j].x;
                explosions[j].PrevY = explosions[j].y;
                explosions[j].image = SmallExplosion0;
                explosions[j].w = 16;
                explosions[j].h = 10;
                explosions[j].on = 1;
                explosions[j].erase = 1;
                explosionCounter[j] = 2; // last for two ticks
                break;
              }
            }
          }

          missile.on = 0;
          missile.erase = 1; // erase la missile
        }
      }
      // missile boss detection
      if(boss.on &&
          missile.x + missile.w > boss.x && // same thing as above but for da boss
          missile.x < boss.x + boss.w &&
          missile.y < boss.y + boss.h &&
          missile.y + missile.h > boss.y){

          Sound_Fastinvader2();
          boss.health--;
          Score = Score + 100;
          if(boss.health==0){
            boss.on = 0;
            boss.erase= 1;
            Score += 450;
          }
          missile.on = 0;
          missile.erase= 1;
      }


    }
    // boss moving 
    if(boss.on){  // same as above but moves boss
        // horizontal bounce
        boss.PrevX = boss.x;
        boss.x += BossDir;
        if(boss.x <= 0 || boss.x + boss.w >= 127){ 
          BossDir = -BossDir; 
        }

        boss.erase = 1;

        // boss bullet
        if(Random(8)==0){
            for(uint8_t i=0;i<MaxEnemyBullets;i++){
               if(enemyBullets[i].on==0){
                   enemyBullets[i].x = boss.x + boss.w/2 -1;
                   enemyBullets[i].y = boss.y + boss.h;
                   enemyBullets[i].PrevX = enemyBullets[i].x;
                   enemyBullets[i].PrevY = enemyBullets[i].y;
                   enemyBullets[i].on = 1; enemyBullets[i].erase=1;
                   break;
               }
            }
        }
    }
    
    // 4) Move enemy bullets
    for(uint8_t i = 0; i < MaxEnemyBullets; i++){
      if(enemyBullets[i].on){
        enemyBullets[i].PrevX = enemyBullets[i].x;
        enemyBullets[i].PrevY = enemyBullets[i].y;
        enemyBullets[i].y += 3;
        enemyBullets[i].erase = 1;
        if(enemyBullets[i].y > 159){
          enemyBullets[i].on = 0;
          enemyBullets[i].erase = 1;
        }

        // bullet bunker collision
        for(uint8_t b=0; b<3; b++){
          if(bunkers[b].on &&   // same thing as above but for bunkers and enemy bullets
            enemyBullets[i].x+enemyBullets[i].w > bunkers[b].x &&
            enemyBullets[i].x < bunkers[b].x+bunkers[b].w &&
            enemyBullets[i].y < bunkers[b].y &&
            enemyBullets[i].y+enemyBullets[i].h > bunkers[b].y-bunkers[b].h){

              bunkers[b].health--;
              bunkers[b].frame++;          // next damaged bitmap
              bunkers[b].erase = 1;
              if(bunkers[b].health==0){
                  bunkers[b].on = 0;
              }
              enemyBullets[i].on = 0;
              enemyBullets[i].erase = 1;
          }
        }


        if(player.on && // same thing as above but for player and enemy bullets
          enemyBullets[i].x+enemyBullets[i].w > player.x &&
          enemyBullets[i].x < player.x+player.w &&
          enemyBullets[i].y < player.y &&
          enemyBullets[i].y+enemyBullets[i].h > player.y-player.h){

          Hits++;
          Sound_Killed();

          if(Hits < Max_Hits){
              player.image = playerSprites[Hits];
              player.erase = 1;                  // redraw with damage frame
              //LED_On(Hits);
          }


          enemyBullets[i].on = 0;
          enemyBullets[i].erase = 1;
          if(Hits >= Max_Hits){
              player.on = 0;               // defer Game‑Over screen to main loop
          }
        }
      }
    }

    // 5) Random enemy bullet firing
    if(Random(10) == 0){
      uint8_t shooter = Random(MaxEnemies);
      if(enemies[shooter].on){
        for(uint8_t i = 0; i < MaxEnemyBullets; i++){
          if(enemyBullets[i].on == 0){
            enemyBullets[i].x = enemies[shooter].x + enemies[shooter].w / 2 - 1;
            enemyBullets[i].y = enemies[shooter].y + enemies[shooter].h;
            enemyBullets[i].PrevX = enemyBullets[i].x;
            enemyBullets[i].PrevY = enemyBullets[i].y;
            enemyBullets[i].on = 1;
            enemyBullets[i].erase = 1;
            break;
          }
        }
      }
    }

    // 6) Animate enemies every 10 frames
    FrameCounter++;
    if(FrameCounter >= 10){
      for(uint8_t i = 0; i < MaxEnemies; i++){
        if(enemies[i].on){
          enemies[i].frame ^= 1; // change the way the enemies look using EOR 
          enemies[i].image = enemySprites[enemies[i].frame];
          enemies[i].erase = 1;
        }
      }
      FrameCounter = 0;
    }

    // 7) Handle explosion countdowns
    for(uint8_t i = 0; i < MaxExplosions; i++){
      if(explosions[i].on){
        if(explosionCounter[i] > 0){
          explosionCounter[i]--;
        }
        if(explosionCounter[i] == 0){
          explosions[i].on = 0;
          explosions[i].erase = 1;
        }
      }
    }


    if(++EnemyTicker >= 2){          // 30 Hz / 2 = 15 Hz
      Enemies_GroupMove();
      EnemyTicker = 0;
    }   


    // 8) Set semaphore for main to update display
    ADCFlag = 1;
    GPIOB->DOUTTGL31_0 = GREEN; // Debug toggle
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}
// erase sprites
void Clear_Sprite(Sprite_t *s){
  ST7735_FillRect(s->PrevX,s->PrevY - s->h + 1,s->w, s->h, ST7735_BLACK);
}
// draw sprites 
void Draw_Sprite(Sprite_t *s){
  if(s->erase){
    Clear_Sprite(s);
    s->erase = 0;
  }
  if(s->on){
    ST7735_DrawBitmap(s->x, s->y, s->image, s->w, s->h);
  }
  s->PrevX = s->x;
  s->PrevY = s->y;
}
// draw everything 
void Draw_Game(void){
  // Background
  

  // Draw Player
  Draw_Sprite(&player);

  // Draw Missile (if active)
  if(missile.on || missile.erase){
    Draw_Sprite(&missile);
  }

  // Draw Enemies
  for(uint8_t i = 0; i < MaxEnemies; i++){
    if(enemies[i].on || enemies[i].erase){
      Draw_Sprite(&enemies[i]);
    }
  }

  // Draw Enemy Bullets
  for(uint8_t i = 0; i < MaxEnemyBullets; i++){
    if(enemyBullets[i].on || enemyBullets[i].erase){
      Draw_Sprite(&enemyBullets[i]);
    }
  }

  // Draw Explosions
  for(uint8_t i = 0; i < MaxExplosions; i++){
    if(explosions[i].on || explosions[i].erase){
      Draw_Sprite(&explosions[i]);
    }
  }

  for (uint8_t i = 0; i < 3; i++) {
    if (bunkers[i].on) {
        ST7735_DrawBitmap(bunkers[i].x, bunkers[i].y,bunkerSprites[bunkers[i].frame], bunkers[i].w, bunkers[i].h);
    } else if (bunkers[i].erase) {
        for (int row = 0; row < bunkers[i].h; row++) {
            for (int col = 0; col < bunkers[i].w; col++) {
                ST7735_DrawBitmap(bunkers[i].PrevX + col, bunkers[i].PrevY + row, OuterSpaceBlock, 1, 1);
            }
        }
      bunkers[i].erase = 0;
    }
  } 


  // HUD
  ST7735_SetCursor(0, 0);
  ST7735_OutString((char *)ScoreTxt[Language]);

  ST7735_FillRect(60,0,40,8,ST7735_BLACK);
  ST7735_SetCursor(7,0); 
  ST7735_OutUDec(Score);

  ST7735_SetCursor(11, 0);
  ST7735_OutString((char *)HitsTxt[Language]);
  ST7735_FillRect(104,0,24,8,ST7735_BLACK);
  ST7735_SetCursor(18,0); 
  ST7735_OutUDec(Hits);

  if (GameState == PAUSED) {
    ST7735_SetCursor(5, 7);
    ST7735_OutString((char *)PausedTxt[Language]);
    lastWasPaused = true;
  }
    else if(lastWasPaused && GameState == RUNNING){
    ST7735_FillRect(5*6, 7*10, 8*6, 10, ST7735_BLACK);
    lastWasPaused = false;
  }


  //BOSS
  if (boss.on || boss.erase) {
    Draw_Sprite(&boss);
  }

}










void Enemies_Init(void){
  for(uint8_t i = 0; i < MaxEnemies; i++){
    enemies[i].x = 10 + i * 20;
    enemies[i].y = 20;
    enemies[i].PrevX = enemies[i].x;
    enemies[i].PrevY = enemies[i].y;
    enemies[i].image = enemySprites[0];  
    enemies[i].w = 16;
    enemies[i].h = 10;
    enemies[i].on = 1;
    enemies[i].health = 1;
    enemies[i].frame = 0;
    enemies[i].erase = 0;
  }
}

void EnemyBullets_Init(void){
  for(uint8_t i = 0; i < MaxEnemyBullets; i++){
    enemyBullets[i].x = 0;
    enemyBullets[i].y = 0;
    enemyBullets[i].PrevX = 0;
    enemyBullets[i].PrevY = 0;
    enemyBullets[i].image = Laser0;
    enemyBullets[i].w = 2;
    enemyBullets[i].h = 9;
    enemyBullets[i].on = 0;
    enemyBullets[i].health = 1;
    enemyBullets[i].frame = 0;
    enemyBullets[i].erase = 0;
  }
}
void Bunkers_Init(void){
  for(uint8_t i = 0; i < 3; i++){
    bunkers[i].x = 30 + i * 35;
    bunkers[i].y = 130;
    bunkers[i].image = bunkerSprites[0];
    bunkers[i].PrevX = bunkers[i].x;
    bunkers[i].PrevY = bunkers[i].y;
    bunkers[i].w = 18;
    bunkers[i].h = 5;
    bunkers[i].on = 1;
    bunkers[i].health = 3;
    bunkers[i].frame = 0;
    bunkers[i].erase = 0;

  }
}

void Explosions_Init(void){
  for(uint8_t i = 0; i < MaxExplosions; i++){
    explosions[i].x = 0;
    explosions[i].y = 0;
    explosions[i].PrevX = 0;
    explosions[i].PrevY = 0;
    explosions[i].image = SmallExplosion0;
    explosions[i].w = 16;
    explosions[i].h = 10;
    explosions[i].on = 0;
    explosions[i].health = 1;
    explosions[i].frame = 0;
    explosions[i].erase = 0;
    explosionCounter[i] = 0;
  }
}

// use main2 to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_BLACKTAB);
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);

  ST7735_DrawBitmap(50, 50, missile0, 12, 12);

  for(uint32_t t=500;t>0;t=t-5){
    SmallFont_OutVertical(t,104,6); // top left
    Clock_Delay1ms(50);              // delay 50 msec
  }
  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  while(1){
  }
}

// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  __enable_irq(); // UART uses interrupts

    uint32_t sw;


  while(1){
    sw = Switch_In(); // bits 0-3 represent switches 0-3 (PA24 to PA27)

    // Turn off all LEDs first
    LED_Off(0);
    LED_Off(1);
    LED_Off(2);

    // Check each switch
    if(sw & 0x01){      // Switch 0 is pressed
      LED_On(0);        // Turn on LED 0 (PA15)
    }
    if(sw & 0x02){      // Switch 1 is pressed
      LED_On(1);        // Turn on LED 1 (PA16)
    }
    if(sw & 0x04){      // Switch 2 is pressed
      LED_On(2);        // Turn on LED 2 (PA17)
    }
    if(sw & 0x08){      // Switch 3 is pressed
      LED_On(0);
      LED_On(1);
      LED_On(2);        // Turn on all LEDs
    }
  }
}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In();  // Read current switch states

    // Only play sound on rising edge (pressed now but wasn't before)
    if((last == 0) && (now != 0)){
      if(now & 0x01){        // SW0 → Sound_Shoot
        Sound_Shoot();
      }
      if(now & 0x02){        // SW1 → Sound_Killed
        Sound_Killed();
      }
      if(now & 0x04){        // SW2 → Sound_Explosion
        Sound_Explosion();
      }
      if(now & 0x08){        // SW3 → Sound_Fastinvader1
        Sound_Fastinvader1();
      }
    }
    last = now;  // Save switch state for edge detection
  }
}



// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_BLACKTAB);
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  Enemies_Init();
  Bunkers_Init();
  Explosions_Init();
  EnemyBullets_Init();

  ST7735_FillScreen(ST7735_BLACK);
  ST7735_DrawString(0,3, "PRESS RIGHT TO PAUSE", ST7735_YELLOW);
  ST7735_DrawString(0,5, "PRESS DOWN TO ", ST7735_YELLOW);
  ST7735_DrawString(0,6, "TOGGLE BETWEEN", ST7735_YELLOW);
  ST7735_DrawString(0,7, "Espa\xA4ol / English", ST7735_YELLOW);
  ST7735_DrawString(0,9, "PRESS UP TO: ", ST7735_YELLOW);
  ST7735_DrawString(0,10, "Begin game", ST7735_YELLOW);
  ST7735_DrawString(0,12, "LEFT BUTTON TO FIRE", ST7735_YELLOW);

  // menu
  while(GameState == MENU){
    uint32_t sw = Switch_In();
    static uint8_t lastMenuSW = 0;
    static Lang_t lastLang = ENGLISH;

    // toggle language
    if((sw&0x04) && !(lastMenuSW&0x04)){
      Language = (Language == ENGLISH)? SPANISH:ENGLISH; // change to what is not being used
      ST7735_FillScreen(ST7735_BLACK);
    }
    if(Language != lastLang){ 
      if(Language == ENGLISH){
      ST7735_DrawString(0, 5, ">  English", ST7735_WHITE);
      ST7735_DrawString(0, 6, " Espa\xA4ol", ST7735_YELLOW);
      ST7735_DrawString(0, 7, "Right to pause", ST7735_YELLOW);
      ST7735_DrawString(0, 8, "Left to shoot", ST7735_YELLOW);
      ST7735_DrawString(0, 9, "UP: Start", ST7735_YELLOW);
    } else {
      ST7735_DrawString(0, 5, " English", ST7735_YELLOW);
      ST7735_DrawString(0, 6, ">  Espa\xA4ol", ST7735_WHITE);
      ST7735_DrawString(0, 7, "Derecho para pausa", ST7735_YELLOW);
      ST7735_DrawString(0, 8, "Izquierda para disparar", ST7735_YELLOW);
      ST7735_DrawString(0, 9, "Arriba: Empezar", ST7735_YELLOW);
    }

   

    lastLang = Language;
  }
    // start game
    if((sw&0x01) && !(lastMenuSW&0x01)){
      GameState = RUNNING;
      
    }
    lastMenuSW  = sw; 
    Clock_Delay1ms(50);
  }

  ST7735_FillScreen(ST7735_BLACK);  
  ST7735_DrawBitmap(0, 159, OuterSpace, 160, 128);



  __enable_irq();
  while(1){ 
    uint32_t sw = Switch_In();
    static uint8_t lastSW = 0;

    if(GameState==RUNNING && (sw&0x08) && !(lastSW&0x08)){
        GameState = PAUSED;
    }else if(GameState==PAUSED && (sw&0x08) && !(lastSW&0x08)){
        GameState = RUNNING;
    }
    lastSW = sw;

    if(GameState==PAUSED){
      Draw_Game();
      Clock_Delay1ms(50);
      continue;
    }

    // wait for semaphore
    if(GameState==RUNNING){
      while(ADCFlag==0){}
      ADCFlag=0;
      Draw_Game();



      bool anyEnemy=false;
      for(uint8_t i=0;i<MaxEnemies;i++){ 
        if(enemies[i].on){ 
          anyEnemy=true;
        }
      }

      if(!anyEnemy && !BossSpawned){
          boss.on = 1;
          boss.health = BOSS_HITS;
          boss.erase = 1;
          BossSpawned = true;
      }
    
      // check for end game or level switch
      
        if((Hits>=Max_Hits) || (BossSpawned && boss.on==0)){
            GameState = GAMEOVER;
            ST7735_FillScreen(ST7735_BLACK);
            ST7735_SetCursor(6,7);
            if(Hits>=Max_Hits){
                ST7735_SetTextColor(ST7735_BLUE);
                ST7735_OutString((char *)GameOverTxt[Language]);
                ST7735_SetCursor(4,8);
                ST7735_OutString((char *)FinalScoreTxt[Language]);
                ST7735_OutUDec(Score);

            }else{
                ST7735_OutString((char *)YouWinTxt[Language]);
                ST7735_SetCursor(2,8);
                ST7735_OutString((char *)FinalScoreTxt[Language]);
                ST7735_OutUDec((Score));
            }
        }
        continue;
    }

    // idle for game over
    if(GameState==GAMEOVER){
        Clock_Delay1ms(100);
    }
  }
}