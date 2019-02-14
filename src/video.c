#include "video.h"
#include "math.h"
#include "macro.h"
#include "Pictionary.xpm"
#include "UART.h"

#include "include.h"

static char *video_mem; /* Process (virtual) address to which VRAM is mapped */
static char *drawing_buf;
static char *background_buf;
static unsigned h_res;          /* Horizontal resolution in pixels */
static unsigned v_res;          /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */
vbe_mode_info_t vmip;

extern uint16_t xposition, yposition, xOld, yOld;
uint32_t ColorArray[16];
uint32_t colors[8] = {black, white, grey, yellow, pink, green, red, blue};
int position;
int old_Position;

extern char word_selected[28];
extern int indexWord;

int indexLoop = 0;
int indexBubble = 0;

bool wordS;

int xp, yp;

int(vbe_get_mode_info__)(uint16_t mode)
{
  mmap_t temp_map;
  struct reg86u reg;

  memset(&reg, 0, sizeof(reg)); /* zero the structure */

  if (lm_init(true) == NULL)
    return 1;

  if (lm_alloc(sizeof(vbe_mode_info_t), &temp_map) == NULL)
    return 1;

  reg.u.w.ax = 0x4F01;
  reg.u.w.es = PB2BASE(temp_map.phys);
  reg.u.w.di = PB2OFF(temp_map.phys);
  reg.u.w.cx = mode;
  reg.u.b.intno = 0x10;

  if (sys_int86(&reg) != OK)
  {
    return 1;
  }

  vmip = *(vbe_mode_info_t *)temp_map.virt;

  if (lm_free(&temp_map) == 0)
    return 1;

  return 0;
}
int set_video_mode(uint16_t mode)
{

  if (vbe_get_mode_info__(mode) != OK)
  {
    return 1;
  }

  int r;
  struct minix_mem_range mr; /*physical memory range*/
  unsigned int vram_base;    /*VRAM’s physical addresss*/
  unsigned int vram_size;    /*VRAM’s size, but you can use the frame-buffer size, instead*/

  vram_base = vmip.PhysBasePtr;
  /*Allow memory mapping*/

  //getting vram size
  vram_size = (vmip.BitsPerPixel * vmip.XResolution * vmip.YResolution) / 8;

  //setting the static variables
  h_res = vmip.XResolution;
  v_res = vmip.YResolution;
  bits_per_pixel = vmip.BitsPerPixel;

  mr.mr_base = (phys_bytes)vmip.PhysBasePtr;
  mr.mr_limit = mr.mr_base + vram_size;

  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r); /*Map memory*/

  video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size);
  drawing_buf = (char *)malloc(h_res * v_res * bits_per_pixel / 8);
  background_buf = (char *)malloc(h_res * v_res * bits_per_pixel / 8);

  if (video_mem == MAP_FAILED)
    panic("couldn’t map video memory");

  //set the mode
  struct reg86u reg;

  memset(&reg, 0, sizeof(reg)); /* zero the structure */

  reg.u.w.ax = 0x4F02;         // VBE call, function 02 -- set VBE mode
  reg.u.w.bx = 1 << 14 | mode; // set bit 14: linear framebuffer
  reg.u.b.intno = 0x10;

  if (sys_int86(&reg) != OK)
  {
    printf("set_vbe_mode: sys_int86() failed \n");
    return 1;
  }
  if (reg.u.w.ax != 0x004F)
    return 1;

  return 0;
}

void set_pixel(uint16_t x, uint16_t y, uint32_t color)
{
  uint8_t num_bytes = (bits_per_pixel / 8) + (bits_per_pixel % 8 > 0 ? 1 : 0);
  memcpy(background_buf + (h_res * y + x) * num_bytes, &color, num_bytes);
}

void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
  uint16_t current_X = x;
  uint16_t current_Y = y;

  for (; current_Y < (y + height); current_Y++)
  {
    for (; current_X < (x + width); current_X++)
    {
      set_pixel(current_X, current_Y, color);
    }
    current_X = x;
  }
}
int DrawMap(char *map, int width, int height, uint16_t x, uint16_t y)
{
  uint32_t tc = xpm_transparency_color(XPM_8_8_8);
  unsigned long color = 0;
  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      memcpy(&color, &map[(j * width + i) * 3], 3);
      if (color != tc)
        set_pixel(x + i, y + j, color);
    }
  }
  return 0;
}
uint32_t determineColour(uint8_t no_rectangles, uint32_t first, uint8_t step, uint8_t row, uint8_t col)
{
  uint8_t num_bytes = (bits_per_pixel / 8) + (bits_per_pixel % 8 > 0 ? 1 : 0);

  if (num_bytes == 1)
  {
    printf("\n bits_per_pixel= %d\n", bits_per_pixel);
    return (first + (row * no_rectangles + col) * step) % (1 << bits_per_pixel); //index
  }
  else
  {

    uint32_t mask_Color = 0, B, G, indiferent_bits;
    uint8_t R = 0;

    indiferent_bits = 32 - (vmip.GreenMaskSize + vmip.RedMaskSize + vmip.BlueMaskSize);

    R = first << (indiferent_bits);
    R = R >> (vmip.GreenMaskSize + vmip.BlueMaskSize);

    G = first << (indiferent_bits + vmip.RedMaskSize);
    G = G >> (indiferent_bits + vmip.RedMaskSize + vmip.BlueMaskSize);

    B = first << (indiferent_bits + vmip.RedMaskSize + vmip.GreenMaskSize);
    B = B >> (indiferent_bits + vmip.RedMaskSize + vmip.GreenMaskSize);

    uint32_t mask_Color1 = 0, mask_Color2 = 0, mask_Color3 = 0;

    mask_Color1 = (R + col * step) % (1 << vmip.RedMaskSize);
    mask_Color1 = mask_Color1 << (vmip.RedFieldPosition);

    mask_Color2 = (G + row * step) % (1 << vmip.GreenMaskSize);
    mask_Color2 = mask_Color2 << (vmip.GreenFieldPosition);

    mask_Color3 = (B + (col + row) * step) % (1 << vmip.BlueMaskSize);
    mask_Color3 = mask_Color3 << (vmip.BlueFieldPosition);

    mask_Color = mask_Color1 | mask_Color2 | mask_Color3;
    printf("Mask color: %x \n", mask_Color);
    return mask_Color;
  }
}
void drawPattern(uint8_t no_rectangles, uint32_t first, uint8_t step)
{
  uint16_t y = 0, x = 0;
  uint32_t color;

  uint16_t size_h = h_res / no_rectangles;
  uint16_t size_y = v_res / no_rectangles;

  printf("tamanho horizontal: %d                tamanho vertical: %d \n", size_h, size_y);

  for (int i = 0; i < no_rectangles; i++) //vertical
  {

    for (int j = 0; j < no_rectangles; j++) //horizontal
    {

      color = determineColour(no_rectangles, first, step, i, j);

      draw_rectangle(x, y, size_h, size_y, color);
      x += size_h;
    }
    x = 0;

    y += size_y;
  }
}
int get_mode_controller()
{
  //making the signature
  // struct VbeInfoBlock *controller;

  //POR VBE2

  void *adress = lm_init(true);
  if (adress == NULL)
    return 1;

  //changing mode

  mmap_t temp_map;
  struct reg86u reg;
  vg_vbe_contr_info_t m;

  memset(&reg, 0, sizeof(reg)); /* zero the structure */

  struct VbeInfoBlock *vbe = lm_alloc(sizeof(struct VbeInfoBlock *), &temp_map);
  if (vbe == NULL)
    return 1;

  phys_bytes mm;
  mm = temp_map.phys;
  reg.u.w.ax = 0x4F01;
  reg.u.w.es = PB2BASE(temp_map.phys);
  reg.u.w.di = PB2OFF(temp_map.phys);
  reg.u.b.intno = 0x10;

  if (sys_int86(&reg) != OK)
  {
    return 1;
  }

  //put vbe2 in signatures

  m.VBESignature[0] = 'V';
  m.VBESignature[1] = 'B';
  m.VBESignature[2] = 'E';
  m.VBESignature[3] = '2';

  if (lm_free(&temp_map) == 0)
    return 1;

  if (vg_display_vbe_contr_info(&m) != OK)
    return 1;

  return 0;
}
Sprite *create_sprite(const char *xpm[], int x, int y, int xspeed, int yspeed)
{
  Sprite *sp = (Sprite *)malloc(sizeof(Sprite));
  sp->map = read_xpm(xpm, &sp->width, &sp->height);

  if (sp == NULL)
    return NULL;
  if (sp->map == NULL)
    return NULL;

  sp->x = x;
  sp->y = y;
  sp->xspeed = xspeed;
  sp->yspeed = yspeed;
  return sp;
}
int get_Quantity_Y(uint16_t x, int declive, int b)
{
  return round(declive * x + b);
}
int get_Quantity_X(uint16_t y, int declive, int b)
{
  return round((y - b) / declive);
}
void draw_line(uint32_t color, uint16_t x, uint16_t y, uint16_t xOld, uint16_t yOld)
{
  /*printf("/////////////////");
  printf("x %d  y %d\n",x,y);
  printf("xold %d  yold %d\n",xOld,yOld);
  int tempy;
  int tempx;
  int declive;
  bool dec = false; //se e uma linha vertical

  if ((xOld - x) == 0)
  {
    printf("herreeeeererr");
    declive = 0;
    dec = true;
  }
  else
    {
      declive = round((yOld - y) / (xOld - x));
    }

  printf("declive: %d",declive);
  int b = -declive * xOld + yOld;

  printf("B: %d",b);
*/
  uint16_t xCurrent, yCurrent;

  xCurrent = xOld;
  yCurrent = yOld;

  /* if (declive != 0)
  {

    while (xCurrent != x) //goes through all of x
    {

      tempy = get_Quantity_Y(xCurrent, declive, b);
      printf("Ytemp: %d \n",tempy);
      set_pixel((599 - xCurrent), (599 - tempy), color);
      set_pixel((599 - xCurrent-1), (599 - tempy), color);
       set_pixel((599 - xCurrent), (599 - tempy-1), color);
      set_pixel((599 - xCurrent + 1), (599 - tempy), color);
      set_pixel((599 - xCurrent), (599 - tempy + 1), color);
      if (xCurrent < x)
        xCurrent++;
      else
        xCurrent--;
    }

    while (yCurrent != y)
    {

      tempx = get_Quantity_X(yCurrent, declive, b);
      set_pixel((599 - tempx), (599 - yCurrent), color);
      set_pixel((599 - tempx-1), (599 - yCurrent), color);
      set_pixel((599 - tempx), (599 - yCurrent-1), color);
      set_pixel((599 - tempx-1), (599 - yCurrent-1), color);
      set_pixel((599 - tempx + 1), (599 - yCurrent), color);
      set_pixel((599 - tempx), (599 - yCurrent + 1), color);
      set_pixel((599 - tempx+1), (599 - yCurrent+1), color);
      if (yCurrent < y)
        yCurrent++;
      else
        yCurrent--;
    }
    //  printf("EXITED DRAW\n");//printf("EXITED DRAW\n");
  }
  else if (!dec)
  //this means that it is a horizontal line because y is always the same
  {
    //  printf("--Doin X CURRENT\n");

    if(yOld==y)
    {
      while (xCurrent != x) //goes through all of x
      {

        set_pixel((599 - xCurrent), (599 - yOld), color);
        set_pixel((599 - xCurrent + 1), (599 - yOld), color);
        set_pixel((599 - xCurrent), (599 - yOld + 1), color);
        set_pixel((599 - xCurrent + 1), (599 + yOld + 1), color);
        set_pixel((599 - xCurrent + 1), (599 + yOld), color);
        set_pixel((599 - xCurrent - 1), (599 - yOld - 1), color);
        set_pixel((599 - xCurrent - 1), (599 - yOld), color);
        set_pixel((599 - xCurrent), (599 - yOld - 1), color);
      
        if (xCurrent < x)
          xCurrent++;
        else
          xCurrent--;
      }
    }
    else //DECLIVE MENOR QUE ZERO
    {*/

  ///MY CODE TAKE TWOOO
  while (xCurrent != x || yCurrent != y) //goes through all of x
  {
    if (xCurrent < x)
    {
      if (abs(xCurrent - x) == abs(yCurrent - y)) //y goes up
      {
        if ((yCurrent - y) != 0)
        {

          set_pixel((599 - xCurrent), (599 - yCurrent), color);
          set_pixel((599 - xCurrent - 1), (599 - yCurrent), color);
          set_pixel((599 - xCurrent), (599 - yCurrent - 1), color);
          set_pixel((599 - xCurrent - 1), (599 - yCurrent - 1), color);
          set_pixel((599 - xCurrent + 1), (599 - yCurrent), color);
          set_pixel((599 - xCurrent), (599 - yCurrent + 1), color);
          set_pixel((599 - xCurrent + 1), (599 - yCurrent + 1), color);

          xCurrent++;
          if (yCurrent < y)
          {
            yCurrent++;
          }
          else
          {
            yCurrent--;
          }
        }
      }
      else if (abs(xCurrent - x) > abs(yCurrent - y)) //only change x
      {
        set_pixel((599 - xCurrent), (599 - yCurrent), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent + 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent + 1), color);
        xCurrent++;
      }
      else if (abs(xCurrent - x) < abs(yCurrent - y)) //only change y
      {
        set_pixel((599 - xCurrent), (599 - yCurrent), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent + 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent + 1), color);
        if ((yCurrent - y) != 0)
        {
          if (yCurrent < y)
            yCurrent++;
          else
          {
            yCurrent--;
          }
        }
      }
    }

    else if (xCurrent > x)
    {
      if (abs(xCurrent - x) == abs(yCurrent - y)) //y goes up
      {
        if ((yCurrent - y) != 0)
        {

          set_pixel((599 - xCurrent), (599 - yCurrent), color);
          set_pixel((599 - xCurrent - 1), (599 - yCurrent), color);
          set_pixel((599 - xCurrent), (599 - yCurrent - 1), color);
          set_pixel((599 - xCurrent - 1), (599 - yCurrent - 1), color);
          set_pixel((599 - xCurrent + 1), (599 - yCurrent), color);
          set_pixel((599 - xCurrent), (599 - yCurrent + 1), color);
          set_pixel((599 - xCurrent + 1), (599 - yCurrent + 1), color);

          xCurrent--;
          if (yCurrent < y)
          {
            yCurrent++;
          }
          else
          {
            yCurrent--;
          }
        }
      }
      else if (abs(xCurrent - x) > abs(yCurrent - y)) //only change x
      {
        set_pixel((599 - xCurrent), (599 - yCurrent), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent + 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent + 1), color);
        xCurrent--;
      }
      else if (abs(xCurrent - x) < abs(yCurrent - y)) //only change y
      {
        set_pixel((599 - xCurrent), (599 - yCurrent), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent - 1), (599 - yCurrent - 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent), color);
        set_pixel((599 - xCurrent), (599 - yCurrent + 1), color);
        set_pixel((599 - xCurrent + 1), (599 - yCurrent + 1), color);
        if ((yCurrent - y) != 0)
        {
          if (yCurrent < y)
            yCurrent++;
          else
          {
            yCurrent--;
          }
        }
      }
    }

    else if (xCurrent == x)
    {
      set_pixel((599 - xCurrent), (599 - yCurrent), color);
      set_pixel((599 - xCurrent - 1), (599 - yCurrent), color);
      set_pixel((599 - xCurrent), (599 - yCurrent - 1), color);
      set_pixel((599 - xCurrent - 1), (599 - yCurrent - 1), color);
      set_pixel((599 - xCurrent + 1), (599 - yCurrent), color);
      set_pixel((599 - xCurrent), (599 - yCurrent + 1), color);
      set_pixel((599 - xCurrent + 1), (599 - yCurrent + 1), color);
      if ((yCurrent - y) != 0)
      {
        if (yCurrent < y)
          yCurrent++;
        else
        {
          yCurrent--;
        }
      }
    }
  }
  //////END OF MY CODE TAKE 2

  ////MY CODEEEEEEEEEEEEEEEEEEEEEEEEE
  /* while (xCurrent != x || yCurrent != y) //goes through all of x
      {

        if (xCurrent < x)
          {
            if(abs(xCurrent-x)>=2)
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                xCurrent++;
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                xCurrent++;
              }
            else 
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                xCurrent++;
              }
          }

        else if(xCurrent > x)
          {
            if(abs(xCurrent-x)>=2)
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                xCurrent--;
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                xCurrent--;
              }
            else 
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                xCurrent--;
              }
          }

        if (yCurrent < y)
          {
            if(abs(yCurrent-y)>=2)
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                yCurrent++;
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                yCurrent++;

                if((xCurrent-x)!=0)
                  {if(xCurrent < x)
                    xCurrent++;
                  else {xCurrent --;}}
              }
            else 
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);set_pixel((599 - xCurrent), (599 - yCurrent), color);
                yCurrent++;

                if((xCurrent-x)!=0)
                  {if(xCurrent < x)
                    xCurrent++;
                  else {xCurrent --;}}
              }
          }
        else
          {
            if(abs(yCurrent-y)>=2)
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                yCurrent--;
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                yCurrent--;

                if((xCurrent-x)!=0)
                  {if(xCurrent < x)
                    xCurrent++;
                  else {xCurrent --;}}
              }
            else 
              {
                set_pixel((599 - xCurrent), (599 - yCurrent), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent-1), (599 - yCurrent-1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent), color);
                set_pixel((599 - xCurrent), (599 - yCurrent+1), color);
                set_pixel((599 - xCurrent+1), (599 - yCurrent+1), color);
                yCurrent--;

                if((xCurrent-x)!=0)
                  {if(xCurrent < x)
                    xCurrent++;
                  else {xCurrent --;}}
              }
          }
      
      }*/

  //////////////////////////////////////ENDDDD OFFF MYYY CODEEE//////////////
  /*
    }
  }

  else if (dec) //vertical line bc x is always the same
  {
    // printf("-- Y CURRENT\n");
    while (yCurrent != y)
    {
      set_pixel((599 - xOld), (599 - yCurrent), color);
      set_pixel((599 - xOld + 1), (599 - yCurrent), color);
      set_pixel((599 - xOld), (599 - yCurrent + 1), color);
      if (yCurrent < y)
        yCurrent++;
      else
        yCurrent--;
    }
  }*/
}

void fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  uint8_t num_bytes = (bits_per_pixel / 8) + (bits_per_pixel % 8 > 0 ? 1 : 0);

  int i = 0;
  uint16_t xi = x;
  uint16_t yi = y;
  for (; x < (xi + width); x++)
  {
    for (; y < (yi + height); y++)
    {
      memcpy(ColorArray + i, video_mem + (h_res * y + x) * num_bytes, num_bytes);
      i++;
    }
    y = yi;
  }
  //printf("%d\n", i);
}
void erase_cursor(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
  uint32_t color = 0;
  int i = 0;
  uint16_t xi = x;
  uint16_t yi = y;
  for (; x < (xi + width); x++)
  {
    for (; y < (yi + height); y++)
    {
      //geting the color of the bytes
      color = ColorArray[i];
      set_pixel(x, y, color);
      i++;
    }
    y = yi;
  }
}

unsigned getHres()
{
  return h_res;
}

unsigned getVres()
{
  return v_res;
}

void drawColorSelection()
{

  //main rectangle
  draw_rectangle(0, 0, 1024, 768, (uint32_t)backround_blue);
  draw_rectangle(0, 0, 602, 602, (uint32_t)black);
  draw_rectangle(0, 0, 600, 600, (uint32_t)white);
  drawLoop();

  //color rectangle
  draw_rectangle(103, 623, 65, 65, (uint32_t)selection_red); //selection square
  draw_rectangle(110, 630, 50, 50, (uint32_t)black);         //black
  draw_rectangle(190, 630, 50, 50, (uint32_t)white);         //white bc why not
  draw_rectangle(270, 630, 50, 50, (uint32_t)grey);          //grey
  draw_rectangle(350, 630, 50, 50, (uint32_t)yellow);        //yellow

  draw_rectangle(110, 710, 50, 50, (uint32_t)pink);  //pink
  draw_rectangle(190, 710, 50, 50, (uint32_t)green); //green
  draw_rectangle(270, 710, 50, 50, (uint32_t)red);   //red
  draw_rectangle(350, 710, 50, 50, (uint32_t)blue);  //blue

  //write box
  draw_rectangle(658, 128, 304, 584, (uint32_t)black);
  draw_rectangle(660, 130, 300, 580, (uint32_t)white);
  draw_rectangle(658, 656, 304, 56, (uint32_t)black);
  draw_rectangle(660, 660, 300, 50, (uint32_t)white);

  xpm_image_t xpm_imageP;
  xpm_load(Pictionary_xpm, XPM_8_8_8, &xpm_imageP);
  DrawMap((char *)xpm_imageP.bytes, 300, 100, 660, 20);
}

void drawLoop()
{
  xpm_image_t xpm_imageB;
  xpm_load(Water_xpm, XPM_8_8_8, &xpm_imageB);

  char *map = (char *)xpm_imageB.bytes;
  int x = indexLoop;
  int y = 550 - indexLoop;
  //indexLoop += 6;

  uint32_t tc = xpm_transparency_color(XPM_8_8_8);
  unsigned long color = 0;
  for (int i = 0; i < xpm_imageB.width; i++)
  {
    for (int j = 0; j < xpm_imageB.height; j++)
    {
      memcpy(&color, &map[(j * xpm_imageB.width + i) * 3], 3);
      if (color != tc)
      {
        uint8_t num_bytes = (bits_per_pixel / 8) + (bits_per_pixel % 8 > 0 ? 1 : 0);
        memcpy(background_buf + (h_res * (y + j) + (x + i) % 600) * num_bytes, &color, num_bytes);
        memcpy(drawing_buf + (h_res * (y + j) + (x + i) % 600) * num_bytes, &color, num_bytes);
      }
    }
  }
  y += 5;
}

void update_selection()
{
  //erase old position
  if (old_Position > 3)
    draw_rectangle(103 + (old_Position % 4) * (80), 707, 65, 55, (uint32_t)backround_blue); //backround color
  else
    draw_rectangle(103 + (old_Position % 4) * (80), 623, 65, 65, (uint32_t)backround_blue); //backround color

  //unselected
  if (old_Position > 3)
    draw_rectangle(110 + (old_Position % 4) * (80), 710, 50, 45, (uint32_t)colors[old_Position]); //old colors
  else
    draw_rectangle(110 + (old_Position % 4) * (80), 630, 50, 50, (uint32_t)colors[old_Position]); //old colors

  //Put new seletion
  if (position > 3)
    draw_rectangle(103 + (position % 4) * (80), 707, 65, 55, (uint32_t)selection_red); //selection square
  else
    draw_rectangle(103 + (position % 4) * (80), 623, 65, 65, (uint32_t)selection_red); //selection square

  //Restore the color
  if (position > 3)
    draw_rectangle(110 + (position % 4) * (80), 710, 50, 45, (uint32_t)colors[position]); //new color
  else
    draw_rectangle(110 + (position % 4) * (80), 630, 50, 50, (uint32_t)colors[position]); //new color

  //updating old position
  old_Position = position;
}

uint32_t select_Color(uint8_t scanCode)
{
  switch (scanCode)
  {
  case up:

    if ((position - 4) >= 0) //else it ignores
    {
      position -= 4;
      update_selection();
      return (uint32_t)colors[position];
    }
  case down:
    if ((position + 4) <= 7)
    {
      position += 4;
      update_selection();
      return (uint32_t)colors[position];
    }
  case right:
    if ((position + 1) <= 7)
    {
      position += 1;
      update_selection();
      return (uint32_t)colors[position];
    }
  case left:
    if ((position - 1) >= 0)
    {
      position -= 1;
      update_selection();
      return (uint32_t)colors[position];
    }
  }
  return (uint32_t)black;
}

void ScanToDraw(uint8_t scan, int xmemory, int ymemory)
{

  xpm_image_t xpm_image;
  switch (scan)
  {
  case /* */ 0x39:
    xpm_load(SPACE_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*0*/ 0x0b:
    xpm_load(ZERO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*1*/ 0x02:
    xpm_load(UM_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*2*/ 0x03:
    xpm_load(DOIS_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*3*/ 0x04:
    xpm_load(TRES_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*4*/ 0x05:
    xpm_load(QUATRO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*5*/ 0x06:
    xpm_load(CINCO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*6*/ 0x07:
    xpm_load(SEIS_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*7*/ 0x08:
    xpm_load(SETE_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*8*/ 0x09:
    xpm_load(OITO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*9*/ 0x0a:
    xpm_load(NOVE_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*A*/ 0x1e:
    xpm_load(A_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*B*/ 0x30:
    xpm_load(B_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*C*/ 0x2e:
    xpm_load(C_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*D*/ 0x20:
    xpm_load(D_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*E*/ 0x12:
    xpm_load(E_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*F*/ 0x21:
    xpm_load(F_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*G*/ 0x22:
    xpm_load(G_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*H*/ 0x23:
    xpm_load(H_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*I*/ 0x17:
    xpm_load(I_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*J*/ 0x24:
    xpm_load(J_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*K*/ 0x25:
    xpm_load(K_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*L*/ 0x26:
    xpm_load(L_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*M*/ 0x32:
    xpm_load(M_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*N*/ 0x31:
    xpm_load(N_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*O*/ 0x18:
    xpm_load(O_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*P*/ 0x19:
    xpm_load(P_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*Q*/ 0x10:
    xpm_load(Q_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*R*/ 0x13:
    xpm_load(R_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*S*/ 0x1f:
    xpm_load(S_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*T*/ 0x14:
    xpm_load(T_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*U*/ 0x16:
    xpm_load(U_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*V*/ 0x2f:
    xpm_load(V_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*W*/ 0x11:
    xpm_load(W_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*X*/ 0x2d:
    xpm_load(X_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*Y*/ 0x15:
    xpm_load(Y_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  case /*Z*/ 0x2c:
    xpm_load(Z_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, xmemory, ymemory);
    break;
  }
}

void IntToDraw(int num, int x, int y)
{
  xpm_image_t xpm_image;
  switch (num)
  {
  case /*0*/ 0:
    xpm_load(ZERO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*1*/ 1:
    xpm_load(UM_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*2*/ 2:
    xpm_load(DOIS_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*3*/ 3:
    xpm_load(TRES_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*4*/ 4:
    xpm_load(QUATRO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*5*/ 5:
    xpm_load(CINCO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*6*/ 6:
    xpm_load(SEIS_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*7*/ 7:
    xpm_load(SETE_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*8*/ 8:
    xpm_load(OITO_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  case /*9*/ 9:
    xpm_load(NOVE_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, 10, 20, x, y);
    break;
  }
}

void DrawTimer(int min, int sec)
{
  xpm_image_t xpm_imageT;
  
  draw_rectangle(1, 1, 53, 20, (uint32_t)white);
  IntToDraw(min / 10, 1, 1);
  IntToDraw(min % 10, 12, 1);

  xpm_load(TWODOTS_xpm, XPM_8_8_8, &xpm_imageT);
  DrawMap((char *)xpm_imageT.bytes, 10, 20, 22, 1);

  IntToDraw(sec / 10, 32, 1);
  IntToDraw(sec % 10, 44, 1);
}

void framePrint()
{
  memcpy(video_mem, drawing_buf, h_res * v_res * bits_per_pixel / 8);
}

int DrawBrush(int16_t x, int16_t y)
{
  memcpy(drawing_buf, background_buf, h_res * v_res * bits_per_pixel / 8);
  xpm_image_t xpm_imageB;
  int width = 46, height = 46;
  xpm_load(Brush_xpm, XPM_8_8_8, &xpm_imageB);
  char *map = (char *)xpm_imageB.bytes;
  uint32_t tc = xpm_transparency_color(XPM_8_8_8);
  unsigned long color = 0;

  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      if ((x + i) > 1 && (x + i) < 600 && (y + j) > 0 && (y + j) < 600)
      {
        memcpy(&color, &map[(j * width + i) * 3], 3);
        if (color != tc)
        {
          uint8_t num_bytes = (bits_per_pixel / 8) + (bits_per_pixel % 8 > 0 ? 1 : 0);
          memcpy(drawing_buf + (h_res * (y + j) + x + i) * num_bytes, &color, num_bytes);
        }
      }
    }
  }
  return 0;
}

int DrawBrushFullScreen(int16_t x, int16_t y)
{
  memcpy(drawing_buf, background_buf, h_res * v_res * bits_per_pixel / 8);
  xpm_image_t xpm_imageB;
  int width = 46, height = 46;
  xpm_load(Brush_xpm, XPM_8_8_8, &xpm_imageB);
  char *map = (char *)xpm_imageB.bytes;
  uint32_t tc = xpm_transparency_color(XPM_8_8_8);
  unsigned long color = 0;

  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      if ((x + i) > 1 && (x + i) < 1024 && (y + j) > 0 && (y + j) < 768)
      {
        memcpy(&color, &map[(j * width + i) * 3], 3);
        if (color != tc)
        {
          uint8_t num_bytes = (bits_per_pixel / 8) + (bits_per_pixel % 8 > 0 ? 1 : 0);
          memcpy(drawing_buf + (h_res * (y + j) + x + i) * num_bytes, &color, num_bytes);
        }
      }
    }
  }
  return 0;
}

void Menu_draw()
{
  xpm_image_t xpm_imageZ;
  xpm_load(Difficulty_level_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 0, 0);
  draw_rectangle(xp, yp, 300, 15, (uint32_t)white);
  backround_to_videomem();
}

void high_score_background()
{
  xpm_image_t xpm_imageZ;
  xpm_load(highscore_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 0, 0);
  draw_To_background();//backround is on draw
}

void bongo_cat_1_draw()
{
  background_To_draw();//now the background has the background
  xpm_image_t xpm_imageZ;
  xpm_load(bongo_cat1_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 799, 169);
  backround_to_videomem();
}

void bongo_cat_2_draw()
{
  background_To_draw();//now the background has the background
  xpm_image_t xpm_imageZ;
  xpm_load(bongo_cat2_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 799, 169);
  backround_to_videomem();
}

void IntToBigDraw(int num, int x, int y)
{
  xpm_image_t xpm_image;
  switch (num)
  {
  case /*0*/ 0:
    xpm_load(Big0_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
    break;
  case /*1*/ 1:
    xpm_load(Big1_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*2*/ 2:
    xpm_load(Big2_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*3*/ 3:
    xpm_load(Big3_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*4*/ 4:
    xpm_load(Big4_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*5*/ 5:
    xpm_load(Big5_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*6*/ 6:
    xpm_load(Big6_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*7*/ 7:
    xpm_load(Big7_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*8*/ 8:
    xpm_load(Big8_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  case /*9*/ 9:
    xpm_load(Big9_xpm, XPM_8_8_8, &xpm_image);
    DrawMap((char *)xpm_image.bytes,  xpm_image.width, xpm_image.height, x, y);
    break;
  }
}

void DrawNumbers(int start,int score)
{
  if(score==0)
  {
     IntToBigDraw(0, start, 417);
    return;
  }
    int temp;
    int x=start;

    temp=score;

    while(temp!=0)
    {
      IntToBigDraw(temp%10, x, 417);
      x+=70;
      temp/=10;
    }
}

void drawScore(int score, int number_of_digits)
{
  //draw backround
  xpm_image_t xpm_imageZ;
  xpm_load(SCORE_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 0, 0);
  backround_to_videomem();

  int middle=627;

  int multiply=number_of_digits/2;

  DrawNumbers(middle-multiply*70,score);
  backround_to_videomem();

}

void backround_to_videomem()
{
  memcpy(video_mem, background_buf, h_res * v_res * bits_per_pixel / 8);
}
void select_Options(uint8_t option, int width_size,int x)
{
  switch (option)
  {

  case down:
    //redrawing old rectangle
    draw_rectangle(xp, yp, x, 15, (uint32_t)black);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)backround_blue);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    //drawing new rectangle
    yp += width_size;
    draw_rectangle(xp, yp, x, 15, (uint32_t)white);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)black);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    backround_to_videomem();
    break;

  case up:
    //redrawing old rectangle
    draw_rectangle(xp, yp, x, 15, (uint32_t)black);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)backround_blue);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    //drawing new rectangle
    yp -= width_size;
    draw_rectangle(xp, yp, x, 15, (uint32_t)white);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)black);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    backround_to_videomem();
    break;
  
  case left://1 to 3
    //redrawing old rectangle
    draw_rectangle(xp, yp, x, 15, (uint32_t)black);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)backround_blue);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    //drawing new rectangle
    yp =yp-width_size-width_size;
    draw_rectangle(xp, yp, x, 15, (uint32_t)white);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)black);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    backround_to_videomem();
    break;
  
  case right://3 to 1
    //redrawing old rectangle
    draw_rectangle(xp, yp, 300, 15, (uint32_t)black);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)backround_blue);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    //drawing new rectangle
    yp =yp+width_size+width_size;
    draw_rectangle(xp, yp, 300, 15, (uint32_t)white);
    /*draw_rectangle(xp, yp, 120, 70, (uint32_t)black);
    draw_rectangle(xp + 10, yp + 10, 100, 50, (uint32_t)white);*/
    backround_to_videomem();
    break;
  }
}

void inicialBackround()
{
  xpm_image_t xpm_imageZ;
  xpm_load(backround_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 0, 0);
  backround_to_videomem();
}

void continue_draw()
{
  
  draw_rectangle(0, 0,1023, 767, 0x0A11BB);
  xpm_image_t xpm_imageZ;
  xpm_load(continue_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 0, 0);
  draw_rectangle(xp, yp, 250, 15, (uint32_t)white);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      

  backround_to_videomem();
}


void new_game_draw()
{
  xpm_image_t xpm_imageZ;
  xpm_load(new_game_xpm, XPM_8_8_8, &xpm_imageZ);
  char *map = (char *)xpm_imageZ.bytes;
  DrawMap(map, xpm_imageZ.width, xpm_imageZ.height, 0, 0);
  draw_rectangle(xp, yp, 300, 15, (uint32_t)white);
  backround_to_videomem();
}

void draw_To_background()
{
  memcpy(drawing_buf, background_buf, h_res * v_res * bits_per_pixel / 8);
}

void background_To_draw()
{
  memcpy(background_buf, drawing_buf, h_res * v_res * bits_per_pixel / 8);
}

void DrawWord(int x, int y)
{
  xpm_image_t xpm_image;
  for (unsigned int i = 0; i < 15; i++)
  {
    switch (word_selected[i])
    {
    case /*A*/ 'A':
      xpm_load(BigA_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*B*/ 'B':
      xpm_load(BigB_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*C*/ 'C':
      xpm_load(BigC_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*D*/ 'D':
      xpm_load(BigD_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*E*/ 'E':
      xpm_load(BigE_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*F*/ 'F':
      xpm_load(BigF_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*G*/ 'G':
      xpm_load(BigG_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*H*/ 'H':
      xpm_load(BigH_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*I*/ 'I':
      xpm_load(BigI_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*J*/ 'J':
      xpm_load(BigJ_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*K*/ 'K':
      xpm_load(BigK_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*L*/ 'L':
      xpm_load(BigL_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*M*/ 'M':
      xpm_load(BigM_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*N*/ 'N':
      xpm_load(BigN_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*O*/ 'O':
      xpm_load(BigO_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*P*/ 'P':
      xpm_load(BigP_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*Q*/ 'Q':
      xpm_load(BigQ_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*R*/ 'R':
      xpm_load(BigR_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*S*/ 'S':
      xpm_load(BigS_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*T*/ 'T':
      xpm_load(BigT_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*U*/ 'U':
      xpm_load(BigU_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*V*/ 'V':
      xpm_load(BigV_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*W*/ 'W':
      xpm_load(BigW_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*X*/ 'X':
      xpm_load(BigX_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*Y*/ 'Y':
      xpm_load(BigY_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*Z*/ 'Z':
      xpm_load(BigZ_xpm, XPM_8_8_8, &xpm_image);
      DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, x, y);
      break;
    case /*\0*/ '\0':
    return;
    }
    x += xpm_image.width;
  }
}

void DisplayWord()
{
  xpm_image_t xpm_image;
  draw_rectangle(0, 0, 1024, 768, (uint32_t)black);

  xpm_load(CYE_xpm, XPM_8_8_8, &xpm_image);
  DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, 246, 336);
  backround_to_videomem();

  sleep(4);
  
  draw_rectangle(0, 0, 1024, 768, (uint32_t)white);
  DrawWord(400, 374);
  backround_to_videomem();
  
  clean_RBR();  
  //uart_write((u_char *)indexWord, THR);
  write_THR((int *)indexWord);

  sleep(4);

  draw_rectangle(0, 0, 1024, 768, (uint32_t)white);
  xpm_load(Big3_xpm, XPM_8_8_8, &xpm_image);
  DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, 477, 344);
  backround_to_videomem();

  sleep(1);

  draw_rectangle(0, 0, 1024, 768, (uint32_t)white);
  xpm_load(Big2_xpm, XPM_8_8_8, &xpm_image);
  DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, 477, 344);
  backround_to_videomem();

  sleep(1);

  draw_rectangle(0, 0, 1024, 768, (uint32_t)white);
  xpm_load(Big1_xpm, XPM_8_8_8, &xpm_image);
  DrawMap((char *)xpm_image.bytes, xpm_image.width, xpm_image.height, 477, 344);
  backround_to_videomem();

  sleep(1);

  //tickdelay(micros_to_ticks(100000));
}


int DrawBrush1Buff(int16_t x, int16_t y)//goes to the backround buff
{
  xpm_image_t xpm_imageB;
  int width = 46, height = 46;
  xpm_load(Brush_xpm, XPM_8_8_8, &xpm_imageB);
  char *map = (char *)xpm_imageB.bytes;
  uint32_t tc = xpm_transparency_color(XPM_8_8_8);
  unsigned long color = 0;

  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      if ((x + i) > 1 && (x + i) < 1024 && (y + j) > 0 && (y + j) < 768)
      {
        memcpy(&color, &map[(j * width + i) * 3], 3);
        if (color != tc)
        {
          uint8_t num_bytes = (bits_per_pixel / 8) + (bits_per_pixel % 8 > 0 ? 1 : 0);
          memcpy(video_mem + (h_res * (y + j) + x + i) * num_bytes, &color, num_bytes);
        }
      }
    }
  }
  return 0;
}

