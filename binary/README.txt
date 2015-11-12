Copy

  RDMaddonRW.exe
  RDMinjectDLL.dll
  StartRDMinject.exe

to Windows directory.

Run StartRDMinject.exe. Look at top of screen. If "InjectDLL is loaded" is there, just close the helper app with a tap on [OK]. If the text is "InjectDLL not loaded", tap on [Help] and then "Activate InjectDLL"]. Any change to the setting requires a reboot.

StartRDMinject.exe will just write or remove the registry key value of "HKLM\System\Kernel\injectDLL" to "\windows\RDMinjectDLL.dll". There are other options to set this registry key.

After a restart injectDLL watches all process. If wpctsc.exe (Remote Desktop Mobile) is started, the process RDMaddonRW.exe is started.

RDMaddonRW.exe adds a small bar on top of the RDM window at the bottom. 

Keypad symbol

The bar shows a keypad to show/hide the Software Input Panel at the most right. 

Battery percentage

The battery symbol changes between charging and critical display (battery percentage < 12%). The battery symbol will change to red for critical value.

The WLAN RSSI symbol

The WiFi RSSI symbol shows the WiFi RSSI (-30 to -100) as a percentage (4 steps). The 'antenna' color will change to red for bad values.

The Home symbol

Clicking the home symbol closes RDM and shows the Intermec Launcher screen.
