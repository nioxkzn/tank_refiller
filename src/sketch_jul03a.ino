#include <Adafruit_SSD1306.h>
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
#define PIN_TRIG 13 // ПИН D7
#define PIN_ECHO 12 // ПИН D6
#define PIN_RELAY 14 // ПИН D5
#define PIN_MAX_SENSOR 2 // ПИН D4 (internal led on esp)
#define PIN_START_BTN 15 //d8
#define LVL_FULL 10
#define LVL_EMPTY 77
#define AVERAGING 10

long duration;
int averageCm, cm, sensorValues[AVERAGING], tmpSensorValues[AVERAGING];;
bool filling = false;

void stopFilling () {
  digitalWrite(PIN_RELAY, LOW);
  filling = false;
  display.clearDisplay();
  display.setCursor(10,15);
  display.setTextSize(2);
  display.println("Full");
  display.display();
  delay(3000);
  display.setTextSize(3);
}

void IRAM_ATTR startFilling () {
  digitalWrite(PIN_RELAY, HIGH);
  filling = true;
  Serial.println("Filling start...");
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.display();
  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  
  Serial.begin (115200); 
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_MAX_SENSOR, INPUT_PULLUP);
 // pinMode(PIN_START_BTN, INPUT); //can only pulldown on my controller...
  attachInterrupt ( digitalPinToInterrupt (PIN_START_BTN), startFilling, CHANGE);
}

int middleValue() {
  
  memcpy(tmpSensorValues, sensorValues, sizeof(tmpSensorValues) );
  // Сортировка массива пузырьком
  int temp; 
  for (int i = 0; i < AVERAGING - 1; i++) {
      for (int j = 0; j < AVERAGING - i - 1; j++) {
          if (tmpSensorValues[j] > tmpSensorValues[j + 1]) {
              // меняем элементы местами
              temp = tmpSensorValues[j];
              tmpSensorValues[j] = tmpSensorValues[j + 1];
              tmpSensorValues[j + 1] = temp;
          }
      }
  }
  return tmpSensorValues[AVERAGING/2];
}

int arrCounter = 0;
void loop() {
  // Создаем короткий импульс длительностью 5 микросекунд.
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  // Установим высокий уровень сигнала
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  //  Определяем задержку сигнала
  duration = pulseIn(PIN_ECHO, HIGH);
  // Преобразуем время задержки в расстояние
  cm = (duration / 2) / 29.1;
  //Serial.print(cm); Serial.println(" unfiltered cm.");
  sensorValues[arrCounter] = cm;

  averageCm = middleValue();

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(3);
  display.print(map(averageCm, LVL_FULL, LVL_EMPTY, 100, 0)); display.println("%");
  display.setTextSize(1);
  display.print(averageCm); display.println("cm");

  if(filling){
    display.setTextSize(3);
    display.println("F");
  }
  display.display();
  
  Serial.print(averageCm); Serial.println(" см.");
  
  //Serial.println(digitalRead(PIN_START_BTN));

  if(!digitalRead(PIN_MAX_SENSOR))
    Serial.println("ok");

  if(!filling && averageCm >= LVL_EMPTY){
    startFilling();
  } 
  if(filling && (averageCm <= LVL_FULL || !digitalRead(PIN_MAX_SENSOR) ) ){
    stopFilling();
  }

  if(arrCounter < AVERAGING)
    arrCounter ++;  
  else
    arrCounter = 0;

  delay(500);
}


