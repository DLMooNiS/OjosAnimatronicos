#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN 500   // Valor mínimo calculado (500us)
#define SERVOMAX 2500  // Valor máximo calculado (2500us)
#define SERVO_FREQ 50

// --- Configuración del Botón ---
#define PIN_BOTON 4            // Usaremos el Pin 2 para el botón
bool ultimoEstadoBoton = LOW;  // Para detectar el cambio de estado
bool movimientoActivo = false;
int servomin_ticks = 0;
int servomax_ticks = 0;

// -- Configuración Jostik
int xPin = A0; // Pin analógico para el eje X
int yPin = A1; // Pin analógico para el eje Y
int buttonPin = 2; // Pin digital para el botón

// Estructura para almacenar los datos del joystick
struct JoystickData {
  int xValue;
  int yValue;
  int buttonState;
};


//CANALES PCA9685
#define SERVO_PESTANADER 4
#define SERVO_PESTANAIZQ 5
#define SERVO_OJODERX 8
#define SERVO_OJODERY 9
#define SERVO_OJOIZQX 12
#define SERVO_OJOIZQY 13

//CALIBRACIÓN ANGULOS GIRO
#define PESTANADER_CERRADA 77
#define PESTANADER_ABIERTA 20
#define PESTANADER_MIN 10
#define PESTANAIZQ_CERRADA 25
#define PESTANAIZQ_ABIERTA 75
#define PESTANAIZQ_MIN 85

#define OJODER_XMAX 70
#define OJODER_XMIN 0
#define OJODER_XCENT 35
#define OJODER_YMAX 80
#define OJODER_YMIN 10
#define OJODER_YCENT 45
#define OJODER_YTOPEARRIBA OJODER_YMAX
#define OJODER_YTOPEABAJO OJODER_YMIN


//#define OJOIZQ_XMAX 90
#define OJOIZQ_XMAX 70
#define OJOIZQ_XMIN 0
#define OJOIZQ_XCENT 40
#define OJOIZQ_YMAX 60
#define OJOIZQ_YMIN 10
#define OJOIZQ_YCENT 40
#define OJOIZQ_YTOPEARRIBA OJOIZQ_YMIN
#define OJOIZQ_YTOPEABAJO OJOIZQ_YMAX

#define LED_PIN 12

// Variables globales para el movimiento suave
int anguloActual[16] = { 0 };  // Almacena el último ángulo enviado a cada canal

// Función manual para convertir microsegundos a ticks (cuentas de 12 bits)
// El chip PCA9685 tiene 4096 "ticks" por ciclo de PWM (12 bits)
uint16_t pulseWidthToTicks(uint16_t microseconds) {
  // Periodo total en microsegundos: (1 / Frecuencia) * 1,000,000 µs
  // Con 50 Hz, el periodo es 20,000 µs.
  // La fórmula de conversión es: (microsegundos / Periodo Total) * 4096

  // Usamos números flotantes para mayor precisión en el cálculo
  float pulse_length_us = (float)microseconds;
  float total_period_us = (1000000.0f / SERVO_FREQ);

  // Convertimos a ticks (cuentas de 12 bits), redondeando al entero más cercano
  uint16_t ticks = (uint16_t)((pulse_length_us / total_period_us) * 4096.0f + 0.5f);
  return ticks;
}


//INICIALIZACIÓN
void setup() {
  Serial.begin(9600);
  Serial.println("Control de Servo Mizuzel MF90 con Pausa/Reanudar!");

  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ);

  // Convertir los valores de microsegundos de configuración del SERVO a ticks y almacenarlos
  servomin_ticks = pulseWidthToTicks(SERVOMIN);
  servomax_ticks = pulseWidthToTicks(SERVOMAX);

  // Configura el pin del botón con la resistencia interna PULLUP activa
  pinMode(PIN_BOTON, INPUT);
  delay(10);

  pinMode(xPin, INPUT); // Configura los pines como entradas
  pinMode(yPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Activa la resistencia pull-up interna para el botón

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

//BOTON PAUSA
// --- Lógica del Botón de Pausa ---
// Leer el estado actual del botón (LOW cuando está pulsado, HIGH cuando no)

void verificaEstadoBoton() {

  bool estadoActualBoton = digitalRead(PIN_BOTON);
  // Detectar si el botón acaba de ser pulsado (transición de HIGH a LOW)
  if (estadoActualBoton == HIGH && ultimoEstadoBoton == LOW) {
    // Esperar un breve momento para el antirrebote (debouncing simple)
    delay(50);

    // Confirmar que sigue pulsado
    if (digitalRead(PIN_BOTON) == HIGH) {
      // Alternar el estado de movimiento (toggle):
      movimientoActivo = !movimientoActivo;
      if (movimientoActivo) {
        Serial.println("Movimiento REANUDADO");
        digitalWrite(LED_PIN, HIGH);
      } else {
        Serial.println("Movimiento PAUSADO");
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
  // Guardar el estado actual para la próxima iteración
  ultimoEstadoBoton = estadoActualBoton;
}


// Función para convertir un ángulo (0-180) a un valor de pulso (ticks)
int angleToPulse(int ang) {
  int pulse = map(ang, 0, 180, servomin_ticks, servomax_ticks);
  return pulse;
}

//MOVIMIENTO DEL SERVO
void moverServo(bool noCheck,int canal, int angulo) {
  verificaEstadoBoton();  //Comprobamos siempre antes si tenemos que continuar o parar
  if (movimientoActivo || noCheck) {
    pwm.setPWM(canal, 0, angleToPulse(angulo));
    Serial.println("Moviendo Servo[" + String(canal) + "] a " + String(angulo) + "º");
  }
}

void pestanear(bool noCheck=false) {
  moverServo(noCheck,SERVO_PESTANADER, PESTANADER_ABIERTA);
  moverServo(noCheck,SERVO_PESTANAIZQ, PESTANAIZQ_ABIERTA);
  delay(300);
  moverServo(noCheck,SERVO_PESTANADER, PESTANADER_CERRADA);
  moverServo(noCheck,SERVO_PESTANAIZQ, PESTANAIZQ_CERRADA);
  delay(300);
  moverServo(noCheck,SERVO_PESTANADER, PESTANADER_ABIERTA);
  moverServo(noCheck,SERVO_PESTANAIZQ, PESTANAIZQ_ABIERTA);
}

void cerrarPestanas(bool noCheck) {
  moverServo(noCheck,SERVO_PESTANADER, PESTANADER_CERRADA);
  moverServo(noCheck,SERVO_PESTANAIZQ, PESTANAIZQ_CERRADA);
  delay(300);
}

void movimientoOjos(bool noCheck) {
  moverServo(noCheck,SERVO_OJODERX, OJODER_XMIN);
  //moverServo(noCheck,SERVO_OJOIZQX,OJOIZQ_XMIN);

  delay(1000);
  moverServo(noCheck,SERVO_OJODERX, OJODER_XMAX);
  delay(300);
}

void testMovimientoOjoDERY() {
  moverServo(false,SERVO_OJODERY, 0);
  delay(500);
  moverServo(false,SERVO_OJODERY, 10);
  delay(500);
  moverServo(false,SERVO_OJODERY, 20);
  delay(500);
  moverServo(false,SERVO_OJODERY, 30);
  delay(500);
  moverServo(false,SERVO_OJODERY, 40);
  delay(500);
  moverServo(false,SERVO_OJODERY, 50);
  delay(500);
  moverServo(false,SERVO_OJODERY, 60);
  delay(500);
  moverServo(false,SERVO_OJODERY, 70);
  delay(500);
  moverServo(false,SERVO_OJODERY, 80);
  delay(500);
}

void testMovimientoOjoDERX() {
  moverServo(false,SERVO_OJODERX, 0);
  delay(500);
  moverServo(false,SERVO_OJODERX, 10);
  delay(500);
  moverServo(false,SERVO_OJODERX, 20);
  delay(500);
  moverServo(false,SERVO_OJODERX, 30);
  delay(500);
  moverServo(false,SERVO_OJODERX, 40);
  delay(500);
  moverServo(false,SERVO_OJODERX, 50);
  delay(500);
  moverServo(false,SERVO_OJODERX, 60);
  delay(500);
  moverServo(false,SERVO_OJODERX, 70);
  delay(500);
}

void testMovimientoOjoIZQY() {
  moverServo(false,SERVO_OJOIZQY, 0);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 10);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 20);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 30);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 40);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 50);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 60);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 70);
  delay(500);
  moverServo(false,SERVO_OJOIZQY, 80);
  delay(500);
}

void testMovimientoOjoIZQX() {
  moverServo(false,SERVO_OJOIZQX, 0);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 10);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 20);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 30);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 40);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 50);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 60);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 70);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 80);
  delay(500);
  moverServo(false,SERVO_OJOIZQX, 90);
  delay(500);
}


void centrarOjos(bool noCheck) {
  moverServo(noCheck,SERVO_OJODERY, OJODER_YCENT);
  moverServo(noCheck,SERVO_OJODERX, OJODER_XCENT);
  moverServo(noCheck,SERVO_OJOIZQY, OJOIZQ_YCENT);
  moverServo(noCheck,SERVO_OJOIZQX, OJOIZQ_XCENT);
}

void barridoIzqDer(bool noCheck) {
  //Centramos
  moverServo(noCheck,SERVO_OJODERY, OJODER_YCENT);
  moverServo(noCheck,SERVO_OJOIZQY, OJOIZQ_YCENT);
  delay(300);
  moverServo(noCheck,SERVO_OJODERX, OJODER_XMIN);
  moverServo(noCheck,SERVO_OJOIZQX, OJOIZQ_XMIN);
  delay(300);
  moverServo(noCheck,SERVO_OJODERX, OJODER_XMAX);
  moverServo(noCheck,SERVO_OJOIZQX, OJOIZQ_XMAX);
  delay(500);
}

// --- NUEVA FUNCIÓN: MOVIMIENTO SUAVE COORDINADO ---
// Mueve los cuatro servos de ojos (X, Y de ambos) simultáneamente.
void smoothMoveCoordinated(int derX_final, int derY_final, int izqX_final, int izqY_final, int velocidad) {
  verificaEstadoBoton();
  if (!movimientoActivo) return;
  smoothMoveCoordinatedBase(derX_final,derY_final,izqX_final,izqY_final,velocidad);
}


// --- MOVIMIENTO SUAVE COORDINADO  - FUNCION BASE ---
// Mueve los cuatro servos de ojos (X, Y de ambos) simultáneamente.
void smoothMoveCoordinatedBase(int derX_final, int derY_final, int izqX_final, int izqY_final, int velocidad) {
  // 1. Definir los puntos de inicio
  int derX_inicio = anguloActual[SERVO_OJODERX];
  int derY_inicio = anguloActual[SERVO_OJODERY];
  int izqX_inicio = anguloActual[SERVO_OJOIZQX];
  int izqY_inicio = anguloActual[SERVO_OJOIZQY];

  // 2. Calcular la distancia máxima para determinar el número de pasos
  // Usamos la distancia más grande entre cualquier eje.
  int dist_derX = abs(derX_final - derX_inicio);
  int dist_derY = abs(derY_final - derY_inicio);
  int dist_izqX = abs(izqX_final - izqX_inicio);
  int dist_izqY = abs(izqY_final - izqY_inicio);

  int max_dist = max(max(dist_derX, dist_derY), max(dist_izqX, dist_izqY));

  // Si no hay movimiento, salimos.
  if (max_dist == 0) return;

  // 3. Calcular los pasos (step) para cada servo, manteniendo la proporción
  float step_derX = (float)(derX_final - derX_inicio) / max_dist;
  float step_derY = (float)(derY_final - derY_inicio) / max_dist;
  float step_izqX = (float)(izqX_final - izqX_inicio) / max_dist;
  float step_izqY = (float)(izqY_final - izqY_inicio) / max_dist;

  // 4. Iterar y mover los servos en paralelo
  for (int i = 1; i <= max_dist; i++) {
    // Calcular la nueva posición redondeada
    int current_derX = derX_inicio + round(step_derX * i);
    int current_derY = derY_inicio + round(step_derY * i);
    int current_izqX = izqX_inicio + round(step_izqX * i);
    int current_izqY = izqY_inicio + round(step_izqY * i);

    // Enviar los comandos a la vez
    pwm.setPWM(SERVO_OJODERX, 0, angleToPulse(current_derX));
    pwm.setPWM(SERVO_OJODERY, 0, angleToPulse(current_derY));
    pwm.setPWM(SERVO_OJOIZQX, 0, angleToPulse(current_izqX));
    pwm.setPWM(SERVO_OJOIZQY, 0, angleToPulse(current_izqY));

    // Actualizar la posición actual para el próximo ciclo
    anguloActual[SERVO_OJODERX] = current_derX;
    anguloActual[SERVO_OJODERY] = current_derY;
    anguloActual[SERVO_OJOIZQX] = current_izqX;
    anguloActual[SERVO_OJOIZQY] = current_izqY;

    delay(velocidad);
  }

  // 5. Asegurarse de que todos lleguen exactamente al destino final (sin floating point errors)
  pwm.setPWM(SERVO_OJODERX, 0, angleToPulse(derX_final));
  pwm.setPWM(SERVO_OJODERY, 0, angleToPulse(derY_final));
  pwm.setPWM(SERVO_OJOIZQX, 0, angleToPulse(izqX_final));
  pwm.setPWM(SERVO_OJOIZQY, 0, angleToPulse(izqY_final));

  anguloActual[SERVO_OJODERX] = derX_final;
  anguloActual[SERVO_OJODERY] = derY_final;
  anguloActual[SERVO_OJOIZQX] = izqX_final;
  anguloActual[SERVO_OJOIZQY] = izqY_final;
}




void randomFixation() {
  int der_x = OJODER_XCENT + random(-5, 6);
  int izq_x = OJOIZQ_XCENT + random(-5, 6);
  int der_y = OJODER_YCENT + random(-5, 6);
  int izq_y = OJOIZQ_YCENT + random(-5, 6);

  der_x = constrain(der_x, OJODER_XMIN, OJODER_XMAX);
  izq_x = constrain(izq_x, OJOIZQ_XMIN, OJOIZQ_XMAX);
  der_y = constrain(der_y, OJODER_YMIN, OJODER_YMAX);
  izq_y = constrain(izq_y, OJOIZQ_YMIN, OJOIZQ_YMAX);
  verificaEstadoBoton();
  if (!movimientoActivo) return;
  smoothMoveCoordinated(der_x, der_y, izq_x, izq_y, 5);
}

// --- FUNCIÓN CENTINELA COORDINADO ---
void centinela() {
  verificaEstadoBoton();
  if (!movimientoActivo) return;

  int v_lenta = 10;
  int v_media = 7;
  int v_rapida = 5;

  // 1. Mirada inicial y pequeña fijación aleatoria
  randomFixation();
  delay(500);
  pestanear();
  delay(1000);

  // 2. Barrido Lento y Suave Derecha (Coordinado)
  smoothMoveCoordinated(OJODER_XMAX, OJODER_YCENT, OJOIZQ_XMAX, OJOIZQ_YCENT, v_lenta);
  delay(1000);
  randomFixation();
  pestanear();
  delay(500);

  // 3. Barrido Lento y Suave Izquierda (Coordinado)
  smoothMoveCoordinated(OJODER_XMIN, OJODER_YCENT, OJOIZQ_XMIN, OJOIZQ_YCENT, v_lenta);
  delay(1000);
  randomFixation();
  delay(500);

  // 4. Mirada Suave Arriba (Coordinado)
  smoothMoveCoordinated(OJODER_XCENT, OJODER_YTOPEARRIBA, OJOIZQ_XCENT, OJOIZQ_YTOPEARRIBA, v_media);
  delay(700);

  // 5. Mirada Suave Abajo (Coordinado)
  smoothMoveCoordinated(OJODER_XCENT, OJODER_YTOPEABAJO, OJOIZQ_XCENT, OJOIZQ_YTOPEABAJO, v_media);
  delay(700);

  // --- NUEVA SECCIÓN DE BARRIDOS DE VIGILANCIA ---

  // 2. Barrido Horizontal Superior (Busca a lo lejos o en la parte de arriba)
  smoothMoveCoordinated(OJODER_XMIN, OJODER_YTOPEARRIBA, OJOIZQ_XMIN, OJOIZQ_YTOPEARRIBA, v_media);  // IZQUIERDA-ARRIBA
  delay(300);
  smoothMoveCoordinated(OJODER_XMAX, OJODER_YTOPEARRIBA, OJOIZQ_XMAX, OJOIZQ_YTOPEARRIBA, v_media);  // DERECHA-ARRIBA
  delay(500);

  // 3. Parpadeo y vuelta al centro
  randomFixation();
  pestanear();
  delay(500);

  // 4. Barrido Horizontal Inferior (Busca cerca o en el suelo)
  smoothMoveCoordinated(OJODER_XMAX, OJODER_YTOPEABAJO, OJOIZQ_XMAX, OJOIZQ_YTOPEABAJO, v_media);  // DERECHA-ABAJO
  delay(300);
  smoothMoveCoordinated(OJODER_XMIN, OJODER_YTOPEABAJO, OJOIZQ_XMIN, OJOIZQ_YTOPEABAJO, v_media);  // IZQUIERDA-ABAJO
  delay(500);

  // 5. Barrido Oblicuo (Simula rastreo diagonal: Abajo-Izq a Arriba-Der)
  smoothMoveCoordinated(OJODER_XMIN, OJODER_YTOPEABAJO, OJOIZQ_XMIN, OJOIZQ_YTOPEABAJO, v_lenta);
  delay(500);
  smoothMoveCoordinated(OJODER_XMAX, OJODER_YTOPEARRIBA, OJOIZQ_XMAX, OJOIZQ_YTOPEARRIBA, v_lenta);
  delay(1000);

  // --- FIN SECCIÓN BARRIDOS ---


  // 6. Vuelve al centro (fijación aleatoria) y parpadeo largo
  randomFixation();
  pestanear();
  delay(3000);  // Pausa más larga antes de repetir la secuencia
}

// Función que lee los valores del joystick y devuelve una estructura JoystickData
JoystickData readJoystickValues() {
  JoystickData data; // Crea una instancia de la estructura
  data.xValue = analogRead(xPin);
  data.yValue = analogRead(yPin);
  data.buttonState = digitalRead(buttonPin);
  return data; // Devuelve la estructura completa
}

void procesaInputJostick(){
    // Llama a la función y almacena los resultados en una variable local
  JoystickData currentData = readJoystickValues();
  // Imprime las lecturas
  Serial.print("X: ");
  Serial.print(currentData.xValue);
  Serial.print(" | Y: ");
  Serial.print(currentData.yValue);
  Serial.print(" | Botón: ");
  Serial.println(currentData.buttonState);
  delay(100);
}

// --- FUNCIÓN: CONTROL DE OJOS CON JOYSTICK (Consonancia Asegurada) ---
void controlOjosConJoystick() {
  verificaEstadoBoton();
  // El control del joystick solo debe ejecutarse si el movimiento automático está activo (o no pausado)
  // Depende de tu lógica: si quieres control manual en modo PAUSA, esta línea debe ser revisada.
  // Mantenemos la lógica anterior: el control manual solo ocurre si !movimientoActivo está en loop().
  if (movimientoActivo) return; 

  JoystickData currentData = readJoystickValues(); 
  int v_rapida = 5; // Velocidad para el movimiento suave.

  // 1. Lógica de Parpadeo con Botón del Joystick
  if (currentData.buttonState == LOW) {
    pestanear(true);
  }

  // 2. Mapeo del Eje X (Horizontal)
  // Ambos ojos se mueven de IZQUIERDA (Joystick 0) a DERECHA (Joystick 1020).
  // La dirección angular es consistente para ambos ojos: de MIN a MAX.
  
  // Ojo Derecho X: Mapeo de 0-1020 -> OJODER_XMIN(0) a OJODER_XMAX(70)
  int anguloX_derecho = map(currentData.xValue, 0, 1020, OJODER_XMIN, OJODER_XMAX);

  // Ojo Izquierdo X: Mapeo de 0-1020 -> OJOIZQ_XMIN(0) a OJOIZQ_XMAX(90)
  int anguloX_izquierdo = map(currentData.xValue, 0, 1020, OJOIZQ_XMIN, OJOIZQ_XMAX);


  // 3. Mapeo del Eje Y (Vertical)
  // Ambos ojos deben mirar ARRIBA (Joystick 0) o ABAJO (Joystick 1020).
  // Los rangos deben usarse según su definición TOPEARRIBA y TOPEABAJO.

  // Ojo Derecho Y: Mapeo de 0-1020 -> ARRIBA(80) a ABAJO(0)
  int anguloY_derecho = map(currentData.yValue, 0, 1020, OJODER_YTOPEARRIBA, OJODER_YTOPEABAJO);

  // Ojo Izquierdo Y: Mapeo de 0-1020 -> ARRIBA(0) a ABAJO(90)
  int anguloY_izquierdo = map(currentData.yValue, 0, 1020, OJOIZQ_YTOPEARRIBA, OJOIZQ_YTOPEABAJO);

  // 4. Mover de forma suave y coordinada
  // El smoothMoveCoordinated garantiza la "consonancia en movimiento" al mover todos
  // los ejes simultáneamente, asegurando que lleguen al destino al mismo tiempo.
  smoothMoveCoordinatedBase(anguloX_derecho, anguloY_derecho, anguloX_izquierdo, anguloY_izquierdo, v_rapida);

  // Debugging
  Serial.print("X-> D:"); Serial.print(anguloX_derecho); Serial.print(" I:"); Serial.print(anguloX_izquierdo);
  Serial.print(" | Y-> D:"); Serial.print(anguloY_derecho); Serial.print(" I:"); Serial.print(anguloY_izquierdo);
  Serial.println();

  delay(10); 
}

// ... (El resto del código, incluyendo loop(), queda igual) ...


void loop() {

  // --- Lógica del Movimiento del Servo ---
  // Este bloque SÓLO se ejecuta si movimientoActivo es TRUE
  // Tu bucle original de movimiento:
  verificaEstadoBoton();  //Comprobamos siempre antes si tenemos que continuar o parar
  if (!movimientoActivo) {
    controlOjosConJoystick();
  }else{    
    centrarOjos(false);
    pestanear(false);
    delay(500);
    barridoIzqDer(false);
    centinela();
  }

  
  // Si movimientoActivo es FALSE, el código no entra en el IF y el loop()
  // se repite rápidamente, pero el servo no recibe nuevos comandos hasta que se reanuda.
}
