TScanner.Package - tscanner

class TScanner
	a class with a raw tcp socket and buffer
	tscanner.TScanner(string ip)
		- creates a new instance of a TScanner
	TScanner.setIP(string ip)
		- changes the current IP
	TScanner.send(short port)
		- sends with the current flags
	TScanner.rstflags()
		- resets the flags to 0 (zero)
	TScanner get/sets
		TScanner.fin - fin flag (0/1)
		TScanner.syn - syn flag (0/1)
		TScanner.rst - rst flag (0/1)
		TScanner.psh - psh flag (0/1)
		TScanner.ack - ack flag (0/1)
		obs: i did not feel the need for the urg flag

class TSniffer
	a class with a raw tcp sniffer
	the info is saved in a binary format on /tmp/info_<ip>
	tscanner.TSniffer(string ip)
		- initializes the sniffer to save data from received packets from ip
	TSniffer.start()
		- starts sniffing for incoming traffic
	TSniffer.stop()
		- stops sniffing
	TSniffer.get_info()
		- get the info of the next packet in format of a tuple
		( port, fin, syn, rst, psh, ack, urg )
	TSniffer get/sets
		TSniffer.ip - get the IP in dot format, sets the IP
			when ip is set this way, the sniffer automatically stops

tscanner methods
	dns(hostname) - converts a hostname to a dot notation IP address