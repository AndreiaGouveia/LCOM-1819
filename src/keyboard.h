#include <lcom/lcf.h>

/**
 * @brief keyboard interuption handler
 * 
 * 
 */
void(kbc_ih)(); 
/**
 * @brief subscribes the keyboard interupts
 * 
 * @param bit_no bit number
 * @return int  if everything is ok
 */
int keyboard_subscribe(uint8_t *bit_no);
/**
 * @brief   subscribes the keyboard interupts 
 * 
 * @return int  if everything is ok 
 */
int keyboard_unsubscribe();
/**
 * @brief says if the scan code is the a make code or a break code
 * 
 * @return true is the scan code is a make code
 * @return false is the scan code is a break code
 */
bool is_make();
/**
 * @brief converts the scan code to ASCI
 * 
 * @return int  if everyone went ok
 */
int ScanToAsc();
/**
 * @brief Draws scan codes
 * 
 * @param scan  scan code
 * @param xmemory 
 * @param ymemory 
 */
void ScanToDraw(uint8_t scan, int xmemory, int ymemory);
