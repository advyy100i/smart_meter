#define USE_RUNNING_STATISTICS 0
#define USE_GENERATOR 1

#if USE_RUNNING_STATISTICS
#include <Filters.h>
#endif

// for some reason intellisense is not able to find iom328p.h
#ifdef __INTELLISENSE__
#include <avr/iom328p.h>
#else
#define __AVR_ATmega328P__
#include <avr/io.h>
#endif

#include <SoftwareSerial.h>
#include <SerialTransfer.h>

#define CURRENT_PIN A0
#define VOLTAGE_PIN A1
#define ESP01_BAUD_RATE 115200 // keep this same on esp-01
#define ESP01_SERIAL_RX 3
#define ESP01_SERIAL_TX 4
#define ESP01_PACKET_MAGIC 0x2A4D
#define UPDATE_THRESHOLD 200

// energy to be calculated in watt hours
float energy;
// last time the current and voltage values were updated
unsigned long last_updated;

#if USE_RUNNING_STATISTICS

float testingFrequency = 50.0;
float windowLength = 20.0 / testingFrequency;

#else

#define SAMPLES_COUNT 400

int high_peak, low_peak;
float amps_peak_peak, amps_rms;
float volts_peak_peak, volts_rms;

#endif

SoftwareSerial esp01_serial(ESP01_SERIAL_RX, ESP01_SERIAL_TX);
SerialTransfer myTransfer;

/**
 * Packet to be sent to the ESP-01 module. This packet will be sent over
 * UART to the ESP-01 module. The ESP-01 module will then send this packet
 * to the server. The server will then process this packet and update
 * the database.
 *
 * Packet structure:
 *  - magic: 2 bytes
 *  - voltage: 2 bytes
 *  - current: 2 bytes
 *  - power: 2 bytes
 *  - energy: 2 bytes
 */
typedef struct __attribute__((packed)) esp01_packet
{
  uint16_t magic;
  uint16_t voltage;
  uint16_t current;
  uint32_t power;
  uint32_t energy;
} esp01_packet_t;

/**
 * Set timer interrupts for the microcontroller.
 */
void set_timer_interrupts()
{
  // stop interrupts
  cli();

  // enable timer1 with frequncy of 1Hz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1 = 0;  // initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624; // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  // allow interrupts
  sei();
}

/**
 * Send packet to the ESP-01 module.
 * @param packet Packet to send to the ESP-01 module.
 */
void send_packet(esp01_packet_t *packet)
{
  esp01_serial.write((uint8_t *)&packet, sizeof(esp01_packet_t));
}

ISR(TIMER1_COMPA_vect)
{
  float power = volts_rms * amps_rms;
  // update the energy only when the current and voltage values are updated
  if (millis() - last_updated < UPDATE_THRESHOLD)
  {
    // update energy
    energy += power / 3600;

    esp01_packet_t packet;
    packet.magic = ESP01_PACKET_MAGIC;
    packet.voltage = static_cast<uint16_t>(volts_rms * 100);
    packet.current = static_cast<uint16_t>(amps_rms * 100);
    packet.power = static_cast<uint32_t>(volts_rms * amps_rms * 100);
    packet.energy = static_cast<uint32_t>(energy * 100);

    // send the packet to esp-01
    myTransfer.txObj(packet, 0, sizeof(esp01_packet_t));
    myTransfer.sendData(sizeof(esp01_packet_t));
  }
  else
  {
    // we have an error cause the current and voltage values are not updated
    // we need to have a proper error handling mechanism

    // TODO: implement error handling
    Serial.println("Outdated data!");
  }
}

void setup()
{
  set_timer_interrupts();

  Serial.begin(9600);
  esp01_serial.begin(ESP01_BAUD_RATE);
  myTransfer.begin(esp01_serial);

  pinMode(CURRENT_PIN, INPUT);
  pinMode(VOLTAGE_PIN, INPUT);
}

#if USE_GENERATOR

/** Generate a random floating point value between min and max. */
float generate_random_float(float min, float max)
{
  return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

#endif

void loop()
{
#if USE_RUNNING_STATISTICS

  RunningStatistics inputStats;
  inputStat.setWindowSecs(windowLength);

  while (true)
  {
    sensorValue = analogRead(CURRENT_PIN);
    inputStat.input(SensorValue);

    // TODO: implement code for sending to wifi
  }

#elif USE_GENERATOR

  // generate a random voltage between 235.00 to 242.00
  // generate a random current between 2 to 5 amps
  volts_rms = generate_random_float(238.0f, 242.0f);
  amps_rms = generate_random_float(2.0f, 5.0f);

  // add a delay to simulate delay in hardware processing
  delay(86);

  last_updated = millis();
  delay(60);

  // Serial.println(volts_rms);

#else

  float amps_high_peak = 0, amps_low_peak = 1023;
  float volts_high_peak = 0, volts_low_peak = 1023;
  int amps_value, volts_value;

  for (int cnt = 0; cnt < SAMPLES_COUNT; cnt++)
  {
    amps_value = analogRead(CURRENT_PIN);
    amps_high_peak = max(amps_high_peak, amps_value);
    amps_low_peak = min(amps_low_peak, amps_value);

    volts_value = analogRead(VOLTAGE_PIN);
    volts_high_peak = max(volts_high_peak, volts_value);
    volts_low_peak = min(volts_low_peak, volts_value);
  }

  amps_peak_peak = amps_high_peak - amps_low_peak;
  volts_peak_peak = volts_high_peak - volts_low_peak;

  // Get RMS value from peak to peak current value
  // amps_peak_peak * (1/sqrt(2)) * 0.1 * (1/2) - 0.88;
  amps_rms = amps_peak_peak * 0.01768034 - 0.088;
  volts_rms = volts_peak_peak * 0.01768034 - 0.088;

  // last time the current and voltage value was updated
  last_updated = millis();

  delay(100);

#endif
}
