// include libraries here
#include "U8g2lib.h"
#include <Wire.h>
#include "bitmaps.h"  // tells device to use the bitmap file
#include "animations.h"

// initialize u8g2 for display and what respective pins should be assigned to
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// define which pins is for which buttons
#define but_up 27
#define but_sel 14
#define but_down 12
#define but_back 13

#define I2C_SDA 21
#define I2C_SCL 22

// set up button clicked mechanism
int button_up_clicked = 0;    // only perform action when button is clicked, and wait until another press
int button_sel_clicked = 0;   // same as above
int button_down_clicked = 0;  // same as above
int button_back_clicked = 0;  // same as above

// set up item selection
int item_sel = 0;  // which item in the menu is selected
int item_sel_top;  // top (prev) item
int item_sel_bot;  // bottom (next) item

// variables for in item mechanics
int tri_spin = 0;
int tri_count = 0;

// current screen differentiates which screen user is on, menu = 0, in item = 1
int current_screen = 0;

//--------------------------------------------------------------------

void setup() {
  u8g2.begin(); // Pass the custom I2C instance to u8g2
  
  // define how the input is recorded for the buttons which is pullup
  pinMode(but_up, INPUT_PULLUP);
  pinMode(but_sel, INPUT_PULLUP);
  pinMode(but_down, INPUT_PULLUP);
  pinMode(but_back, INPUT_PULLUP);

  u8g2.setColorIndex(1);
  u8g2.setBitmapMode(1);
}

//--------------------------------------------------------------------

void loop() {

  // --------------------- this section sets the commands for menu selection stuff -------------------------
  if (current_screen == 0) {  //if current screen is menu screen, then the up and down buttons will change selection and select will only select items highlighted

    if (digitalRead(but_up) == LOW && (button_up_clicked == 0)) {  //button up is pressed, change selected item to the top one
      item_sel = item_sel - 1;
      button_up_clicked = 1;  //set "button_up_cliecked" to 1 therefore, forcing the click requirement to fail
      if (item_sel < 0) {     //if selected item is less than the first item, rotate to the last
        item_sel = NUM_ITEMS - 1;
      }
    } else if (digitalRead(but_down) == LOW && (button_down_clicked == 0)) {  //button down is pressed, change selected item to the bottom one
      item_sel = item_sel + 1;
      button_down_clicked = 1;      //set "button_down_cliecked" to 1 therefore, forcing the click requirement to fail
      if (item_sel >= NUM_ITEMS) {  //if selected item is more than the last item, rotate to the first
        item_sel = 0;
      }
    } else if ((digitalRead(but_sel) == LOW) && (button_sel_clicked == 0)) {  // select button is clicked for the 'first time'
      button_sel_clicked = 1;                                                           // set button to clicked to stop spamming
      current_screen = 1;                                                               // if clicked in menu screen, change current screen to in item screen (2)
    }
    if ((digitalRead(but_up) == HIGH) && (button_up_clicked == 1)) {  // unclick
      button_up_clicked = 0;
    }
    if ((digitalRead(but_down) == HIGH) && (button_down_clicked == 1)) {  // unclick
      button_down_clicked = 0;
    }
    if ((digitalRead(but_sel) == HIGH) && (button_sel_clicked == 1)) {  // unclick
      button_sel_clicked = 0;
    }


    item_sel_top = item_sel - 1;
    if (item_sel_top < 0) { item_sel_top = NUM_ITEMS - 1; }  // turn top item into last item if its before the first item
    item_sel_bot = item_sel + 1;
    if (item_sel_bot >= NUM_ITEMS) { item_sel_bot = 0; }  // turn bottom item into first item if its beyond last item
  }


  // ------------------ this section sets the commands for in app stuff ------------------
  if (current_screen == 1) {                                                    // if in item do these
    if ((digitalRead(but_back) == LOW) && (button_back_clicked == 0)) {  // back button pressed
      button_back_clicked = 1;                                                  // set button to clicked
      current_screen = 0;                                                       // go back to menu
    }
    }


    if ((digitalRead(but_back) == HIGH) && (button_back_clicked == 1)) {  // unclick
      button_back_clicked = 0;
    }
  

            // --------------------OLED DISPLAY RELATED---------------------------------------
  u8g2.clearBuffer();         
  if (current_screen == 0) {  // -------------------for menu screen, draw the menu-----------------------
    // draw top item
    u8g2.setFont(u8g_font_7x14);
    u8g2.drawStr(22, 15, menu_items[item_sel_top]);
    u8g2.drawXBMP(2, 2, 16, 16, bitmap_icons[item_sel_top]);

    // draw selected item
    u8g2.setFont(u8g_font_7x14);
    u8g2.drawStr(28, 37, menu_items[item_sel]);  // draw label
    u8g2.drawXBMP(7, 24, 16, 16, bitmap_icons[item_sel]);

    // draw bottom item
    u8g2.setFont(u8g_font_7x14);
    u8g2.drawStr(22, 59, menu_items[item_sel_bot]);  // draw label
    u8g2.drawXBMP(2, 46, 16, 16, bitmap_icons[item_sel_bot]);

    //draw background selector
    u8g2.drawXBMP(0, 22, 128, 20, bitmap_selector);
    // draw scroll
    u8g2.drawXBMP(120, 0, 8, 64, bitmap_scroll);
    u8g2.drawBox(124, 64 / NUM_ITEMS * item_sel, 3, 64 / NUM_ITEMS);  //draw scroll handle
  }
    // ----------------------------------for in item screen, draw these------------------------------------
  else if (current_screen == 1) {  // item keyboard visuals
    u8g2.setFont(u8g_font_7x14);
    if (item_sel == 3) {                                              // draw triangle on item_keyboard
      u8g2.drawTriangle(10 + tri_spin * 2, 50 - tri_spin, 30 - tri_spin * 3, 20 + tri_spin, 40 + tri_spin * 2, 60 - tri_spin);
    } 
    
    else if (item_sel == 1) {                                         // animate carrot on item_carrot
      u8g2.drawXBMP(32, 0, 64, 64, animated_allArray[tri_spin]);
      if (tri_count == 0) {
        tri_spin = tri_spin + 1;
      } else {
        tri_spin = tri_spin - 1;
      }
      if ((tri_spin >= 25) && (tri_count == 0)) {
        tri_count = 1;
      } else if ((tri_spin == 0) && (tri_count == 1)) {
        tri_count = 0;
      }
    }

//    else if (item_sel == 2) {
//      item_hr()
//    }

    else {
      u8g2.drawStr(22, 37, "salah cui");
    }
  }

  u8g2.sendBuffer();
}