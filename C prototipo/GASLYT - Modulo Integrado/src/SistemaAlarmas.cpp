#include "SistemaAlarmas.h"

SistemaAlarmas::SistemaAlarmas(int pinLED, int pinBuzzer, int pinExtractor, bool extractorAlambrico) :
    pinBuzzer(pinBuzzer), pinExtractor(pinExtractor), extractorAlambrico(extractorAlambrico),
    estadoActual(NORMAL), ultimaAlarma(0), intervaloAlarma(1000), alarmaActiva(false) {
    
    // Inicializar LED RGB
    ledRGB = new Adafruit_NeoPixel(1, pinLED, NEO_GRB + NEO_KHZ800);
}

SistemaAlarmas::~SistemaAlarmas() {
    if (ledRGB) {
        delete ledRGB;
    }
}

bool SistemaAlarmas::inicializar() {
    // Inicializar LED RGB
    if (ledRGB) {
        ledRGB->begin();
        ledRGB->setBrightness(50); // Brillo moderado para ahorrar energía
        ledRGB->show();
    }
    
    // Configurar pines
    if (pinBuzzer >= 0) {
        pinMode(pinBuzzer, OUTPUT);
        digitalWrite(pinBuzzer, LOW);
    }
    
    if (pinExtractor >= 0 && extractorAlambrico) {
        pinMode(pinExtractor, OUTPUT);
        digitalWrite(pinExtractor, LOW);
    }
    
    // Estado inicial
    estadoActual = NORMAL;
    alarmaActiva = false;
    
    Serial.println("Sistema de alarmas inicializado");
    Serial.println("Pin LED: " + String(ledRGB->getPin()));
    Serial.println("Pin Buzzer: " + String(pinBuzzer));
    Serial.println("Pin Extractor: " + String(pinExtractor));
    Serial.println("Extractor alámbrico: " + String(extractorAlambrico ? "Sí" : "No"));
    
    return true;
}

void SistemaAlarmas::actualizarEstado(EstadoSistema estado) {
    if (estado == estadoActual) {
        return; // No cambiar si es el mismo estado
    }
    
    estadoActual = estado;
    alarmaActiva = (estado == ALARMA);
    
    Serial.println("Estado del sistema cambiado a: " + String(estado));
    
    // Aplicar cambios según el estado
    switch (estado) {
        case NORMAL:
            indicarNormal();
            break;
        case ADVERTENCIA:
            indicarAdvertencia();
            break;
        case ALARMA:
            indicarAlarma();
            break;
        case SIN_WIFI:
            indicarSinWifi();
            break;
        case ERROR_SENSOR:
            indicarError();
            break;
    }
}

void SistemaAlarmas::procesarAlarmas() {
    unsigned long tiempoActual = millis();
    
    // Procesar alarmas periódicas
    if (tiempoActual - ultimaAlarma >= intervaloAlarma) {
        ultimaAlarma = tiempoActual;
        
        switch (estadoActual) {
            case ADVERTENCIA:
                reproducirSonido(sonidoAdvertencia);
                break;
            case ALARMA:
                reproducirSonido(sonidoAlarma);
                break;
            case SIN_WIFI:
                reproducirSonido(sonidoSinWifi);
                break;
            case ERROR_SENSOR:
                reproducirSonido(sonidoError);
                break;
            default:
                break;
        }
    }
}

void SistemaAlarmas::establecerColor(uint8_t rojo, uint8_t verde, uint8_t azul) {
    if (ledRGB) {
        ledRGB->setPixelColor(0, ledRGB->Color(rojo, verde, azul));
        ledRGB->show();
    }
}

void SistemaAlarmas::establecerColor(ColorRGB color) {
    establecerColor(color.rojo, color.verde, color.azul);
}

void SistemaAlarmas::parpadear(int veces, int intervalo) {
    if (!ledRGB) return;
    
    for (int i = 0; i < veces; i++) {
        ledRGB->setPixelColor(0, ledRGB->Color(255, 255, 255));
        ledRGB->show();
        delay(intervalo / 2);
        
        ledRGB->setPixelColor(0, ledRGB->Color(0, 0, 0));
        ledRGB->show();
        delay(intervalo / 2);
    }
}

void SistemaAlarmas::apagarLED() {
    if (ledRGB) {
        ledRGB->setPixelColor(0, ledRGB->Color(0, 0, 0));
        ledRGB->show();
    }
}

void SistemaAlarmas::reproducirSonido(SonidoAlarma sonido) {
    for (int i = 0; i < sonido.repeticiones; i++) {
        tocarBuzzer(sonido.frecuencia, sonido.duracion);
        if (i < sonido.repeticiones - 1) {
            delay(sonido.pausaEntreRepeticiones);
        }
    }
}

void SistemaAlarmas::tocarBuzzer(int frecuencia, int duracion) {
    if (pinBuzzer >= 0) {
        tone(pinBuzzer, frecuencia, duracion);
    }
}

void SistemaAlarmas::apagarBuzzer() {
    if (pinBuzzer >= 0) {
        noTone(pinBuzzer);
    }
}

void SistemaAlarmas::activarExtractor() {
    if (pinExtractor >= 0 && extractorAlambrico) {
        digitalWrite(pinExtractor, HIGH);
        Serial.println("Extractor activado");
    } else if (!extractorAlambrico) {
        // Enviar señal inalámbrica (implementar según protocolo)
        Serial.println("Señal inalámbrica enviada para activar extractor");
    }
}

void SistemaAlarmas::desactivarExtractor() {
    if (pinExtractor >= 0 && extractorAlambrico) {
        digitalWrite(pinExtractor, LOW);
        Serial.println("Extractor desactivado");
    } else if (!extractorAlambrico) {
        // Enviar señal inalámbrica (implementar según protocolo)
        Serial.println("Señal inalámbrica enviada para desactivar extractor");
    }
}

void SistemaAlarmas::toggleExtractor() {
    if (pinExtractor >= 0 && extractorAlambrico) {
        bool estadoActual = digitalRead(pinExtractor);
        digitalWrite(pinExtractor, !estadoActual);
        Serial.println("Extractor " + String(!estadoActual ? "activado" : "desactivado"));
    }
}

// Estados específicos
void SistemaAlarmas::indicarNormal() {
    establecerColor(colorNormal);
    apagarBuzzer();
    desactivarExtractor();
    Serial.println("Estado: NORMAL - LED Verde");
}

void SistemaAlarmas::indicarAdvertencia() {
    establecerColor(colorAdvertencia);
    Serial.println("Estado: ADVERTENCIA - LED Amarillo");
}

void SistemaAlarmas::indicarAlarma() {
    establecerColor(colorAlarma);
    activarExtractor();
    Serial.println("Estado: ALARMA - LED Rojo, Extractor activado");
}

void SistemaAlarmas::indicarSinWifi() {
    establecerColor(colorSinWifi);
    Serial.println("Estado: SIN WIFI - LED Azul");
}

void SistemaAlarmas::indicarError() {
    establecerColor(colorError);
    Serial.println("Estado: ERROR - LED Magenta");
}

void SistemaAlarmas::establecerIntervaloAlarma(unsigned long intervalo) {
    intervaloAlarma = intervalo;
}

void SistemaAlarmas::establecerExtractorAlambrico(bool alambrico) {
    extractorAlambrico = alambrico;
}

void SistemaAlarmas::establecerPinExtractor(int pin) {
    pinExtractor = pin;
    if (pin >= 0 && extractorAlambrico) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}

// Getters
SistemaAlarmas::EstadoSistema SistemaAlarmas::obtenerEstadoActual() const {
    return estadoActual;
}

bool SistemaAlarmas::esAlarmaActiva() const {
    return alarmaActiva;
}

bool SistemaAlarmas::esExtractorActivo() const {
    if (pinExtractor >= 0 && extractorAlambrico) {
        return digitalRead(pinExtractor) == HIGH;
    }
    return false;
}

// Utilidades
void SistemaAlarmas::imprimirEstado() const {
    Serial.println("=== ESTADO SISTEMA ALARMAS ===");
    Serial.println("Estado actual: " + String(estadoActual));
    Serial.println("Alarma activa: " + String(alarmaActiva ? "Sí" : "No"));
    Serial.println("Extractor activo: " + String(esExtractorActivo() ? "Sí" : "No"));
    Serial.println("Extractor alámbrico: " + String(extractorAlambrico ? "Sí" : "No"));
    Serial.println("Pin LED: " + String(ledRGB ? ledRGB->getPin() : -1));
    Serial.println("Pin Buzzer: " + String(pinBuzzer));
    Serial.println("Pin Extractor: " + String(pinExtractor));
    Serial.println("Intervalo alarma: " + String(intervaloAlarma) + " ms");
    Serial.println("===============================");
}

void SistemaAlarmas::resetear() {
    estadoActual = NORMAL;
    alarmaActiva = false;
    ultimaAlarma = 0;
    
    apagarLED();
    apagarBuzzer();
    desactivarExtractor();
    
    Serial.println("Sistema de alarmas reseteado");
}

void SistemaAlarmas::apagarTodo() {
    apagarLED();
    apagarBuzzer();
    desactivarExtractor();
    Serial.println("Todos los elementos del sistema de alarmas apagados");
}
