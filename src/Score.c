#include "Score.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int high_score;

void readHighScore()
{
    FILE *fp;
    char line [ 1024 ]; 
    //size_t sz;

    fp = fopen("/home/lcom/labs/proj/src/score", "rb");
    
    if (fp == NULL)
        {
            printf("NAO EXISTE");
            return;
        }

    while(!feof(fp)) 
        {
            fgets(line, 1024, fp);
        }

    high_score=0;

    for (size_t i=0;line[i]!='-';i++)
    {
        if(line[i]>='0'&&line[i]<='9')
            {
                high_score*=10;

                high_score+= (line[i] - '0');
            }
    }
    
    fclose(fp);

    return ;
}

void store_information(int score, int second, int minute, int hour, int day, int month, int year)
{
    FILE *fp;

    fp = fopen("/home/lcom/labs/proj/src/score", "a");
    
    if (fp == NULL)
        {
            printf("NAO EXISTE");
            return;
        }
    
    fprintf(fp,"\nScore:  %d points      Date:   %d/%d/%d  Hour:   %d:%d:%d\n",score,day,month,year,hour,minute,second);
    fprintf(fp,"////////////////////\n");
    fprintf(fp,"Highscore:  %d-points",high_score);

    fclose(fp);

    return ;
}

int find_number_of_digits(int score)
{
    if(score<10)
        return 1;
        else if(score<100)
            return 2;
                else if(score<1000)
                    return 3;
                        else if(score<10000)
                            return 4;
                                else if(score<100000)
                                    return 5;
                                        else if(score<1000000)
                                            return 6;
                                                else if(10000000)
                                                    return 7;

    return 0; //in case of error
}

void only_store_score(int score, int second, int minute, int hour, int day, int month, int year)
{
    FILE *fp;

    fp = fopen("/home/lcom/labs/y/score", "a");
    
    if (fp == NULL)
        {
            //printf("NAO EXISTE");
            return;
        }
    
    fprintf(fp,"\nScore:  %d points      Date:   %d/%d/%d  Hour:   %d:%d:%d\n",score,day,month,year,hour,minute,second);
    
    fclose(fp);

    return ;
}
