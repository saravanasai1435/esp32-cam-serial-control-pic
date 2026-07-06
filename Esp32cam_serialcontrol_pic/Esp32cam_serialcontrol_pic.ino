#include "esp_camera.h"
#include <FS.h>
#include <SD_MMC.h>

#define PHOTO_BUTTON 15   // Button connected to GND
#define FLASH_LED 4       // Onboard flash LED

void takePhoto();

void setup() {
  Serial.begin(115200);
  pinMode(PHOTO_BUTTON, INPUT_PULLUP);
  pinMode(FLASH_LED, OUTPUT);
  digitalWrite(FLASH_LED, LOW);

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  if(esp_camera_init(&config) != ESP_OK){
    Serial.println("❌ Camera init failed!");
    return;
  }

  if(!SD_MMC.begin()){
    Serial.println("❌ SD Card Mount Failed!");
    return;
  }

  Serial.println("✅ Ready! Press button or send '1' in Serial Monitor to take a photo.");
  delay(500); // prevent false trigger on boot
}

void loop() {
  // ---- BUTTON CONTROL ----
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(PHOTO_BUTTON);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    digitalWrite(FLASH_LED, HIGH);
    delay(150);
    takePhoto();
    digitalWrite(FLASH_LED, LOW);
  }
  lastButtonState = currentButtonState;

  // ---- SERIAL CONTROL ----
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '1') {
      Serial.println("📩 Command: Take Photo");
      digitalWrite(FLASH_LED, HIGH);
      delay(150);
      takePhoto();
      digitalWrite(FLASH_LED, LOW);
    }
  }
}

void takePhoto() {
  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb){
    Serial.println("❌ Capture failed");
    return;
  }

  String path = "/photo_" + String(millis()) + ".jpg";
  File file = SD_MMC.open(path, FILE_WRITE);
  if(file){
    file.write(fb->buf, fb->len);
    Serial.printf("📸 Saved: %s (%d bytes)\n", path.c_str(), fb->len);
    file.close();
  } else {
    Serial.println("❌ Save failed!");
  }
  esp_camera_fb_return(fb);
}

