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

bool data_sep_flag = false;

void data_field_separator(char c) {
  if (data_sep_flag)
    data_append_char(c);
  data_sep_flag = true;
}

void data_field_restart() {
  data_sep_flag = false;
}

// url encoding functions

char to_hex(int nibble) {
  static const char hex[] = "0123456789abcdef";
  return hex[nibble & 15];
}

bool is_url_safe(char c) {
  if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
    || c == '-' || c == '_' || c == '.' || c == '~' || c == '!' || c == '*' || c == '\'' || c == '(' || c == ')')
    return true;
  return false;
}

int url_encoded_strlen(const char* s) {
  int len = strlen(s);
  int ret = 0;
  while (len--) {
    ret += is_url_safe(*s++) ? 1 : 3;
  }
  return ret;
}

// return count of consumed source characters (that fit the buffer after encoding)
int url_encoded_strlcpy(char* dst, int maxlen, const char* src) {
  int len = strlen(src);
  int count = 0;
  while (len > 0 && maxlen > 4) {
    char c = *src++;
    ++count;
    --len;
    if (is_url_safe(c)) {
      *dst++ = c;
      --maxlen;
    } else {
      *dst++ = '%';
      *dst++ = to_hex(c >> 4);
      *dst++ = to_hex(c & 15);
      maxlen -= 3;
    }
  }
  *dst = '\0';
  return count;
}

/**
* This is default collect data function for HTTP
*/
void collect_all_data(int ignitionState) {
  debug_print(F("collect_all_data() started"));

  data_field_restart();
  
  //get current time and add to this data packet
  gsm_get_time();

  //attach time to data packet
  data_append_string(time_char);

  //collect all data
  //indicate start of GPS data packet
  data_append_char('[');

  data_field_restart();

  collect_gps_data();
  
  //indicate stop of GPS data packet
  data_append_char(']');

  data_field_restart();

  if(DATA_INCLUDE_BATTERY_LEVEL) {
    // append battery level to data packet
    float sensorValue = analogRead(AIN_S_INLEVEL);
    float outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    char batteryLevel[20];
    snprintf(batteryLevel,20,"%.2f",outputValue);

    data_field_separator(',');
    data_append_string(batteryLevel);
  }

  // ignition state
  if(DATA_INCLUDE_IGNITION_STATE) {
    data_field_separator(',');
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

    data_field_separator(',');
    data_append_string(runningTimeString);
  }

  addon_collect_data();

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

  data_field_restart();

  if(SEND_RAW_INCLUDE_IMEI) {
    data_append_string(config.imei);
  }

  if(SEND_RAW_INCLUDE_KEY) {
    data_field_separator(',');
    data_append_string(config.key);
  }

  if(SEND_RAW_INCLUDE_TIMESTAMP) {
    data_field_separator(',');
    gsm_get_time();
    data_append_string(time_char);
  }

  data_field_separator(',');
  collect_gps_data();

  if(DATA_INCLUDE_BATTERY_LEVEL) {
    data_field_separator(',');

    // append battery level to data packet
    float sensorValue = analogRead(AIN_S_INLEVEL);
    float outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    char batteryLevel[20];
    snprintf(batteryLevel,20,"%.2f",outputValue);

    data_append_string(batteryLevel);
  }

  // ignition state
  if(DATA_INCLUDE_IGNITION_STATE) {
    data_field_separator(',');
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
    data_field_separator(',');

    unsigned long currentRunningTime = engineRunningTime;
    char runningTimeString[32];

    if(engineRunning == 0) {
      currentRunningTime += (millis() - engine_start);
    }

    snprintf(runningTimeString,32,"%ld",(unsigned long) currentRunningTime / 1000);

    data_append_string(runningTimeString);
  }

  //end of data packet
  data_append_char('\n');

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
