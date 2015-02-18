#include "FastSPI_LED2.h"
#define NUM_LEDS 98

CRGB pixels[NUM_LEDS];
int     leds[294];
int     max_overall_brightness = 255;
int     overall_brightness = 255;
float   red_max = 1;
float   green_max = 1;
float   blue_max = 1;
int     animation_rate = 1;
int     mode = 0; // Animation mode index
boolean on = true;
char    incoming_command = 'H';

// These variables are used within the animation methods.
// They are scoped by name, but global in the scetch,
// so that animations will not restart when commands are received.

// Beacon Vars
double beacon_angle = 0;
int    beacon_led_angle = 0;
double beacon_left_distance = 0;
double beacon_absolute_distance = 0.0;
int    beacon_beam_width = 30; // in degrees
int    beacon_falloff = 35; // in degrees
int    beacon_pixel_brightness = 0; // in degrees

// Water Effect Vars
long  water_next_wave = millis(); // Buffer for wave timing
int   water_color_ripple_frames[98];    // Buffer for ripple timing, to smooth flicker
int   water_white_ripple_frames[98];    // Buffer for ripple timing, to smooth flicker
int   water_blue, water_red, water_green;      // Buffers for a frame's color adjustments
int   water_streak_start;          // Random wave streak starting pixel
int   water_streak_length;         // Random streak length in pixels for wave
int   water_streak_frame;          // Decrementing buffer for current frame in wave progression
int   water_streak_total_frames;    // Total frames in current wave
float water_streak_intensity;      // Random peak intensity (brightness) for wave
int   water_chance_to_adjust = 0;  // Buffer for randomizing adjustment

volatile boolean animate = true;
volatile long animation_change_timeout;

void setup() {
  LEDS.setBrightness(255);
  LEDS.addLeds<WS2811, 6>(pixels, NUM_LEDS);
  Serial.begin(9600);
}

void loop() {
  switch (incoming_command) {
    case 'D': // On
      mode = 0;
      //Fall through to current settings
      
    case 'A': // Brighter
      if(overall_brightness <= max_overall_brightness - 10) {
        overall_brightness += 10;
      }
      break;
    case 'B': // Dimmer
      if(overall_brightness >= 30) {
        overall_brightness -= 20;
      } else {
        overall_brightness = 5;
      }
      break;
    
    // Color Buttons
    case 'E':
      red_max = 1;
      blue_max = 0;
      green_max = 0;
      break;
    case 'F':
      red_max = 0;
      blue_max = 0;
      green_max = 1;
      break;
    case 'G':
      red_max = 0;
      blue_max = 1;
      green_max = 0;
      break;
    case 'H':
      red_max = 1;
      blue_max = 1;
      green_max = 1;
      break;
    case 'I':
      red_max = 1;
      blue_max = 0;
      green_max = 0.4;
      break;
    case 'J':
      red_max = 0.33;
      blue_max = 0.33;
      green_max = 1;
      break;
    case 'K':
      red_max = 0.1;
      blue_max = .9;
      green_max = 0.2;
      break;
    case 'M':
      red_max = 1;
      blue_max = 0.2;
      green_max = 0.4;
      break;
    case 'N':
      red_max = 0.25;
      blue_max = 1;
      green_max = 0.45;
      break;
    case 'O':
      red_max = 0.15;
      blue_max = 1;
      green_max = 0;
      break;
    case 'Q':
      red_max = 1;
      blue_max = 0.35;
      green_max = 0.6;
      break;
    case 'R':
      red_max = 0.10;
      blue_max = 1;
      green_max = 0.65;
      break;
    case 'S':
      red_max = 0.3;
      blue_max = 1;
      green_max = 0;
      break;
    case 'U':
      red_max = 1;
      blue_max = 0.1;
      green_max = 0.9;
      break;
    case 'V':
      red_max = 0;
      blue_max = 1;
      green_max = 0.5;
      break;
    case 'W':
      red_max = 0.35;
      blue_max = 1;
      green_max = 0.15;
      break;
      
    //Mode Buttons
    case 'L':
      if (mode == 0) {
        increment_animation_rate();
      } else {
        animation_rate = 1;
        mode = 0;
      }
      break;
    case 'P':
      if (mode == 1) {
        increment_animation_rate();
      } else {
        red_max = 0;
        blue_max = 1;
        green_max = 0;
        animation_rate = 1;
        mode = 1;
      }
      break;
    case 'T':
      if (mode == 2) {
        increment_animation_rate();
      } else {
        red_max = 0;
        blue_max = 1;
        green_max = 0;
        animation_rate = 1;
        mode = 2;
      }
      break;
    case 'X':
      if (mode == 3) {
        increment_animation_rate();
      } else {
        animation_rate = 1;
        mode = 3;
      }
      break;
    case 'C':
      mode = 4;
      break;
  }
  
  switch (mode) {
    case 0:
      beacon();
      break;
    case 1:
      water(false);
      break;
    case 2:
      water(true);
      break;
    case 3:
      lamp();
      break;
    case 4:
      rest();
      break;
  }
  animation_change_timeout = 0;
  animate = true;
}

void check_for_input() {
  if (animation_change_timeout > 100) {
    if (Serial.available() > 0) {
      // read the incoming byte:
      incoming_command = Serial.read();
      
      // say what you got:
      Serial.print("I received: ");
      Serial.println(incoming_command);
      animate = false;
    }
  }
}

void increment_animation_rate() {
  if (animation_rate == 10) {
    animation_rate = 1;
  } else {
   animation_rate++;
  }
}

void rest () {
  long loop_start = millis();
  while(animate) {
    for(int i=0; i<NUM_LEDS; i++) {
      pixels[i] = CRGB(0, 0, 0);
    }
    FastLED.show();
    animation_change_timeout = millis() - loop_start;
    check_for_input();
  }
}
  
void lamp () {
  long loop_start = millis();
  while(animate) {
    //Some quick and dirty grascale whitebalance and min brigtness corrections
    int red = max_overall_brightness;
    int green = max_overall_brightness - 3/max_overall_brightness;
    if (green < 0) green = 0;
    if (max_overall_brightness > 1 && green < 2) green = 2;
    int blue = max_overall_brightness * .45;
    if (max_overall_brightness > 1 && blue < 2) blue = 2;
      
    // Aapply color maximums
    red *= red_max * (overall_brightness / 255.0);
    green *= green_max * (overall_brightness / 255.0);
    blue *= blue_max * (overall_brightness / 255.0);
    
    for(int i=0; i<NUM_LEDS; i++) {
      pixels[i] = CRGB(green, red, blue);
    }
    FastLED.show();
    animation_change_timeout = millis() - loop_start;
    delay(100);
    check_for_input();
  }
}

void water (boolean waves) {
  // A rippling blue water effect with optional waves.

  //Set random starting point for first frame when mode is selected
   for(int i=0; i<NUM_LEDS; i++) {
     leds[(i*3)+2] = 10;
     water_color_ripple_frames[i] = 0;
     water_white_ripple_frames[i] = 0;
   }
  
  // Set start time for minimum time to change interval
  long loop_start = millis();
  
  // Main loop for mode, execute while button remains un-pressed
  while(animate) {
    
    // Initiate waves on timer, or proceed with wave in progress
    if (millis() > water_next_wave) {
      water_next_wave = millis() + 7000; // Time until next wave
      water_streak_start = random(0, NUM_LEDS - 10); // Start pixel index for streak
      water_streak_length = random(1, 30); // Wave streak length in pixels
      water_streak_total_frames = 300; // Total frames in current wave
      water_streak_frame = water_streak_total_frames; // Decrementing frame counter for new wave
      water_streak_intensity  = random(150, 256); // Wave peak intesity
    } else {
      water_streak_frame--; // Wave in progress, decriments to 0, when wave is over
    }

    // Loop through LED array and update each pixel
    for(int i=0; i<NUM_LEDS; i++) {

      // Get last frame's values from buffer
      water_green = leds[i*3];
      water_red = leds[(i*3)+1];
      water_blue = leds[(i*3)+2];

      //Do waves if requested
      if (waves) {
        // If a wav is in progress, update it's pixels
        if (water_streak_frame >= 0) {
          
          // If current pixel is part of the wave streak
          if (i > water_streak_start && i < water_streak_start + water_streak_length) {
            
            // Values for new brightness value this frame
            int new_blue;
            
            // Streaks increase until the last phase, then hold for a bit
            // before being wiped out by ripples
            int wave_hold_phase_length = 60;
            
            if (water_streak_frame > 60) {
              new_blue = water_streak_intensity - (water_streak_frame - 60);
            } else {
              // Peak intensity for last phase
              new_blue = water_streak_intensity;
            }
          
            // Be sure new frame is not less bright than previous frame
            // so that fade in starts at ambient brightess regardless of
            // what that is.
            if (new_blue > water_blue) water_blue = new_blue;
            
            // Cap to maximum intensity
            if (water_blue > max_overall_brightness) water_blue = max_overall_brightness;

            // Add some whitecapping at peak of intense waves
            // Start at current white and add if needed

            int new_green = (water_blue - 100) / 2;
            if (new_green > water_green) water_green = new_green;
            water_red = water_green * 0.7;

          } else {
            //Pixel is not in current streak, break down old wave
            //If this pixel was part of one
            if (water_blue > 60) water_blue -= random(1,3);
          }
        }
      }

      // Create ripples on blue pixels
      int ripple_adjustment = 0; // frames left in current pixel ramp
      water_chance_to_adjust = random(0,5); // lower chance to adjust to slow effect

      // Adjust up or down one if in an adjustment cylce
      if (water_color_ripple_frames[i] > 0) {

        if (water_chance_to_adjust > 2) ripple_adjustment = (random(0,2) - 1);
        water_blue += ripple_adjustment;
        water_color_ripple_frames[i] += ripple_adjustment;

      } else if (water_color_ripple_frames[i] < 0) {
        
        if (water_chance_to_adjust > 2) ripple_adjustment = random(0,2);
        water_blue += ripple_adjustment;
        water_color_ripple_frames[i] += ripple_adjustment;

      // Or start an adjustment cycle 
      } else {

        // If brightness down low, move up
        if (water_blue < 5) {
          ripple_adjustment = (random(0,6) - 4) * (6);
        // If brightness up high, move down
        } else if (water_blue > 120) {
          ripple_adjustment = random(0,5) * (5);
        // Else pick at random, with a slight bias down to make
        // bright twinkles possible, but rare
        } else {
          ripple_adjustment = (random(0,8) - 2) * (3);
        }
        
        //Store adjustment plan for next frame
        water_color_ripple_frames[i] = ripple_adjustment;

      }

      // Brightness change between off and min is huge and jarring
      // Keep all pixels minimally lit at all times to avoid it
      if (water_blue < 2) water_blue = 2;
    
      // Add white ripples at some fraction of the brightness of blue
      // We'll set green and then later set red equal to it
      ripple_adjustment = 0; // frames left in current pixel ramp
      water_chance_to_adjust = random(0,4); // lower chance to adjust to slow effect
      
      // Adjust up or down one if in an adjustment cylce
      if (water_white_ripple_frames[i] > 0) {
        if (water_chance_to_adjust > 2) ripple_adjustment = random(0,2) - 1;
        water_green += ripple_adjustment;
        water_white_ripple_frames[i] += ripple_adjustment;
      } else if (water_white_ripple_frames[i] < 0) {
        if (water_chance_to_adjust > 2) ripple_adjustment = random(0,2);
        water_green += ripple_adjustment;
        water_white_ripple_frames[i] += ripple_adjustment;
      
      // Or start an adjustment cycle 
      } else {
        // If brightness down low, move up
        if (water_green < 1) {
          ripple_adjustment = (random(0,4) - 3) * 3;
        // If brightness up high, move down
        } else if (water_green > 20) {
          ripple_adjustment = random(0,5) * 3;
        // Else pick at random, with a slight bias down to make
        // bright twinkles possible, but rare
        } else {
          ripple_adjustment = (random(0,7) - 1) * 2;
        }
        
        //Store adjustment plan for next frame
        water_white_ripple_frames[i] = ripple_adjustment;
      }

      // Keep red and green at a fraction of blue regardless
      if (water_green > water_blue / 3) water_green = water_blue / 3;
      if (water_green < 0) water_green = 0;
      
      //Set green to a little more than red to make white
      water_red = water_green * 0.7;

      // Write new color values back to buffer
      leds[i*3] = water_green;
      leds[(i*3)+1] = water_red;
      leds[(i*3)+2] = water_blue;
      
      // Write new color values to pixel buffers
      pixels[i] = CRGB(water_green, water_red, water_blue);
    }

    // Write pixel buffers to pixel strand
    FastLED.show();
    animation_change_timeout = millis() - loop_start;
    check_for_input();
  }
}

void beacon() {
  // Set start time for minimum time to chenge interval
  long loop_start = millis();
  
  while (animate) {
    for(int i=0; i<NUM_LEDS; i++) {
      // Beacon angle is normalized to -180 to 180 degrees to make angular distance calc easier
      
      //Find the angle on the tube of the current LED
      beacon_led_angle = fmod((i * (360.0 / 11.41)), 360.0);
      
      //Normalize heading to -180 to 180 for angular distance to beacon calc
      if (beacon_led_angle < beacon_angle) beacon_led_angle += 360;
      beacon_left_distance = beacon_led_angle - beacon_angle;
      
      if (beacon_left_distance < 180) {
        beacon_absolute_distance = abs(beacon_left_distance);
      } else {
        beacon_absolute_distance = abs(360.0 - beacon_left_distance);
      }
      
      //Find pixel brightness given beam characteristics
      if (beacon_absolute_distance <= (beacon_beam_width / 2)) {
        beacon_pixel_brightness = overall_brightness;
      } else if (beacon_absolute_distance <= ((beacon_beam_width / 2) + beacon_falloff)) {
        beacon_pixel_brightness = overall_brightness - ((overall_brightness/(float)beacon_falloff) * (beacon_absolute_distance - beacon_beam_width/2));
      } else {
        beacon_pixel_brightness = 0;
      }
      
      //Some quick and dirty grascale whitebalance and min brigtness corrections
      int red = beacon_pixel_brightness;
      int green = beacon_pixel_brightness - 3/beacon_pixel_brightness;
      if (green < 0) green = 0;
      if (beacon_pixel_brightness > 1 && green < 2) green = 2;
      int blue = beacon_pixel_brightness * .45;
      if (beacon_pixel_brightness > 1 && blue < 2) blue = 2;
      
      //Finally, apply color maximums
      red *= red_max;
      green *= green_max;
      blue *= blue_max;
      
      //Assign color
      pixels[i] = CRGB(green, red, blue);
    }
    beacon_angle += (0.05 * (animation_rate * animation_rate * 3));
    FastLED.show();
    if (beacon_angle >= 360) {
      beacon_angle -= 360;
    }
    animation_change_timeout = millis() - loop_start;
    check_for_input();
  }
}
