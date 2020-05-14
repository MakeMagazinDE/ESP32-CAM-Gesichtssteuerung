#include "esp_camera.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"


#define relayPin 2 // (also Red LED -> Locked)
#define GreenLEDPin 12 // Green LED -> Open

unsigned long currentMillis = 0;
unsigned long openedMillis = 0;
long interval = 5000;           // open lock for ... milliseconds

#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7

mtmn_config_t init_config()
{
  mtmn_config_t mtmn_config = {0};
  mtmn_config.type = FAST;
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.707;
  mtmn_config.pyramid_times = 4;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;
  return mtmn_config;
}

static face_id_list id_list = {0};

camera_fb_t * fb = NULL;
mtmn_config_t mtmn_config = init_config();
dl_matrix3du_t *aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);



void setup() {
  Serial.begin(115200);

  digitalWrite(relayPin, LOW);
  pinMode(relayPin, OUTPUT);
  digitalWrite(GreenLEDPin, LOW);
  pinMode(GreenLEDPin, OUTPUT);

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
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  if (!aligned_face) {
    Serial.println("Could not allocate face recognition buffer");
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);

  // Set according to camera orientation (also check while face enrollment!)
  s->set_vflip(s, 1);

  face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  read_face_id_from_flash(&id_list);// Read current face data from on-board flash
}

void rzoCheckForFace() {
  currentMillis = millis();
  if (run_face_recognition()) { // face recognition function has returned true
    Serial.println("Face recognised");
    digitalWrite(relayPin, HIGH); //close (energise) relay also means RED LED TURN OFF! (Buffer on Relay card!)
    digitalWrite(GreenLEDPin, LOW); //LOW means GREEEN LED TURN ON! (Current flows from 3.3 to PIN! 100Ohm Resitor)

    openedMillis = millis(); //time relay closed
  }
  if (currentMillis - interval > openedMillis) { // current time - face recognised time > 5 secs
    digitalWrite(relayPin, LOW); //open relay
    digitalWrite(GreenLEDPin, HIGH); //HIGH  means GREEEN LED TURN OFF!
  }
}

bool run_face_recognition() {
  bool faceRecognised = false; // default
  int64_t start_time = esp_timer_get_time();
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return false;
  }

  int64_t fb_get_time = esp_timer_get_time();
  //Serial.printf("Get one frame in %u ms.\n", (fb_get_time - start_time) / 1000); // this line can be commented out

  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  uint32_t res = fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);
  if (!res) {
    Serial.println("to rgb888 failed");
    dl_matrix3du_free(image_matrix);
  }

  esp_camera_fb_return(fb);

  box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

  if (net_boxes) {
    Serial.printf ("OK");
    if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK) {

      int matched_id = recognize_face(&id_list, aligned_face);
      if (matched_id >= 0) {
        Serial.printf("Match Face ID: %u\n", matched_id);
        faceRecognised = true; // function will now return true
      } else {
        Serial.println("No Match Found");
        matched_id = -1;
      }
    } else {
      Serial.println("Face Not Aligned");
    }

    free(net_boxes->box);
    free(net_boxes->landmark);
    free(net_boxes);
  }
  //  esp_camera_fb_return(fb);
  dl_matrix3du_free(image_matrix);
  return faceRecognised;
}

void loop() {
  rzoCheckForFace();
}
