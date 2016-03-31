// Interface for Add-on board functions
// (define the following symbol in "tracker.h" to provide your own implementation)
#if !ADDON_INTERFACE

void addon_delay(long ms) {
  // called for delays that can run addon code
  // default implementation just calls delay()
  delay(ms);
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

#endif

// event types
enum {
  ON_MODEM_REPLY,
  ON_SEND_STARTED,
  ON_SEND_DATA,
  ON_SEND_COMPLETED,
  ON_SEND_FAILED,
  ON_RECEIVE_STARTED,
  ON_RECEIVE_DATA,
  ON_RECEIVE_COMPLETED,
  ON_RECEIVE_FAILED,
  ON_LOCATION_FIXED,
  ON_LOCATION_LOST,
};

