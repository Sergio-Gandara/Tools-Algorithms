

#!/bin/python3
from unicodedata import name
from selenium import webdriver
from selenium.webdriver.common.by import By # to find elements
from selenium.webdriver.chrome.service import Service
import re

import os



def drivin(a):

    exe_path="/usr/local/sbin/chromedriver"
    URL="[CENSORED]"

    i = int(a)
    URL = URL + str(i)
    
    print("hi")
    agencies=[] # this is where I will put the agencies
    driver_service = Service(exe_path)
    # driver =webdriver.Chrome(exe_path) # I have the chromedriver in  /usr/local/sbin THIS OPTION IS DEPRECATED
    driver =webdriver.Chrome(service=driver_service)
    driver.get(URL)
    



    data=driver.page_source
    brute = re.findall("data-agency-name=........................", data)
    
    refined = [re.split("\"", brute[item])[1] for item in range(len(brute))]
    
    print(refined)
    
    return refined


def main():
    
    total_agencies=[]
    extracted_agencies=[]
    for i in range(7):
        extracted_agencies=drivin(i)
        total_agencies=total_agencies+extracted_agencies
    print("----------------------------------\n")
    print(total_agencies)
    

 

if __name__=='__main__':
    main()


