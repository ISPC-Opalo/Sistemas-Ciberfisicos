#include "GasSensor.h"

GasSensor::GasSensor(int pin, const String& tipo) : 
    pinSensor(pin), tipoSensor(tipo), umbralAlarma(1000.0), 
    ultimaLectura(0.0), ultimaMedicion(0), alarmaActiva(false) {
    
    sensor = new MQUnifiedsensor("ESP32", 3.3, 12, pin, "MQ-2");
    
    // Configuración por defecto
    config.tipo = tipo;
    config.voltaje = 3.3;
    config.pin = pin;
    config.unidad = "ppm";
    config.ratioAireLimpio = 9.83; // Valor típico para MQ-2
}

GasSensor::~GasSensor() {
    if (sensor) {
        delete sensor;
    }
}

bool GasSensor::inicializar() {
    if (!sensor) {
        Serial.println("Error: Sensor no inicializado");
        return false;
    }
    
    // Configurar el sensor
    sensor->setRegressionMethod(1); // PPM =  a*ratio^b
    sensor->setA(987.99); 
    sensor->setB(-2.162); // Configurar valores A y B según el gas a detectar
    
    // Configurar ratio de aire limpio
    sensor->setR0(establecerRatioAireLimpio(config.ratioAireLimpio));
    
    Serial.println("Sensor de gas inicializado correctamente");
    Serial.println("Tipo: " + tipoSensor);
    Serial.println("Pin: " + String(pinSensor));
    Serial.println("Ratio aire limpio: " + String(config.ratioAireLimpio));
    
    return true;
}

float GasSensor::leerConcentracion() {
    if (!sensor) {
        return -1.0;
    }
    
    // Leer valor analógico
    sensor->update();
    
    // Obtener lectura en PPM
    float lectura = sensor->readSensor();
    
    // Validar lectura
    if (esLecturaValida(lectura)) {
        ultimaLectura = lectura;
        ultimaMedicion = millis();
        
        // Verificar si supera el umbral
        alarmaActiva = (lectura >= umbralAlarma);
        
        return lectura;
    } else {
        Serial.println("Error: Lectura inválida del sensor");
        return -1.0;
    }
}

bool GasSensor::verificarUmbral() {
    return alarmaActiva;
}

void GasSensor::calibrar() {
    if (!sensor) {
        return;
    }
    
    Serial.println("Iniciando calibración del sensor...");
    Serial.println("Asegúrese de que el sensor esté en aire limpio");
    
    // Esperar 20 segundos para estabilización
    for (int i = 20; i > 0; i--) {
        Serial.println("Calibrando... " + String(i) + " segundos restantes");
        delay(1000);
    }
    
    // Calibrar R0
    sensor->setR0(establecerRatioAireLimpio(config.ratioAireLimpio));
    
    Serial.println("Calibración completada");
    Serial.println("Nuevo R0: " + String(sensor->getR0()));
}

void GasSensor::establecerUmbral(float umbral) {
    if (umbral > 0) {
        umbralAlarma = umbral;
        Serial.println("Umbral de alarma establecido en: " + String(umbral) + " ppm");
    }
}

void GasSensor::establecerTipoSensor(const String& tipo) {
    tipoSensor = tipo;
    
    // Configurar parámetros según el tipo de sensor
    if (tipo == "MQ-2") {
        sensor->setA(987.99);
        sensor->setB(-2.162);
        config.ratioAireLimpio = 9.83;
    } else if (tipo == "MQ-3") {
        sensor->setA(0.39);
        sensor->setB(-1.504);
        config.ratioAireLimpio = 60.0;
    } else if (tipo == "MQ-4") {
        sensor->setA(1012.7);
        sensor->setB(-2.786);
        config.ratioAireLimpio = 4.4;
    } else if (tipo == "MQ-5") {
        sensor->setA(1163.8);
        sensor->setB(-3.874);
        config.ratioAireLimpio = 6.5;
    } else if (tipo == "MQ-6") {
        sensor->setA(2127.2);
        sensor->setB(-2.526);
        config.ratioAireLimpio = 10.0;
    } else if (tipo == "MQ-7") {
        sensor->setA(99.042);
        sensor->setB(-1.518);
        config.ratioAireLimpio = 27.5;
    } else if (tipo == "MQ-8") {
        sensor->setA(976.97);
        sensor->setB(-0.688);
        config.ratioAireLimpio = 70.0;
    } else if (tipo == "MQ-9") {
        sensor->setA(599.65);
        sensor->setB(-2.244);
        config.ratioAireLimpio = 9.6;
    } else if (tipo == "MQ-135") {
        sensor->setA(110.47);
        sensor->setB(-2.862);
        config.ratioAireLimpio = 3.6;
    }
    
    config.tipo = tipo;
    Serial.println("Tipo de sensor cambiado a: " + tipo);
}

void GasSensor::establecerRatioAireLimpio(float ratio) {
    config.ratioAireLimpio = ratio;
    if (sensor) {
        sensor->setR0(ratio);
    }
}

// Getters
float GasSensor::obtenerUltimaLectura() const {
    return ultimaLectura;
}

bool GasSensor::esAlarmaActiva() const {
    return alarmaActiva;
}

unsigned long GasSensor::obtenerTiempoUltimaMedicion() const {
    return ultimaMedicion;
}

String GasSensor::obtenerTipoSensor() const {
    return tipoSensor;
}

// Utilidades
void GasSensor::imprimirLectura() const {
    Serial.println("=== LECTURA DEL SENSOR ===");
    Serial.println("Tipo: " + tipoSensor);
    Serial.println("Pin: " + String(pinSensor));
    Serial.println("Lectura: " + String(ultimaLectura) + " " + config.unidad);
    Serial.println("Umbral: " + String(umbralAlarma) + " " + config.unidad);
    Serial.println("Alarma: " + String(alarmaActiva ? "ACTIVA" : "INACTIVA"));
    Serial.println("Tiempo última medición: " + String(ultimaMedicion) + " ms");
    Serial.println("=========================");
}

bool GasSensor::esLecturaValida(float lectura) const {
    return lectura >= 0 && lectura <= 10000; // Rango válido para sensores MQ
}

float GasSensor::convertirUnidades(float lectura, const String& unidadDestino) const {
    if (unidadDestino == "ppm") {
        return lectura; // Ya está en ppm
    } else if (unidadDestino == "mg/m3") {
        // Conversión aproximada para gas inflamable (metano)
        return lectura * 0.717; // mg/m3 = ppm * densidad del gas
    } else if (unidadDestino == "%LEL") {
        // Límite inferior de explosividad (LEL)
        return (lectura / 50000.0) * 100; // Aproximación para metano
    }
    
    return lectura; // Retornar valor original si no se puede convertir
}
