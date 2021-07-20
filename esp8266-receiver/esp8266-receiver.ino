/*
 * Author: Juan Gomez
 * Date: July 2021
 * 
 * This code works on the NodeMCU ESP8266. 
 * The code receives data image from an ESP32-CAM, pixel by pixel
 * and reconstruct the image in this side. 
 * 
 * The transmitter: 1) sends "start communicaation"
 *                  2) sends buffer length, aka quantity of pixels
 *                  3) send pixel data one by one
 *                  
 */
#include <SPI.h>
#include <RH_NRF24.h>

//fucntion declaration
void setup_nrf24();

RH_NRF24 nrf24(2, 4);

//to compare and know when the communication starts
uint8_t start_com[] = "start communication";

//to store the buffer_length sended by the transmitter
int buffer_length = 0;

//to know which step of the communication are we in
uint16_t counter = 0;

//pointer to the image buffer
uint8_t * image; 

void setup() 
{
  Serial.begin(115200);
  setup_nrf24();
}

void loop()
{ 
  if (nrf24.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (nrf24.recv(buf, &len))
    {
      //following the communication steps, this are the pixels one by one
      if(counter > 1){

        image[0 + (counter - 2)] = atoi((char*)buf);
        counter++;
      }

      //if counter == to this, it means we reached the final pixel
      if(counter == 7){

        for(int i = 0; i < 100; i++){
          Serial.println(image[i]);
        }
        
        //free the allocated memory for the image buffer
        free(image);
        counter = 0;
        Serial.println("finish transmission");
      }
      
      //following the communication steps, counter == 1 is the buffer length
      if(counter == 1){
      
        Serial.print("got request: ");
        Serial.println((char*)buf);
        buffer_length = atoi((char*)buf);
        //allocate a very big memory for the image buffer
        image = (uint8_t*)malloc(buffer_length);

        counter++;
      }
    
      //if buf == "start communication"
      //then the communication is starting
      if(memcmp(buf, start_com, 20) == 0){
        
        Serial.println("Communication starting");
        counter++;

        Serial.print("got request: ");
        Serial.println((char*)buf);
      }

      /*
      Serial.print("got request: ");
      Serial.println((char*)buf);
      */
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}

/*
 * initialize the NRF24L01 module
 */
void setup_nrf24(){

  if (!nrf24.init()){
    
    Serial.println("nrf24 init failed");
  }
  else{

    Serial.println("nrf24 init succeed");
  }
    
  if (!nrf24.setChannel(1)){

    Serial.println("nrf24 setChannel failed");
  }
  else{
    
    Serial.println("nrf24 setChannel succeed");  
  }
    
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)){

    Serial.println("nrf24 setRF failed");  
  }
  else{

    Serial.println("nrf24 setRF succeed");
  }

  Serial.println("nrf24 ready to go");
}
