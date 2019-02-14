#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>
#include "words.h"
#include "Mouse.h"
#include "i8254.h"
#include "macro.h"
#include "video.h"
#include "keyboard.h"
#include "rtc.h"
#include "Score.h"

#include "UART.h"
#include "include.h"

//keyboard
extern uint8_t scan_Code;
extern uint32_t cnt;
uint8_t bit_no_keyboard = 1;

//mouse
extern uint8_t p1;
extern int counter;
extern bool ok;
extern struct packet packet_array;
extern state_t status_;
uint8_t bit_no_mouse = 12;

//timer
extern int timerCounter;
uint8_t bit_no_timer = 0;

//video
extern int position;
extern int old_Position;
extern int xp, yp;
extern int indexLoop;
extern int indexBubble;

//words
extern char word_selected[15];
char actualString[28];
bool guess = false;
int score = 0;

//UART
int bit_no_uart = 4;
extern bool wordS;

//SCORE
extern int high_score;

int main(int argc, char *argv[])
{
  lcf_set_language("EN-US");

  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  lcf_log_output("/home/lcom/labs/proj/output.txt");

  if (lcf_start(argc, argv))
    return 1;

  lcf_cleanup();

  return 0;
}
void initial_Mode();

void menu();

bool new_game();

bool continue_game();

bool game();

void draw_bongo();

void show_score();

void wordScreen();

int(proj_main_loop)(int argc, char *argv[])
{
  readHighScore();

  if (keyboard_subscribe(&bit_no_keyboard) != OK)
    return 1;
  if (set_video_mode(0x118))
    return 1;
  if (timer_subscribe_int(&bit_no_timer) != OK)
    return 1;
  initial_Mode();
  if (mouse_Enable_Data_Report() != OK)
    return 1;
  if (subscribe_mouse_interupts(&bit_no_mouse) != OK)
    return 1;

  serial_port_subscribe_int(&bit_no_uart);
  clean_RBR();


  initial_Mode();

  menu();
    while (new_game())
          game(); 

  
  if (subscribe_rtc() != OK)
    return 1;
  if (timer_unsubscribe_int() != OK)
    return 1;
  if (keyboard_unsubscribe() != OK)
    return 1;
  if (unsubscribe_mouse_interupts() != OK)
    return 1;

  if (mouse_Disable_Data_Report() != OK)
    return 1;

  if (vg_exit() != OK)
    return 1;


  update_hour();

  store_information(score, date.seconds, date.minutes, date.hour, date.day, date.month, date.year);

  if (unsubscribe_rtc() != OK)
    return 1;

  serial_port_unsubscribe_int(&bit_no_uart);
  printf("%s(%d,%p):", __func__, argc, argv);
  return 0;
}

void draw_bongo()
{
  int counter = 0;
  int show_times_counter = 0;
  int r;
  int ipc_status;
  message msg;

  high_score_background();
  //already copy the xpm to the background

  bongo_cat_1_draw();

  while (true) //enquanto nao selecionou
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:

        if (msg.m_notify.interrupts & BIT(bit_no_timer))
        {
          timer_int_handler();

          if (timerCounter % 60 == 0)
          {
            if (counter == 0)
            {
              bongo_cat_2_draw();
              counter++;
            }
            else if (counter == 1)
            {
              bongo_cat_1_draw();
              counter--;
            }

            show_times_counter++;
          }

          if (show_times_counter == 4)
          {
            return;
          }
        }

        if (msg.m_notify.interrupts & BIT(bit_no_mouse))
        {
          ok = false;
          mouse_ih();
        }

        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
        {
          kbc_ih();
        }
      }
    }
  }
}

void show_score()
{
  //animation
  bool special_animation = false;

  if (score > high_score)
  {
    high_score = score;
    special_animation = true;
  }

  if (special_animation)
  {
    draw_bongo();
  }

  int number = find_number_of_digits(score);
  drawScore(score, number);

  int r;
  int ipc_status;
  message msg;

  while (true) //enquanto nao selecionou
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:

        if (msg.m_notify.interrupts & BIT(bit_no_timer))
        {
          timer_int_handler();

          if (timerCounter % 240 == 0)
            return;
        }
        if (msg.m_notify.interrupts & BIT(bit_no_mouse))
        {
          ok = false;
          mouse_ih();
        }

        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
        {
          kbc_ih();
        }
      }
    }
  }
}

void initial_Mode()
{
  inicialBackround();
  printDate();

  int r;
  int ipc_status;
  message msg;

  while (true) //enquanto nao selecionou
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
        {
          kbc_ih();

          if (scan_Code == ENTER_MAKE)
          {
            draw_rectangle(462, 620, 217, 10, black);
            return;
          }
        }
        if (msg.m_notify.interrupts & BIT(bit_no_mouse))
          break;
        /*
        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
          {
            kbc_ih();
        }*/
      }
    }
  }
}

void menu()
{
  ////mouse
  short x = 0, y = 0, xOld = 0, yOld = 0;
  struct packet packet_array;
  uint8_t mask_packet = 0x08;
  /////keyboard and graph
  xp = 350;
  yp = 380;

  Menu_draw();
  backround_to_videomem();
  int counter1 = 1;
  bool special_Char = false;
  int r;
  int ipc_status;
  message msg;
  bool first_time = false;

  uint32_t indexW = 0;

  while (true) //enquanto nao selecionou
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & BIT(bit_no_timer))
        {
          timer_int_handler();

          DrawBrushFullScreen((1024 - x), (722 - y));

          framePrint();
        }
        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
        {
          kbc_ih();
          if (scan_Code == 0xe0 || scan_Code == 0xe1)
          {
            special_Char = true;
          }
          else
          {

            if (special_Char)
            {
              if (scan_Code == up)
              {
                if ((counter1 - 1) < 1)
                  counter1 = 1;
                else
                {
                  counter1--;
                  select_Options(scan_Code, 150,300);
                }
              }
              if (scan_Code == down)
              {
                if ((counter1 + 1) > 3)
                  counter1 = 3;
                else
                {
                  counter1++;
                  select_Options(scan_Code, 150,300);
                }
              }
            }
            if (scan_Code == ENTER_MAKE && first_time)
            {
              wordsSelection1(counter1);
              return;
            }
            first_time = true;
          }
        }
        if (msg.m_notify.interrupts & BIT(bit_no_mouse))
        {
          ok = false;
          mouse_ih();
          if (ok)
          {
            if (counter == 1)
            {
              if (p1 & mask_packet)
              {
                packet_array.bytes[0] = p1;
              }
              else
                counter--;
            }
            else if (counter == 2)
            {
              packet_array.bytes[1] = p1;
            }
            else if (counter == 3)
            {
              packet_array.bytes[2] = p1;
              counter = 0;

              packet_array.lb = packet_array.bytes[0] & LB;
              packet_array.mb = packet_array.bytes[0] & MB;
              packet_array.rb = packet_array.bytes[0] & RB;

              packet_array.x_ov = packet_array.bytes[0] & X_OVF;
              packet_array.y_ov = packet_array.bytes[0] & Y_OVF;

              packet_array.delta_x = packet_array.bytes[1];
              packet_array.delta_y = packet_array.bytes[2];

              if (packet_array.bytes[0] & X_SIG)
              {
                packet_array.delta_x |= COMPL;
              }
              if (packet_array.bytes[0] & Y_SIG)
              {
                packet_array.delta_y |= COMPL;
              }

              if (packet_array.delta_x != 0 || packet_array.delta_y != 0) //so that it stops when the mouse isnt doing anything
              {

                x = -packet_array.delta_x;
                y = packet_array.delta_y;

                if (xOld + x < 2) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else if (xOld + x > 1024) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else
                {
                  x += xOld;
                }

                if (yOld + y < 2) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else if (yOld + y > 768) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else
                {
                  y += yOld;
                }
                if (y != yOld || x != xOld)
                {
                  if (x >= 296 && y >= 368 && x <= 749 && y <= 472) //1st option
                  {
                    if ((counter1 - 1) < 1)
                      counter1 = 1;
                    else
                    {
                      if (counter1 == 2)
                      {
                        counter1--;
                        select_Options(up, 150,300);
                      }
                      else if (counter1 == 3)
                      {
                        counter1 = 1;
                        select_Options(left, 150,300);
                      }
                    }
                    if (packet_array.lb == 1)
                    {
                      wordsSelection1(counter1);
                      return;
                    }
                  }

                  if (x >= 296 && y >= 218 && x<=749 && y <= 322) //2nd option
                  {
                    if (counter1 == 2)
                      counter1 = 2;
                    else
                    {
                      if (counter1 == 1)
                      {
                        counter1++;
                        select_Options(down, 150,300);
                      }
                      else if (counter1 == 3)
                      { //counter1 is 3
                        counter1--;
                        select_Options(up, 150,300);
                      }
                    }
                    if (packet_array.lb == 1)
                    {
                      wordsSelection1(counter1);
                      return;
                    }
                  }

                  if (x >= 296 && y >= 74 && x <= 749 && y <= 171) //3rd option
                  {
                    if ((counter1 + 1) > 3) //if already is 3
                      counter1 = 3;
                    else
                    {
                      if (counter1 == 2)
                      {
                        counter1++;
                        select_Options(down, 150,300);
                      }
                      else if (counter1 == 1)
                      {
                        counter1 = 3;
                        select_Options(right, 150,300);
                      }
                    }
                    if (packet_array.lb == 1)
                    {
                      wordsSelection1(counter1);
                      return;
                    }
                  }

                  yOld = y;
                  xOld = x;
                }
              }
              else if ((packet_array.lb == 1) && ((x >= 296 && y >= 74 && x <= 749 && y <= 171) || (x >= 296 && y >= 218 && x <= 749 && y <= 322) || (x >= 296 && y >= 368 && x <= 749 && y <= 472)))
              {
                wordsSelection1(counter1);
                return;
              }
            }
          }
        }
        if (!wordS)
        {
          if (msg.m_notify.interrupts & BIT(bit_no_uart))
          {
            uart_handler(&indexW);
          }
        }
      }
    }
  }
}

bool new_game()
{
  ////mouse
  short x = 0, y = 0, xOld = 0, yOld = 0;
  struct packet packet_array;
  uint8_t mask_packet = 0x08;
  /////keyboard and graph
  xp = 400;
  yp = 300;
  new_game_draw();

  int counter1 = 1;
  bool special_Char = false;
  int r;
  int ipc_status;
  message msg;
  bool first_time = false;

  while (true) //enquanto nao selecionou
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & BIT(bit_no_timer))
        {
          timer_int_handler();

          DrawBrushFullScreen((1024 - x), (722 - y));

          framePrint();
        }
        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
        {
          kbc_ih();
          if (scan_Code == 0xe0 || scan_Code == 0xe1)
          {
            special_Char = true;
          }
          else
          {

            if (special_Char)
            {
              if (scan_Code == up)
              {
                if ((counter1 - 1) < 1)
                  counter1 = 1;
                else
                {
                  counter1--;
                  select_Options(scan_Code, 385,300);
                }
              }
              if (scan_Code == down)
              {
                if ((counter1 + 1) > 2)
                  counter1 = 2;
                else
                {
                  counter1++;
                  select_Options(scan_Code, 385,300);
                }
              }
            }
            if (scan_Code == ENTER_MAKE && first_time)
            {
              if (counter1 == 1)
              {
                wordsSelection1(counter1);
                word();
                DisplayWord();
                return true;
              }
              else
                return false;
            }

            first_time = true;
          }
        }
        if (msg.m_notify.interrupts & BIT(bit_no_mouse))
        {
          ok = false;
          mouse_ih();
          if (ok)
          {
            if (counter == 1)
            {
              if (p1 & mask_packet)
              {
                packet_array.bytes[0] = p1;
              }
              else
                counter--;
            }
            else if (counter == 2)
            {
              packet_array.bytes[1] = p1;
            }
            else if (counter == 3)
            {
              packet_array.bytes[2] = p1;
              counter = 0;

              packet_array.lb = packet_array.bytes[0] & LB;
              packet_array.mb = packet_array.bytes[0] & MB;
              packet_array.rb = packet_array.bytes[0] & RB;

              packet_array.x_ov = packet_array.bytes[0] & X_OVF;
              packet_array.y_ov = packet_array.bytes[0] & Y_OVF;

              packet_array.delta_x = packet_array.bytes[1];
              packet_array.delta_y = packet_array.bytes[2];

              if (packet_array.bytes[0] & X_SIG)
              {
                packet_array.delta_x |= COMPL;
              }
              if (packet_array.bytes[0] & Y_SIG)
              {
                packet_array.delta_y |= COMPL;
              }

              if (packet_array.delta_x != 0 || packet_array.delta_y != 0) //so that it stops when the mouse isnt doing anything
              {

                x = -packet_array.delta_x;
                y = packet_array.delta_y;

                if (xOld + x < 2) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else if (xOld + x > 1024) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else
                {
                  x += xOld;
                }

                if (yOld + y < 2) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else if (yOld + y > 800) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else
                {
                  y += yOld;
                }

                if (y != yOld || x != xOld)
                {

                  if (x >= 63 && y >= 417 && x <= 924 && y <= 646)
                  {
                    if ((counter1 - 1) < 1)
                      counter1 = 1;
                    else
                    {
                      counter1--;
                      select_Options(up, 385,300);
                    }
                    if (packet_array.lb == 1)
                    {
                      return true;
                    }
                  }

                  if (x >= 238 && y >= 49 && x <= 717 && y <= 257)
                  {
                    if ((counter1 + 1) > 2)
                      counter1 = 2;
                    else
                    {
                      counter1++;
                      select_Options(down, 385,300);
                    }
                    if (packet_array.lb == 1)
                    {
                      return false;
                    }
                  }

                  yOld = y;
                  xOld = x;
                }
              }
              if ((packet_array.lb == 1) && ((x >= 238 && y >= 49 && x <= 717 && y <= 257) || (x >= 63 && y >= 417 && x <= 924 && y <= 646)))
              {

                if (counter1 == 1)
                {
                  score = 0;
                  wordsSelection1(counter1);
                  word();
                  DisplayWord();
                  return true;
                }
                if (counter1 == 2)
                  return false;
              }
            }
          }
        }
      }
    }
  }

  return false;
}

bool continue_game()
{
  /////mouse
  short x = 0, y = 0, xOld = 0, yOld = 0;
  struct packet packet_array;
  uint8_t mask_packet = 0x08;
  /////keyboard and graph

  xp = 385;
  yp = 498;
  continue_draw();

  int counter1 = 1;
  bool special_Char = false;
  int r;
  int ipc_status;
  message msg;
  bool first_time = false;

  while (true) //enquanto nao selecionou
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & BIT(bit_no_timer))
        {
          timer_int_handler();

          backround_to_videomem(); //writes the back ground in video mem

          DrawBrush1Buff((1024 - x), (722 - y)); //shows the brush in video mem
        }
        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
        {
          kbc_ih();
          if (scan_Code == 0xe0 || scan_Code == 0xe1)
          {
            special_Char = true;
          }
          else
          {

            if (special_Char)
            {
              if (scan_Code == up)
              {
                if ((counter1 - 1) < 1)
                  counter1 = 1;
                else
                {
                  counter1--;
                  select_Options(scan_Code, 193,250);
                }
              }
              if (scan_Code == down)
              {
                if ((counter1 + 1) > 2)
                  counter = 2;
                else
                {
                  counter1++;
                  select_Options(scan_Code, 193,250);
                }
              }
            }
            if (scan_Code == ENTER_MAKE && first_time)
            {
              if (counter1 == 1)
                return true;
              return false;
            }

            first_time = true;
          }
        }

        if (msg.m_notify.interrupts & BIT(bit_no_mouse))
        {
          ok = false;
          mouse_ih();
          if (ok)
          {
            if (counter == 1)
            {
              if (p1 & mask_packet)
              {
                packet_array.bytes[0] = p1;
              }
              else
                counter--;
            }
            else if (counter == 2)
            {
              packet_array.bytes[1] = p1;
            }
            else if (counter == 3)
            {
              packet_array.bytes[2] = p1;
              counter = 0;

              packet_array.lb = packet_array.bytes[0] & LB;
              packet_array.mb = packet_array.bytes[0] & MB;
              packet_array.rb = packet_array.bytes[0] & RB;

              packet_array.x_ov = packet_array.bytes[0] & X_OVF;
              packet_array.y_ov = packet_array.bytes[0] & Y_OVF;

              packet_array.delta_x = packet_array.bytes[1];
              packet_array.delta_y = packet_array.bytes[2];

              if (packet_array.bytes[0] & X_SIG)
              {
                packet_array.delta_x |= COMPL;
              }
              if (packet_array.bytes[0] & Y_SIG)
              {
                packet_array.delta_y |= COMPL;
              }

              if (packet_array.delta_x != 0 || packet_array.delta_y != 0) //so that it stops when the mouse isnt doing anything
              {

                x = -packet_array.delta_x;
                y = packet_array.delta_y;

                if (xOld + x < 2) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else if (xOld + x > 1024) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else
                {
                  x += xOld;
                }

                if (yOld + y < 2) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else if (yOld + y > 768) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else
                {
                  y += yOld;
                }

                if (y != yOld || x != xOld)
                {

                  if (x >= 371 && y >= 237 && x <= 659 && y <= 398)
                  {
                    if ((counter1 - 1) < 1)
                      counter1 = 1;
                    else
                    {
                      counter1--;
                      select_Options(up, 193,250);
                    }
                  }

                  if (x >= 369 && y >= 46 && x <= 657 && y <= 206)
                  {
                    if ((counter1 + 1) > 2)
                      counter1 = 2;
                    else
                    {
                      counter1++;
                      select_Options(down, 193,250);
                    }
                  }

                  yOld = y;
                  xOld = x;
                }
              }
              if ((packet_array.lb == 1) && ((x >= 369 && y >= 46 && x <= 657 && y <= 206) || (x >= 371 && y >= 237 && x <= 659 && y <= 398)))
              {
                if (counter1 == 1)
                  return true;
                return false;
              }
            }
          }
        }
      }
    }
  }

  return false;
}
bool game()
{
  //mouse
  short x = 0, y = 0, xOld = 0, yOld = 0;
  struct packet packet_array;
  uint8_t mask_packet = 0x08;

  //graph
  uint32_t color = black;
  xOld = 0;
  yOld = 0;
  int xinicial = 670;
  int yinicial = 675;

  //keyboard
  bool special_Char = false, make;
  int scan_counter = 0;
  uint8_t indexASC[28];
  for (unsigned int i = 0; i < 28; i++)
    actualString[i] = '\0';
  int indexN = 0;
  int indexesP = 0;
  int indexesS = 0;
  int xmemory = xinicial;
  int ymemory = 150;
  //u_char indexW = 0;

  //counter
  int second_counter = 30;
  int minute_counter = 1;

  ok = false;
  int r;
  int ipc_status;
  message msg;

  indexLoop = 0;

  drawColorSelection();
  DrawTimer(minute_counter, second_counter);

  while (scan_Code != ESC)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & BIT(bit_no_mouse))
        {
          ok = false;
          mouse_ih();
          if (ok)
          {
            if (counter == 1)
            {
              if (p1 & mask_packet)
              {
                packet_array.bytes[0] = p1;
              }
              else
                counter--;
            }
            else if (counter == 2)
            {
              packet_array.bytes[1] = p1;
            }
            else if (counter == 3)
            {
              packet_array.bytes[2] = p1;
              counter = 0;

              packet_array.lb = packet_array.bytes[0] & LB;
              packet_array.mb = packet_array.bytes[0] & MB;
              packet_array.rb = packet_array.bytes[0] & RB;

              packet_array.x_ov = packet_array.bytes[0] & X_OVF;
              packet_array.y_ov = packet_array.bytes[0] & Y_OVF;

              packet_array.delta_x = packet_array.bytes[1];
              packet_array.delta_y = packet_array.bytes[2];

              if (packet_array.bytes[0] & X_SIG)
              {
                packet_array.delta_x |= COMPL;
              }
              if (packet_array.bytes[0] & Y_SIG)
              {
                packet_array.delta_y |= COMPL;
              }

              if (packet_array.delta_x != 0 || packet_array.delta_y != 0) //so that it stops when the mouse isnt doing anything
              {

                x = -packet_array.delta_x;
                y = packet_array.delta_y;

                if (xOld + x < 2) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else if (xOld + x > 599) //se ultrapassa o ecra
                {
                  x = xOld;
                }
                else
                {
                  x += xOld;
                }

                if (yOld + y < 2) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else if (yOld + y > 599) //se ultrapassa o ecra
                {
                  y = yOld;
                }
                else
                {
                  y += yOld;
                }

                if (y != yOld || x != xOld)
                {
                  if (packet_array.lb == 1)
                  {
                    if (indexLoop + 50 < y && indexLoop + 50 < yOld)
                    {
                      draw_line(color, x, y, xOld, yOld);
                    }
                  }

                  if (packet_array.rb == true)
                  {
                    if (indexLoop + 80 < y && indexLoop + 80 < yOld){
                      draw_rectangle(600-x,600-y,35,35,0xFFFFFF);
                      backround_to_videomem();
                    }
                  }

                  yOld = y;
                  xOld = x;
                }
              }
            }
          }
        }
        if (msg.m_notify.interrupts & BIT(bit_no_timer))
        {
          timer_int_handler();

          if (second_counter == 0 && minute_counter != 0)
          {
            minute_counter--;
            second_counter = 60;
          }
          if (minute_counter == 0 && second_counter == 0)
          {
            show_score();
            update_hour();
            only_store_score(score, date.seconds, date.minutes, date.hour, date.day, date.month, date.year);
            score=0;
            return true;
          }
          DrawBrush((600 - x), (554 - y));
          if (timerCounter % 60 == 0)
          {
            second_counter--;
            DrawTimer(minute_counter, second_counter);
            indexLoop += 6;
            drawLoop();
          }
          if (timerCounter % 5 == 0)
          {
            indexBubble++;
          }
          framePrint();
        }
        if (msg.m_notify.interrupts & BIT(bit_no_keyboard))
        {
          kbc_ih();
          if (scan_Code == ESC)
          {
            draw_To_background();

            if (continue_game() == false)
            {
              score=0;
              return false;
            }
            
            background_To_draw();

            framePrint();
            continue;
          }
          if (scan_Code == 0xe0 || scan_Code == 0xe1)
          {
            special_Char = true;
            scan_counter++;
          }
          else
          {
            if (special_Char)
            {
              if ((scan_Code == up || scan_Code == down || scan_Code == left || scan_Code == right) && scan_counter > 1)
              {
                color = select_Color(scan_Code);
                scan_counter = 0;
                special_Char = false;
              }
            }
          }
          make = is_make();
          indexN = ScanToAsc();
          if (make)
          {

            if (indexN == 8) //backspace
            {
              if (xinicial != 670)
              {
                xinicial -= 10;
                indexesP--;
                actualString[indexesS] = '\0';
                indexesS--;
                draw_rectangle(xinicial, yinicial, 10, 25, (uint32_t)white);
              }
            }
            else
            {
              if (indexN == 11) //enter
              {
                if (indexesP != 0)
                {
                  if (ymemory > 630)
                  {
                    ymemory = 150;
                  }
                  guess = verifyAnswer();
                  if (!guess)
                    draw_rectangle(660, ymemory, 300, 25, (uint32_t)white);
                  else{//if correct
                    draw_rectangle(660, ymemory, 300, 25, 0x99FF33);
                    word();//chooses a new word
                    draw_To_background();//kepp in the drawing
                    DisplayWord();
                    background_To_draw();//draw to background
                    framePrint();

                  }
                  for (int i = 0; i < indexesP; i++)
                  {
                    ScanToDraw(indexASC[i], xmemory, ymemory);

                    xmemory += 10;
                  }
                  xinicial = 670;
                  xmemory = 670;
                  ymemory += 25;
                  indexesP = 0;
                  indexesS = 0;
                  guess = verifyAnswer();
                  for (unsigned int i = 0; i < 28; i++)
                    actualString[i] = '\0';
                  draw_rectangle(658, 656, 304, 56, (uint32_t)black);
                  draw_rectangle(660, 660, 300, 50, (uint32_t)white);
                }
              }
              else
              {
                if (indexN != 0) //alphabetic char
                {
                  if (xinicial < 950)
                  {
                    indexASC[indexesP] = scan_Code;
                    actualString[indexesS] = ScanToAsc();
                    ScanToDraw(scan_Code, xinicial, yinicial);
                    xinicial += 10;
                    tickdelay(micros_to_ticks(DELAY_US));

                    indexesP++;
                    indexesS++;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return true;
}

void wordScreen()
{
  //mouse
  short xOld, yOld;

  //graph
  xOld = 0;
  yOld = 0;

  uint32_t indexW = 0;
  int irq_uart = BIT(4);

  ok = false;
  int r;
  int ipc_status;
  message msg;

  while (scan_Code != ESC)
  {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    {
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE:
        if (msg.m_notify.interrupts & BIT(bit_no_timer))
        {
          timer_int_handler();
        }
        if (msg.m_notify.interrupts & irq_uart)
        {
          uart_handler(&indexW);
        }
      }
    }
  }
}
