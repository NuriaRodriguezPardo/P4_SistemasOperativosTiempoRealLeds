#include <Arduino.h>
#include <semphr.h>

SemaphoreHandle_t interruptSemaphore;

void TaskLed(void *pvParameters);
void TaskBlink(void *pvParameters);
void debounceInterrupt();

const int buttonPin = 2; // Pin del botón
const long debouncing_time = 150; // Tiempo de rebote del botón en milisegundos

void setup() {
  Serial.begin(115200); // Iniciar comunicación serial
  
  pinMode(buttonPin, INPUT_PULLUP); // Configurar el pin del botón como entrada con pull-up
  pinMode(8, OUTPUT); // Configurar el pin 8 como salida para el LED controlado por la tarea TaskLed
  pinMode(7, OUTPUT); // Configurar el pin 7 como salida para el LED parpadeante controlado por la tarea TaskBlink

  interruptSemaphore = xSemaphoreCreateBinary(); // Crear semáforo binario
  if (interruptSemaphore != NULL) {
    attachInterrupt(digitalPinToInterrupt(buttonPin), debounceInterrupt, FALLING); // Asociar la interrupción al botón
  }
  
  xTaskCreate(TaskLed,  "Led", 5000, NULL, 1, NULL); // Crear tarea para controlar el LED
  xTaskCreate(TaskBlink,  "LedBlink", 5000, NULL, 2, NULL); // Crear tarea para el LED parpadeante
}

void loop() {}

void interruptHandler() {
  xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}

void TaskLed(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
      bool ledState = digitalRead(8);
      digitalWrite(8, !ledState); // Cambiar el estado del LED controlado por TaskLed
      Serial.println("LED controlado por TaskLed encendido/apagado");
    }
    vTaskDelay(200);
  }
}

void TaskBlink(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    digitalWrite(7, HIGH);
    vTaskDelay(200);
    digitalWrite(7, LOW);
    vTaskDelay(200);
  }
}

void debounceInterrupt() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  // Debouncing
  if (interrupt_time - last_interrupt_time > debouncing_time) {
    interruptHandler();
    Serial.println("Interrupción detectada");
    //delayMicroseconds(1000); // Incrementa el retardo aquí
  }
  last_interrupt_time = interrupt_time;
}
