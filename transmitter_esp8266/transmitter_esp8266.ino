#include <SPI.h>  
#include <RH_NRF24.h>

//large array
char dummy_text[] = "Virgin Galactic owner Richard Branson rocketed into space Sunday, an edge-of-the-seat sub-orbital test flight intended to demonstrate his company's air-launched spaceplane is ready for passengers who can afford the ultimate thrill ride. And it appeared to do just that, zooming to an altitude just above 50 miles and giving Branson and his five crewmates about three minutes of weightlessness and spectacular views of Earth before plunging back into the atmosphere for a spiraling descent to touchdown at Virgin's New Mexico launch site. I have dreamt of this moment since I was a kid but honestly, nothing could prepare you for the view of Earth from space,Branson, 70, said after landing, at a rare loss for words. It was just magical.... I'm just taking it all in, it's unreal.";
char chunk[28]; //28bytes is the maximum payload size for NRF24 Radiohead library

char chunk_message(char *message, int chunks, int counter, int last_chunk);

uint8_t message_to_send[28];
uint8_t first[12] = "first chunk";
uint8_t start_com[10] = "start com";

RH_NRF24 nrf24(2,4); //CSN and CE in D2 and D4 
 
void setup() 
{
  Serial.begin(9600);
  
  if (!nrf24.init()){
  
    Serial.println("init failed");
  }
  else{
   
    Serial.println("init succeed");
  }
    
  if (!nrf24.setChannel(1)){
    
    Serial.println("setChannel failed");
  }
  else{
    
    Serial.println("setChannel succeed");
  }
    
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm)){
    
    Serial.println("setRF failed");
  }
  else{
    Serial.println("setRF succeed");
  }
  
  Serial.println("NRF24L01 ready to go");
  
}
 
void loop()
{
  int text_size = strlen(dummy_text);

  int chunks = text_size / 28; //how many chunks will be the text divided into

  int last_chunk = text_size % chunks; //how many bytes are left in the last chunk

  //let the receiver know the transfer is starting
  Serial.println((char*)start_com);
  nrf24.send(start_com, sizeof(start_com));
  nrf24.waitPacketSent();
  delay(500);
  
  //send the chunks quantity
  char cstr[16];
  itoa(chunks, cstr, 10);
  nrf24.send((uint8_t*)cstr, sizeof(cstr));
  nrf24.waitPacketSent();
  delay(500);
  
  //let the receiver know when you will send the first chunk
  Serial.println((char*)first);
  nrf24.send(first, sizeof(first));
  nrf24.waitPacketSent();
  delay(500);
  
  
  Serial.println("Sending large array");
  int i = 0;
  //send large array
  for(i = 0; i <= chunks; i++){   
    
    Serial.println(i);
    
    chunk_message(dummy_text, chunks, i, last_chunk);
    
    //save chunk data (char) in uint8_t variable. for the radiohead library
    memcpy(message_to_send, chunk, strlen(chunk)+1);
    
    // Send chunk of message
    nrf24.send(message_to_send, sizeof(message_to_send));
    nrf24.waitPacketSent();

    delay(500);
  }   
}

char chunk_message(char *message, int chunks, int counter, int last_chunk){

  int i = 0;

  //this means we are ar the final chunk, so we use the last_chunk variable
  if(counter == chunks){
    
    for(i = counter * 28; i <= (counter * 28) + last_chunk; i++){
      
      chunk[i - (counter * 28)] = message[i];
    }
  }
  else{
    //iterate the large array from the beggining
    for(i = 0 + (counter * 28); i < 28 + (counter * 28); i++){
 
      chunk[i - (counter * 28)] = message[i];
    }
  }

  return 0;
}
