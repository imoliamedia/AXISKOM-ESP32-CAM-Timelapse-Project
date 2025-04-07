# ESP32-CAM Timelapse Project

## Overzicht
Dit project maakt een timelapse-systeem met een ESP32-CAM. Het neemt automatisch foto's op vooraf ingestelde intervallen, tijdens ingestelde uren van de dag, en slaat deze op op een SD-kaart. Het biedt ook een webinterface om de opnamen te bekijken, instellingen aan te passen en te integreren met een Hydroponisch Master Dashboard.

## Functies
- 📸 Automatische timelapse-fotografie tijdens ingestelde uren
- 🕒 Instelbare intervallen en opnametijdstippen via webinterface
- 💾 Opslagbeheer op SD-kaart met georganiseerde mapstructuur per dag
- 🌐 Volledige webinterface voor het bekijken en downloaden van foto's
- 📱 Iframe-ondersteuning voor integratie met Hydroponisch Master Dashboard
- 🔄 Live streaming functionaliteit (30 seconden)
- ⚙️ Persistent opslaan van instellingen in flash-geheugen

## Vereisten
- ESP32-CAM module
- Micro SD-kaart (geformatteerd als FAT32)
- Arduino IDE met ESP32 ondersteuning
- WiFi-netwerk

## Bestandsstructuur

Het project gebruikt een modulaire codestructuur:

| Bestand | Beschrijving |
|---------|--------------|
| ESP32_TimeLapse.ino | Hoofdbestand met setup() en loop() |
| config.h | Configuratie en globale variabelen definities |
| camera.h/cpp | Camera initialisatie en beheer |
| sd_card.h/cpp | SD-kaart operaties |
| wifi_manager.h/cpp | WiFi-verbinding configuratie |
| time_manager.h/cpp | NTP-tijdsynchronisatie |
| settings_manager.h/cpp | Instellingen opslaan/laden |
| web_server.h/cpp | Basis webserver en routering |
| web_handlers.h/cpp | Endpoint handlers voor verschillende URL-paden |
| web_views.h/cpp | HTML-content generatie functies |
| web_utils.h/cpp | Hulpfuncties voor webserver-gerelateerde taken |
| html_templates.h | HTML-templates voor de webinterface |

## Installatie

1. Clone of download deze repository
2. Open het ESP32_TimeLapse.ino bestand in Arduino IDE
3. Installeer de benodigde bibliotheken:
   - ESP32 Camera
   - WiFi
   - FS
   - SD_MMC
   - Time
4. Pas de WiFi-instellingen aan in `wifi_manager.h`
5. Verbind de ESP32-CAM met je computer
6. Selecteer het juiste board (ESP32 AI Thinker)
7. Upload de code

## Gebruik

### Timelapse Camera

Na het opstarten zal de ESP32-CAM:
1. Verbinding maken met WiFi
2. Tijd synchroniseren via NTP
3. Beginnen met het maken van timelapse foto's volgens de instellingen
4. Een webserver starten op het toegewezen IP-adres

Bezoek het IP-adres van de camera in je browser om toegang te krijgen tot de webinterface.

In de webinterface kun je:
- Foto's bekijken georganiseerd per dag
- Handmatig foto's maken
- Een livestream van 30 seconden bekijken
- De instellingen aanpassen (interval, opnametijdstippen, etc.)
- De SD-kaart wissen indien nodig

### Integratie met Hydroponisch Dashboard

Het project kan worden geïntegreerd in het Axiskom Hydroponisch Master Dashboard:
1. Installeer de ESP32Hydro.html en ESP32Hydro.js bestanden
2. Open het dashboard in een browser
3. Voeg je timelapse-camera toe als een systeem

Het dashboard zal:
- Automatisch detecteren dat het een camera-systeem is
- Een aangepaste interface weergeven voor de camera
- Directe links bieden naar alle camerafuncties

## Instellingen aanpassen

Je kunt de volgende instellingen aanpassen via de webinterface:
- **Foto interval**: Tijd tussen foto's (in minuten)
- **Dagelijkse opnameperiode**: Start- en eindtijd voor opnamen (in uren, 24-uurs formaat)
- **Beeldkwaliteit**: JPEG-kwaliteit (10-63, lagere waarden = hogere kwaliteit)

Deze instellingen worden automatisch opgeslagen in flash-geheugen en blijven behouden na herstarten.

## Modulaire Webserver

De webserver is modulair opgesplitst in verschillende componenten:
- **web_server**: Basis routering en request handling
- **web_handlers**: Functies voor het afhandelen van verschillende endpoints
- **web_views**: Functies voor het genereren van HTML-content
- **web_utils**: Hulpfuncties voor webserver-gerelateerde taken

Deze modulaire aanpak maakt de code beter onderhoudbaar en makkelijker uit te breiden.

## Probleemoplossing

### Geen SD-kaart gedetecteerd
- Controleer of de SD-kaart correct is geplaatst
- Formatteer de SD-kaart als FAT32
- Gebruik een kaart kleiner dan 32GB voor betere compatibiliteit

### Geen WiFi-verbinding
- Controleer de WiFi-instellingen in de code
- Zorg ervoor dat de ESP32-CAM binnen bereik is van je WiFi-netwerk

### Camera niet zichtbaar in dashboard
- Controleer of het IP-adres correct is ingevoerd
- Zorg ervoor dat beide apparaten op hetzelfde netwerk zitten

## Credits
Dit project is ontwikkeld voor en gedeeld door AXISKOM, een Nederlands kennisplatform voor zelfredzaamheid, prepping en zelfvoorzienend leven.

Meer info: [AXISKOM.nl](https://axiskom.nl)