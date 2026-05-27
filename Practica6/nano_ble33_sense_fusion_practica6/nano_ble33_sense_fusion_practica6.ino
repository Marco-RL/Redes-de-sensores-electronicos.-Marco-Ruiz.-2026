/* Edge Impulse ingestion SDK
 * Modificado para iluminar LED RGB según posición del microcontrolador
 * Clases: idle, pitch_neg_1, pitch_pos_1, roll_neg_1, roll_pos_1, yaw_neg_1, yaw_pos_1
 */

/* Includes ---------------------------------------------------------------- */
#include <Marco827133-project-1_inferencing.h>
#include <Arduino_LSM9DS1.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_HTS221.h>
#include <Arduino_APDS9960.h>

enum sensor_status {
    NOT_USED = -1,
    NOT_INIT,
    INIT,
    SAMPLED
};

typedef struct{
    const char *name;
    float *value;
    uint8_t (*poll_sensor)(void);
    bool (*init_sensor)(void);    
    sensor_status status;
} eiSensors;

/* Constant defines -------------------------------------------------------- */
#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  2.0f
#define N_SENSORS           18

/* ---- Umbrales por clase ------------------------------------------------- */
#define THRESH_IDLE       0.80f
#define THRESH_PITCH_NEG  0.75f
#define THRESH_PITCH_POS  0.75f
#define THRESH_ROLL_NEG   0.75f
#define THRESH_ROLL_POS   0.75f
#define THRESH_YAW_NEG    0.65f   // más bajo por confusión con yaw_pos
#define THRESH_YAW_POS    0.65f   // más bajo por confusión con yaw_neg

/* ---- Helpers LED -------------------------------------------------------- */
// LED RGB del Nano 33 BLE: lógica INVERTIDA (LOW = encendido, HIGH = apagado)
void setLED(bool r, bool g, bool b) {
    digitalWrite(LEDR, r ? LOW : HIGH);
    digitalWrite(LEDG, g ? LOW : HIGH);
    digitalWrite(LEDB, b ? LOW : HIGH);
}

void applyColor(const char* label, float score) {
    // Comprueba umbral específico por clase antes de encender
    if      (strcmp(label, "idle") == 0        && score >= THRESH_IDLE)      { setLED(false, false, false); } // Apagado
    else if (strcmp(label, "roll_pos_1") == 0  && score >= THRESH_ROLL_POS)  { setLED(true,  false, false); } // Rojo
    else if (strcmp(label, "roll_neg_1") == 0  && score >= THRESH_ROLL_NEG)  { setLED(true,  false, true);  } // Rosa  (R+B atenuado)
    else if (strcmp(label, "pitch_pos_1") == 0 && score >= THRESH_PITCH_POS) { setLED(false, true,  false); } // Verde
    else if (strcmp(label, "pitch_neg_1") == 0 && score >= THRESH_PITCH_NEG) { setLED(true,  true,  false); } // Amarillo (R+G)
    else if (strcmp(label, "yaw_pos_1") == 0   && score >= THRESH_YAW_POS)   { setLED(false, false, true);  } // Azul
    else if (strcmp(label, "yaw_neg_1") == 0   && score >= THRESH_YAW_NEG)   { setLED(true,  false, true);  } // Morado (R+B)
    // Si ninguna clase supera su umbral → LED apagado (posición ambigua)
}

/* Forward declarations ------------------------------------------------------- */
float ei_get_sign(float number);
bool init_IMU(void);
bool init_HTS(void);
bool init_BARO(void);
bool init_APDS(void);
uint8_t poll_acc(void);
uint8_t poll_gyr(void);
uint8_t poll_mag(void);
uint8_t poll_HTS(void);
uint8_t poll_BARO(void);
uint8_t poll_APDS_color(void);
uint8_t poll_APDS_proximity(void);
uint8_t poll_APDS_gesture(void);

/* Private variables ------------------------------------------------------- */
static const bool debug_nn = false;
static float data[N_SENSORS];
static bool ei_connect_fusion_list(const char *input_list);
static int8_t fusion_sensors[N_SENSORS];
static int fusion_ix = 0;

eiSensors sensors[] =
{
    "accX", &data[0], &poll_acc, &init_IMU, NOT_USED,
    "accY", &data[1], &poll_acc, &init_IMU, NOT_USED,
    "accZ", &data[2], &poll_acc, &init_IMU, NOT_USED,
    "gyrX", &data[3], &poll_gyr, &init_IMU, NOT_USED,
    "gyrY", &data[4], &poll_gyr, &init_IMU, NOT_USED,
    "gyrZ", &data[5], &poll_gyr, &init_IMU, NOT_USED,
    "magX", &data[6], &poll_mag, &init_IMU, NOT_USED,
    "magY", &data[7], &poll_mag, &init_IMU, NOT_USED,
    "magZ", &data[8], &poll_mag, &init_IMU, NOT_USED,
    "temperature", &data[9],  &poll_HTS,          &init_HTS,  NOT_USED,
    "humidity",    &data[10], &poll_HTS,          &init_HTS,  NOT_USED,
    "pressure",    &data[11], &poll_BARO,         &init_BARO, NOT_USED,
    "red",         &data[12], &poll_APDS_color,   &init_APDS, NOT_USED,
    "green",       &data[13], &poll_APDS_color,   &init_APDS, NOT_USED,
    "blue",        &data[14], &poll_APDS_color,   &init_APDS, NOT_USED,
    "brightness",  &data[15], &poll_APDS_color,   &init_APDS, NOT_USED,
    "proximity",   &data[16], &poll_APDS_proximity,&init_APDS,NOT_USED,
    "gesture",     &data[17], &poll_APDS_gesture, &init_APDS, NOT_USED,
};

/* -------------------------------------------------------------------------
 * SETUP
 * ---------------------------------------------------------------------- */
void setup()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Edge Impulse - Clasificacion de posicion con LED RGB\r\n");

    /* Inicializar pines LED RGB */
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    setLED(false, false, false); // Apagado al inicio

    /* Conectar sensores usados por el modelo */
    if(ei_connect_fusion_list(EI_CLASSIFIER_FUSION_AXES_STRING) == false) {
        ei_printf("ERR: Errors in sensor list detected\r\n");
        return;
    }

    for(int i = 0; i < fusion_ix; i++) {
        if (sensors[fusion_sensors[i]].status == NOT_INIT) {
            sensors[fusion_sensors[i]].status = (sensor_status)sensors[fusion_sensors[i]].init_sensor();
            if (!sensors[fusion_sensors[i]].status) {
              ei_printf("%s sensor initialization failed.\r\n", sensors[fusion_sensors[i]].name);             
            } else {
              ei_printf("%s sensor initialization successful.\r\n", sensors[fusion_sensors[i]].name);
            }
        }
    }

    /* Tabla de colores para referencia en consola */
    Serial.println("---------------------------------------");
    Serial.println("Mapa de colores:");
    Serial.println("  idle       -> LED apagado");
    Serial.println("  roll_pos   -> Rojo");
    Serial.println("  roll_neg   -> Rosa");
    Serial.println("  pitch_pos  -> Verde");
    Serial.println("  pitch_neg  -> Amarillo");
    Serial.println("  yaw_pos    -> Azul");
    Serial.println("  yaw_neg    -> Morado");
    Serial.println("---------------------------------------\r\n");
}

/* -------------------------------------------------------------------------
 * LOOP
 * ---------------------------------------------------------------------- */
void loop()
{
    ei_printf("\nStarting inferencing in 2 seconds...\r\n");
    delay(2000);

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != fusion_ix) {
        ei_printf("ERR: Sensors don't match the model requirements\r\n"
                  "Required sensors: %s\r\n", EI_CLASSIFIER_FUSION_AXES_STRING);
        return;
    }

    ei_printf("Sampling...\r\n");

    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
        int64_t next_tick = (int64_t)micros() + ((int64_t)EI_CLASSIFIER_INTERVAL_MS * 1000);

        for(int i = 0; i < fusion_ix; i++) {
            if (sensors[fusion_sensors[i]].status == INIT) {
                sensors[fusion_sensors[i]].poll_sensor();
                sensors[fusion_sensors[i]].status = SAMPLED;
            }
            if (sensors[fusion_sensors[i]].status == SAMPLED) {
                buffer[ix + i] = *sensors[fusion_sensors[i]].value;
                sensors[fusion_sensors[i]].status = INIT;
            }
        }

        int64_t wait_time = next_tick - (int64_t)micros();
        if(wait_time > 0) delayMicroseconds(wait_time);
    }

    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        ei_printf("ERR:(%d)\r\n", err);
        return;
    }

    ei_impulse_result_t result = { 0 };
    err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR:(%d)\r\n", err);
        return;
    }

    /* ---- Imprimir predicciones ----------------------------------------- */
    ei_printf("Predictions (DSP: %d ms, Classification: %d ms):\r\n",
        result.timing.dsp, result.timing.classification);

    float max_score = 0.0f;
    const char* max_label = "idle";

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("  %s: %.5f\r\n",
            result.classification[ix].label,
            result.classification[ix].value);

        if (result.classification[ix].value > max_score) {
            max_score = result.classification[ix].value;
            max_label = result.classification[ix].label;
        }
    }

    /* ---- Aplicar color al LED ------------------------------------------ */
    ei_printf(">> Clase detectada: %s (%.3f)\r\n", max_label, max_score);
    applyColor(max_label, max_score);

    if (max_score < 0.65f) {
        ei_printf(">> Confianza baja - LED apagado\r\n");
        setLED(false, false, false);
    }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("   anomaly score: %.3f\r\n", result.anomaly);
#endif
}

#if !defined(EI_CLASSIFIER_SENSOR) || (EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_FUSION && EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_ACCELEROMETER)
#error "Invalid model for current sensor"
#endif

/* ---- Funciones auxiliares ----------------------------------------------- */

static int8_t ei_find_axis(char *axis_name)
{
    for(int ix = 0; ix < N_SENSORS; ix++) {
        if(strstr(axis_name, sensors[ix].name)) return ix;
    }
    return -1;
}

static bool ei_connect_fusion_list(const char *input_list)
{
    char *buff;
    bool is_fusion = false;
    char *input_string = (char *)ei_malloc(strlen(input_list) + 1);
    if (input_string == NULL) return false;
    memset(input_string, 0, strlen(input_list) + 1);
    strncpy(input_string, input_list, strlen(input_list));
    memset(fusion_sensors, 0, N_SENSORS);
    fusion_ix = 0;
    buff = strtok(input_string, "+");
    while (buff != NULL) {
        int8_t found_axis = 0;
        is_fusion = false;
        found_axis = ei_find_axis(buff);
        if(found_axis >= 0) {
            if(fusion_ix < N_SENSORS) {
                fusion_sensors[fusion_ix++] = found_axis;
                sensors[found_axis].status = NOT_INIT;
            }
            is_fusion = true;
        }
        buff = strtok(NULL, "+ ");
    }
    ei_free(input_string);
    return is_fusion;
}

float ei_get_sign(float number) {
    return (number >= 0.0) ? 1.0 : -1.0;
}

bool init_IMU(void) {
  static bool init_status = false;
  if (!init_status) init_status = IMU.begin();
  return init_status;
}

bool init_HTS(void) {
  static bool init_status = false;
  if (!init_status) init_status = HTS.begin();
  return init_status;
}

bool init_BARO(void) {
  static bool init_status = false;
  if (!init_status) init_status = BARO.begin();
  return init_status;
}

bool init_APDS(void) {
  static bool init_status = false;
  if (!init_status) init_status = APDS.begin();
  return init_status;
}

uint8_t poll_acc(void) {
    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(data[0], data[1], data[2]);
        for (int i = 0; i < 3; i++) {
            if (fabs(data[i]) > MAX_ACCEPTED_RANGE)
                data[i] = ei_get_sign(data[i]) * MAX_ACCEPTED_RANGE;
        }
        data[0] *= CONVERT_G_TO_MS2;
        data[1] *= CONVERT_G_TO_MS2;
        data[2] *= CONVERT_G_TO_MS2;
    }
    return 0;
}

uint8_t poll_gyr(void) {
    if (IMU.gyroscopeAvailable())
        IMU.readGyroscope(data[3], data[4], data[5]);
    return 0;
}

uint8_t poll_mag(void) {
    if (IMU.magneticFieldAvailable())
        IMU.readMagneticField(data[6], data[7], data[8]);
    return 0;
}

uint8_t poll_HTS(void) {
    data[9]  = HTS.readTemperature();
    data[10] = HTS.readHumidity();
    return 0;
}

uint8_t poll_BARO(void) {
    data[11] = BARO.readPressure();
    return 0;
}

uint8_t poll_APDS_color(void) {
    int temp_data[4];
    if (APDS.colorAvailable()) {
        APDS.readColor(temp_data[0], temp_data[1], temp_data[2], temp_data[3]);
        data[12] = temp_data[0];
        data[13] = temp_data[1];
        data[14] = temp_data[2];
        data[15] = temp_data[3];
    }
    return 0;
}

uint8_t poll_APDS_proximity(void) {
    if (APDS.proximityAvailable())
        data[16] = (float)APDS.readProximity();
    return 0;
}

uint8_t poll_APDS_gesture(void) {
    if (APDS.gestureAvailable())
        data[17] = (float)APDS.readGesture();
    return 0;
}
