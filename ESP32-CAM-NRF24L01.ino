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

//functions declaration
void setup_nrf24();
void setup_camera();

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//create a SPI with software
RHSoftwareSPI spi;

//nrf24 constructor
RH_NRF24 nrf24(15, 2, spi);

//camera config variable
camera_config_t config;

int pixel = 0;
uint8_t pixel_2;
char cstr[16];

void setup() 
{ 
  spi.setPins(12, 13, 14);//choose which pins to use with SPI as a software
  Serial.begin(115200);
  setup_nrf24();
  setup_camera();
}


void loop()
{ 
  //this part takes the picture
  camera_fb_t * fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
  int fb_length = fb->len; 
  
  Serial.println(fb_length);
  Serial.println(fb->buf[0]);
  Serial.println(sizeof(fb->buf[0]));

  for(int i = 0; i< 10; i++){
    Serial.println(fb->buf[i]);
  }

  pixel_2 = fb->buf[1];
  Serial.println(pixel_2);
  Serial.println(sizeof(pixel_2));

  pixel = pixel_2;
  itoa(pixel, cstr, 10);
  Serial.println(cstr);

  /*
   * the esp_camera_fb_get() creates a uint8_t buffer(aka unsigned 8 bit integer), 
   * but send() expects a uint8_t*(aka pointer to unsigned 8 bit integer. That's why
   * you need to change the data types before sending the data
   */
  nrf24.send((uint8_t*)cstr, sizeof(cstr));
  nrf24.waitPacketSent();
  
  esp_camera_fb_return(fb);
  
  //this part sends the message
  Serial.println("Sending to nrf24_server");
  // Send a message to nrf24_server
  uint8_t data[] = "Hello World!";
  nrf24.send(data, sizeof(data));
  
  nrf24.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (nrf24.waitAvailableTimeout(500))
  { 
    // Should be a reply message for us now   
    if (nrf24.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is nrf24_server running?");
  }
  delay(5000);
}

void setup_nrf24(){

    
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
}

void setup_camera(){

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
