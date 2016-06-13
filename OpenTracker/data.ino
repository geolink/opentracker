//collect and send GPS data for sending

void data_append_char(char c) {
  if (data_index < sizeof(data_current) - 1)
    data_current[data_index++] = c;
}

void data_append_string(const char *str) {
  int len = strlen(str);
  for(int i=0; i<len && data_index < sizeof(data_current) - 1; )
    data_current[data_index++] = str[i++];
}

/**
* This is default collect data function for HTTP
*/
void collect_all_data(int ignitionState) {
  debug_print(F("collect_all_data() started"));

  //get current time and add to this data packet
  gsm_get_time();

  //attach time to data packet
  data_append_string(time_char);

  //collect all data
  //indicate start of GPS data packet
  data_append_char('[');

  collect_gps_data();

  //indicate stop of GPS data packet
  data_append_char(']');

  if(DATA_INCLUDE_BATTERY_LEVEL) {
    // append battery level to data packet
    float sensorValue = analogRead(AIN_S_INLEVEL);
    float outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    char batteryLevel[20];
    snprintf(batteryLevel,20,"%.2f",outputValue);

    data_append_string(batteryLevel);
  }

  // ignition state
  if(DATA_INCLUDE_IGNITION_STATE) {
    if(DATA_INCLUDE_BATTERY_LEVEL) {
      data_append_char(',');
    }
    if(ignitionState == -1) {
      data_append_char('2'); // backup source
    } else if(ignitionState == 0) {
      data_append_char('1');
    } else {
      data_append_char('0');
    }
  }

  // engine running time
  if(DATA_INCLUDE_ENGINE_RUNNING_TIME) {
    unsigned long currentRunningTime = engineRunningTime;
    char runningTimeString[32];

    if(engineRunning == 0) {
      currentRunningTime += (millis() - engine_start);
    }

    snprintf(runningTimeString,32,"%ld",(unsigned long) currentRunningTime / 1000);

    if(DATA_INCLUDE_IGNITION_STATE || DATA_INCLUDE_BATTERY_LEVEL) {
      data_append_char(',');
    }
    data_append_string(runningTimeString);
  }

  //end of data packet
  data_append_char('\n');

  //terminate data_current
  data_current[data_index] = '\0';
  data_index++;

  debug_print(F("collect_all_data() completed"));
}

/**
* This function collects data for RAW TCP
*/
void collect_all_data_raw(int ignitionState) {
  debug_print(F("collect_all_data_raw() started"));

  gsm_get_time();

  if(SEND_RAW_INCLUDE_IMEI) {
    data_append_string(config.imei);
  }

  if(SEND_RAW_INCLUDE_KEY) {
    if(data_index > 0) {
      data_append_char(',');
    }
    for(int i=0;i<strlen(config.key);i++) {
      data_current[data_index++] = config.key[i];
    }
  }

  if(SEND_RAW_INCLUDE_TIMESTAMP) {
    if(data_index >0) {
      data_append_char(',');
    }
    for(int i=0;i<strlen(time_char);i++) {
      data_current[data_index++] = time_char[i];
    }
  }

  if(SEND_RAW_INCLUDE_KEY || SEND_RAW_INCLUDE_TIMESTAMP) {
    data_append_char(',');
  }

  collect_gps_data();

  if(DATA_INCLUDE_BATTERY_LEVEL) {
    if(data_index >0) {
      data_append_char(',');
    }

    // append battery level to data packet
    float sensorValue = analogRead(AIN_S_INLEVEL);
    float outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    char batteryLevel[20];
    snprintf(batteryLevel,20,"%.2f",outputValue);

    for(int i=0; i<strlen(batteryLevel); i++) {
      data_current[data_index++] = batteryLevel[i];
    }
  }

  // ignition state
  if(DATA_INCLUDE_IGNITION_STATE) {
    if(data_index >0) {
      data_append_char(',');
    }
    if(ignitionState == -1) {
      data_append_char('2'); // backup source
    } else if(ignitionState == 0) {
      data_append_char('1');
    } else {
      data_append_char('0');
    }
  }

  // engine running time
  if(DATA_INCLUDE_ENGINE_RUNNING_TIME) {
    if(data_index >0) {
      data_append_char(',');
    }

    unsigned long currentRunningTime = engineRunningTime;
    char runningTimeString[32];

    if(engineRunning == 0) {
      currentRunningTime += (millis() - engine_start);
    }

    snprintf(runningTimeString,32,"%ld",(unsigned long) currentRunningTime / 1000);

    data_append_string(runningTimeString);
  }

  //terminate data_current
  data_current[data_index] = '\0';
  data_index++;

  debug_print(F("collect_all_data_raw() completed"));
}

/**
 * This function send collected data using HTTP or TCP
 */
void send_data() {
  debug_print(F("Current:"));
  debug_print(data_current);

  if(SEND_DATA) {
    int i = gsm_send_data();
    if(i != 1) {
      //current data not sent, save to sd card
      debug_print(F("Can not send data, saving to flash memory"));
      
      #if STORAGE
        storage_save_current();   //in case this fails - data is lost
      #endif
      
    } else {
      debug_print(F("Data sent successfully."));
    }
  }
}
