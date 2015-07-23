 
  //gsm functions
  
  
   void gsm_setup()
    {
      //setup modem pins
      debug_print(F("gsm_setup() started"));
      
      pinMode(PIN_C_PWR_GSM, OUTPUT);
      digitalWrite(PIN_C_PWR_GSM, LOW); 
      
      pinMode(PIN_C_KILL_GSM, OUTPUT);         
      digitalWrite(PIN_C_KILL_GSM, LOW); 
      
      pinMode(PIN_STATUS_GSM, INPUT);
      pinMode(PIN_RING_GSM, INPUT); 
      pinMode(PIN_WAKE_GSM, INPUT);         
      
      debug_print(F("gsm_setup() finished"));
            
    }
    
   void gsm_on_off()
    {
      //turn on the modem
      debug_print(F("gsm_on_off() started"));
      
      digitalWrite(PIN_C_PWR_GSM, HIGH);  
      delay(4000);
      digitalWrite(PIN_C_PWR_GSM, LOW);        
      
      debug_print(F("gsm_on_off() finished"));
      
    }
    
   
 void gsm_restart()
  {
    debug_print(F("gsm_restart() started"));
    
    //blink modem restart
      for (int i = 0; i < 5; i++) 
       {
          if (ledState == LOW)
           ledState = HIGH;
          else
           ledState = LOW;
         
        digitalWrite(PIN_POWER_LED, ledState);   // set the LED on
        delay(200);
       }
    
    
    //restart modem
    //check if modem is ON (PWRMON is HIGH)
     int pwrmon = digitalRead(PIN_STATUS_GSM);
     if(pwrmon == HIGH)
       {
          debug_print(F("PWRMON is HIGH. Modem already running."));         
          
         //modem already on, turn modem off 
         gsm_on_off();
         delay(5000);    //wait for modem to shutdown         
       }
       else
       {
         debug_print(F("PWRMON is LOW. Modem is not running."));   
       }
       
    //turn on modem   
    gsm_on_off();    
    
    debug_print(F("gsm_restart() completed"));    
  }
  
  void gsm_set_time()
    {
      int i;

      debug_print(F("gsm_set_time() started"));  
   
      //setting modems clock from current time var
      gsm_port.print("AT+CCLK=\"");
      gsm_port.print(time_char);
      gsm_port.print("\"\r"); 
      
      delay(1000);       
      gsm_get_reply();
   
      debug_print(F("gsm_set_time() completed"));     
   
    }   
  
  void gsm_set_pin()
    {

      debug_print(F("gsm_set_pin() started"));      
         
      //checking if PIN is set 
      gsm_port.print("AT+CPIN?");
      gsm_port.print("\r"); 
      
      delay(1000);       
      gsm_get_reply();
      
      char *tmp = strstr(modem_reply, "SIM PIN");
       if(tmp!=NULL)
        {
          debug_print(F("gsm_set_pin(): PIN is required"));
         
          //checking if pin is valid one
          if(config.sim_pin[0] == -1)
            {
              debug_print(F("gsm_set_pin(): PIN is not supplied."));
            }
            else
            {
               if(strlen(config.sim_pin) == 4)
                 {
                   debug_print(F("gsm_set_pin(): PIN supplied, sending to modem."));
                   
                   gsm_port.print("AT+CPIN=");
                   gsm_port.print(config.sim_pin);                       
                   gsm_port.print("\r"); 
                   
                   delay(1000);       
                   gsm_get_reply();
                   
                   tmp = strstr(modem_reply, "OK"); 
                   if(tmp!=NULL)
                     {
                       debug_print(F("gsm_set_pin(): PIN is accepted"));
                     }
                     else
                     {
                       debug_print(F("gsm_set_pin(): PIN is not accepted")); 
                     }
                   
                 }
                 else
                 {
                   debug_print(F("gsm_set_pin(): PIN supplied, but has invalid length. Not sending to modem.")); 
                 }
              
            }

        }
        else
        {
          debug_print(F("gsm_set_pin(): PIN is not requered")); 
          
        }
           
      debug_print(F("gsm_set_pin() completed"));      
           
    }
    
  void gsm_get_time()
    {
      int i;

      debug_print(F("gsm_get_time() started"));    
   
      //clean any serial data       
      
      gsm_get_reply();       

      //get time from modem
      gsm_port.print("AT+CCLK?");
      gsm_port.print("\r"); 
      
      delay(1000);       



      gsm_get_reply();

      char *tmp = strstr(modem_reply, "+CCLK: \"");
      tmp += strlen("+CCLK: \"");
      char *tmpval = strtok(tmp, "\"");
                
      //copy data to main time var
      for(i=0; i<strlen(tmpval); i++)
        {
          time_char[i] = tmpval[i];  
                   
          if(i > 17)  //time can not exceed 20 chars
            {
              break; 
            } 
        }
        
       //null terminate time
       time_char[i+1] = '\0';
       
     debug_print(F("gsm_get_time() result:"));
     debug_print(time_char);
     
     debug_print(F("gsm_get_time() completed"));
      
    }  
   
  void gsm_startup_cmd()
    {              
      debug_print(F("gsm_startup_cmd() started"));      
      
      //disable echo for TCP data
       gsm_port.print("AT+QISDE=0");
       gsm_port.print("\r");   
      
       delay(1000);      
       gsm_get_reply();  
       
      //set receiving TCP data by command
      gsm_port.print("AT+QINDI=1");
      gsm_port.print("\r");   
      
      delay(1000);      
      gsm_get_reply();  
      
      //set SMS as text format
      gsm_port.print("AT+CMGF=1");
      gsm_port.print("\r");   
      
      delay(1000);      
      gsm_get_reply();  
      
      debug_print(F("gsm_startup_cmd() completed"));      
     
    } 
   
  void gsm_get_imei()
    {     
      int i;
      char preimei[20];             //IMEI number
      
      debug_print(F("gsm_get_imei() started"));      
      
      //get modem's imei 
      gsm_port.print("AT+GSN");
      gsm_port.print("\r");    
      
      delay(1000);      
      gsm_get_reply();     
      
      //reply data stored to modem_reply[200]
      char *tmp = strstr(modem_reply, "AT+GSN\r\r\n");
      tmp += strlen("AT+GSN\r\r\n");
      char *tmpval = strtok(tmp, "\r");
           
      //copy data to main IMEI var
      for(i=0; i<strlen(tmpval); i++)
        {
          preimei[i] = tmpval[i];            
          if(i > 17)  //imei can not exceed 20 chars
            {
              break; 
            } 
        }
        
     //null terminate imei
     preimei[i+1] = '\0';   

     debug_print(F("gsm_get_imei() result:")); 
     debug_print(preimei); 
     memcpy( config.imei, preimei, 20 );
     

                     
                  
     debug_print(F("gsm_get_imei() completed"));          
            
           
    }
  
  void gsm_send_at()
    {
    debug_print(F("gsm_send_at() started")); 
    
      gsm_port.print("AT");
      gsm_port.print("\r");
      delay(1000);
      gsm_port.print("AT");
      gsm_port.print("\r");      
      delay(1000);
  
      gsm_get_reply();

    debug_print(F("gsm_send_at() completed")); 
      
    }
  
 
  int gsm_disconnect()
    {
      int ret = 0;          
      debug_print(F("gsm_disconnect() started")); 
      
      //disconnect GSM 
      gsm_port.print("AT+QIDEACT");
      gsm_port.print("\r");  
      delay(4000);
      gsm_get_reply();
      
      //check if result contains DEACT OK
      char *tmp = strstr(modem_reply, "DEACT OK");
      
      if(tmp!=NULL)
        {
          debug_print(F("gsm_disconnect(): DEACT OK found")); 
          ret = 1;  
        }
        else
        {
          debug_print(F("gsm_disconnect(): DEACT OK not found.")); 
        }
      
     debug_print(F("gsm_disconnect() completed")); 
     return ret;    
      
    }
  
  int gsm_set_apn()  
    {
      debug_print(F("gsm_set_apn() started")); 
           
      //set all APN data, dns, etc
      gsm_port.print("AT+QIREGAPP=\"");
      gsm_port.print(config.apn);
      gsm_port.print("\",\"");
      gsm_port.print(config.user);  
      gsm_port.print("\",\"");    
      gsm_port.print(config.pwd);
      gsm_port.print("\"");
      gsm_port.print("\r"); 
      
      delay(1500);       
      gsm_get_reply();
      
      gsm_port.print("AT+QIDNSCFG=\"8.8.8.8\"");
      gsm_port.print("\r");       
      
      delay(1500);       
      gsm_get_reply();
      
      gsm_port.print("AT+QIDNSIP=1");
      gsm_port.print("\r"); 
      
      delay(1500); 
      gsm_get_reply();
             
      debug_print(F("gsm_set_apn() completed")); 
      
      return 1; 
    }  
    
    
  int gsm_connect()  
    {
      int ret = 0;

      debug_print(F("gsm_connect() started")); 
      
      //try to connect multiple times
      for(int i=0;i<CONNECT_RETRY;i++)
        {
          debug_print(F("Connecting to remote server..."));
          debug_print(i);
          
          //open socket connection to remote host
          //opening connection
          gsm_port.print("AT+QIOPEN=\"");
          gsm_port.print(PROTO);
          gsm_port.print("\",\"");
          gsm_port.print(HOSTNAME);
          gsm_port.print("\",\"");
          gsm_port.print(HTTP_PORT);
          gsm_port.print("\"");      
          gsm_port.print("\r"); 
          
          delay(4000);  //might take sometime to open socket
          gsm_get_reply();
          
          char *tmp = strstr(modem_reply, "CONNECT OK");      
          if(tmp!=NULL)
            {           
                debug_print(F("Connected to remote server: "));
                debug_print(HOSTNAME);  
                
                ret = 1;  
                break;
               
             }
             else
             {
               debug_print(F("Can not connect to remote server: "));
               debug_print(HOSTNAME);
             }  
             
        }
        
      debug_print(F("gsm_connect() completed")); 
      return ret;
      
    }
   
   
  int gsm_validate_tcp()
    {
      char *str;     
      int nonacked = 0;      
      int ret = 0;
      
      char *tmp;
      char *tmpval;
      
      debug_print(F("gsm_validate_tcp() started."));           
     
      //todo check in the loop if everything delivered
      for(int k=0;k<10;k++)
        {
      
            gsm_port.print("AT+QISACK");
            gsm_port.print("\r");
            
            delay(500);      
            gsm_get_reply();
            
            //todo check if everything is delivered
            tmp = strstr(modem_reply, "+QISACK: ");
            tmp += strlen("+QISACK: ");
            tmpval = strtok(tmp, "\r");
            
            
            //checking how many bytes NON-acked
           str = strtok_r(tmpval, ", ", &tmpval);
           str = strtok_r(NULL, ", ", &tmpval);
           str = strtok_r(NULL, ", ", &tmpval);
                     
           //non-acked value
           nonacked = atoi(str);
           
           if(nonacked <= PACKET_SIZE_DELIVERY)
             {
               //all data has been delivered to the server , if not wait and check again            
                debug_print(F("gsm_validate_tcp() data delivered.")); 
                ret = 1;
                
                break;
              }   
              else
              {
                 debug_print(F("gsm_validate_tcp() data not yet delivered."));  
                 delay(3000);
              }
            
            
        }
      
      
      debug_print(F("gsm_validate_tcp() completed.")); 
      return ret;
    } 
  
  void gsm_send_http_current()
    {
      //send HTTP request, after connection if fully opened
      //this will send Current data

       debug_print(F("gsm_send_http(): sending data."));
       debug_print(data_current);      
       
       //getting length of data full package
       int http_len = strlen(config.imei)+strlen(config.key)+strlen(data_current);
       http_len = http_len+13;    //imei= &key= &d=
       
       debug_print(F("gsm_send_http(): Length of data packet:"));
       debug_print(http_len);

       //length of header package
       char tmp_http_len[7];  
       itoa(http_len, tmp_http_len, 10);
       
       int tmp_len = strlen(HTTP_HEADER1)+strlen(tmp_http_len)+strlen(HTTP_HEADER2);  
       
       debug_print(F("gsm_send_http(): Length of header packet:"));
       debug_print(tmp_len);

       //sending header packet to remote host
       gsm_port.print("AT+QISEND=");
       gsm_port.print(tmp_len); 
       gsm_port.print("\r");
       
       delay(500);
       gsm_get_reply();
       
       //sending header                     
       gsm_port.print(HTTP_HEADER1); 
       gsm_port.print(http_len); 
       gsm_port.print(HTTP_HEADER2);           
       
       //validate header delivery
       gsm_validate_tcp();
                
       debug_print(F("gsm_send_http(): Sending IMEI and Key"));
       
       //sending imei and key first
       gsm_port.print("AT+QISEND=");
       gsm_port.print(13+strlen(config.imei)+strlen(config.key)); 
       gsm_port.print("\r");
     
       delay(500);
       gsm_get_reply();  
       
 
       gsm_port.print("imei=");
       gsm_port.print(config.imei);
       gsm_port.print("&key=");
       gsm_port.print(config.key);
       gsm_port.print("&d=");
                        
       delay(500);
       gsm_get_reply(); 
               
       debug_print(F("gsm_send_http(): Sending body"));
               
       tmp_len = strlen(data_current);
       int chunk_len;
       int chunk_pos = 0;
       int chunk_check = 0;
 
           
       if(tmp_len > PACKET_SIZE)
         {
           chunk_len = PACKET_SIZE;
         }
         else
         {
           chunk_len = tmp_len;
         }
         
       debug_print(F("gsm_send_http(): Body packet size:"));  
       debug_print(chunk_len);         
        
       int k=0;       
       for(int i=0;i<tmp_len;i++)
         {
           
             if((i == 0) || (chunk_pos >= PACKET_SIZE))
              {
                 
                debug_print(F("gsm_send_http(): Sending data chunk:"));  
                debug_print(chunk_pos);
                                
                if(chunk_pos >= PACKET_SIZE)
                  {               
                     delay(1000);     
                     gsm_get_reply();        
                    
                     //validate previous transmission  
                     gsm_validate_tcp();
                    
                     //next chunk, get chunk length, check if not the last one                            
                     chunk_check = tmp_len-i;

                    if(chunk_check > PACKET_SIZE)
                      {
                        chunk_len = PACKET_SIZE; 
                      }
                      else
                      {
                        //last packet
                        chunk_len = chunk_check;
                      }
                                                
                    chunk_pos = 0; 
                  }
                   
                    debug_print(F("gsm_send_http(): chunk length:"));  
                    debug_print(chunk_len);
                                    
                    //sending chunk
                    gsm_port.print("AT+QISEND=");
                    gsm_port.print(chunk_len); 
                    gsm_port.print("\r");  
                    
                    delay(1000);  
                   
              }

            //sending data 
            gsm_port.print(data_current[i]);           
            chunk_pos++;
            k++;
            
         }
       
     debug_print(F("gsm_send_http(): data sent."));   
    
    }  
 
    
  int gsm_send_data()  
    {     
      int ret_tmp = 0;     
      
      //send 2 ATs
      gsm_send_at(); 
      
      //disconnect GSM
      ret_tmp = gsm_disconnect();
      if(ret_tmp == 1)
       {
        debug_print(F("GPRS deactivated."));
       } 
       else
       {
        debug_print(F("Error deactivating GPRS.")); 
       }    
     
      //opening connection
      ret_tmp = gsm_connect();
      if(ret_tmp == 1)
        {
          //connection opened, just send data 
          gsm_send_http_current();  //send all current data
          delay(4000);
          
          //get reply and parse
          ret_tmp = parse_receive_reply();           
          
        }
        else
        {
          debug_print(F("Error, can not send data, no connection."));          
          gsm_disconnect();
          
          delay(1000);
          gsm_get_reply();          
        }
      
      return ret_tmp;
    }
    
   
   
    
   void gsm_get_reply()
    {
      //get reply from the modem
      byte index = 0;      
      char inChar=-1; // Where to store the character read
            
      while(gsm_port.available())   
        {
         
           if(index < 200) // One less than the size of the array
           {
              inChar = gsm_port.read(); // Read a character
              modem_reply[index] = inChar; // Store it
              index++; // Increment where to write next
             
              if(index == 200)   //some data still available, keep it in serial buffer
                {
                  break; 
                }
           }
           
        }
     
       modem_reply[index] = '\0'; // Null terminate the string
      
       //debug  
       debug_print(F("Modem Reply:"));
       debug_print(modem_reply);     
      
    }  
    
     
  void gsm_debug()
    {
   
   gsm_port.print("AT+QLOCKF=?");
   gsm_port.print("\r");     
   delay(2000);   
   gsm_get_reply();

   gsm_port.print("AT+QBAND?");
   gsm_port.print("\r");     
   delay(2000);   
   gsm_get_reply();
   
   gsm_port.print("AT+CGMR");
   gsm_port.print("\r");     
   delay(2000);   
   gsm_get_reply();

   gsm_port.print("AT+CGMM");
   gsm_port.print("\r");     
   delay(2000);   
   gsm_get_reply();

   gsm_port.print("AT+CGSN");
   gsm_port.print("\r");     
   delay(2000);   
   gsm_get_reply();
   
   
   gsm_port.print("AT+CREG?");
   gsm_port.print("\r");   
   
   delay(2000);   
   gsm_get_reply();
   
   gsm_port.print("AT+CSQ");
   gsm_port.print("\r");   
   
   delay(2000);   
   gsm_get_reply();     
   
   gsm_port.print("AT+QENG?");
   gsm_port.print("\r");   
   
   delay(2000);   
   gsm_get_reply();   

   gsm_port.print("AT+COPS?");
   gsm_port.print("\r");   
   
   delay(2000);   
   gsm_get_reply();
   
   gsm_port.print("AT+COPS=?");
   gsm_port.print("\r");   
   
   delay(6000);   
   gsm_get_reply();
    
    }  
  
   
