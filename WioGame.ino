#include <LIS3DHTR.h>
#include <Wire.h>
#include <vector>

#include "TFT_eSPI.h"
#include "classes.hpp"

#define COULEURFOND TFT_DARKGREY
#define HAUTEURBANDEAU 40
#define BALLRADIUS 7
#define PIECERADIUS 5
#define BUZZER_PIN WIO_BUZZER

EtatJeu EtatActuel = MENU;
TFT_eSPI tft;
LIS3DHTR<TwoWire> lis;

// interuptions var:
volatile byte C_pressed =0; 
volatile byte B_pressed = 0;
volatile byte A_pressed = 0;

// timer :
unsigned long startTime;
int remainingTime = 30;
int malusTime = 0;

// utilisation déjà faite du bouton de droite?
bool btnABonusUsed=false;

// listes de pointeurs sur piece
std::vector<PieceBonus*> listPieceBonus;
std::vector<PieceMalus*> listPieceMalus;

//variable statique de la valeur des pièces dollar
int PieceBonus::valeur=10;

//différents modes de jeux
String level[]={"novice","confirme","expert"};
int itModeJeu = 0;

Ball ball;

//affichage du score
void afficherScore(){
  tft.fillRect(140, 0, 40, HAUTEURBANDEAU, TFT_BLUE); 
  tft.drawNumber(ball.getScore(), 150, 10);
}

/*---------------------------------
      DEFINITION DES METHODES  
------------------------------------*/

// piece Bonus

PieceBonus::PieceBonus(int x, int y):x{x}, y{y}{}

void PieceBonus::dessinerPiece(){
    tft.fillCircle(x, y, PIECERADIUS, TFT_YELLOW);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);  
    tft.drawString("$",x-2,y-3);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);  
}
void PieceBonus::pieceTouchee(){
    ball.setScore(ball.getScore()+valeur);
    afficherScore();
    tft.fillCircle(x,y, PIECERADIUS, COULEURFOND);
    // tone(BUZZER_PIN, 1500, 150);
}
int PieceBonus::getX() {return x;}
int PieceBonus::getY() {return y;}

void PieceBonus::setValeur(int val){
  valeur=val;
}


// piece Malus

PieceMalus::PieceMalus(int x, int y):x{x}, y{y} {}

void PieceMalus::dessinerPiece(){
    tft.fillCircle(x, y, PIECERADIUS, TFT_BLACK);
}
void PieceMalus::pieceTouchee(){
  tft.fillCircle(x,y, PIECERADIUS,COULEURFOND);
  malusTime += 10;
  // tone(BUZZER_PIN, 400, 250);
}
int PieceMalus::getX() {return x;}
int PieceMalus::getY() {return y;}


// Ball
void Ball::deplacer(float x, float y){
  tft.fillCircle(ballx, bally, BALLRADIUS, COULEURFOND);

  ballx += int(-y*10);
  bally += int(x*10);


  tft.fillCircle(ballx, bally, BALLRADIUS, TFT_RED);
}

void Ball::effacer(float x, float y){
  tft.fillCircle(ballx, bally, BALLRADIUS, COULEURFOND);
}
int Ball::getScore() { return score; }
int Ball::getX() { return ballx; }
int Ball::getY() { return bally; }

void Ball::setScore(int nvScore)  { score = nvScore; } 


/* ---------------------------
    DEFINITION DES INTERUPTIONS
------------------------------ */

void changerModeInterrupt() {//interruption pour capter un appui sur le bouton C(gauche)
  C_pressed=1;
}

void lancerPartieInterrupt(){//interruption pour capter un appui sur le bouton B(milieu)
  B_pressed=1;
}

void ajouterTempsInterrupt(){//interruption pour capter un appui sur le bouton B(milieu)
  A_pressed=1;
}

/* ----------------------------------
      DEFINITION DU CHRONOMETRE
------------------------------------- */

void lancerChrono() {
  startTime = millis();
  remainingTime = 30;
}

void afficherChrono() {
  int elapsed = (millis() - startTime) / 1000;
  remainingTime = 30 - elapsed - malusTime;
  if (remainingTime < 0) remainingTime = 0;

  tft.fillRect(280, 0, 40, HAUTEURBANDEAU, TFT_BLUE); // efface ancienne zone
  tft.drawNumber(remainingTime, 290, 10); // affiche chrono
}

/*--------------------------------
      MELODIES
  ---------------------------------*/

// void melodieVictoire() {
//   int notes[] = { 880, 988, 1047, 1175, 1319, 1397, 1568 };
//   int durees[] = { 200, 200, 200, 200, 200, 200, 400 };

//   for (int i = 0; i < 7; i++) {
//     tone(BUZZER_PIN, notes[i], durees[i]);
//     delay(durees[i] * 1.30); 
//   }
// }

// void sonEchec() {
//   int notes[] = { 400, 300, 200, 100 };
//   int durees[] = { 200, 200, 200, 400 };

//   for (int i = 0; i < 4; i++) {
//     tone(BUZZER_PIN, notes[i], durees[i]);
//     delay(durees[i] * 1.3);
//   }
// }

void bipFinChrono() {
  for (int i = 0; i < 5; i++) {
    analogWrite(BUZZER_PIN, 127);
    delay(150);
  }
}

/*---------------------------
      EVENEMENTS
------------------------------*/



void gererCollisions(){
  // Parcours des pièces bonus
  for (size_t i = 0; i < listPieceBonus.size(); ++i){
    int dx = ball.getX() - listPieceBonus[i]->getX();
    int dy = ball.getY() - listPieceBonus[i]->getY();
    if (dx * dx + dy * dy <= (BALLRADIUS + PIECERADIUS) * (BALLRADIUS + PIECERADIUS)){
      listPieceBonus[i]->pieceTouchee();
      delete listPieceBonus[i]; // Libération memoire
      listPieceBonus.erase(listPieceBonus.begin() + i);

      tft.fillCircle(ball.getX(), ball.getY(), BALLRADIUS, TFT_RED); // Redessiner la balle
      break;
    }
  }
  for (size_t i = 0; i < listPieceMalus.size(); ++i){
    int dx = ball.getX() - listPieceMalus[i]->getX();
    int dy = ball.getY() - listPieceMalus[i]->getY();
    if (dx * dx + dy * dy <= (BALLRADIUS + PIECERADIUS) * (BALLRADIUS + PIECERADIUS)){
      listPieceMalus[i]->pieceTouchee();
      delete listPieceMalus[i]; // Libération memoire
      listPieceMalus.erase(listPieceMalus.begin() + i);

      tft.fillCircle(ball.getX(), ball.getY(), BALLRADIUS, TFT_RED); // Redessiner la balle
      
      break;
    }
  }
}

bool checkOutOfTime(){
  return remainingTime <= 0;
}

bool checkGameWin(){
  return listPieceBonus.empty();
  //verifie qu'il y a plus de piece bonus
}

bool checkBorderCollision(){
  if (ball.getY() > 240 - BALLRADIUS || ball.getY() < 55 - BALLRADIUS || ball.getX() > 320 - BALLRADIUS || ball.getX() < BALLRADIUS) return true;
  return false;

}

/*----------------------------------
    DEFINITION DES NIVEAUX
------------------------------------*/

void afficherLevel(int itlvl){
  tft.fillRect(0, 0,110, HAUTEURBANDEAU, TFT_BLUE); // efface ancienne zone
  tft.drawString(level[itlvl], 10, 10); // affiche nouveau niveau
}


void setNvUn(){
  // Enlever precedents
  listPieceBonus.clear();
  listPieceMalus.clear();


  tft.fillRect(0,40, 320, 200, COULEURFOND);
  
  listPieceBonus.push_back(new PieceBonus(220, 160));
  listPieceBonus.push_back(new PieceBonus(60, 140));
  listPieceBonus.push_back(new PieceBonus(110, 80));
  listPieceBonus.push_back(new PieceBonus(75, 200));
  listPieceBonus.push_back(new PieceBonus(290, 70));
  for(PieceBonus* i : listPieceBonus){ 
    i->dessinerPiece();
  }
}

void setNvDeux(){
  
  // Enleveler precedents
  listPieceBonus.clear();
  listPieceMalus.clear();

  
  tft.fillRect(0,40, 320, 200, COULEURFOND); 

  listPieceBonus.push_back(new PieceBonus(20, 50));   
  listPieceBonus.push_back(new PieceBonus(300, 60));  
  listPieceBonus.push_back(new PieceBonus(30, 220));  
  listPieceBonus.push_back(new PieceBonus(290, 210)); 
  listPieceBonus.push_back(new PieceBonus(160, 55));  

  for(PieceBonus* i : listPieceBonus){ 
    i->dessinerPiece();
  }
}

void setNvTrois(){
  setNvDeux(); // placer les pieces du niveau deux

  listPieceMalus.push_back(new PieceMalus(35, 50));
  listPieceMalus.push_back(new PieceMalus(285, 60));
  listPieceMalus.push_back(new PieceMalus(45, 220));
  listPieceMalus.push_back(new PieceMalus(275, 210));
  listPieceMalus.push_back(new PieceMalus(175, 55));
  for(PieceMalus* i : listPieceMalus){
    i->dessinerPiece();
  }
}

  

/*----------------------------------
    SETUP ET BOUCLE DE JEU
---------------------------------------*/

void setup() {
 
  tft.begin();
  tft.setRotation(3); // Définis le coin de l'écran de coordonnées 0,0
  tft.fillScreen(COULEURFOND); // fond gris
  

  lis.begin(Wire1); 
  if (!lis) {
    while(1);
    //erreur detection accèlèromètre

  }
  // Design 
  tft.fillRect(0, 0, 320, HAUTEURBANDEAU, TFT_BLUE); // bandeau
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  afficherLevel(itModeJeu); //affihe niveau de difficulté
  afficherScore(); //affiche le score
  tft.drawString("pts", 190, 10); // affiche "pts"
  tft.drawNumber(30, 290, 10); // affiche chrono
  setNvUn(); //affichage du premier niveau
  tft.fillCircle(160, 120, BALLRADIUS, TFT_RED);

  //setup buzzer
  pinMode(WIO_BUZZER, OUTPUT);

  lis.setOutputDataRate(LIS3DHTR_DATARATE_50HZ); 
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G);
  //setup chrono
  //pour interruption appui bouton du centre
  attachInterrupt(digitalPinToInterrupt(WIO_KEY_C), changerModeInterrupt, FALLING);
  // pour interruption appui bouton de gauche
  attachInterrupt(digitalPinToInterrupt(WIO_KEY_B), lancerPartieInterrupt, FALLING);
  delay(100);
}


void loop() {
  //ici in switch case avec les differents états du jeu
  switch(EtatActuel){

    case MENU:{

      if(C_pressed==1){

        // rotation mode de jeu
        if(itModeJeu==2){
          itModeJeu=0;
        }
        else itModeJeu++;
        afficherLevel(itModeJeu);
        switch(itModeJeu){
          case 0:
            setNvUn();
            break;

          case 1:
            setNvDeux();
            break;

          case 2:
            setNvTrois();
            break;
        }
        
        tft.fillCircle(160, 120, BALLRADIUS, TFT_RED);
        C_pressed=0;
      }
      else if(B_pressed==1){
        lancerChrono(); //remet le chrono à 30 et démarre le décompte
        afficherScore();
        EtatActuel = EN_JEU;
        B_pressed=0;
        //permet de ne plus prendre en compte les boutons du haut
        detachInterrupt(digitalPinToInterrupt(WIO_KEY_B));
        detachInterrupt(digitalPinToInterrupt(WIO_KEY_C));
        // pour interruption appui bouton de droite
        attachInterrupt(digitalPinToInterrupt(WIO_KEY_A), ajouterTempsInterrupt, FALLING);
        break;
      }
      delay(50);
      break;

      case EN_JEU:{

        if(A_pressed==1 && btnABonusUsed!=true){
          remainingTime+=10;
          PieceBonus::setValeur(5);
          btnABonusUsed=true;
          A_pressed=0;
        }

        float x = lis.getAccelerationX();
        float y = lis.getAccelerationY();
        
        ball.deplacer(x,y);

        afficherChrono();
        gererCollisions();
      
        if(checkBorderCollision()){
          EtatActuel = FIN_PARTIE;
          ball.setScore(0);
          // sonEchec();
          afficherScore();
          
        }

        if (checkOutOfTime()){
          EtatActuel = FIN_PARTIE;
          bipFinChrono();
        }
        if (checkGameWin()){
          if(btnABonusUsed==false){
            ball.setScore(ball.getScore()+remainingTime);
            afficherScore();
          }
          else{
            btnABonusUsed=false;
            PieceBonus::setValeur(10);
          }
          EtatActuel = FIN_PARTIE;
          // melodieVictoire();
          
        }

        delay(50);
        break;
      }
      case FIN_PARTIE:{
        detachInterrupt(digitalPinToInterrupt(WIO_KEY_A));
        delay(1000);
        //reprend en compte les boutons
        attachInterrupt(digitalPinToInterrupt(WIO_KEY_C), changerModeInterrupt, FALLING); 
        attachInterrupt(digitalPinToInterrupt(WIO_KEY_B), lancerPartieInterrupt, FALLING);
        ball.effacer(ball.getX(),ball.getY());
        ball = Ball();
        malusTime=0;
        //réaffiche les pièces
        switch(itModeJeu){
          case 0:
            setNvUn();
            break;

          case 1:
            setNvDeux();
            break;

          case 2:
            setNvTrois();
            break;
        }
        tft.fillCircle(160, 120, BALLRADIUS, TFT_RED);//réaffiche la balle au milieu de l'écran
        EtatActuel = MENU;
        break;
      }
    }
  }
}
