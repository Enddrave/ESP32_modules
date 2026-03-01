#include <esp_display_panel.hpp>

void setup() {
   pinMode(6, OUTPUT); 

}

void loop() {
  digitalWrite(6, HIGH);  // LED ON
  delay(100);            // 1 second wait

  digitalWrite(6, LOW);   // LED OFF
  delay(100);            // 1 second wait
}