#include "Mouse.h"
#include "macro.h"
#include <lcom/lcf.h>

int hook_ID_M = 12;
int index_ = 0;
state_t status_ = INIT;
struct packet packet_array;

uint8_t p1;
int counter = 0; //packet counter
bool ok;

int subscribe_mouse_interupts(uint8_t *bit_no)
{

  *bit_no = hook_ID_M;

  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_ID_M) != OK)
    return 1;

  return 0;
}

int unsubscribe_mouse_interupts()
{

  if (sys_irqrmpolicy(&hook_ID_M) != OK)

    return 1;

  return 0;
}

int write_command_argument(uint32_t command, uint32_t argumment)
{
  uint32_t acknowledgment_byte = 0;
  //int response = -1;
  for (int i = 0; i < 3; i++)
  {
    if (issue_command_kbc(command) != OK)
    {
      return 1;
    }
    if (issue_argument_kbc(argumment) != OK)
    {
      return 1;
    }

    sys_inb(STAT_REG, &status_);

    if (status_ & OBF)
    { //If the buffer is full

      sys_inb(OUT_BUF, &acknowledgment_byte);
      if (acknowledgment_byte == ACK) //Everything is ok
      {
        return 0;
      }
      else if (acknowledgment_byte == NACK) //do the while again
      {
      }
      else if (acknowledgment_byte == ERROR) //stop the function
        return 1;
    }
  }
  return 0;
}

int mouse_Enable_Data_Report()
{
  return write_command_argument(MOUSE_COMMAND, CMD_ENABLE_DATA);
}

int mouse_Disable_Data_Report()
{
  return write_command_argument(MOUSE_COMMAND, CMD_DISABLE_DATA);
}

uint32_t read_output_buffer()
{
  uint32_t status;
  for (int i = 0; i < 5; i++)
  {
    if (sys_inb(KBC_CMD_REG, &status) != OK)
      return 1;

    if (status & OBF)
    {
      if (sys_inb(OUT_BUF, &status) != OK)
        return 1;
      else
        return status;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  return 1;
}

//here we will get the packets
void(mouse_ih)()
{

  uint32_t status_;
  uint32_t data_;

  sys_inb(STAT_REG, &status_); //assuming it returns OK

  if (status_ & OBF)
  { //if it is full

    sys_inb(OUT_BUF, &data_); //assuming it returns OK

    if ((status_ & (PAR_ERR | TO_ERR)) == 0)
    {

      p1 = (uint8_t)data_; //converting our information to an uint8_t format

      ok = true;

      counter++; //increment the packet counter
    }
  }
}

int issue_command_kbc(uint8_t command)
{
  uint32_t status_;
  uint8_t status1;
  for (int i = 0; i < 5; i++)
  {
    sys_inb(STAT_REG, &status_);
    status1 = (uint8_t)status_;

    if ((status_ & IBF) == 0)
    {
      if (sys_outb(KBC_CMD_REG, command) != OK)
      {
        return 1;
      }
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  return 1;
}

int issue_argument_kbc(uint8_t argument)
{
  uint32_t status_;
  uint8_t status1;
  for (int i = 0; i < 5; i++)
  {
    sys_inb(STAT_REG, &status_);
    status1 = (uint8_t)status_;

    if ((status_ & IBF) == 0)
    {
      if (sys_outb(IN_BUF, argument) != OK)
      {
        return 1;
      }
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  return 1;
}
