
void reboot() {
  debug_print(F("reboot() started"));

  //reset GPS
  gps_off();

  //emergency power off GSM
  gsm_off(1);

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
  
  debug_print(F("reboot() failed"));
}

void enter_low_power() {
  debug_print(F("enter_low_power() started"));

  addon_event(ON_DEVICE_STANDBY);
  
  // enter standby/sleep mode

  gps_standby();
  gps_close();

  gsm_standby();
  gsm_close();

  addon_event(ON_CLOCK_PAUSE);

  // switch to low-power mode with interrupts disabled
  cpu_irq_disable();

  // disable USB
  debug_enable = false;
  // Console.end() does nothing, manually disable USB serial console
  USBDevice.detach(); // detach from Host

  // de-init procedure (reverses UDD_Init)
  otg_freeze_clock();
  pmc_disable_udpck();
  pmc_disable_upll_clock();
  pmc_disable_periph_clk(ID_UOTGHS);
  NVIC_DisableIRQ((IRQn_Type) ID_UOTGHS);

  // slow down CPU
  pmc_mck_set_prescaler(PMC_MCKR_PRES_CLK_16); // master clock prescaler
  pmc_switch_mainck_to_fastrc(CKGR_MOR_MOSCRCF_4_MHz);

  cpu_irq_enable();
  
  // update timer settings
  SystemCoreClockUpdate();
  SysTick_Config(SystemCoreClock / 1000);

  addon_event(ON_CLOCK_RESUME);
}

void exit_low_power() {
  addon_event(ON_CLOCK_PAUSE);

  // re-init clocks to full speed
  SystemInit();
  SysTick_Config(SystemCoreClock / 1000);

  // re-initialize USB
  UDD_Init();
  USBDevice.attach(); // re-attach to Host
  debug_enable = true;

  addon_event(ON_CLOCK_RESUME);

  // enable serial ports
  gsm_open();
  gsm_wakeup();
  
  gps_open();
  gps_wakeup();

  addon_event(ON_DEVICE_WAKEUP);
  
  debug_print(F("exit_low_power() completed"));
}

