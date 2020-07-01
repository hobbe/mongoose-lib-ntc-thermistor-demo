/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * Copyright (c) 2020 hobbe <https://github.com/hobbe>
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"
#include "mgos_ntc_thermistor.h"

static struct NTC_Thermistor *ntc = NULL;

static void timer_cb(void *arg) {
  static bool s_tick_tock = false;
  LOG(LL_INFO,
      ("%s uptime: %.2lf, RAM: %lu, %lu free", (s_tick_tock ? "Tick" : "Tock"),
       mgos_uptime(), (unsigned long) mgos_get_heap_size(),
       (unsigned long) mgos_get_free_heap_size()));
  s_tick_tock = !s_tick_tock;
#ifdef LED_PIN
  mgos_gpio_toggle(LED_PIN);
#endif

  if (ntc != NULL) {
    double temperature = mgos_ntc_thermistor_read_celsius(ntc);
    LOG(LL_INFO, ("Temperature: %.1f C", temperature));
  }

  (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {
#ifdef LED_PIN
  mgos_gpio_setup_output(LED_PIN, 0);
#endif

  // Initialize NTC thermistor library
  if ((ntc = mgos_ntc_thermistor_create()) == NULL) {
    LOG(LL_ERROR, ("Unable to initialize NTC Thermistor"));
  } else {
    // Shelly 1PM
    const unsigned char pin = 0;                  // A0 on esp8266
    const double referenceResistance = 10000;     // 10k
    const double nominalResistance = 32000;       // 32k
    const double nominalTemperatureCelsius = 25;  // 25°C
    const double bValue = 3350;                   // 3350
    const int adcResolution = 1024;               // 1024
    mgos_ntc_thermistor_begin(ntc, pin, referenceResistance, nominalResistance,
                              nominalTemperatureCelsius, bValue, adcResolution);
    LOG(LL_INFO, ("NTC Thermistor initialized"));
  }

  mgos_set_timer(2000 /* ms */, MGOS_TIMER_REPEAT, timer_cb, NULL);

  return MGOS_APP_INIT_SUCCESS;
}
