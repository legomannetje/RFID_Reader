# RFID_Reader

# How to install:
  Connect the arduino Mega (of with the help of the pinout guide an other arduino, change a couple of values or it won't work)
  to the RFID-module, download the program and upload it to your arduino. Run the program and open the serial monitor
  
     Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
  
# How to use it:
  If its uploaded to your connect something to the alarm and controller pin, in my case these go to an alarmmodule and a controller, one
  deactivates the alarm and the otherone activates the alarm. For this example you can use leds. 
  Present a card to the reader to see the values of that card, and write them down in the program, first value in the upper row, second in   the other row
  Reupload the code, and your card will be recoised by the program
  
# Serial monitor
  Using the serial monitor there is a posibilty to see the values, and change the behavioir of the outputs from the arduino, the functions 
  explane themselfs

# How it works:
  The working of the program is explaned within the program, if you have any questions about it leave me a command
  


