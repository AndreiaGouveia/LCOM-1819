#define DELAY_US 20000

#define BIT(n) (0x01 << (n))

#define KBD_IRQ 1

#define MOUSE_IRQ 12

#define RTC_IRQ 8

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////KEYS/////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define ESC 0x81

#define ENTER_MAKE 0x1c

#define up 0xC8

#define down 0XD0

#define right 0xCD

#define left 0xCB

////////////////////////////////////////////////////////////////////////////
///////////////////////////////////MAKS/////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define MASK_INT 0x01

#define mask_make 0x80
////////////////////////////////////////////////////////////////////////////
//////////////////////////////COMMANDS AND PORTS////////////////////////////
////////////////////////////////////////////////////////////////////////////
#define IN_BUF 0x60

#define OUT_BUF 0x60

#define STAT_REG 0x64

#define CMD_BYTE 0x20

#define KBC_CMD_REG 0x64

#define CMD_STREAM_MODE 0xEA

#define CMD_READ_DATA 0XEB

#define CMD_DISABLE_DATA 0xF5

#define CMD_ENABLE_DATA 0xF4

#define CMD_MOUSE_DISABLE 0xFD

#define CMD_MOUSE_ENABLE 0xFF

#define SET_REMOTE_MODE 0xF0

#define SET_STREAM_MODE BIT(1)

#define MOUSE_COMMAND 0xD4

#define MOUSE_ENABLE 0xA8

#define MOUSE_DISABLE 0xA7

#define WRITE_COMMAND_BYTE 0x60

#define RTC_ADRR_REG 0x70

#define RTC_DATA_REG 0x71

#define SECONDS 0x00

#define MINUTES 0x02

#define HOURS 0x04

#define DATE_OF_MONTH 0x07

#define MONTH 0x08

#define YEAR 0x09
//////////////////////////////////////1///////////////////////////////////////
//////////////////////////////STATUS REGISTER////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define PAR_ERR BIT(7)

#define TO_ERR BIT(6)

#define OBF BIT(0)

#define IBF BIT(1)
/////////////////////////////////////////////////////////////////////////////
//////////////////////////////PACKET VERIFICATION////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define ACK 0xFA //if everything OK

#define NACK 0xFE //if invalid byte (may be because of a serial communication error)

#define ERROR 0xFC //second consecutive invalid byte

#define LB BIT(0)
#define RB BIT(1)
#define MB BIT(2)
#define PACK_FIRST BIT(3)
#define X_SIG BIT(4)
#define Y_SIG BIT(5)
#define X_OVF BIT(6)
#define Y_OVF BIT(7)
#define COMPL 0xFF00

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////COLORS/////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define selection_red 0x600000
#define backround_blue 0x50505080
#define black 0
#define white 0xFFFFFFFF
#define grey 0xAAAAAAAA
#define yellow 0xCCCCC00
#define pink 0xCCCC6666
#define green 0x25000
#define red 0xCCCC0000
#define blue 0x250

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////OTHERS//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define RTC_FAIL 70;

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////UART///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define COM1 0x3F8
#define COM2 0x2F8

#define COM1_IRQ 4

#define RBR 0
#define THR 0
#define DDL 0
#define DLM 1
#define IER 1
#define IIR 2
#define FCR 2
#define LCR 3
#define LSR 5

//IER
#define RD_EN BIT(0)          /**< @brief Enables the Received Data Available Interrupt */
#define THR_EN BIT(1)         /**< @brief Enables the Transmitter Holding Register Empty Interrupt */
#define RLS_EN BIT(2)         /**< @brief Enables the Receiver Line Status Interrupt This event is generated when there is a change in the state of bits 1 to 4, i.e. the error bits, of the LSR */
#define THR_EMP BIT(5)        /**< @brief When set, means that the UART is ready to accept a new character for transmitting */
#define RLS (BIT(1) | BIT(2)) /**< @brief Receiver Line Status Interrupt pending prioritized */
#define RD BIT(2)             /**< @brief Recived Data Available Interrupt pending prioritized */

#define SER_LSR 5
#define SER_DATA 0
#define SER_TX_RDY (1 << 5)
