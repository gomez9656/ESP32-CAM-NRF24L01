
/*
 * Author: Juan Pablo Gomez
 * Date: July 2021
 * 
 * This code takes the pixel data in the esp32-cam and send the data using the NRF24L01 module
 */

#include <SPI.h>
#include <RH_NRF24.h>
#include <RHSoftwareSPI.h>
#include "esp_camera.h"
#include "camera_gpio.h"

//function declaration
void setup_nrf24();
void setup_camera();

//create a SPI with software
RHSoftwareSPI spi;


//NRF24L01 constructor. This library allows SPI as a software, 
//so we can choose any pin we want for MOSI, MISO and CSK.
RH_NRF24 nrf24(15, 2, spi);

//To store the pixel data one by one and then send it 
uint8_t pixel = 0;
char char_pixel[4];

//16bits because some images are 50-60kbytes long
uint16_t buffer_length = 0;
char char_buffer_length[5];

uint8_t start_com[] = "start communication";

void setup() 
{ 
  spi.setPins(12, 13, 14);//choose which pins to use with SPI as a software
  Serial.begin(115200);
  setup_nrf24();
  setup_camera();
}


void loop()
{ 
  //take an images
  camera_fb_t * image = esp_camera_fb_get();
  if(!image) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  /*
  //test
 
  for(int i = 0; i < 5; i++){
    Serial.println(image->buf[i]);
  }
   uint8_t * buff;
   buff = (uint8_t*)malloc(5);
   for(int i = 0; i < 5; i++){
    buff[i] = 155;
    Serial.println(buff[i]);
   }
  //puedes guardar números al pointer creado con malloc
  buff[1] = image->buf[1];
  Serial.println(buff[1]);

  //el buffer tiene 255, 253, ... NO "255", "253",... son números, no un array de caracteres
  uint8_t valor2 = image->buf[0] - 1;
  Serial.println(valor2);

  //convierte un uint8_t valor[] en int. el primero es un array de caracteres, como lo que recive radiohead
  uint8_t valor[6] = "123";
  uint8_t valor3 = atoi((char*)valor); 
  Serial.println(valor3);

  //convierte un array de caracteres y guardalo directamente en el buffer 
  buff[2] = atoi((char*)valor);
  Serial.println(buff[2]);
  free(buff);
  //test
  */
  buffer_length = image->len;
  itoa(buffer_length, char_buffer_length, 10);
  Serial.println(char_buffer_length);

  //ESP8266 only accepts mallocs with less than 52k,
  //so to be sure, send only images < 45000 bytes
  if(buffer_length < 45000){
    
    //let the receiver know the transfer is starting
    Serial.println("Start communication");
    nrf24.send(start_com, sizeof(start_com));
    nrf24.waitPacketSent();
    delay(100);
    
    //send the length of the buffer, so the receiver can be able to reconstruct it
    nrf24.send((uint8_t*)char_buffer_length, sizeof(char_buffer_length));
    nrf24.waitPacketSent();
    delay(100);
    
    //send the pixel data one by one
    for(int i = 0; i < 5; i++){
     pixel = image->buf[i];
     itoa(pixel, char_pixel, 10);
     nrf24.send((uint8_t*)char_pixel, sizeof(char_pixel));
     nrf24.waitPacketSent();
     delay(100);
    }

    for(int i = 0; i < 100; i++){
      Serial.println(image->buf[i]);
    }
    
    Serial.println("finished");
  }
 

  /*
  * the esp_camera_fb_get() creates a uint8_t buffer(aka unsigned 8 bit integer), 
  * but send() expects a uint8_t*(aka pointer to unsigned 8 bit integer. That's why
  * you need to change the data types before sending the data
  */

  
  esp_camera_fb_return(image);
  
  delay(10000);
}

void setup_nrf24(){

    
  if (!nrf24.init()){
    
    Serial.println("init nrf24 failed");
  }
  else{
    
    Serial.println("init nrf24 succeed");
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
}

void setup_camera(){

  //camera configuration variable
  camera_config_t config; 
  
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  else if(err == ESP_OK){
    
    Serial.println("Camera init succeed");
  }
}
