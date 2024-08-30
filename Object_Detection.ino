#include "esp_camera.h"
#include <WiFi.h>
#include <esp_http_server.h>

// Define your network credentials
const char* ssid = "**********";
const char* password = "*********";

// Define the threshold for detecting a specific color
int colorThreshold[3] = {200, 50, 50}; // Adjust these values as per your target color (R, G, B)

// Initialize the camera and WiFi
void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.pixel_format = PIXFORMAT_RGB565; // Use RGB format for color detection
  
  // Adjust for PSRAM presence
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // Capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Process the captured frame
  int detected = detectColor(fb->buf, fb->len);
  
  // Release the frame buffer
  esp_camera_fb_return(fb);

  if (detected) {
    Serial.println("Object detected!");
  } else {
    Serial.println("No object detected.");
  }

  delay(1000); // Delay between frames
}

int detectColor(uint8_t *buffer, size_t length) {
  // Example simple color detection logic
  // This is a very basic check; adjust the method based on your requirements
  
  int redSum = 0, greenSum = 0, blueSum = 0;
  int numPixels = length / 2; // since it's RGB565, 2 bytes per pixel

  for (int i = 0; i < length; i += 2) {
    uint8_t high = buffer[i];
    uint8_t low = buffer[i+1];

    // Extract the RGB values from RGB565 format
    int red = (high & 0xF8);
    int green = ((high & 0x07) << 5) | ((low & 0xE0) >> 3);
    int blue = (low & 0x1F) << 3;

    // Add the values to the sum
    redSum += red;
    greenSum += green;
    blueSum += blue;
  }

  // Calculate the average color
  int avgRed = redSum / numPixels;
  int avgGreen = greenSum / numPixels;
  int avgBlue = blueSum / numPixels;

  // Compare the average color to the threshold
  if (avgRed > colorThreshold[0] && avgGreen < colorThreshold[1] && avgBlue < colorThreshold[2]) {
    return 1; // Object detected
  } else {
    return 0; // No object detected
  }
}
