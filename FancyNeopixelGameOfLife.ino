// Markus Hofstetter's implementation of Conways Game of Life.
// Please note, that this implementation is not very memory efficient. If you want to use a game-board that is larger
// than  13 by 13 pixels you will need more RAM than is available on an Arduino UNO

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "RGB.h"
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif

#define PIN 2


#define HEIGHT 8
#define WIDTH 8

// MATRIX DECLARATION:
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN,
                            NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
                            NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                            NEO_GRB            + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

//this is the internal representation of the game board
uint16_t lastBoard[WIDTH][HEIGHT];
uint16_t board[WIDTH][HEIGHT];
uint16_t nextBoard[WIDTH][HEIGHT];
uint16_t generationNumber = 0;

uint16_t lightsToFadeOn[128];
uint16_t lightsToFadeOff[128];

int deadCounter = 0;
int numOfGames = 1;
bool newGame = true;

RGB randomColor = {0, 0, 0};





void setup() {

  Serial.begin(9600);
  Serial.println("Conways Game of Life for Neopixel");
  matrix.begin();
  matrix.setTextWrap(true);
  matrix.setBrightness(5);
  matrix.setTextColor(colors[3]);
  matrix.setTextSize(6);
  matrix.fillScreen(0);
  initBoard();
  initLightFadeArrays();
  pickRandomColor();
  //  printFreeRam();
}

void loop() {
  //  linetest(green, 1000);
  //  colorWipe(blue, 1000);

  if (newGame) {
    newGame = false;
    pickRandomColor();
    delay(1000);
  }

  drawBoard(true);
  //delay();

  calculateNextState();
  // printFreeRam();

  if(Serial.available()){
  int serialData = Serial.readString().toInt();
  matrix.setBrightness(serialData);
  }

}





void pickRandomColor() {
  int R = 0;
  int G = 0;
  int B = 0;


  R = random(0, 2);
  G = random(0, 2);
  B = random(0, 2);

  while (R == 0 && G == 0 && B == 0) {
    R = random(0, 2);
    G = random(0, 2);
    B = random(0, 2);
  }

  randomColor.r = R;
  randomColor.g = G;
  randomColor.b = B;

}

void initLightFadeArrays() {
  for (int i = 0; i < 127; i++) {
    lightsToFadeOn[i] = 256;
    lightsToFadeOff[i] = 256;
  }
}

void markPixelToFadeOn(int x, int y) {
  for (int i = 0; i < 127; i++) {
    if (lightsToFadeOn[i] == 256) {
      lightsToFadeOn[i] = x;
      lightsToFadeOn[i + 1] = y;
      return;
    }
  }
}

void markPixelToFadeOff(int x, int y) {
  for (int i = 0; i < 127; i++) {
    if (lightsToFadeOff[i] == 256) {
      lightsToFadeOff[i] = x;
      lightsToFadeOff[i + 1] = y;
      return;
    }
  }

}



void initBoard() {
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      lastBoard[x][y] = 0;
      board[x][y] = 0;
      nextBoard[x][y] = 0;
    }
  }
  //Gliderpattern
  //board[1][0] = 1;
  //board[2][1] = 1;
  //board[2][2] = 1;
  //board[1][2] = 1;
  //board[0][2] = 1;

  // R-pentomino
  //  board[2][1] = 1;
  //  board[3][1] = 1;
  //  board[1][2] = 1;
  //  board[2][2] = 1;
  //  board[2][3] = 1;

  randomize();
}

void randomize()
{
  uint16_t i, j;
  randomSeed(millis());
  for (i = 0; i < WIDTH; i++)
  {
    for (j = 0; j < HEIGHT; j++) {
      board[i][j] = random(2);
    }
  }
}


// Fill the pixels one after the other with a color
void colorWipe(RGB color, uint8_t wait) {

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {

      matrix.drawPixel(x, y, matrix.Color(color.r, color.g, color.b));
      matrix.show();
      delay(wait);
    }
  }


}

void linetest(RGB color, uint8_t wait) {


  uint16_t y = 0;
  for (uint16_t x = 0; x < WIDTH; x++) {

    matrix.drawPixel(x, y, matrix.Color(color.r, color.g, color.b));
    matrix.show();
    delay(wait);
  }

}

void calculateNextState() {

  //Serial.println("Calculating next step");
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {

      int numNeighbours = countNeighbours(x, y);
      //under- or overpopulation
      if (numNeighbours < 2 || numNeighbours > 3) {
        nextBoard[x][y] = 0;
      }
      else if (board[x][y] == 0 && numNeighbours == 3) {
        nextBoard[x][y] = 1;
      } else {
        nextBoard[x][y] =  board[x][y];
      }

    }
  }
  generationNumber++;
  copyArraystate();
  printStateToSerial();
}

void printStateToSerial() {
  Serial.print("Generation ");
  Serial.println(generationNumber);
  /*for (uint16_t y = 0; y < HEIGHT; y++) {
    Serial.print(" ");
    for (uint16_t x = 0; x < WIDTH; x++) {

      Serial.print(board[x][y]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }*/


}

void checkIfDead() {
  if (generationNumber > 250) {
    generationNumber = 0;
    randomize();
    newGame = true;
    numOfGames++;
    deadCounter = 0;
    return;
  }

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {

      if (board[x][y] != 0) {
        return;
      }
    }
  }
  deadCounter ++;
  if (deadCounter >= 10) {
    randomize();
    newGame = true;
    numOfGames++;
    deadCounter = 0;

  }
}

void printFreeRam() {
  //Serial.print("Free Ram ");
  Serial.println(freeRam());

}
int countNeighbours(uint16_t xu, uint16_t yu ) {
  //casting here is kinda wrong but since our board is small it does not currentyl matter
  int x = (int) xu;
  int y = (int) yu;
  int neighbours = 0;
  int checked = 0;
  for (int j = (y - 1); j <= (y + 1) ; j++) {
    for (int k = (x - 1); k <= (x + 1); k++) {
      //ensure we are never out of bounds by using an endlessly wrapping board
      int kWrapped = ((k + WIDTH) % WIDTH);
      int jWrapped = ((j + HEIGHT) % HEIGHT);


      if ((kWrapped >= 0 && kWrapped < WIDTH) && jWrapped >= 0 && jWrapped < HEIGHT) {// for additional oob safety
        if (kWrapped == x && jWrapped == y ) {
          continue;
        }
        checked++ ;
        if (board[kWrapped][jWrapped] != 0) {
          neighbours ++;
        }

      }

    }
  }
  //  if (neighbours > 0) {
  //    Serial.print(x);
  //    Serial.print(":");
  //    Serial.print(y);
  //    Serial.print("checked");
  //    Serial.print(checked);
  //    Serial.print("has:");
  //    Serial.println(neighbours);
  //  }
  return neighbours;

}

void drawBoard(bool fade) {
  checkIfDead();
  RGB color;
  initLightFadeArrays();

  //if(fade){
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {

      if (lastBoard[x][y] != 0 && board[x][y] != 0) {
        matrix.drawPixel(x, y, matrix.Color(randomColor.r * 255, randomColor.g * 255, randomColor.b * 255));
      }
      if (lastBoard[x][y] == 0 && board[x][y] == 0) {
        matrix.drawPixel(x, y, matrix.Color(0, 0, 0));
      }
      if (lastBoard[x][y] == 0 && board[x][y] != 0) {
        markPixelToFadeOn(x, y);
      }
      if (lastBoard[x][y] != 0 && board[x][y] == 0) {
        markPixelToFadeOff(x, y);
      }

    }
  }


  for (int lum = 0; lum < 255; lum += 5) {
    for (int i = 0; i < 127; i += 2) {
      if (lightsToFadeOn[i] != 256) {
        matrix.drawPixel(lightsToFadeOn[i], lightsToFadeOn[i + 1], matrix.Color(randomColor.r * lum, randomColor.g * lum, randomColor.b * lum));
      }
    }

    for (int i = 0; i < 127; i += 2) {
      if (lightsToFadeOff[i] != 256) {
        matrix.drawPixel(lightsToFadeOff[i], lightsToFadeOff[i + 1], matrix.Color(randomColor.r * (255 - lum), randomColor.g * (255 - lum), randomColor.b * (255 - lum)));
      }
    }
    matrix.show();
    delay(1);
  }/*}else{
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      if (board[x][y] != 0) {
        color = blue;

      } else {
        color = off;
      }
      matrix.drawPixel(x, y, matrix.Color(color.r, color.g, color.b));



    }
  }
  }*/
  matrix.show();

}

void copyArraystate() {
  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      lastBoard[x][y] = board[x][y];
      board[x][y] =  nextBoard[x][y];
      nextBoard[x][y] = 0;
    }
  }
}

//report ammount of free ram
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
