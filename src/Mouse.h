#include <lcom/lcf.h>

typedef enum { INIT,
               DRAW1,
               WAIT,
               DRAW2,
               COMP } state_t;
typedef enum { RDOWN,
               RUP,
               LDOWN,
               LUP,
               MOVE } ev_type_t;
/**
 * @brief subcribes the mouse interupts
 * 
 * @param bit_no    bit number 
 * @return int  if verything went ok
 */
int subscribe_mouse_interupts(uint8_t *bit_no);
/**
 * @brief   unsubcribes the mouse interupts 
 * 
 * @return int  if verything went ok 
 */
int unsubscribe_mouse_interupts();
/**
 * @brief writes a command with its argumment
 * 
 * @param command 
 * @param argumment 
 * @return int  if verything went ok  
 */
int write_command_argument(uint32_t command,uint32_t argumment);
/**
 * @brief enables data report
 * 
 * @return int  if verything went ok 
 */
int mouse_Enable_Data_Report();
/**
 * @brief disables data report
 * 
 * 
 * @return int  if verything went ok 
 */
int mouse_Disable_Data_Report();
/**
 * @brief reads output buffer
 * 
 * @return uint32_t returns the data from the output buffer
 */
uint32_t read_output_buffer();
/**
 * @brief mouse interuption handler
 * 
 */
void (mouse_ih)();
/**
 * @brief issues a command to the kbc
 * 
 * @param port  command to issue
 * @return int  if everything went ok 
 */
int issue_command_kbc(uint8_t port);
/**
 * @brief issues an argument to the kbc
 * 
 * @param argument  argumment to issue
 * @return int  if everything went ok 
 */
int issue_argument_kbc(uint8_t argument);


