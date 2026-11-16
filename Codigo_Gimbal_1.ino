#include <Wire.h>
#include <Servo.h>

Servo motorGimbal;
const int MPU = 0x68; 
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;

float angulo_acelerometro;
float angulo_filtrado = 0;
unsigned long tiempo_previo;

// Sintonización
float Kp = 1.2; 

void setup() {
  Wire.begin();
  
  // Acelerar la velocidad del bus I2C a 400kHz para lecturas sin lag
  Wire.setClock(400000); 
  
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  motorGimbal.attach(9);
  motorGimbal.write(90); 

  // ¡CAMBIO CLAVE! Velocidad ultra rápida para no bloquear el código
  Serial.begin(115200);
  tiempo_previo = millis();
}

void loop() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true); 

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Wire.read() << 8 | Wire.read(); 
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  unsigned long tiempo_actual = millis();
  float dt = (tiempo_actual - tiempo_previo) / 1000.0;
  tiempo_previo = tiempo_actual;

  angulo_acelerometro = atan2(AcY, AcZ) * (180.0 / PI);
  float tasa_giroscopio = GyX / 131.0;

  angulo_filtrado = 0.96 * (angulo_filtrado + tasa_giroscopio * dt) + 0.04 * angulo_acelerometro;

  float error = 0 - angulo_filtrado; 
  int posicion_servo = 90 + (Kp * error); 

  if (posicion_servo > 180) posicion_servo = 180;
  if (posicion_servo < 0)  posicion_servo = 0;

  motorGimbal.write(posicion_servo);

  // Telemetría optimizada (Sin textos largos, solo datos separados por comas)
  Serial.print(angulo_acelerometro);
  Serial.print(",");
  Serial.print(angulo_filtrado);
  Serial.print(",");
  Serial.println(posicion_servo);

  // Reducimos el delay para mayor fluidez (aprox 50Hz para el servo)
  delay(15); 
}
