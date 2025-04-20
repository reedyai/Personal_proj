from scapy.all import sniff
import time


def smell(durr, out):
    start = time.time()
    while (time.time() - start) < durr or durr == 0:
        sniff(prn=lambda packet: packet_out(packet, out), store=0)

def packet_out(packet, out):
    print(packet.show())



if __name__=="__main__":
    running = True
    print("Welcome to packet sniffer!")
    while running:
        print("What do you want to do?\n 1-Start sniffing\n 2-Analyze something\n 3-quit")
        choice = input("Enter you choice: ")

        if choice not in ['1', '2', '3']:
            print("Invalid choice. Please enter 1, 2, or 3.")
            time.sleep(.5)
            continue

        if choice == '1':
            print("hi")
            duration = input("Enter how long would you like to smell for in seconds (0 will run forever): ")
            output = input("Do you want the data put into a file?[y/n]:")
            if not isinstance(duration, int):
                print("That's not a correct input.")
                continue
            smell(duration, output)
