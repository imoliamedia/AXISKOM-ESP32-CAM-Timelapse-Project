# ESP32-CAM Plantengroei Timelapse Project

Dit project gebruikt een ESP32-CAM om automatisch timelapse foto's te maken van plantengroei en slaat deze op op een SD-kaart. Een geïntegreerde webinterface maakt het bekijken en downloaden van de foto's eenvoudig, zodat je de groei van je planten van dag tot dag kunt volgen.

## Functionaliteiten
- Automatisch foto's maken op instelbare intervallen
- Foto's alleen maken tijdens bepaalde uren (energiebesparend)
- Foto's opslaan op SD-kaart in dagmappen
- Webinterface voor het bekijken en downloaden van foto's
- Live stream functie (30 seconden)
- Handmatig foto's maken via de webinterface
- Opties om de SD-kaart te wissen wanneer nodig

## Hardware benodigdheden
- ESP32-CAM module
- SD-kaart (8GB of meer aanbevolen)
- USB naar TTL converter voor het programmeren
- 5V voeding voor de ESP32-CAM
- Optioneel: behuizing voor buiten gebruik

## Aanpassingen in de code

### WiFi-instellingen wijzigen
Verander de volgende regels om verbinding te maken met je eigen WiFi-netwerk:
```cpp
// WiFi-instellingen
const char* ssid = "JouwWiFiNaam";
const char* password = "JouwWiFiWachtwoord";
```

### Fotomomenten aanpassen
Je kunt het interval tussen foto's en de uren van de dag wanneer foto's worden gemaakt aanpassen:
```cpp
// Configuratie-instellingen
int photoInterval = 5;  // Tijd tussen foto's in minuten (standaard 5)
int dayStartHour = 8;   // Start tijdstip voor foto's (8:00)
int dayEndHour = 20;    // Eind tijdstip voor foto's (20:00)
```

### Beeldkwaliteit aanpassen
Verander de jpegQuality-waarde voor betere/slechtere afbeeldingskwaliteit (lagere waarde = hogere kwaliteit):
```cpp
int jpegQuality = 10;   // JPEG kwaliteit (0-63, lagere waarde = hogere kwaliteit)
```

## Uploaden en gebruiken

1. **Uploaden van de code:**
   - Sluit de ESP32-CAM aan op een USB-TTL converter
   - Zet de GPIO0 pin naar GND tijdens het opstarten (programmeermodus)
   - Upload de code via de Arduino IDE
   - Verwijder de verbinding tussen GPIO0 en GND en reset de camera

2. **SD-kaart voorbereiden:**
   - Formatteer een SD-kaart als FAT32
   - Plaats de SD-kaart in de ESP32-CAM voordat je deze inschakelt

3. **Camera gebruiken:**
   - Schakel de ESP32-CAM in
   - Verbind met hetzelfde WiFi-netwerk als ingesteld in de code
   - Open een browser en ga naar het IP-adres dat wordt getoond in de seriële monitor
   - De camera maakt automatisch foto's volgens het ingestelde interval

4. **Webinterface gebruiken:**
   - Bekijk foto's georganiseerd per dag
   - Download individuele foto's
   - Gebruik de livestream functie om de camera in realtime te bekijken
   - Maak handmatig foto's wanneer gewenst
   - Wis de SD-kaart wanneer deze vol raakt

## Probleemoplossing

- **SD-kaart niet gedetecteerd:** Controleer of de SD-kaart correct is geformatteerd en goed is geplaatst
- **WiFi verbinding mislukt:** Controleer de SSID en wachtwoord instellingen
- **Foto's worden niet opgeslagen:** Controleer of de SD-kaart niet vol of beschermd tegen schrijven is
- **Camera neemt geen foto's:** Controleer of de huidige tijd binnen de ingestelde dag uren valt

## ESP32-CAM pinout voor programmeren

| ESP32-CAM | FTDI Adapter |
|-----------|--------------|
| GND       | GND          |
| 5V/VCC    | VCC (5V)     |
| U0R (GPIO3)| TX          |
| U0T (GPIO1)| RX          |
| GPIO0*    | GND (tijdens programmeren) |

*Verbind GPIO0 met GND alleen tijdens het opstarten voor programmeren, verwijder deze verbinding daarna.

## Credits

Dit project is ontwikkeld voor AXISKOM en wordt gedeeld door hen als een open-source tool voor plantengroei monitoring en zelfvoorzieningssystemen. AXISKOM is een Nederlandstalig kennisplatform gericht op zelfredzaamheid, prepping, outdoor skills en zelfvoorzienend leven.

Bezoek [axiskom.nl](https://axiskom.nl) voor meer projecten en informatie over zelfvoorzienend leven.

---

Ontwikkeld door Axiskom Team | Laatste update: Februari 2025
