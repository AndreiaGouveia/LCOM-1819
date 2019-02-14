#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include "words.h"

char wordsLevel[20][28];
char word_selected[28];
extern char actualString[28];
int incremento_pontuacao=0;
extern int score;
int indexWord=0;

void wordsSelection1(int dificulty)

{
printf("     %d    ",dificulty);
    switch (dificulty)
    {
    case 1: //easy

printf("hereee");
        strcpy(wordsLevel[0], "TRANSPORT");
        strcpy(wordsLevel[1], "CURLY");
        strcpy(wordsLevel[2], "COOK");
        strcpy(wordsLevel[3], "WING");
        strcpy(wordsLevel[4], "FIVE");
        strcpy(wordsLevel[5], "RAIN");
        strcpy(wordsLevel[6], "BOILING");
        strcpy(wordsLevel[7], "YEAR");
        strcpy(wordsLevel[8], "BLUSH");
        strcpy(wordsLevel[9], "THRISTY");
        strcpy(wordsLevel[10], "ALARM");
        strcpy(wordsLevel[11], "SMILE");
        strcpy(wordsLevel[12], "FLUFFY");
        strcpy(wordsLevel[13], "GAME");
        strcpy(wordsLevel[14], "OCEANIC");
        strcpy(wordsLevel[15], "CORN");
        strcpy(wordsLevel[16], "FACE");
        strcpy(wordsLevel[17], "VISIT");
        strcpy(wordsLevel[18], "DAMP");
        strcpy(wordsLevel[19], "POWDER");

        incremento_pontuacao=2;
        break;

    case 2: //medium

        strcpy(wordsLevel[0], "OBEY");
        strcpy(wordsLevel[1], "FANCY");
        strcpy(wordsLevel[2], "CONFUSE");
        strcpy(wordsLevel[3], "PUMPED");
        strcpy(wordsLevel[4], "COMBATIVE");
        strcpy(wordsLevel[5], "TREAT");
        strcpy(wordsLevel[6], "POST");
        strcpy(wordsLevel[7], "KNIT");
        strcpy(wordsLevel[8], "FURNITURE");
        strcpy(wordsLevel[9], "ADVENTUROUS");
        strcpy(wordsLevel[10], "GAZE");
        strcpy(wordsLevel[11], "DIFFERENT");
        strcpy(wordsLevel[12], "MARK");
        strcpy(wordsLevel[13], "WORRY");
        strcpy(wordsLevel[14], "ABUNDANT");
        strcpy(wordsLevel[15], "ELITE");
        strcpy(wordsLevel[16], "LUCKY");
        strcpy(wordsLevel[17], "MARKET");
        strcpy(wordsLevel[18], "BARBAROUS");
        strcpy(wordsLevel[19], "MELODIC");

        incremento_pontuacao=5;
        break;

    case 3: //hard
        strcpy(wordsLevel[0], "PARSIMONIOUS");
        strcpy(wordsLevel[1], "DISAGREE");
        strcpy(wordsLevel[2], "SQUEAL");
        strcpy(wordsLevel[3], "WHOLESALE");
        strcpy(wordsLevel[4], "DESTROY");
        strcpy(wordsLevel[5], "FAR");
        strcpy(wordsLevel[6], "IGNORE");
        strcpy(wordsLevel[7], "SLEET");
        strcpy(wordsLevel[8], "LIMIT");
        strcpy(wordsLevel[9], "RELY");
        strcpy(wordsLevel[10], "THOUGHTFUL");
        strcpy(wordsLevel[11], "ACOUSTIC");
        strcpy(wordsLevel[12], "TEMPER");
        strcpy(wordsLevel[13], "PLACID");
        strcpy(wordsLevel[14], "VORACIOUS");
        strcpy(wordsLevel[15], "THRUTH");
        strcpy(wordsLevel[16], "LIE");
        strcpy(wordsLevel[17], "EXUBERANT");
        strcpy(wordsLevel[18], "WOEBEGONE");
        strcpy(wordsLevel[19], "INEXPENSIVE");

        incremento_pontuacao=10;
        break;
    }
}

void word()
{
    printf("here");
    srand((unsigned)time(NULL));

    int i = rand() % 20;
    indexWord=i;
    printf("%d\n",indexWord);
    strcpy(word_selected, wordsLevel[i]);
    printf("%s",wordsLevel[i]);
}

bool verifyAnswer()
{
    for (unsigned int i = 0; i < 28; i++)
    {
        if (word_selected[i] != actualString[i])
            return false;
        else if (word_selected[i] == '\0' && actualString[i] == '\0'){
            score+=incremento_pontuacao;
            return true;
        }
    }
    return true;
}
