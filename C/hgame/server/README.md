#hgameOS server side

### Server source files for the game

Not using threads, simple for running on a Wii

Feel free to download, understand it, run it anywhere (preferably **Linux**)

Made on *Ubuntu*

###File listing:

*hgame_server.c*	- main file, compile and run

*decs.h*		- main header file, change SERVER to your LAN IP (e.g. 192.168.1.2)

*create_npc.c*		- create a NPC server

*register.c*		- register a user, don't use your common password

*create_service.c*	- create a service, SSH/FTP, best for updating NPS's software

*logins.bin*		- store login information, passwords are kept in plain text

*folders with odd names*	- these are the 'virtual' servers, those names are the hex representation of the IP

*ip.list*		- **recommended**, not standard, store the current 'live' IPs
