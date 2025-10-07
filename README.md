# ESP32-Interactive-Learning-System
Proyecto de IoT educativo desarrollado en ESP32 con servidor local y comunicación WebSocket. Permite registrar estudiantes y realizar ejercicios interactivos que combinan sensores físicos, pantalla OLED y una interfaz web dinámica. Ideal para entornos STEAM y aprendizaje activo.


Descripción / Description

 Proyecto educativo IoT que convierte una ESP32 en un servidor web interactivo para actividades de aprendizaje físico.
Incluye dos módulos:

 Módulo 1: Secuencia de colores con sensores capacitivos y LED RGB.

 Módulo 2: Conteo de objetos (manzanas) con pantalla OLED y control DIP Switch.

🇬🇧 IoT learning system turning an ESP32 into an interactive local web server for physical learning activities.
Includes two modules:

 Module 1: Color sequence recognition using capacitive sensors and RGB LED.

 Module 2: Object counting with OLED screen and DIP switch control.

⚙️ Tecnologías / Technologies

ESP32 WiFi Server (AsyncWebServer + WebSocket)

HTML + JavaScript frontend embebido

OLED SH1106 + Adafruit GFX library

Sensores capacitivos, ultrasónico y DIP switch

Buzzer y LED RGB PWM controlado

 Arquitectura / Architecture
[Usuario Web]
      ↓ (WebSocket)
[ESP32 Servidor Local]
      ↓
[OLED + Sensores + LED RGB]


El servidor ESP32 aloja una interfaz web embebida, gestiona usuarios y registra resultados en tiempo real desde sensores físicos.
The ESP32 hosts an embedded web interface, handles users, and logs physical sensor data live.

 Módulos / Modules
Módulo	Descripción	Hardware
 Colores	Secuencia de colores interactiva con sensores capacitivos	LED RGB, sensores táctiles, buzzer
 Números	Conteo de manzanas con retroalimentación visual	OLED SH1106, DIP switch, botón, sensor ultrasónico
 Vista del sistema / System Preview


 Autor / Author

Juan Pablo Conrado Molina
🔗 GitHub: JuanWimmin

💡 Proyecto personal de integración IoT y educación interactiva.

📄 Licencia / License

Distribuido bajo la licencia MIT.
Free to use and modify for educational and research purposes.
