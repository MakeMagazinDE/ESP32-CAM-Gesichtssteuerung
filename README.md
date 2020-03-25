![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH
*** 

# ESP32-CAM-Gesichtssteuerung
I/O per Gesicht steuern

Mit dem Sketch ESP32-Face können Sie das Bild der ESP32-CAM im Browser anzeigen, ihr Gesicht antrainieren und automatisch im Flash speichern.

Der Sketch Face-Relais nimmt kontinuierlich Bilder von Gesichtern auf und vergleicht sie mit den im Flash gespeicherten Bilder. Bei einem "Match" schaltet der ESP32 die On-Board-LED an und ein eventuell externes angeschlossenes Relais.
