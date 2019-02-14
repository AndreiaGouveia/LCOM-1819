#include <lcom/lcf.h>

/**
 * @brief struct com
 * 
 */
struct info{
    int seconds;/**< seconds of the date */
    int minutes;/**< minutes of the date */
    int hour;/**< hour of the date */
    int day;/**< day of the date */
    int month;/**< month of the date */
    int year;/**< year of the date */
} date;

/**
 * @brief updates the struct with the correct hours
 * 
 */
void update_hour ();

/**
 * @brief subscribes the rtc
 * 
 * @return int returns if it went well
 */
int subscribe_rtc();

/**
 * @brief   unsubscribes the rtc 
 * 
 * @return int if it went ok
 */
int unsubscribe_rtc();

/**
 * @brief gets the year of the date
 * 
 * @return int returns the year 
 */
int year();

/**
 * @brief gets the year of the date
 * 
 * @return int returns the month 
 */
int month();

/**
 * @brief gets the year of the date
 * 
 * @return int  returns the day
 */
int day();

/**
 * @brief gets the year of the date
 * 
 * @return int  returns the hour 
 */
int hour();

/**
 * @brief gets the year of the date
 * 
 * @return int  returns the minutes  
 */
int minutes();

/**
 * @brief gets the year of the date
 * 
 * @return int  returns the seconds 
 */
int seconds();

void printDate();
