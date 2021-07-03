# Wurlitzer-Statesman-Jukebox
Reanimation of an 1970 Wurlitzer Statesman jukebox

Took the cable that goes to the junktion box
Solded some cables to the pins for the letters and the numbers
More cables for the original selection relay

Function mode
The Arduino Mega runs 5 tasks (RTOS)
First task (disables all other tasks)

Scans if a Letter has been pressed.
If yes it activates (via a 5 V Relay) the original Wurlitzer relay, hence the big CLACK (20 Volts)

Then the second taks checks if a number has been pressed.

Task 3 reads letter
Task 4 reads number
Task 5 sends CODE:letter-number to serial output and releases the Wurlitzer relay (CLACK again)

The Raspberry Pi reads and displays all serial communication. Only if the word CODE is found, it plays the corresponding LETTER NUMBER song.

I use a SQLight database for storing the the LETTER-NUMBER combination to the corresponding mp3 file.

Rapberry Pi is configured as AP

Samba share for copying the mp3 files and editing the database

Actually the jukebox has no speakers an the original amplifier is broken. This will be done somewhere in the future.
