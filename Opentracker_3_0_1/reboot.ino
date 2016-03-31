
void reboot() {
  debug_print(F("reboot() started"));

  //reset GPS
  gps_off();

  //emergency power off GSM
  gsm_off(1);

  //power cycle everything
  debug_print(F("Power cycling hardware."));

  //disable USB to allow reboot
  //serial monitor on the PC won't work anymore if you don't close it before reset completes
  //otherwise, close the serial monitor, detach the USB cable and connect it again

  // Console.end() does nothing, manually disable USB
  USBDevice.detach(); // detach from Host

  cpu_irq_disable();

  rstc_start_software_reset(RSTC);
  for (;;)
  {
    // If we do not keep the watchdog happy and it times out during this wait,
    // the reset reason will be wrong when the board starts the next time around.
    WDT_Restart(WDT);
  }
}
