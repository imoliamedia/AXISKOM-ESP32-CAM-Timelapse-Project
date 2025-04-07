#include "html_templates.h"

// HTML template voor CSS stijlen
// Deze stijlen worden gebruikt in alle pagina's
const char* CSS_STYLES = R"rawliteral(
body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }
h1, h2 { color: #333; }
.container { max-width: 900px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }
.btn { display: inline-block; padding: 10px 20px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 4px; margin: 5px; }
.btn-warning { background-color: #f44336; }
.btn-info { background-color: #2196F3; }
.btn-primary { background-color: #673AB7; }
.status { margin: 20px 0; padding: 15px; background-color: #e7f3fe; border-left: 6px solid #2196F3; }
.actions { margin: 20px 0; }
.day-list { margin: 20px 0; }
.day-item { padding: 12px; margin-bottom: 8px; background-color: #f9f9f9; border-radius: 4px; display: flex; justify-content: space-between; align-items: center; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }
.day-link { color: #2196F3; text-decoration: none; font-weight: bold; font-size: 1.1em; }
.day-link:hover { text-decoration: underline; }
.message { padding: 15px; margin-bottom: 15px; border-radius: 4px; }
.success { background-color: #d4edda; border-left: 6px solid #28a745; }
.error { background-color: #f8d7da; border-left: 6px solid #dc3545; }
.warning { background-color: #ffdddd; border-left: 6px solid #f44336; padding: 15px; margin-bottom: 15px; }
.tab-btn { background-color: #ddd; border: none; padding: 10px 20px; border-radius: 4px 4px 0 0; cursor: pointer; outline: none; margin-right: 5px; }
.tab-btn:hover { background-color: #ccc; }
.tab-btn.active { background-color: #4CAF50; color: white; }
.tab-content { border-top: 2px solid #4CAF50; padding-top: 20px; }
.photos { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 20px; }
.photo-item { width: calc(33.333% - 10px); margin-bottom: 20px; box-shadow: 0 0 5px rgba(0,0,0,0.2); border-radius: 4px; overflow: hidden; }
.photo-item img { width: 100%; height: auto; display: block; }
.photo-info { padding: 10px; background-color: white; }
.photo-actions { display: flex; justify-content: space-between; margin-top: 10px; }
@media (max-width: 768px) { .photo-item { width: calc(50% - 10px); } }
@media (max-width: 480px) { .photo-item { width: 100%; } }
)rawliteral";

// JavaScript voor tabbladen
const char* TABS_SCRIPT = R"rawliteral(
function openTab(tabName) {
  var tabs = document.getElementsByClassName('tab-content');
  for (var i = 0; i < tabs.length; i++) {
    tabs[i].style.display = 'none';
  }
  var buttons = document.getElementsByClassName('tab-btn');
  for (var i = 0; i < buttons.length; i++) {
    buttons[i].className = buttons[i].className.replace(' active', '');
  }
  document.getElementById(tabName + '-tab').style.display = 'block';
  event.currentTarget.className += ' active';
}
)rawliteral";

// HTML template voor de hoofdpagina
const char* MAIN_PAGE_HTML_START = R"rawliteral(
<!DOCTYPE html><html>
<head><meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="Content-Security-Policy" content="frame-ancestors 'self' *">
<title>HYDRO Timelapse</title>
<style>
)rawliteral";

const char* MAIN_PAGE_HTML_MIDDLE = R"rawliteral(
</style>
</head>
<body>
<div class="container">
<h1>Hydro Plantengroei Timelapse</h1>

<!-- Tabbladen navigatie -->
<div style="margin: 20px 0;">
  <button class="tab-btn active" onclick="openTab('photos')">Foto's</button>
  <button class="tab-btn" onclick="openTab('settings')">Instellingen</button>
</div>

<!-- Status sectie -->
<div class="status">
)rawliteral";

const char* MAIN_PAGE_HTML_ACTIONS = R"rawliteral(
</div>

<!-- Acties sectie -->
<div class="actions">
  <a href="/photo" class="btn btn-primary">Maak Nu Een Foto</a>
  <a href="/stream" target="_blank" class="btn btn-info">Open Live Stream (30 sec)</a>
  <a href="/confirmwipe" class="btn btn-warning">Wis SD-kaart</a>
</div>
)rawliteral";

const char* MAIN_PAGE_HTML_END = R"rawliteral(
</div> <!-- container einde -->
<script>
)rawliteral";

const char* MAIN_PAGE_HTML_FINAL = R"rawliteral(
</script>
</body>
</html>
)rawliteral";

// HTML template voor het instellingentabblad
const char* SETTINGS_TAB_HTML = R"rawliteral(
<div id="settings-tab" class="tab-content" style="display:none;">
  <h2>Timelapse Instellingen</h2>

  <form action="/savesettings" method="post">
    <div style="margin-bottom: 15px;">
      <label style="display: block; margin-bottom: 5px; font-weight: bold;">Foto interval (minuten):</label>
      <input type="number" name="photoInterval" value="{photoInterval}" min="1" max="60" style="padding: 8px; width: 100px;">
    </div>

    <div style="margin-bottom: 15px;">
      <label style="display: block; margin-bottom: 5px; font-weight: bold;">Dagelijkse opnameperiode:</label>
      <div style="display: flex; gap: 10px; align-items: center;">
        <span>Van</span>
        <input type="number" name="dayStartHour" value="{dayStartHour}" min="0" max="23" style="padding: 8px; width: 80px;">
        <span>tot</span>
        <input type="number" name="dayEndHour" value="{dayEndHour}" min="0" max="23" style="padding: 8px; width: 80px;">
        <span>uur</span>
      </div>
    </div>

    <div style="margin-bottom: 15px;">
      <label style="display: block; margin-bottom: 5px; font-weight: bold;">Beeldkwaliteit (10-63, lager=beter):</label>
      <input type="number" name="jpegQuality" value="{jpegQuality}" min="10" max="63" style="padding: 8px; width: 100px;">
      <p style="margin-top: 5px; font-size: 12px; color: #666;">Waarschuwing: Hogere kwaliteit (lagere waarde) gebruikt meer opslagruimte.</p>
    </div>

    <button type="submit" class="btn btn-primary">Instellingen opslaan</button>
  </form>
</div>
)rawliteral";

// HTML template voor iframe modus
const char* IFRAME_HTML = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="Content-Security-Policy" content="frame-ancestors 'self' *">
  <title>Timelapse Camera</title>
  <style>
  body { font-family: Arial, sans-serif; margin: 0; padding: 10px; background: white; }
  h2 { color: #3c6e47; margin-top: 0; font-size: 18px; }
  .status { background-color: #e7f3fe; border-left: 6px solid #2196F3; padding: 10px; margin-bottom: 15px; }
  .btn { display: inline-block; padding: 8px 15px; background-color: #d35400; color: white; text-decoration: none; border-radius: 4px; margin: 5px 0; font-size: 14px; }
  .btn:hover { background-color: #3c6e47; }
  </style>
</head>
<body>
  <h2>Timelapse Camera</h2>

  <!-- Status weergeven -->
  <div class="status">
    {status_content}
  </div>
  
  <!-- Knoppen -->
  <a href="/photo" target="_blank" class="btn">Maak Nu Foto</a>
  <a href="/" target="_blank" class="btn">Open Dashboard</a>
</body>
</html>
)rawliteral";