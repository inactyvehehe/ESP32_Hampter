/* 
--------------------------------------------------------------------------------------------------------------------------------
Hello future nico or whoever got this file, this version (HampterVA03 or Hampter Version Alpha 03) mainly focuses on improving
documentation. Code blocks are also better managed for ease of reading and understanding. Other than that, this version
has already explored and implemented a crude version of reading analog signals and using custom pins to supply power to modules
that does not require constant power, meaning that when module is not used, power will not be supplied.

Carrot animation has already been partially removed, it still exists in this file but is commented.

As of 5/25/2024 02:14, VA03 will be archived while VA04 will begin its development where functions will be used more often
as well as the better usage of loop and return for better software efficiency and optimization.
Enjoy your stay in this relic!
--------------------------------------------------------------------------------------------------------------------------------
*/



// include libraries here
#include "U8g2lib.h"
#include <Wire.h>
#include "bitmaps.h"  // tells device to use the bitmap file
// #include "animations.h"

// initialize u8g2 for display and what respective pins should be assigned to
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// define which pins is for which buttons
#define but_up 27
#define but_sel 14
#define but_down 12
#define but_back 13

// I think these are unused lmao
#define I2C_SDA 21
#define I2C_SCL 22

// define analog pins for: Pulse Sensor
#define HR_out_pin 34
#define HR_power_pin 18


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

int HR_time_count = 0;  // time keeping variable for
int HR_value = 0;       // value variable of HR output
int bpm = 0;            // variable to keep BPM
int beat = 0;

// current screen differentiates which screen user is on, menu = 0, in item = 1
int current_screen = 0;

//--------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  // initialize custom Power pins
  pinMode(HR_power_pin, OUTPUT);
  // turn off unnecessary items and module during boot up
  digitalWrite(HR_power_pin, LOW);       // Turn off HR module


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
  // if current screen is menu screen, then the up and down buttons will change selection and select will only select items highlighted
  if (current_screen == 0) 
  {  
    // button up is pressed, change selected item to the top one
    if (digitalRead(but_up) == LOW && (button_up_clicked == 0))
      {
        item_sel = item_sel - 1;
        button_up_clicked = 1;  // set "button_up_cliecked" to 1 therefore, forcing the click requirement to fail
        if (item_sel < 0)       // if selected item is less than the first item, rotate to the last 
          {
            item_sel = NUM_ITEMS - 1;
          }
      } 
    
    // button down is pressed, change selected item to the bottom one
    else if (digitalRead(but_down) == LOW && (button_down_clicked == 0))
      {
        item_sel = item_sel + 1;   // move selection down
        button_down_clicked = 1;   // set "button_down_cliecked" to 1 therefore, forcing the click requirement to fail
        if (item_sel >= NUM_ITEMS) // if selected item is more than the last item, rotate to the first
          {
            item_sel = 0;
          }
      } 
    
    // select button is clicked in menu
    else if ((digitalRead(but_sel) == LOW) && (button_sel_clicked == 0))
      {
        button_sel_clicked = 1;                                       // set button to clicked to stop spamming
        current_screen = 1;                                           // if clicked in menu screen, change current screen to in item screen (2)
      }
    
    if ((digitalRead(but_up) == HIGH) && (button_up_clicked == 1)) {      // unclick up
      button_up_clicked = 0;
    }
    if ((digitalRead(but_down) == HIGH) && (button_down_clicked == 1)) {  // unclick down
      button_down_clicked = 0;
    }
    if ((digitalRead(but_sel) == HIGH) && (button_sel_clicked == 1)) {    // unclick select
      button_sel_clicked = 0;
    }

    item_sel_top = item_sel - 1;                            // always make displayed item above selected item to be the previous item in line
    if (item_sel_top < 0) { item_sel_top = NUM_ITEMS - 1; } // turn top item into last item if its before the first item
    item_sel_bot = item_sel + 1;                            // always make displayed item below selected item to be the next item in line
    if (item_sel_bot >= NUM_ITEMS) { item_sel_bot = 0; }    // turn bottom item into first item if its beyond last item
  }


  // ------------------ this section sets the commands for in app stuff ------------------
  if (current_screen == 1) // if in item do these
    {
      if ((digitalRead(but_back) == LOW) && (button_back_clicked == 0)) // if back button pressed, go back to menu screen and other things
        {
          button_back_clicked = 1;          // set button to clicked
          current_screen = 0;               // go back to menu

          // close items / turn off modules upon returning to menu
          // Heart Rate related
          digitalWrite(HR_power_pin, LOW);       // Turn off HR module
          HR_time_count = 0;
          bpm = 0;
          beat = 0;
        }
      
    }

  if ((digitalRead(but_back) == HIGH) && (button_back_clicked == 1)) // unclick back button
      {
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
    // ----------------------------------for in item screen, draw these, and possibly app functions------------------------------------
  else if (current_screen == 1) // item keyboard visuals (WIP)
    {
    u8g2.setFont(u8g_font_7x14);
    if (item_sel == 3) // draw triangle on item_keyboard
      {
        u8g2.drawTriangle(10 + tri_spin * 2, 50 - tri_spin, 30 - tri_spin * 3, 20 + tri_spin, 40 + tri_spin * 2, 60 - tri_spin);
      } 
    
    // disabled carrot cuz wtf 
    /*
      else if (item_sel == 1) // animate carrot on item_carrot
        {
          u8g2.drawXBMP(32, 0, 64, 64, animated_allArray[tri_spin]);
            if (tri_count == 0) 
              {
                tri_spin = tri_spin + 1;
              } 
              else 
              {
                tri_spin = tri_spin - 1;
              }
          if ((tri_spin >= 25) && (tri_count == 0)) 
            {
              tri_count = 1;
            } 
            else if ((tri_spin == 0) && (tri_count == 1)) 
              {
                tri_count = 0;
              }
        }
    */

      else if (item_sel == 2) // HR app functions and visuals
        {
          digitalWrite(HR_power_pin, HIGH);                         // turn on HR sensor
          HR_value = 55 - (round((int)analogRead(HR_out_pin)/85));  // process raw signal to be used in graphs. NOT TO BE USED FOR PULSE STRENGTH MEASUREMENT
          HR_time_count = HR_time_count + 1;                        // time counter
          if(HR_time_count > 128)                                   // reset time counter to fit graph
            {
              HR_time_count = 0;
            }
          Serial.println("HR Value: " + (String)HR_value);          // debug line
          u8g2.drawPixel(HR_time_count, HR_value);                  // (FAILED) draw pixels for graph
          u8g2.drawBox(HR_time_count + 1, 10, 1, 45);               // (FAILED) clear pixels to make new ones
        }

      else // selecting items without function will display this thing
        {
          u8g2.drawStr(18, 37, "Still WIP hehe");
        }
  }
  u8g2.sendBuffer();
}