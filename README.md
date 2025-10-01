# Sistema de Alerta ORP con Arduino

Este repositorio contiene el código fuente para la lectura del potencial de óxido-reducción (ORP) en agua tratada usando un sensor analógico ORP de Atlas Scientific, conectado a una tarjeta de desarrollo Arduino.

## 📦 Requisitos

- Arduino UNO, MEGA o R4
- Sensor Gravity ORP Meter V2.0 de Atlas Scientific
- Cable SMA y sonda ORP
- Conexión a puerto analógico (A0)
- Arduino IDE (https://www.arduino.cc/en/software)
- Monitor serial para visualización de datos

## ⚙️ Instalación

1. **Descarga este repositorio**:
   ```bash
   git clone https://github.com/VillarrealA/ORPSensor/alerta-orp.git
   ```
2. **Abre `Codigo_alertas.ino` con Arduino IDE**.

3. **Configura la tarjeta y el puerto**:
   - Placa: selecciona tu modelo (UNO, MEGA o R4).
   - Puerto: selecciona el COM correspondiente.

4. **Sube el código al Arduino**.

## 🔌 Conexiones

| Sensor ORP       | Arduino       |
|------------------|---------------|
| VCC              | 5V o 3.3V     |
| GND              | GND           |
| OUT              | A0            |

> **Nota**: Asegúrate de realizar las conexiones en un entorno libre de interferencias eléctricas. Si observas lecturas inestables, considera el uso de un aislador analógico.

## 🧪 Calibración

Para una medición precisa, usa una solución de calibración estándar (por ejemplo, 225 mV):

1. Sumerge el sensor en la solución.
2. Abre el monitor serial.
3. Envía el comando: `CAL,225`.
4. Espera confirmación de calibración exitosa.

## 📈 Conversión de voltaje a ORP

El sensor entrega un voltaje entre 0V y 3V, que puede convertirse a mV usando la ecuación lineal:

```
ORP (mV) = ((Voltaje en V) - 1.5) * 1000
```

## 📋 Licencia

Este código se distribuye bajo la licencia MIT. Ver el archivo `LICENSE` para más información.

## 👨‍🔬 Autores

Desarrollado por:

- Escobar A.
- Villarreal A.
- Rojas-Olivos A. (CIIDIR Oaxaca - IPN)

## 📫 Contacto

Si tienes dudas o sugerencias, puedes escribir a: [alejandro.v@ciencias.unam.mx](mailto:alejandro.v@ciencias.unam.mx)
