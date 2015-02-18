#include <IRremote.h>

int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;
char command;
boolean understood = false;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    understood = true;
    switch(results.value) {
      case 16752735:
        command = 'A';
        break;
      case 16720095:
        command = 'B';
        break;
      case 16736415:
        command = 'C';
        break;
      case 16769055:
        command = 'D';
        break;
      case 16748655:
        command = 'E';
        break;
      case 16716015:
        command = 'F';
        break;
      case 16732335:
        command = 'G';
        break;
      case 16764975:
        command = 'H';
        break;
      case 16756815:
        command = 'I';
        break;
      case 16724175:
        command = 'J';
        break;
      case 16740495:
        command = 'K';
        break;
      case 16773135:
        command = 'L';
        break;
      case 16754775:
        command = 'M';
        break;
      case 16722135:
        command = 'N';
        break;
      case 16738455:
        command = 'O';
        break;
      case 16771095:
        command = 'P';
        break;
      case 16750695:
        command = 'Q';
        break;
      case 16718055:
        command = 'R';
        break;
      case 16734375:
        command = 'S';
        break;
      case 16767015:
        command = 'T';
        break;
      case 16746615:
        command = 'U';
        break;
      case 16713975:
        command = 'V';
        break;
      case 16730295:
        command = 'W';
        break;
      case 16762935:
        command = 'X';
        break;
      default:
        understood = false;
    }
    if (understood) Serial.println(command);
    irrecv.resume(); // Receive the next value
  }
}
