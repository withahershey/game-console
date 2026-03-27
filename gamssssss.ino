#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>

#define sclk 18
#define mosi 23
#define cs   5
#define rst  4
#define dc   2
#define leftButton 25
#define rightButton 32
 
int score = 0;
int level = 1;
const int width = 96;
const int height = 64;

int paddlex = 7;
const int paddleWidth = 20;
const int paddley = height -6 ;

int ballx = paddlex + paddleWidth/2;
int bally = paddley -4;
float ball_dir_x = 1;
float ball_dir_y = -1;
int paddleSpeed = 2;
int lastSpeedIncrease = 0;
const int rows = 5;
const int col = 12;
bool gameRunning = true;
bool scoreScreenDrawn = false;

Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, cs, dc, rst);

bool bricks[rows][col];

void drawGame() {
  display.fillScreen(0x0000);

  int brickWidth = width / col;
  int brickHeight = 6;

  uint16_t colors[] = {0xF800, 0xFFE0, 0x07E0, 0x001F, 0xF81F, 0x07FF};

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < col; c++) {
      if (bricks[r][c]) {
        int brickx = c * brickWidth;
        int bricky = r * brickHeight;

        display.fillRect(brickx, bricky,brickWidth - 2,brickHeight - 2,colors[r]);
      }
    }
  }

  display.fillRect(paddlex, paddley, paddleWidth, 4, 0x07E0);
  display.fillRect(ballx, bally, 2, 2, 0xFFFF);
}
void resetGame() {
  paddlex = 7;

  ballx = paddlex + paddleWidth/2;
  bally = paddley -4;
  ball_dir_x = 1;
  ball_dir_y = -1;

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < col; c++) {
      bricks[r][c] = true;
    }
  }

  score = 0;
  gameRunning = true;
}

void showScoreScreen() {
  display.fillScreen(0x0000);

  display.setTextColor(0xFFFF);
  display.setTextSize(1.5);
  display.setCursor(5, 5);
  display.print("ROUND OVER! ");

  display.setTextSize(2);
  display.setCursor(40, 20);
  display.print(score);

  display.setTextSize(0.5);
  display.setCursor(10, 45);
  display.print("try again lil bro lol ");
  delay(3000);
}

bool allBricksGone() {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < col; c++) {
      if (bricks[r][c]) {
        return false;   // still bricks left
      }
    }
  }
  return true;  // none left
}

void resetBricks() {
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < col; c++) {
      bricks[r][c] = true;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);

  // Initialize bricks
  for(int r=0; r<rows; r++) {
    for(int c=0; c<col; c++) {
      bricks[r][c] = true; 
    }
  }

  display.begin();
  display.fillScreen(0x0000);
  resetGame();
}

void loop() {
  if (!gameRunning) {

    if (!scoreScreenDrawn) {
      showScoreScreen();
      scoreScreenDrawn = true;
    }

    if (digitalRead(leftButton) == LOW && digitalRead(rightButton) == LOW) {
      delay(300);
      resetGame();
    }

    delay(50);
    return;
  }

  if(digitalRead(leftButton) == LOW && paddlex > 0){
    paddlex -= paddleSpeed;
  }
  if(digitalRead(rightButton) == LOW && paddlex < width - paddleWidth){
    paddlex += paddleSpeed;
  }

  ballx += ball_dir_x;
  bally += ball_dir_y;

  if (ballx <= 0) {
    ballx = 0;
    ball_dir_x *= -1;
  }

  if (ballx >= width - 2) {
    ballx = width - 2;
    ball_dir_x *= -1;
  }
  
  if (bally <= 0) {
    bally = 0;           // snap back inside screen
    ball_dir_y *= -1;    // then bounce
  }


  if(bally >= paddley - 2 && ballx >= paddlex && ballx <= paddlex + paddleWidth) {
    ball_dir_y *= -1;
    bally = paddley - 2; // Snap to top of paddle to prevent "sticking"
  }


  int brickWidth = width / col;
  int brickHeight = 8;

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < col; c++) {
      if (bricks[r][c]) {
        int brickX = c * brickWidth;
        int brickY = r * brickHeight;

        
        if (ballx + 2 > brickX && ballx < brickX + (brickWidth - 2) &&
            bally + 2 > brickY && bally < brickY + (brickHeight - 2)) {
          
          bricks[r][c] = false;
          ball_dir_y *= -1;
          score++;
          
          
          if(score % 5 == 0) {
            ball_dir_x *= 1.05; 
            ball_dir_y *= 1.05; 
            paddleSpeed *= 1.06;
          } 
          goto skipBrickCheck; 
        }
      }
    }
  }
  skipBrickCheck:
  if (allBricksGone()) {

    level++;

    // speed up ball slightly
    ball_dir_x *= 1.2;
    ball_dir_y *= 1.2;
    paddleSpeed *= 1.3;
    display.fillScreen(0x0000);
    display.setCursor(20, 30);
    display.print("LEVEL ");
    display.print(level);
    delay(5000);
    resetBricks();

    // reset ball above paddle
    ballx = paddlex + paddleWidth / 2;
    bally = paddley - 4;

    delay(500);   // short pause before next level
  }

  
  if(bally >= height) {
    gameRunning = false;
    scoreScreenDrawn = false;
  }

  
  drawGame();
  delay(10); 
}
