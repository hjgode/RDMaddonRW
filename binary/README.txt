Copy

  RDMaddonRW.exe
  RDMinjectDLL.dll
  StartRDMinject.exe

to Windows directory.

Run StartRDMinject.exe. Look at top of screen. If "InjectDLL is loaded" is there, just close the helper app with a tap on [OK]. If the text is "InjectDLL not loaded", tap on [Help] and then "Activate InjectDLL"]. Any change to the setting requires a reboot.

StartRDMinject.exe will just write or remove the registry key value of "HKLM\System\Kernel\injectDLL" to "\windows\RDMinjectDLL.dll". There are other options to set this registry key.

After a restart injectDLL watches all process. If wpctsc.exe (Remote Desktop Mobile) is started, the process RDMaddonRW.exe is started.

RDMaddonRW.exe adds a small bar on top of the RDM window at the bottom.

A simple clock

The clock is shown at the most right of the bar (new in v0.2)

Keypad symbol

The bar shows a keypad to show/hide the Software Input Panel at the right. 

Battery percentage

The battery symbol changes between charging and critical display (battery percentage < 12%). The battery symbol will change to red for critical value.

The WLAN RSSI symbol

The WiFi RSSI symbol shows the WiFi RSSI (-30 to -100) as a percentage (4 steps). The 'antenna' color will change to red for bad values.

The Home symbol

Clicking the home symbol closes RDM and shows the Intermec Launcher screen.

# history

v0.1:
  first official release
v0.2:
  add a clock to the right of all symbols
  the clock is painted every second, the other symbols are re-drawn all 10 seconds
  there is a switch in stdafx.h defining the RSSI function to use: USE_RSSI_ITC
    if set, the Intermec getRSSI() Api is used, this loads a bunch of other DLLs on every call and therefor was banned
    if unset, standard MS API calls to NDIS SNMP are used to get the RSSI of the first network adapter
    