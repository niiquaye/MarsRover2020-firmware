#include "LimServo.h"

LimServo::LimServo(PinName pin) : Servo(pin) {
  m_range = DEFAULT_RANGE;
  m_pos   = 0;

  m_max_pulse_ms = DEFAULT_MAX;
  m_min_pulse_ms = DEFAULT_MIN;

  // INIT PWM DEFAULT
  m_pwm.period_us(PERIOD * 1000);
}

LimServo::LimServo(PinName pin, float range) : LimServo(pin) {
  m_range = range;
}

LimServo::LimServo(PinName pin, float range, float max_pulse_ms, float min_pulse_ms) : LimServo(pin, range) {
  m_max_pulse_ms = max_pulse_ms;
  m_min_pulse_ms = min_pulse_ms;
}

mbed_error_status_t LimServo::setRange(float range) {
  m_range = range;
  return MBED_SUCCESS;
}

float LimServo::getRange(void) {
  return m_range;
}

mbed_error_status_t LimServo::setPosition(float angle) {
  m_pos = (std::abs(angle) < m_range) ? angle : m_range * getSign(angle);
  // angle = -1 * m_range -> pwm m_min_pulse_ms; angle = +1 * m_range -> pwm m_max_pulse_ms
  // now, if we add m_range to angle, angle = 0 -> pwm m_min_pulse_ms; angle = 2 * m_range -> pwm m_max_pulse_ms
  // now we have angle as a value from 0 to 2 * m_range, which can be linearly mapped to m_min_pulse_ms to
  // m_max_pulse_ms, with ease
  angle += m_range;
  m_pwm.pulsewidth_us(int(((m_max_pulse_ms - m_min_pulse_ms) * angle / (m_range * 2) + m_min_pulse_ms) * 1000));
  return MBED_SUCCESS;
}

float LimServo::read(void) {
  return m_pos;
}
