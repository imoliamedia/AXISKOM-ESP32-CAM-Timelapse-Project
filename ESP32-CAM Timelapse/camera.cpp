#include "camera.h"

// Initialiseer de camera met de juiste instellingen
bool initCamera() {
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
  
  // Initiële instellingen voor de camera
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA; // 1600x1200 voor hoge kwaliteit 
    config.jpeg_quality = jpegQuality;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera initialiseren
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialisatie mislukt met foutcode 0x%x", err);
    return false;
  }
  
  // Camera instellingen aanpassen voor betere kwaliteit
  updateCameraSettings();
  
  return true;
}

// Update camera instellingen met actuele waardes
void updateCameraSettings() {
  sensor_t * s = esp_camera_sensor_get();
  s->set_quality(s, jpegQuality);
  s->set_brightness(s, 0);
  s->set_contrast(s, 0);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0);
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 0);
  s->set_gain_ctrl(s, 1);
  s->set_agc_gain(s, 0);
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_dcw(s, 1);
}

// Maak een foto en sla deze op de SD-kaart op
bool takeSavePhoto() {
  if (!sdCardAvailable) return false;
  
  // Huidige tijd ophalen voor de bestandsnaam
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  // Foto maken
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Foto maken mislukt");
    return false;
  }
  
  // Flash LED aan voor foto (optioneel)
  // digitalWrite(FLASH_LED_PIN, HIGH);
  // delay(100);
  // digitalWrite(FLASH_LED_PIN, LOW);
  
  // Bestandsnaam maken met timestamp: HH-MM-SS.jpg
  sprintf(filePath, "%s/%02d-%02d-%04d_%02d-%02d-%02d.jpg", 
          folderPath, 
          timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  
  Serial.printf("Foto opslaan als: %s\n", filePath);
  
  // Bestand openen en schrijven
  File file = SD_MMC.open(filePath, FILE_WRITE);
  if (!file) {
    Serial.println("Bestand openen mislukt");
    esp_camera_fb_return(fb);
    return false;
  }
  
  // Foto naar bestand schrijven
  if (file.write(fb->buf, fb->len)) {
    Serial.printf("Bestand opgeslagen: %s (%u bytes)\n", filePath, fb->len);
  } else {
    Serial.println("Schrijven naar bestand mislukt");
    file.close();
    esp_camera_fb_return(fb);
    return false;
  }
  
  // Bestand sluiten en buffer vrijgeven
  file.close();
  esp_camera_fb_return(fb);
  
  return true;
}