 //collect GPS data for sending 
 
   void  collect_all_data(int ignitionState) {
   
    debug_print(F("collect_all_data() started"));       
   
    //get current time and add to this data packet
    gsm_get_time();
   
    //attach time to data packet 
    for(int i=0;i<strlen(time_char);i++)
        {
         data_current[data_index] = time_char[i]; 
         data_index++;
        }
       
    //collect all data   
    //indicate start of GPS data packet
    data_current[data_index] = '['; 
    data_index++; 
    
    
    collect_gps_data();   
    
    //indicate stop of GPS data packet
    data_current[data_index] = ']'; 
    data_index++; 

    if (DATA_INCLUDE_BATTERY_LEVEL) {
        // append battery level to data packet
        float sensorValue = analogRead(AIN_S_INLEVEL);
        float outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
        char batteryLevel[20];
        snprintf(batteryLevel,20,"%.2f",outputValue);

        for (int i=0; i<strlen(batteryLevel); i++) {
          data_current[data_index++] = batteryLevel[i];
        }  
    }

    // ignition state
    if (DATA_INCLUDE_IGNITION_STATE) {
        if (DATA_INCLUDE_BATTERY_LEVEL) {
            data_current[data_index++] = ',';
        }
        if (ignitionState == 0) {
          data_current[data_index++] = '1';
        } else {
          data_current[data_index++] = '0';
        }
    }

    // engine running time
    if (DATA_INCLUDE_ENGINE_RUNNING_TIME) {
        unsigned long currentRunningTime = engineRunningTime;
        char runningTimeString[32];

        if (engineRunning == 0) {
          currentRunningTime += (millis() - engine_start);
        }

        snprintf(runningTimeString,32,"%ld",(unsigned long) currentRunningTime / 1000);

        if (DATA_INCLUDE_IGNITION_STATE || DATA_INCLUDE_BATTERY_LEVEL) {
            data_current[data_index++] = ',';
        }
        for (int i=0; i<strlen(runningTimeString); i++) {
          data_current[data_index++] = runningTimeString[i];
        }
    }

    //end of data packet   
    data_current[data_index] = '\n';
    data_index++;
    
    //terminate data_current 
    data_current[data_index] = '\0';   
    data_index++; 
    
    
    debug_print(F("collect_all_data() completed"));
    
  }
  
  void collect_all_data_raw(int ignitionState)
  {
    debug_print(F("collect_all_data_raw() started"));

    gsm_get_time();

    if (SEND_RAW_INCLUDE_KEY) {
        for (int i=0;i<strlen(config.key);i++) {
            data_current[data_index++] = config.key[i];
        }
    }

    if (SEND_RAW_INCLUDE_TIMESTAMP) {
        if (data_index >0) {
            data_current[data_index++] = ',';
        }
        for(int i=0;i<strlen(time_char);i++) {
            data_current[data_index++] = time_char[i];
        }
    }

    if (SEND_RAW_INCLUDE_KEY || SEND_RAW_INCLUDE_TIMESTAMP) {
      data_current[data_index++] = ',';
    }
    
    collect_gps_data();

    if (DATA_INCLUDE_BATTERY_LEVEL) {
        if (data_index >0) {
            data_current[data_index++] = ',';
        }

        // append battery level to data packet
        float sensorValue = analogRead(AIN_S_INLEVEL);
        float outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
        char batteryLevel[20];
        snprintf(batteryLevel,20,"%.2f",outputValue);

        for (int i=0; i<strlen(batteryLevel); i++) {
          data_current[data_index++] = batteryLevel[i];
        }
    }

    // ignition state
    if (DATA_INCLUDE_IGNITION_STATE) {
        if (data_index >0) {
            data_current[data_index++] = ',';
        }
        if (ignitionState == 0) {
          data_current[data_index++] = '1';
        } else {
          data_current[data_index++] = '0';
        }
    }

    // engine running time
    if (DATA_INCLUDE_ENGINE_RUNNING_TIME) {
        if (data_index >0) {
            data_current[data_index++] = ',';
        }

        unsigned long currentRunningTime = engineRunningTime;
        char runningTimeString[32];

        if (engineRunning == 0) {
          currentRunningTime += (millis() - engine_start);
        }

        snprintf(runningTimeString,32,"%ld",(unsigned long) currentRunningTime / 1000);

        for (int i=0; i<strlen(runningTimeString); i++) {
          data_current[data_index++] = runningTimeString[i];
        }
    }

    if (!SEND_RAW) {
        //end of data packet
        data_current[data_index] = '\n';
        data_index++;
    }

    //terminate data_current
    data_current[data_index] = '\0';
    data_index++;

    debug_print(F("collect_all_data_raw() completed"));
  }
