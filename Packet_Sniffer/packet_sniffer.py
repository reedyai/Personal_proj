from scapy.all import sniff, IP, Ether, TCP, UDP, DNS, send
import scapy
import time
import sys
import requests

ip_info_token = "GET_YOUR_OWN_FREE_KEY_AT_IPINFO.IO"


def smell(durr, out):
    print("Sniffing...")
    try:
        start = time.time()
        if durr > 0:
            sniff(prn=lambda packet: packet_out(packet, out), store=0, timeout=durr)
        else:
            sniff(prn=lambda packet: packet_out(packet, out), store=0)
    except KeyboardInterrupt:
        print("\nSniffing interrupted by user.")

def packet_out(packet, out):
    
    outputs = [sys.stdout]
    if out == "y":
        try:
            f = open("Packets.txt", "a")
            outputs.append(f)
        except IOError:
            print("Failed to open file for writing.")
            return

    for stream in outputs:
        print_packet(packet, stream)

    if out == "y":
        f.close()

def print_packet(packet, out_stream):
    print("\n--- New Packet ---", file=out_stream)
    if packet.haslayer(Ether):
        print("Ethernet Layer:", file=out_stream)
        print(f"     -Source MAC Address: {packet[Ether].src}", file=out_stream)
        print(f"     -Destination MAC Address: {packet[Ether].dst}", file=out_stream)
    if packet.haslayer(IP):
        print("IP Layer:", file=out_stream)
        print(f"     -Source IP Address: {packet[IP].src}", file=out_stream)
        print(f"     -Destination IP Address: {packet[IP].dst}", file=out_stream)
    if packet.haslayer(TCP):
        print("TCP Layer:", file=out_stream)
        print(f"     -Source Port: {packet[TCP].sport}", file=out_stream)
        print(f"     -Destination Port: {packet[TCP].dport}", file=out_stream)
        print(f"     -Payload: {bytes(packet[TCP].payload)}", file=out_stream)
        print(f"     -Sequence Number: {packet[TCP].seq}", file=out_stream)
        print(f"     -Ack Number: {packet[TCP].ack}", file=out_stream)
    if packet.haslayer(UDP):
        print("UDP Layer:", file=out_stream)
        print(f"     -Source Port: {packet[UDP].sport}", file=out_stream)
        print(f"     -Destination Port: {packet[UDP].dport}", file=out_stream)
        print(f"     -Payload: {bytes(packet[UDP].payload)}", file=out_stream)
    if packet.haslayer(DNS):
        print("DNS Layer:", file=out_stream)
        print(f"     -Query details: {packet[DNS].qd}", file=out_stream)

def tcp_rst():
    src_ip = int(input("Enter the source IP: "))
    dst_ip = int(input("Enter the destination IP: "))
    src_port = int(input("Enter the source port: "))
    dst_port = int(input("Enter the destination port: "))
    seq_num = int(input("Enter the sequence number: "))
    ack_num = int(input("Enter the acknowlegement number: "))


    ip = IP(src=src_ip, dst=dst_ip)
    tcp = TCP(sport=src_port, dport=dst_port, flags="R", seq=seq_num, ack=ack_num)
    pkt = ip / tcp
    send(pkt)
    print(f"Sent TCP RST from {src_ip}:{src_port} to {dst_ip}:{dst_port}")

def ip_info():
    ip = input("Enter the IP you want info from: ")
    try:
        response = requests.get(f"https://ipinfo.io/{ip}/json?token={ip_info_token}")
        if response.status_code == 200:
            data = response.json()
            print("IP Information:")
            print(f"IP: {data.get('ip')}")
            print(f"Hostname: {data.get('hostname')}")
            print(f"City: {data.get('city')}")
            print(f"Region: {data.get('region')}")
            print(f"Country: {data.get('country')}")
            print(f"Location: {data.get('loc')}")
            print(f"Organization: {data.get('org')}")
    except requests.RequestException:
        print("Error during IP info lookup. Check your internet connection or token.")




if __name__=="__main__":
    running = True
    print("Welcome to packet sniffer!")
    while running:
        print("What do you want to do?\n 1-Pick a tool\n 2-Analyze something\n 3-quit")
        choice = input("Enter you choice: ")

        if choice not in ['1', '2', '3']:
            print("Invalid choice. Please enter 1, 2, or 3.")
            time.sleep(.5)
            continue

        if choice == '1':
            tool = input("Input the number of the tool you want to use\n 1-Packet Sniffer\n 2-TCP reset\nEnter you choice: ")
            if tool == '1':
                duration = int(input("Enter how long would you like to smell for in seconds (0 will run forever): "))
                output = input("Do you want the data put into a file?[y/n]:")
                if not isinstance(duration, int):
                    print("That's not a correct input.")
                    continue
                smell(duration, output)
            elif tool == '2':
                tcp_rst()
        
        if choice == '2':
            thing = input("Input the number of the thing you want to analyze\n 1-IP\nEnter you choice: ")
            if thing == '1':
                ip_info()
        if choice == '3':
            running = False
