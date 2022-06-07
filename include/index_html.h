#ifndef indexHTML_h
#define indexHTML_h

// Web Page Code to be stored in Memory
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Thermostat Web server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    h3 {font-size: 2.0rem; color: #FF0000;}
  </style>
  
  </head><body>
  <h2>Defumadeira Controle Remoto</h2> 
  <h3>%TEMPERATURE% &deg;C</h3>
  <h2>Níveis de Temperatura</h2>
  <form action="/get">
    Temperatura Máxima <input type="number" step="0.1" name="threshold_max" value="%THRESHOLD_MAX%" required><br>
    Temperatura Mínima <input type="number" step="0.1" name="threshold_min" value="%THRESHOLD_MIN%" required><br>
    <br/>
    <input type="submit" value="Enviar">
  </form>
</body></html>)rawliteral";

#endif