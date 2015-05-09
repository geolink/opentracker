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

    // append battery level to data packet
    float sensorValue = analogRead(AIN_S_INLEVEL);
    float outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    char batteryLevel[20];
    snprintf(batteryLevel,20,"%.2f",outputValue);

    for (int i=0; i<strlen(batteryLevel); i++) {
      data_current[data_index++] = batteryLevel[i];
    }  

    // ignition state
    data_current[data_index++] = ',';
    if (ignitionState == 0) {
      data_current[data_index++] = '1';
    } else {
      data_current[data_index++] = '0';
    }

    //end of data packet   
    data_current[data_index] = '\n';
    data_index++;
    
    //terminate data_current 
    data_current[data_index] = '\0';   
    data_index++; 
    
    
    debug_print(F("collect_all_data() completed"));
    
  }
  
  
  
  
  
