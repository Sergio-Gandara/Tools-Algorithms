#!/bin/bash 

"""
scanPorts was a python tool to scan local networks in the search 
of vulnerable ports and services using both nmap and bash utilities
I programmed to solve a pentesting machine, here I recreated the basic concept
"""

import subprocess as sp

# sub = sp.run(["", ""], capture_output=True)
# print(sub.stdout.decode())

class machine: # Every machine in a network will be catalogued with its ip
    def __init__(ip):
        # vuln_status=0 # ranging form 10 to 0 evaluates vulnerability based existing services
        os = "unknown" # Operative System
        services = {} # a Dictionary, each port (key) points to a value (service and version)

def scan(ip_range):
    scan = sp.run(["nmap", "-T5", "-sV", str(ip_range)], capture_output=True)
    return scan.decode()
    


def classify(output):
    print(output)

def main():
    print("Welcome to the scanPorts utility!!")
    ip_range=input("Please insert an IP range to work with: ")
    sp.run("clear")
    classify(scan(ip_range))

if __name__=='__main__':
    main()
