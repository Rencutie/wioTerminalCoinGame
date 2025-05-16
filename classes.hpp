enum EtatJeu { MENU, EN_JEU, FIN_PARTIE };

class PieceBonus{
private:
    static int valeur;
    int x, y;
public :
    void pieceTouchee();
    void dessinerPiece();
    PieceBonus(int x, int y);
    int getX();
    int getY();
    static void setValeur(int val);
};

class PieceMalus{
private:
    int valeur = -10;
    int x, y;
public:
    void pieceTouchee();
    void dessinerPiece();
    PieceMalus(int x, int y);
    int getX();
    int getY();

};

class Ball{
private:
    int ballx = 160;
    int bally = 120;
    int score = 0;
public:
    void deplacer(float x, float y);
    void effacer(float x,float y);
    int getScore();
    void setScore(int nvScore);
    int getX();
    int getY();
};
