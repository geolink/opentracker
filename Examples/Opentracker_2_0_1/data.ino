 //collect GPS data for sending 
 
   void  collect_all_data() {
   
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
    

    //end of data packet   
    data_current[data_index] = '\n';
    data_index++;
    
    //terminate data_current 
    data_current[data_index] = '\0';   
    data_index++; 
    
    
    debug_print(F("collect_all_data() completed"));
    
  }
  
  
  
  
  
