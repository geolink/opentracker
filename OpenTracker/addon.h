// Interface for Add-on board functions
// (define the following symbol in "tracker.h" to provide your own implementation)
#if !ADDON_INTERFACE

void addon_delay(long ms) {
  // called for delays that can run addon code
  // default implementation just calls status_delay()
  void status_delay(long ms);
  status_delay(ms);
}

void addon_init() {
  // called at the beginning of setup()
  // use to configure addon board hardware interface (expansion header)
}

void addon_setup() {
  // called at the end of setup()
  // use to initialize external hardware on the addon board
}

void addon_loop() {
  // called inside main loop() or other waiting loops
}

void addon_event(int event) {
  // called on some tracker events (see below list)
}

void addon_sms_command(char *cmd, char *arg, const char *phone) {
  // called to handle unknown SMS commands
}

void addon_collect_data() {
  // called inside collect_all_data() to append custom data
}

#endif

struct addon_settings;

// event types
enum {
  ON_DEVICE_STANDBY,      // before going to low power mode
  ON_DEVICE_WAKEUP,       // after going back to full power mode
  ON_DEVICE_KILL,         // before killing power consumption (no more operational afterwards)
  ON_CLOCK_PAUSE,         // before changing system clock
  ON_CLOCK_RESUME,        // after changing system clock
  ON_SETTINGS_DEFAULT,    // when loading default settings in volatile memory
  ON_SETTINGS_LOAD,       // when loading saved settings from volatile memory
  ON_SETTINGS_SAVE,       // when saving settings to persistent storage
  ON_MODEM_REPLY,         // after each modem reply (can snoop modem_reply[] buffer)
  ON_MODEM_ACTIVATION,    // during the first modem GPRS activation
  ON_SEND_STARTED,        // before initiating a new data connection
  ON_SEND_DATA,           // before sending a block of data
  ON_SEND_COMPLETED,      // after the end of successful transmission
  ON_SEND_FAILED,         // after the end of failed trasmission
  ON_RECEIVE_STARTED,     // before starting to receive data
  ON_RECEIVE_DATA,        // after receiving each block of data
  ON_RECEIVE_COMPLETED,   // after the end of successful reception
  ON_RECEIVE_FAILED,      // after the end of failed receiption
  ON_LOCATION_FIXED,      // after GPS acquired fix (valid location)
  ON_LOCATION_LOST,       // after GPS failed to acquire new fix (invalid location)
};

