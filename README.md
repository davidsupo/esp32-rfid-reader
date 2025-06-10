# Proyecto ESP32 con RFID RC522, LCD I2C, LED y Buzzer

Este proyecto utiliza un ESP32 para leer tarjetas RFID RC522 y registrar los datos en una hoja de cálculo de Google Sheets. Además, cuenta con una pantalla LCD con comunicación I2C, un LED y un buzzer para indicar el estado de la lectura.


## 🛠 Configuración de Pines
| Componente  | Pin ESP32  |
|-------------|------------|
| LCD SDA     | GPIO 4     |
| LCD SCL     | GPIO 15    |
| RC522 SDA   | GPIO 21    |
| RC522 SCK   | GPIO 18    |
| RC522 MOSI  | GPIO 23    |
| RC522 MISO  | GPIO 19    |
| RC522 RST   | GPIO 22    |
| LED         | GPIO 25    |
| Buzzer      | GPIO 26    |
| Button      | GPIO 27    |


## Configuración de Google Sheets y Apps Script

### 1. Crear una hoja de cálculo en Google Sheets
1. Abre [Google Sheets](https://docs.google.com/spreadsheets/).
2. Renombra la hoja de cálculo con el nombre deseado.
3. En la celda **A1**, escribe `Número de DNI`.
4. En la celda **B1**, escribe `Fecha y hora`.

### 2. Crear y configurar Apps Script
1. Abre el editor de Apps Script: **Extensiones > Apps Script**.
2. Borra el código existente y agrega el siguiente:

```javascript
function doGet(e) {
  var sheet = SpreadsheetApp.openById("TU_SHEET_ID_AQUI").getActiveSheet();
  var registros = e.parameter.registros;

  if (registros) {
    var listaRegistros = registros.split(";");
    listaRegistros.forEach(function(registro) {
      var datos = registro.split(":");
      if (datos.length === 7) {
        var idTarjeta = datos[0];
        var fecha = datos[1] + "/" + datos[2] + "/" + datos[3];
        var hora = datos[4] + ":" + datos[5] + ":" + datos[6];
        var fechaHora = new Date();
        sheet.appendRow([idTarjeta, fecha, hora, fechaHora]);
      }
    });
  }

  return ContentService.createTextOutput("Datos recibidos correctamente");
}
```

3. Reemplaza `TU_SHEET_ID_AQUI` con el ID de tu hoja de cálculo (está en la URL después de `/d/` y antes de `/edit`).

### 3. Desplegar como servicio web
1. Ve a **Implementar > Nueva implementación**.
2. En **Seleccionar tipo**, elige **Aplicación web**.
3. En **¿Quién tiene acceso?**, selecciona **Cualquiera**.
4. Haz clic en **Implementar** y autoriza los permisos.
5. Copia la URL generada que usaremos en el ESP32.

### 4. Cambiar la zona horaria en la configuración de Google Sheets
1. Abre Google Sheets.
2. Ve a **Archivo > Configuración**.
3. En **Zona horaria**, selecciona tu zona horaria correspondiente.


