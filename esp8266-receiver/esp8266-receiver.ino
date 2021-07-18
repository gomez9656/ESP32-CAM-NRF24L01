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
char char_buffer_length[5];
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
      //if buf == start communication
      //then the communication is starting
      if(memcmp(buf, start_com, 20) == 0){
        
        Serial.println("Communication starting");
        counter++;
      }
      //following the steps, counter == 1 would be buffer length
      if(counter == 1){

        memcpy(char_buffer_length, buf, 5);
        Serial.println(char_buffer_length);
        buffer_length = atoi(char_buffer_length);

        //allocate a very big memory for the image buffer
        image = (uint8_t*)malloc(buffer_length);
      }
      
      Serial.print("got request: ");
      Serial.println((char*)buf);
 
    }
    else
    {
      Serial.println("recv failed");
    }
  }

  //free the allocated memory for the image buffer
  free(image);
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
