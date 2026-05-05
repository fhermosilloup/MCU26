/*
 *
 * @example
 * 	#include "mpu6050.h"
 *
 *	mpu6050_t imu;
 *
 *	int main(void)
 *	{
 *		uart_init(USART1, 9600, UART_MODE_TX, UART_PARITY_NONE, UART_STOP_1);
 *
 *		i2c_init(I2C1, I2C_MODE_STANDARD);
 *		i2c_set_speed(I2C1, 100000, I2C_DUTY_2);
 *
 *		if (!mpu6050_init(&imu, I2C1, MPU6050_DEVICE_0))
 *		{
 *			uart_println("MPU6050 not detected");
 *			while (1);
 *		}
 *		uart_println("MPU6050 detected");
 *
 *		// Deshabilitar eje Z del giroscopio (ejemplo didáctico)
 *		mpu6050_set_axis_enable(&imu, MPU6050_AXIS_GYRO_Z, false);
 *
 *		// Configurar rangos
 *		mpu6050_set_accel_range(&imu, MPU6050_ACCEL_RANGE_2G);
 *		mpu6050_set_gyro_range(&imu, MPU6050_GYRO_RANGE_250DPS);
 *
 *		// Configurar filtro + sample rate
 *		uint16_t sr;
 *		mpu6050_set_sample_rate_cfg(&imu, MPU6050_BANDWIDTH_3, 9, &sr);
 *		//Since bandwidth > 0, sample rate should be 1000Hz/(9+1) = 100Hz
 *
 *		while (1)
 *		{
 *			if (mpu6050_read_raw(&imu))
 *			{
 *				float ax = imu.data.accel.x / imu.accel_scale;
 *				float ay = imu.data.accel.y / imu.accel_scale;
 *				float az = imu.data.accel.z / imu.accel_scale;
 *
 *				float gx = imu.data.gyro.x / imu.gyro_scale;
 *				float gy = imu.data.gyro.y / imu.gyro_scale;
 *				float gz = imu.data.gyro.z / imu.gyro_scale;
 *			}
 *			delay_ms(100);
 *		}
 *	}
 */
#ifndef MPU6050_H_
#define MPU6050_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32f103_hal.h"

/* Typedefs ------------------------------------------------------*/
typedef enum {
    MPU6050_DEVICE_0 = 0,  ///< AD0 = 0 (Slave Address = 0x68)
    MPU6050_DEVICE_1       ///< AD0 = 1 (Slave Address = 0x69)
} mpu6050_devid_t;


typedef enum {
    MPU6050_ACCEL_RANGE_2G = 0,  ///< Rango ±2g
    MPU6050_ACCEL_RANGE_4G,      ///< Rango ±4g
    MPU6050_ACCEL_RANGE_8G,      ///< Rango ±8g
    MPU6050_ACCEL_RANGE_16G      ///< Rango ±16g
} mpu6050_accel_range_t;

typedef enum {
    MPU6050_GYRO_RANGE_250DPS = 0,  ///< Rango ±250 °/s
    MPU6050_GYRO_RANGE_500DPS,      ///< Rango ±500 °/s
    MPU6050_GYRO_RANGE_1000DPS,     ///< Rango ±1000 °/s
    MPU6050_GYRO_RANGE_2000DPS      ///< Rango ±2000 °/s
} mpu6050_gyro_range_t;

typedef enum {
    MPU6050_CLOCK_INT_8MHZ  = 0,  ///< Oscilador interno
    MPU6050_CLOCK_PLL_YGYRO,      ///< PLL usando giroscopio eje Y
    MPU6050_CLOCK_PLL_XGYRO,      ///< PLL usando giroscopio eje X
    MPU6050_CLOCK_PLL_ZGYRO,      ///< PLL usando giroscopio eje Z
    MPU6050_CLOCK_EXT_32KHZ,      ///< Reloj externo 32 kHz
    MPU6050_CLOCK_EXT_19MHZ,      ///< Reloj externo 19.2 MHz
    MPU6050_CLOCK_RESET           ///< Reset de fuente de reloj
} mpu6050_clock_t;

// Low Pass Filter (se mantiene igual)
typedef enum {					/**<           accelerometer                     gyroscope             */
    MPU6050_BANDWIDTH_0 = 0,	/**< bandwidth(Hz) Fs(KHz) delay(ms)   bandwidth(Hz) fs(KHz) delay(ms) */
    MPU6050_BANDWIDTH_1,      	/**<      260         1         0          256          8      0.98    */
    MPU6050_BANDWIDTH_2,       	/**<      184         1       2.0          188          1       1.9    */
    MPU6050_BANDWIDTH_3,       	/**<       94         1       3.0           98          1       2.8    */
    MPU6050_BANDWIDTH_4,       	/**<       44         1       4.9           42          1       4.8    */
    MPU6050_BANDWIDTH_5,       	/**<       21         1       8.5           20          1       8.3    */
    MPU6050_BANDWIDTH_6         /**<       10         1      13.8           10          1      13.4    */
} mpu6050_bandwidth_t;

typedef enum {
    MPU6050_PIN_MODE_PP = 0,    ///< Salida push-pull
    MPU6050_PIN_MODE_OD         ///< Salida open-drain
} mpu6050_pin_mode_t;

typedef enum {
    MPU6050_PIN_LEVEL_HIGH = 0,  ///< Activo en nivel alto
    MPU6050_PIN_LEVEL_LOW        ///< Activo en nivel bajo
} mpu6050_pin_level_t;

typedef enum {
    MPU6050_PIN_TRIG_PULSE = 0,  ///< Pulso corto por evento
    MPU6050_PIN_TRIG_LATCH       ///< Permanece activo hasta limpiar
} mpu6050_pin_trig_t;

typedef enum {
    MPU6050_IRQ_CLEAR_BY_STATUS = 0, ///< Se limpia al leer INT_STATUS
    MPU6050_IRQ_CLEAR_BY_READ        ///< Se limpia al leer cualquier dato
} mpu6050_irq_clear_t;

typedef enum {
    MPU6050_IRQ_MOTION        = 6,  ///< Interrupción por detección de movimiento
    MPU6050_IRQ_FIFO_OVERFLOW = 4,  ///< Desbordamiento del FIFO
    MPU6050_IRQ_I2C_MAST      = 3,  ///< Evento del maestro I2C auxiliar
    MPU6050_IRQ_DMP           = 1,  ///< Evento del DMP
    MPU6050_IRQ_DATA_READY    = 0   ///< Datos nuevos disponibles
} mpu6050_irq_t;

typedef enum {
    MPU6050_FIFO_TEMP   = 0x07,  ///< Temperatura
    MPU6050_FIFO_XG     = 0x06,  ///< Giroscopio eje X
    MPU6050_FIFO_YG     = 0x05,  ///< Giroscopio eje Y
    MPU6050_FIFO_ZG     = 0x04,  ///< Giroscopio eje Z
    MPU6050_FIFO_ACCEL  = 0x03   ///< Acelerómetro (XYZ)
} mpu6050_fifo_src_t;

typedef enum {
    MPU6050_AXIS_GYRO_Z = 0, ///< Eje Z del giroscopio
    MPU6050_AXIS_GYRO_Y = 1, ///< Eje Y del giroscopio
    MPU6050_AXIS_GYRO_X = 2, ///< Eje X del giroscopio
    MPU6050_AXIS_ACC_Z  = 3, ///< Eje Z del acelerómetro
    MPU6050_AXIS_ACC_Y  = 4, ///< Eje Y del acelerómetro
    MPU6050_AXIS_ACC_X  = 5  ///< Eje X del acelerómetro
} mpu6050_axis_t;

/**
 * @brief Vector de 3 ejes en punto flotante.
 */
typedef struct {
    float x; ///< Eje X
    float y; ///< Eje Y
    float z; ///< Eje Z
} mpu6050_pointf_t;

/**
 * @brief Vector de 3 ejes en formato crudo (LSB, complemento a 2).
 */
typedef struct {
    int16_t x; ///< Eje X
    int16_t y; ///< Eje Y
    int16_t z; ///< Eje Z
} mpu6050_point_t;

/**
 * @brief Estructura de datos completos del sensor.
 *
 * Contiene las mediciones crudas de acelerómetro, giroscopio
 * y temperatura obtenidas directamente del dispositivo.
 */
typedef struct {
    mpu6050_point_t accel;   ///< Aceleración cruda
    mpu6050_point_t gyro;    ///< Velocidad angular cruda
    int16_t temperature;     ///< Temperatura cruda
} mpu6050_data_t;

typedef struct {
  mpu6050_pointf_t accel;   ///< Aceleración calibrada
  mpu6050_pointf_t gyro;    ///< Velocidad angular calibrada
  int16_t temperature;     ///< Temperatura calibrada
} mpu6050_data_calib_t;

/**
 * @brief Estructura de calibración.
 *
 * Almacena los offsets de acelerómetro y giroscopio
 * calculados durante el proceso de calibración.
 */
typedef struct {
    mpu6050_point_t gyro_bias;   ///< Offset giroscopio
    mpu6050_point_t accel_bias;  ///< Offset acelerómetro
} mpu6050_calib_t;

/**
 * @brief Estructura principal del dispositivo.
 *
 * Contiene la configuración del periférico I2C, dirección del dispositivo,
 * datos actuales, parámetros de calibración y factores de escala para
 * conversión a unidades físicas.
 */
typedef struct {
    I2C_TypeDef *I2Cx;   ///< Instancia I2C
    uint8_t address;     ///< Dirección I2C

    mpu6050_data_t data;   ///< Datos actuales
    mpu6050_calib_t calib; ///< Parámetros de calibración

    float accel_scale; ///< Factor de escala acelerómetro
    float gyro_scale;  ///< Factor de escala giroscopio
} mpu6050_t;


/* Exported prototype function ------------------------------------*/
/**
 * @brief Inicializa el dispositivo MPU6050.
 *
 * Configura la interfaz I2C, establece la dirección del dispositivo
 * y realiza la inicialización básica del sensor.
 *
 * @param dev Puntero a la estructura del dispositivo
 * @param I2Cx Instancia del periférico I2C
 * @param id Selección de dirección del dispositivo basada en AD0
 *
 * @return true si la inicialización fue exitosa
 */
bool mpu6050_init(mpu6050_t *dev,
                  I2C_TypeDef *I2Cx,
                  mpu6050_devid_t id);

/**
 * @brief Verifica la presencia del dispositivo en el bus I2C.
 *
 * Realiza una lectura de identificación para confirmar
 * que el dispositivo responde correctamente.
 *
 * @param dev Puntero a la estructura del dispositivo
 *
 * @return true si el dispositivo responde correctamente
 */
bool mpu6050_probe(mpu6050_t *dev);


/**
 * @brief Habilita o deshabilita un eje específico.
 *
 * Permite activar o poner en modo standby cada eje
 * del acelerómetro o giroscopio de manera independiente.
 *
 * @param dev Puntero al dispositivo
 * @param axis Eje a habilitar/deshabilitar
 * @param enabled true para habilitar, false para deshabilitar
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_axis_enable(mpu6050_t *dev,
                               mpu6050_axis_t axis,
                               bool enabled);

/**
 * @brief Controla el modo sleep del dispositivo.
 *
 * En modo sleep el sensor reduce significativamente su consumo
 * y detiene la adquisición de datos.
 *
 * @param dev Puntero al dispositivo
 * @param enable true para activar sleep, false para operar normalmente
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_sleep(mpu6050_t *dev, bool enable);

/**
 * @brief Realiza un reset completo del dispositivo.
 *
 * Restablece todos los registros a su estado por defecto.
 *
 * @param dev Puntero al dispositivo
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_reset(mpu6050_t *dev);


/**
 * @brief Configura el rango del acelerómetro.
 *
 * Define la escala de medición del acelerómetro y ajusta
 * el factor de conversión correspondiente.
 *
 * @param dev Puntero al dispositivo
 * @param range Rango seleccionado
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_accel_range(mpu6050_t *dev,
                             mpu6050_accel_range_t range);

/**
 * @brief Configura el rango del giroscopio.
 *
 * Define la escala de medición del giroscopio y ajusta
 * el factor de conversión correspondiente.
 *
 * @param dev Puntero al dispositivo
 * @param range Rango seleccionado
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_gyro_range(mpu6050_t *dev,
                            mpu6050_gyro_range_t range);

/**
 * @brief Configura el filtro digital y la frecuencia de muestreo.
 *
 * Ajusta el ancho de banda mediante el filtro DLPF y define
 * la frecuencia de muestreo usando un divisor.
 *
 * Sample Rate = Internal Rate / (1 + divider)
 *
 * @param dev Puntero al dispositivo
 * @param bw Configuración de bandwidth (DLPF)
 * @param divider Divisor de frecuencia
 * @param sample_rate_out Frecuencia resultante (opcional)
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_odr(mpu6050_t *dev,
                             mpu6050_bandwidth_t bw,
                             uint8_t divider,
                             uint16_t *sample_rate_out);

/**
 * @brief Selecciona la fuente de reloj del dispositivo.
 *
 * Permite elegir entre el oscilador interno, fuentes externas
 * o PLL basado en el giroscopio.
 *
 * @param dev Puntero al dispositivo
 * @param clksrc Fuente de reloj
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_clock_source(mpu6050_t *dev,
                              mpu6050_clock_t clksrc);


/**
 * @brief Configura el comportamiento del pin de interrupción.
 *
 * Define el tipo de salida, nivel activo y modo de señal del pin INT.
 *
 * @param dev Puntero al dispositivo
 * @param mode Tipo de salida
 * @param level Nivel activo
 * @param trig Tipo de disparo
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_int_pin(mpu6050_t *dev,
                         mpu6050_pin_mode_t mode,
                         mpu6050_pin_level_t level,
                         mpu6050_pin_trig_t trig);

/**
 * @brief Habilita o deshabilita una interrupción específica.
 *
 * @param dev Puntero al dispositivo
 * @param irq Fuente de interrupción
 * @param enable true para habilitar
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_interrupt_enable(mpu6050_t *dev,
                                  mpu6050_irq_t irq,
                                  bool enable);

/**
 * @brief Define el método de limpieza de interrupciones.
 *
 * Permite seleccionar si la interrupción se limpia al leer
 * el registro de estado o al leer cualquier registro del sensor.
 *
 * @param dev Puntero al dispositivo
 * @param mode Modo de limpieza
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_irq_clear_mode(mpu6050_t *dev,
                                mpu6050_irq_clear_t mode);

/**
 * @brief Habilita o deshabilita el FIFO global.
 *
 * @param dev Puntero al dispositivo
 * @param enable true para habilitar
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_fifo_set_enable(mpu6050_t *dev, bool enable);

/**
 * @brief Habilita una fuente dentro del FIFO.
 *
 * @param dev Puntero al dispositivo
 * @param src Fuente de datos
 * @param enable true para habilitar
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_set_fifo_source(mpu6050_t *dev,
                             mpu6050_fifo_src_t src,
                             bool enable);

/**
 * @brief Obtiene el número de bytes disponibles en el FIFO.
 *
 * Lee el contador interno del FIFO (FIFO_COUNTH / FIFO_COUNTL).
 *
 * @param dev Puntero al dispositivo
 * @param count Puntero donde se almacenará el número de bytes disponibles
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_fifo_get_count(mpu6050_t *dev, uint16_t *count);

/**
 * @brief Lee datos desde el FIFO.
 *
 * Extrae una cantidad específica de bytes desde el FIFO interno.
 * La lectura es secuencial y consume los datos del buffer.
 *
 * @param dev Puntero al dispositivo
 * @param data Buffer de salida
 * @param len Número de bytes a leer
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_fifo_read(mpu6050_t *dev, uint8_t *data, uint16_t len);

/**
 * @brief Reinicia el FIFO.
 *
 * Borra completamente el contenido del buffer FIFO.
 *
 * @param dev Puntero al dispositivo
 *
 * @return true si la operación fue exitosa
 */
bool mpu6050_fifo_reset(mpu6050_t *dev);


/**
 * @brief Realiza calibración en reposo.
 *
 * Calcula los offsets de acelerómetro y giroscopio
 * promediando múltiples muestras sin movimiento.
 *
 * @param dev Puntero al dispositivo
 * @param num_samples Número de muestras promediadas
 *
 * @return true si la calibración fue exitosa
 */
bool mpu6050_calibrate(mpu6050_t *dev, uint16_t num_samples);


/**
 * @brief Lee datos crudos del sensor.
 *
 * Actualiza la estructura interna con valores en formato
 * entero con signo.
 *
 * @param dev Puntero al dispositivo
 *
 * @return true si la lectura fue exitosa
 */
bool mpu6050_read_raw(mpu6050_t *dev);

/**
 * @brief Lee datos calibrados del sensor.
 *
 * Aplica offsets y factores de escala para obtener valores
 * corregidos en unidades físicas.
 *
 * @param dev Puntero al dispositivo
 * @param data_cal Estructura de salida
 *
 * @return true si la lectura fue exitosa
 */
bool mpu6050_read_cal(mpu6050_t *dev, mpu6050_data_calib_t *data_cal);

#ifdef __cplusplus
}
#endif

#endif
