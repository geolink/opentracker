void settings_load()
    {
      //load all settings from EEPROM
      int tmp; 
      
      debug_print(F("settings_load() started"));
      
      byte* b = dueFlashStorage.readAddress(STORAGE_CONFIG_PAGE); // byte array which is read from flash at adress        
      memcpy(&config, b, sizeof(settings)); // copy byte array to temporary struct
  
      
      //setting defaults in case nothing loaded      
      debug_print(F("settings_load(): config.interval:"));
      debug_print(config.interval);    
      
      if((config.interval == -1) || (config.interval == NULL))
        {
          debug_print(F("settings_load(): interval not found, setting default"));
          config.interval = INTERVAL;
          
          debug_print(F("settings_load(): set config.interval:"));
          debug_print(config.interval);    
        }

      
      //interval send
      debug_print(F("settings_load(): config.interval_send:"));
      debug_print(config.interval_send);      

      if((config.interval_send == -1) || (config.interval_send == NULL))
        {
          debug_print(F("settings_load(): interval_send not found, setting default"));
          config.interval_send = INTERVAL_SEND;
          
          debug_print(F("settings_load(): set config.interval_send:"));
          debug_print(config.interval_send);              
        }
      
      //powersave
      debug_print(F("settings_load(): config.powersave:"));
      debug_print(config.powersave);      

      if((config.powersave != 1) && (config.powersave != 0))
        {
          debug_print(F("settings_load(): powersave not found, setting default"));
          config.powersave = POWERSAVE;
          
          debug_print(F("settings_load(): set config.powersave:"));
          debug_print(config.powersave);              
        }
      
      tmp = config.key[0];
      if(tmp == 255)
        {
          debug_print(F("settings_load(): key not found, setting default"));                    
          strlcpy(config.key, KEY, 12);
        }
        
       tmp = config.sms_key[0]; 
       if(tmp == 255)
        {
           debug_print("settings_load(): SMS key not found, setting default");                 
           strlcpy(config.sms_key, SMS_KEY, 12);
        }         
        
        tmp = config.apn[0];       
        if(tmp == 255)
        {
           debug_print("settings_load(): APN not set, setting default");                 
           strlcpy(config.apn, DEFAULT_APN, 20);
        }  
        
        tmp = config.user[0];  
        if(tmp == 255)
        {
           debug_print("settings_load(): APN user not set, setting default");                 
           strlcpy(config.user, DEFAULT_USER, 20);
        }  
        
        tmp = config.pwd[0];  
        if(tmp == 255)
        {
           debug_print("settings_load(): APN password not set, setting default");                 
           strlcpy(config.pwd, DEFAULT_PASS, 20);
        } 
        
       
      debug_print(F("settings_load() finished"));      
    }
    
 void settings_save()
    {
      debug_print(F("settings_save() started"));
      
      //save all settings to flash
      byte b2[sizeof(settings)]; // create byte array to store the struct
      memcpy(b2, &config, sizeof(settings)); // copy the struct to the byte array
      dueFlashStorage.write(STORAGE_CONFIG_PAGE, b2, sizeof(settings)); // write byte array to flash
 
      
      debug_print(F("settings_save() finished"));
    }
    
    
 
