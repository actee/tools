
import tscanner

def main():
	ip = '192.168.1.1'	# wireless router
	ports = [20,21,22,23,25,53,80,443]

	scanner = tscanner.TScanner(ip)
	sniffer = tscanner.TSniffer(ip)

	# syn-ack scan
	scanner.syn = 1

	print("Initializing sniffer...")
	sniffer.start()

	print "Sending packets..." 
	for i in range(len(ports)):
		scanner.send(ports[i])

	sniffer.stop()

	control = True

	while control:
		try:
			info = sniffer.get_info()

			# print all flags
			print("port %d fin %d syn %d rst %d psh %d ack %d urg %d" % (info[0],info[1],info[2],info[3],info[4],info[5],info[6]))

			if info[2] == 1 and info[5] == 1:
				# syn - ack open
				print("port %d is open" % info[0])

			if info[3] == 1:
				# rst closed
				print("port %d is closed" % info[0])
		except IOError:
			# reached EOF
			control = False

	del scanner
	del sniffer

if __name__ == '__main__':
	print("Do stuff")
	main()