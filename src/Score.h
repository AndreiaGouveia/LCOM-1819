#include <lcom/lcf.h>

/**
 * @brief reads the highscore from a file 
 * 
 */
void readHighScore();
/**
 * @brief stores the information of the score and the highscore in a text file 
 * 
 * @param score its the last score that the player got 
 * @param second    gets the second that the score was established 
 * @param minute    gets the minute that the score was established  
 * @param hour  gets the hour that the score was established  
 * @param day   gets the day that the score was established  
 * @param month gets the month that the score was established  
 * @param year  gets the year that the score was established  
 */
void store_information(int score, int second, int minute, int hour, int day, int month, int year);
/**
 * @brief   finds how many digits are in a score 
 * 
 * @param score its the last score that the player got 
 * @return int returns the number of digits 
 */
int find_number_of_digits(int score);
/**
 * @brief   Only stores the score, not the highscore
 * 
 * @param score its the last score that the player got  
 * @param second    gets the second that the score was established 
 * @param minute    gets the minute that the score was established  
 * @param hour  gets the hour that the score was established   
 * @param day   gets the day that the score was established 
 * @param month gets the month that the score was established 
 * @param year  gets the year that the score was established   
 */
void only_store_score(int score, int second, int minute, int hour, int day, int month, int year);
