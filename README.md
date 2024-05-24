# ** Urządzenie do testowania szybek/folii na telefon firmy MyScreen. **

## **Jak skonfigurować urządzenie do pokazu?**

Do poprawnego działania programu ESP (płytka odpowiedzialna za sterowanie silnikiem oraz innymi urządzeniami) wymaga sieci WiFi, z wykorzystaniem Hotspota możemy takową stworzyć. 

Następnie należy zklonować całe repozytorium tego projektu. Otwieramy plik z rozszerzeniem .ino który zawiera kod źródłowy do sterowania tym urządzeniem. 

Następnie odszukujemy linijki odpowiedzialne za credentialsy (SSID, hasło do sieci Hotspot). Wprowadzamy tam nazwę sieci (SSID) oraz hasło do sieci. 

```
const char* ssid = "WPROWADŹ TUTAJ NAZWĘ SWOJEJ SIECI DO KTÓREJ CHCESZ PODŁĄCZYĆ ESP";
const char* password = "HASŁO DO SIECI";
```


Potencjalne problemy:

1. Podczas uploadowania pliku błąd 0x13 (Wrong boot mode detected) - Aby rozwiązać problem należy podczas uploadowania pliku wcisnąć przycisk na płytce "BOOT"

2. Podczas uploadowania pliku błąd: Failed to connect to ESP32: No data serial received. Aby rozwiązać ten problem należy sprawdzić czy płytka nie uległa uszkodzeniu tzn. czy nie ma luźnych przewodów itd.

3. Strona nie odpowiada. Aby rozwiązać ten problem należy upewnić się czy jesteśmy w tej samej sieci jak ESP jest.

4. Monitor szeregowy jest zasypany różnymi logami związanymi z restartowniem ESP - aby rozwiązać ten problem należy, odpiąć około 10 sekund zasilanie płytki np. z portu USB laptopa lub zasilacza. Jeżeli to nie pomoże oznacza to, że najprawdopobniej mamy problem w programie który zakłóća działania mikrokontrolera.

