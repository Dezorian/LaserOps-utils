/* Decode Nerf LaserOps signals and display on the Circuitplayground LEDs
 *
 * Display the team colour that "hit" the circuit playground reciver by matching
 * to a known IRLib Hash of the signal.
 */

#define CPLAY_NEOPIXELPIN 8 // neopixel pin
#define CPLAY_IR_RECEIVER 26 // IR receiver pin

// Use only HashRaw to return a 32 bit hash.
#include <IRLibDecodeBase.h>
#include <IRLib_HashRaw.h>  //Must be last protocol
#include <IRLibCombo.h>     // After all protocols, include this
// All of the above automatically creates a universal decoder
// class called "IRdecode" containing only the protocols you want.
// Now declare an instance of that decoder.
IRdecode myDecoder;

// Include a receiver
#include <IRLibRecv.h>
IRrecv myReceiver(CPLAY_IR_RECEIVER);

// include the adafruit neopixel library
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(10, CPLAY_NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

static const unsigned long ANIMATION_DELAY = 5; //ms
static const unsigned long SLOW_ANIMATION_DELAY = 100; //ms
static const unsigned long INDICATION_ANIMATION_DELAY = 1000; //ms
static const unsigned long HOLD_DELAY = 250; //ms

/* IR hashes for Nerf LaserOps standard game */
static const long PURPLE = 0x67228B44;
static const long RED = 0x78653B0E;
static const long BLUE = 0x2FFEA610;

#define CPLAY_LEFTBUTTON 4        ///< left button pin
#define CPLAY_RIGHTBUTTON 5       ///< right button pin
#define CPLAY_SLIDESWITCHPIN 7    ///< slide switch pin
#define CPLAY_NEOPIXELPIN 8       ///< neopixel pin
#define CPLAY_REDLED 13           ///< red led pin
#define CPLAY_IR_EMITTER 25       ///< IR emmitter pin
#define CPLAY_IR_RECEIVER 26      ///< IR receiver pin
#define CPLAY_BUZZER A0           ///< buzzer pin
#define CPLAY_LIGHTSENSOR A8      ///< light sensor pin
#define CPLAY_THERMISTORPIN A9    ///< thermistor pin
#define CPLAY_SOUNDSENSOR A4      ///< TBD I2S
#define CPLAY_LIS3DH_CS -1        ///< LIS3DH chip select pin
#define CPLAY_LIS3DH_INTERRUPT 27 ///< LIS3DH interrupt pin
#define CPLAY_LIS3DH_ADDRESS 0x19 ///< LIS3DH I2C address

enum Color { red, blue, purple, undefined};
enum Color _selectedTeam = undefined;
bool _leftbuttonPressed = false;
bool _rightbuttonPressed = false;
int _hitPoints = 10;

void setup() {
  pinMode(CPLAY_LEFTBUTTON, INPUT_PULLDOWN);
  pinMode(CPLAY_RIGHTBUTTON, INPUT_PULLDOWN);
  pinMode(CPLAY_REDLED, OUTPUT);

  Serial.begin(115200);
  //while (!Serial); //delay to wait for serial port
  myReceiver.enableIRIn(); // Start the receiver
  Serial.println(F("Ready to receive IR signals"));
  strip.begin();
  strip.setBrightness(64);
  strip.clear();
  strip.show();
}

void baseDestroyed(Color setColor)
{
  Serial.println("Base destroyed!"); 
  for (int loops = 0; loops < 10; loops++)
  {
    for (int i = 0; i < strip.numPixels(); i++)
    {
      //turn strip off
      strip.clear();

      //turn one led on
      switch (setColor)
      {
      case purple:
        strip.setPixelColor(i, 0xFF, 0x00, 0xFF);            
        break;
      case blue:
        strip.setPixelColor(i, 0x00, 0x00, 0xFF);
        break;
      case red:
        strip.setPixelColor(i, 0xFF, 0x00, 0x00);      
        break;    
      default:
        break;
      }
      strip.show();
      delay(SLOW_ANIMATION_DELAY);
    }
  }
  //Set hitpoints back to 10
  _hitPoints = 10;

  for (int flash = 0; flash < 10; flash++)
  {
    strip.clear();
    strip.show();
    delay(INDICATION_ANIMATION_DELAY);

    for (int i=0; i<=strip.numPixels(); i++) 
    {      
      strip.setPixelColor(i, 0xFF, 0xFF, 0xFF);
      strip.show();
    }
    delay(INDICATION_ANIMATION_DELAY);    
  }
}

void setPixels(Color setColor, int hitPoints)
{  
  int nrOfPixels;

  strip.clear();

  if (hitPoints == -1) {
    nrOfPixels = strip.numPixels();    
  } else {
    nrOfPixels = hitPoints-1;
  }  

  switch (setColor)
  {
  case purple:
    for (int i=0; i<=nrOfPixels; i++) {
      strip.setPixelColor(i, 0xFF, 0x00, 0xFF);
      strip.show();      
    }
    break;
  case blue:
    for (int i=0; i<=nrOfPixels; i++) {
      strip.setPixelColor(i, 0x00, 0x00, 0xFF);
      strip.show();      
    }
    break;
  case red:
    for (int i=0; i<=nrOfPixels; i++) {
      strip.setPixelColor(i, 0xFF, 0x00, 0x00);
      strip.show();      
    }
    break;    
  default:
    for (int i=0; i<=nrOfPixels; i++) {
      strip.setPixelColor(i, 0x00, 0x00, 0x00);
      strip.show();
    }
    break;
  }

  delay(HOLD_DELAY);
  strip.clear();
//  strip.show();
}

void handleIR()
{
    if(myReceiver.getResults()) {
      myDecoder.decode();
      
    if(myDecoder.protocolNum==UNKNOWN) {     

      //Laser code received, someone shot the base
      if (myDecoder.value == PURPLE && _selectedTeam == purple) {
        if (_hitPoints > 1) {
          _hitPoints = _hitPoints -1;
          setPixels(purple, _hitPoints); 
          Serial.print("Purple shot received, hitpoints left: "); 
          Serial.print(_hitPoints); 
          Serial.println();           
        } else {
          baseDestroyed(purple);          
        }        
      } else if (myDecoder.value == RED && _selectedTeam == blue) {
        if (_hitPoints > 1) {
          _hitPoints = _hitPoints -1;
          setPixels(red, _hitPoints); 
          Serial.print("Red shot received, hitpoints left: "); 
          Serial.print(_hitPoints); 
          Serial.println();           
        } else {
          baseDestroyed(red);
        }   
      } else if (myDecoder.value == BLUE && _selectedTeam == red) {
        if (_hitPoints > 1) {
          _hitPoints = _hitPoints -1;
          setPixels(blue, _hitPoints); 
          Serial.print("Blue shot received, hitpoints left: "); 
          Serial.print(_hitPoints); 
          Serial.println();           
        } else {
          baseDestroyed(blue);
        }   
      }
    }


  }

  myReceiver.enableIRIn();
  
}

void handleButtons()
{
    // If the left button is pressed....
  if (digitalRead(CPLAY_LEFTBUTTON) && !_leftbuttonPressed) {
    _leftbuttonPressed = true;
    if (_selectedTeam == undefined) {
      _selectedTeam = red;
    } else {
      _selectedTeam=(Color)(_selectedTeam+1);      
    }
    Serial.print("SelectedTeam +1 : "); 
    Serial.print(_selectedTeam); 
    Serial.println(); 
    setPixels(_selectedTeam, -1);    
  } 
  else
  {
    _leftbuttonPressed = false;
  }

  if (digitalRead(CPLAY_RIGHTBUTTON) && !_rightbuttonPressed) {
    _rightbuttonPressed = true;
    if (_selectedTeam == red) {
      _selectedTeam = undefined;
    } else {
      _selectedTeam=(Color)(_selectedTeam-1);
    }
    Serial.print("SelectedTeam - 1: "); 
    Serial.print(_selectedTeam); 
    Serial.println(); 
    setPixels(_selectedTeam, -1);    
  }
  else
  {
    _rightbuttonPressed = false;    
  }
}

void loop() {  

  handleButtons();

  handleIR();
}
