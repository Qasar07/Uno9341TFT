#include <Uno_GFX.h>    // Core graphics library
#include <Uno_TFTLCD.h> // Hardware-specific library

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
// Using values with identical high and low bytes gives faster
// Rendering of filled regions
#define BLACK   0b0000000000000000
#define BLUE    0b0001101100011011
#define RED     0b1110000011100000
#define GREEN   0b0000111100001111
#define CYAN    0b0001111100011111
#define MAGENTA 0b1111100011111000
#define YELLOW  0b1110111111101111
#define WHITE   0b1111111111111111

Uno_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define NVERTICES 8
float cube_verticies[NVERTICES*3] = {
   1, 1, 1,
   1, 1,-1,
   1,-1, 1,
   1,-1,-1,
  -1, 1, 1,
  -1, 1,-1,
  -1,-1, 1,
  -1,-1,-1,
};

#define NEDGES 12
float cube_edges[NEDGES*2] = {
  0,1,
  0,2,
  1,3,
  2,3,
  0,4,
  4,5,
  4,6,
  3,7,
  5,7,
  6,7,
};

void setup() {
  tft.reset();
  tft.begin(0x9341);
  tft.fillScreen(BLACK);

  // 3 bit color mode
  PORTC=0b1110001;
  PORTB=PORTD=0x39;
  PORTC=0b1110011;

  // Low frame rate in 3-bit mode
  PORTC=0b1110001;
  PORTB=PORTD=0xB2;
  PORTC=0b1110011;
  PORTC=0b1110101;
  PORTB=PORTD=3; //DIVB register (2 bit clock divider, 0 is system clock and 3 is divide by 8)
  PORTC=0b1110111;
  PORTC=0b1110101;
  PORTB=PORTD=31; // RTNB register (between 16 and 31, number of divided clock cycles to wait between frames)
  PORTC=0b1110111;
  // Low frame rate in normal mode
  PORTC=0b1110001;
  PORTB=PORTD=0xB1;
  PORTC=0b1110011;
  PORTC=0b1110101;
  PORTB=PORTD=3; //DIVB register (2 bit clock divider, 0 is system clock and 3 is divide by 8)
  PORTC=0b1110111;
  PORTC=0b1110101;
  PORTB=PORTD=31; // RTNB register (between 16 and 31, number of divided clock cycles to wait between frames)
  PORTC=0b1110111;
  
  // High frame rate in 3-bit mode
  PORTC=0b1110001;
  PORTB=PORTD=0xB2;
  PORTC=0b1110011;
  PORTC=0b1110101;
  PORTB=PORTD=0; //DIVB register (2 bit clock divider, 0 is system clock and 3 is divide by 8)
  PORTC=0b1110111;
  PORTC=0b1110101;
  PORTB=PORTD=16; // RTNB register (between 16 and 31, number of divided clock cycles to wait between frames)
  PORTC=0b1110111;
  PORTC=0b1110001;
  // High frame rate in normal mode
  PORTB=PORTD=0xB1;
  PORTC=0b1110011;
  PORTC=0b1110101;
  PORTB=PORTD=0; //DIVB register (2 bit clock divider, 0 is system clock and 3 is divide by 8)
  PORTC=0b1110111;
  PORTC=0b1110101;
  PORTB=PORTD=16; // RTNB register (between 16 and 31, number of divided clock cycles to wait between frames)
  PORTC=0b1110111;
  
  color_test();
  delay(500);
  cube();
}

void loop() {
  mask_test();
}

#include <math.h>
#define NVERTS 8
#define NEDGES 8

PROGMEM const uint8_t edges[NEDGES*2]={
  0,2,
  0,4,
  1,3,
  1,5,
  2,6,
  4,5,
  3,7,
  6,7};

void mask_test() {
  /* Test the masking feature. We need to be able to do the following:
   *  Draw a line
   *  Draw the next frame for the line in-place
   *  Erase the previous line
   *  Repeat
   *  
   *  Notes: masked drawing skips point if
   *  secret_mask_flag matches the current pixel data
   *  so the "new frame" pixels should match secret_mask_flag
   *  
   *  This test currently passes.
   */

  uint16_t color1 = 0b1111011111111111;
  uint16_t color2 = 0b1111111111111111;
  
  // clear screen
  tft.secret_masking_on = 0;
  tft.fillScreen(BLACK);

  // Draw a line with secret_mask_flag off
  tft.drawLine(0,0,240,320,color1);
  delay(500);

  // Draw the "next line" with secret_mask_flag set
  tft.drawLine(0,1,240,321,color2);
  delay(500);

  // Erase the old line by overdrawing in black
  // The old frame has secret_mask_flag off
  // The new frame has secret_mask_flag set
  // Pixels are skipped (not erased) if 
  // secret_mask_flag equals the pixel data
  // so secret_mask_flag set should match that
  // of the "new" frame, i.e. 
  // secret_mask_flag set should be set in this case
  tft.secret_mask_flag = 0b1000;
  tft.secret_masking_on = 1;
  tft.drawLine(0,0,240,320,BLACK);
  delay(500);

  // We should be able to repeat that for the next frame
  
  // Draw a line with secret_mask_flag off
  tft.secret_masking_on = 0;
  tft.drawLine(0,2,240,322,color1);
  delay(500);

  tft.secret_mask_flag = 0b0000;
  tft.secret_masking_on = 1;
  tft.drawLine(0,0,240,320,BLACK);
  delay(500);
  
}

void mask_debug() {
  tft.secret_masking_on = 0;
  tft.fillScreen(BLACK);
  tft.secret_mask_flag = 0;

  // Pixels matching the secret mask flag ARE NOT drawn
  // Secret mask flag is lowest bit of 5-bit Red channel
  // Draw two lines, one with and one without that bit set.
  // Later, we'll test masking by trying to erase both of them.
  uint16_t color1 = 0b1111011111111111;
  uint16_t color2 = 0b1111111111111111;
  uint16_t color3 = 0b0000011111111111;
  uint16_t color4 = 0b0000111111111111;
  tft.drawLine(  0,0, 60,320,color1);
  tft.drawLine( 60,0,120,320,color2);
  tft.drawLine(120,0,180,320,color3);
  tft.drawLine(180,0,240,320,color4);

  
  // This is not working, so let's do a test read. 
  uint32_t pixel1 = tft.readRGB( 59,319); // pixel from line 1
  uint32_t pixel2 = tft.readRGB(0,0); // pixel from line 1
  uint32_t pixel3 = tft.readRGB( 60,0); // pixel from line 2
  uint32_t pixel4 = tft.readRGB(120,0); // pixel from line 2
  uint32_t pixel5 = tft.readRGB(180,0); // pixel from line 2

  // Print out the pixel values
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);  tft.setTextSize(1);
  for (int i=31; i>=0; i--) {
    tft.print(pixel1>>i&0b1);
    if (i%8==0) tft.print(' ');
  }
  tft.println("");
  for (int i=31; i>=0; i--) {
    tft.print(pixel2>>i&0b1);
    if (i%8==0) tft.print(' ');
  }
  tft.println("");
  for (int i=31; i>=0; i--) {
    tft.print(pixel3>>i&0b1);
    if (i%8==0) tft.print(' ');
  }
  tft.println("");
  for (int i=31; i>=0; i--) {
    tft.print(pixel4>>i&0b1);
    if (i%8==0) tft.print(' ');
  }
  tft.println("");
  for (int i=31; i>=0; i--) {
    tft.print(pixel5>>i&0b1);
    if (i%8==0) tft.print(' ');
  }
  tft.println("");
  

  // Pixes are sent out in 565 RGB format
  // But for some reason are read back in 888 RGB format.
  // The internal representation of colors is 6 bit. 
  // They are padded to 8 bit when returned. 
  // The lowest order bit in a 5-bit red value should be
  // the second bit of the 6-bit.
  tft.secret_mask_flag  = 0b1000;
  tft.secret_masking_on = 1;
  delay(2000);

  // Erase both lines. Only one of them should be erased in theory.
  //tft.drawLine(  0,0,120,320,BLACK);
  //tft.drawLine(120,0,240,320,BLACK);

  tft.drawLine(120,0,180,320,BLACK);
  tft.drawLine(180,0,240,320,BLACK);
  tft.drawLine(  0,0, 60,320,BLACK);
  tft.drawLine( 60,0,120,320,BLACK);
  
  delay(2000);
}

void cube() {
  float buff[NVERTS*2*2];
  float *vbuff1 = &buff[0];
  float *vbuff2 = &buff[NVERTS*2];
  uint16_t color = WHITE;
  uint16_t theta_x = 0;
  uint16_t theta_y = 0;
  uint16_t x0    = 120;
  uint16_t y0    = 160;
  tft.fillScreen(BLACK);
  tft.secret_mask_flag = 0;
  while (1) 
  {
    theta_x += 117/4;
    float tx = (float)theta_x * 9.587379924285257e-05;
    float ctx = cos(tx);
    float stx = cos(tx+1.5707963267948966f);
    theta_y += 337/4;
    float ty = (float)theta_y * 9.587379924285257e-05;
    float cty = cos(ty);
    float sty = cos(ty+1.5707963267948966f);
    // Rotate vertices into place
    int i;
    for (i=0; i<NVERTS; i++) {
      float  *q = &vbuff1[i*2];
      float   x = (float)(((i<<1)&2)-1);
      float   y = (float)((i&2)-1);
      float   z = (float)(((i>>1)&2)-1);
      q[1] = (ctx*y-stx*z)*50+y0;
      z    = stx*y + ctx*z;
      q[0] = (cty*x-sty*z)*50+x0;
    }
    // Draw next frame
    tft.secret_mask_flag ^= 0b1000;
    color = 0b1111011111111111 | ((uint16_t)tft.secret_mask_flag<<8);
    tft.secret_masking_on = 0;
    for (i=0; i<NEDGES; i++) {
      const uint8_t *e = &edges[i*2];
      float *p = &vbuff1[pgm_read_byte(&e[0])*2];
      float *q = &vbuff1[pgm_read_byte(&e[1])*2];
      tft.drawLine(p[0],p[1],q[0],q[1],color);
      // Parts matching the mask flag will NOT be erased
      //delay(100);
      tft.secret_masking_on = 1;
      p = &vbuff2[pgm_read_byte(&e[0])*2];
      q = &vbuff2[pgm_read_byte(&e[1])*2];
      tft.drawLine(p[0],p[1],q[0],q[1],BLACK);
      tft.secret_masking_on = 0;
      //delay(100);
    }
    // Flip vertex buffers
    float *temp = vbuff1;
    vbuff1 = vbuff2;
    vbuff2 = temp;
  }
}

void color_test() {
  tft.fillScreen(BLACK);
  uint8_t i=0;
  for (int y=0; y<320; y+=15) {
    for (int x=0; x<240; x+=15) {
      uint16_t color = (i<<8)|i;
      tft.fillRect(x,y,15,15,color);
      if (i==255) break;
      i++;
    }
    if (i==255) break;
  }
  tft.fillRect(00,300,20,20,BLACK);
  tft.fillRect(20,300,20,20,WHITE);
  tft.fillRect(40,300,20,20,RED);
  tft.fillRect(60,300,20,20,GREEN);
  tft.fillRect(80,300,20,20,BLUE);
  tft.fillRect(100,300,20,20,YELLOW);
  tft.fillRect(120,300,20,20,MAGENTA);
  tft.fillRect(140,300,20,20,CYAN);
}



