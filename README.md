![GitHub Logo](http://www.heise.de/make/icons/make_logo.png)

Maker Media GmbH
*** 

# ESP32-CAM-Gesichtssteuerung
I/O per Gesicht steuern

Mit dem Sketch _ESP32-Face_ können Sie das Bild der ESP32-CAM im Browser anzeigen, Ihr Gesicht antrainieren und automatisch im Flash speichern.

Der Sketch _Face-Relais_ nimmt kontinuierlich Bilder von Gesichtern auf und vergleicht sie mit den im Flash gespeicherten Bilder. Bei einem "Match" schaltet der ESP32 die On-Board-LED an und ein eventuell externes angeschlossenes Relais.

Der Sketch _Cthulhu_Relay_ ist eine angepasste Version von Face-Relais, der in einer **[ausgehöhlten Cthulhu-Statue aus dem 3D-Drucker](https://heise.de/-4711670)** dafür sorgt, dass erkannte und unbekannte Gesichter mit unterschiedlichen LED-Augenfarben angefunkelt werden. 
