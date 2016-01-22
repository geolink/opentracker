
//gsm functions
#ifndef GSM_STAY_ONLINE
  #define GSM_STAY_ONLINE 0   // 0 == Disconnect Session after each send of data (Default). 1 == Stay Online to keep session active
#endif

void gsm_setup() {
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

void gsm_on_off() {
  //turn on the modem
  debug_print(F("gsm_on_off() started"));

  unsigned long timeout = millis();

  if(digitalRead(PIN_STATUS_GSM) == LOW) { // now off, turn on
    digitalWrite(PIN_C_PWR_GSM, HIGH);
    while (digitalRead(PIN_STATUS_GSM) == LOW || millis() < timeout + 5000)
      delay(100);
    digitalWrite(PIN_C_PWR_GSM, LOW);
    delay(1000);
  } else { // now on, turn off
    digitalWrite(PIN_C_PWR_GSM, HIGH);
    delay(800);
    digitalWrite(PIN_C_PWR_GSM, LOW);
    while (digitalRead(PIN_STATUS_GSM) == HIGH || millis() < timeout + 12000)
      delay(100);
  }
  digitalWrite(PIN_C_PWR_GSM, LOW);

  debug_print(F("gsm_on_off() finished"));
}

void gsm_restart() {
  debug_print(F("gsm_restart() started"));

  //blink modem restart
  for(int i = 0; i < 5; i++) {
    if(ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;

    digitalWrite(PIN_POWER_LED, ledState);   // set the LED on
    delay(200);
  }

  //restart modem
  //check if modem is ON (PWRMON is HIGH)
  int pwrmon = digitalRead(PIN_STATUS_GSM);
  if(pwrmon == HIGH) {
    debug_print(F("PWRMON is HIGH. Modem already running."));

    //modem already on, turn modem off
    gsm_on_off();
  } else {
    debug_print(F("PWRMON is LOW. Modem is not running."));
  }

  //turn on modem
  gsm_on_off();

  debug_print(F("gsm_restart() completed"));
}

void gsm_set_time() {
  int i;

  debug_print(F("gsm_set_time() started"));

  //setting modems clock from current time var
  gsm_port.print("AT+CCLK=\"");
  gsm_port.print(time_char);
  gsm_port.print("\"\r");

  gsm_wait_for_reply(1,0);

  debug_print(F("gsm_set_time() completed"));
}

void gsm_set_pin() {
  debug_print(F("gsm_set_pin() started"));

  //checking if PIN is set
  gsm_port.print("AT+CPIN?");
  gsm_port.print("\r");

  delay(1000);
  gsm_get_reply(0);

  char *tmp = strstr(modem_reply, "SIM PIN");
  if(tmp!=NULL) {
    debug_print(F("gsm_set_pin(): PIN is required"));

    //checking if pin is valid one
    if(config.sim_pin[0] == -1) {
      debug_print(F("gsm_set_pin(): PIN is not supplied."));
    } else {
      if(strlen(config.sim_pin) == 4) {
        debug_print(F("gsm_set_pin(): PIN supplied, sending to modem."));

        gsm_port.print("AT+CPIN=");
        gsm_port.print(config.sim_pin);
        gsm_port.print("\r");

        gsm_wait_for_reply(1,0);

        tmp = strstr(modem_reply, "OK");
        if(tmp!=NULL) {
          debug_print(F("gsm_set_pin(): PIN is accepted"));
        } else {
          debug_print(F("gsm_set_pin(): PIN is not accepted"));
        }
      } else {
        debug_print(F("gsm_set_pin(): PIN supplied, but has invalid length. Not sending to modem."));
      }
    }
  } else {
    debug_print(F("gsm_set_pin(): PIN is not requered"));
  }

  debug_print(F("gsm_set_pin() completed"));
}

void gsm_get_time() {
  int i;

  debug_print(F("gsm_get_time() started"));

  //clean any serial data

  gsm_get_reply(0);

  //get time from modem
  gsm_port.print("AT+CCLK?");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,1);

  char *tmp = strstr(modem_reply, "+CCLK: \"");
  tmp += strlen("+CCLK: \"");
  char *tmpval = strtok(tmp, "\"");

  //copy data to main time var
  for(i=0; i<strlen(tmpval); i++) {
    time_char[i] = tmpval[i];

    if(i > 17) { //time can not exceed 20 chars
      break;
    }
  }
  //null terminate time
  time_char[i+1] = '\0';

  debug_print(F("gsm_get_time() result:"));
  debug_print(time_char);

  debug_print(F("gsm_get_time() completed"));
}

void gsm_startup_cmd() {
  debug_print(F("gsm_startup_cmd() started"));

  //disable echo for TCP data
  gsm_port.print("AT+QISDE=0");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,0);

  //set receiving TCP data by command
  gsm_port.print("AT+QINDI=1");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,0);

  //set SMS as text format
  gsm_port.print("AT+CMGF=1");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,0);

  debug_print(F("gsm_startup_cmd() completed"));
}

void gsm_get_imei() {
  int i;
  char preimei[20];             //IMEI number

  debug_print(F("gsm_get_imei() started"));

  //get modem's imei
  gsm_port.print("AT+GSN");
  gsm_port.print("\r");

  delay(1000);
  gsm_get_reply(1);

  //reply data stored to modem_reply[200]
  char *tmp = strstr(modem_reply, "AT+GSN\r\r\n");
  tmp += strlen("AT+GSN\r\r\n");
  char *tmpval = strtok(tmp, "\r");

  //copy data to main IMEI var
  for(i=0; i<strlen(tmpval); i++) {
    preimei[i] = tmpval[i];
    if(i > 17) { //imei can not exceed 20 chars
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

void gsm_send_at() {
  debug_print(F("gsm_send_at() started"));

  gsm_port.print("AT");
  gsm_port.print("\r");

  gsm_port.print("AT");
  gsm_port.print("\r");

  debug_print(F("gsm_send_at() completed"));
}

int gsm_disconnect(int waitForReply) {
  #if GSM_STAY_ONLINE
    debug_print(F("gsm_disconnect() Disabled"));
    return 1;
  #else
    int ret = 0;
    debug_print(F("gsm_disconnect() started"));

    //disconnect GSM
    gsm_port.print("AT+QIDEACT");
    gsm_port.print("\r");

    if(waitForReply) {
      gsm_wait_for_reply(0,0);
    }

    //check if result contains DEACT OK
    char *tmp = strstr(modem_reply, "DEACT OK");

    if(tmp!=NULL) {
      debug_print(F("gsm_disconnect(): DEACT OK found"));
      ret = 1;
    } else {
      debug_print(F("gsm_disconnect(): DEACT OK not found."));
    }

    debug_print(F("gsm_disconnect() completed"));
    return ret;
  #endif
}

int gsm_set_apn()  {
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

  gsm_wait_for_reply(1,0);

  gsm_port.print("AT+QIDNSCFG=\"8.8.8.8\"");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,0);

  gsm_port.print("AT+QIDNSIP=1");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,0);

  debug_print(F("gsm_set_apn() completed"));

  return 1;
}

int gsm_connect()  {
  int ret = 0;

  debug_print(F("gsm_connect() started"));

  //try to connect multiple times
  for(int i=0;i<CONNECT_RETRY;i++) {
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

    gsm_wait_for_reply(0,0);

    char *tmp = strstr(modem_reply, "CONNECT OK");
    if(tmp!=NULL) {
      debug_print(F("Connected to remote server: "));
      debug_print(HOSTNAME);

      ret = 1;
      break;
    } else {
      debug_print(F("Can not connect to remote server: "));
      debug_print(HOSTNAME);
    }
  }

  debug_print(F("gsm_connect() completed"));
  return ret;
}

int gsm_validate_tcp() {
  char *str;
  int nonacked = 0;
  int ret = 0;

  char *tmp;
  char *tmpval;

  debug_print(F("gsm_validate_tcp() started."));

  //todo check in the loop if everything delivered
  for(int k=0;k<10;k++) {
    gsm_port.print("AT+QISACK");
    gsm_port.print("\r");

    gsm_wait_for_reply(1,0);

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

    if(nonacked <= PACKET_SIZE_DELIVERY) {
      //all data has been delivered to the server , if not wait and check again
      debug_print(F("gsm_validate_tcp() data delivered."));
      ret = 1;
      break;
    } else {
      debug_print(F("gsm_validate_tcp() data not yet delivered."));
    }
  }

  debug_print(F("gsm_validate_tcp() completed."));
  return ret;
}

void gsm_send_http_current() {
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

  gsm_wait_for_reply(1,0);

  //sending header
  gsm_port.print(HTTP_HEADER1);
  gsm_port.print(tmp_http_len);
  gsm_port.print(HTTP_HEADER2);

  //validate header delivery
  gsm_validate_tcp();
  debug_print(F("gsm_send_http(): Sending IMEI and Key"));

  //sending imei and key first
  gsm_port.print("AT+QISEND=");
  gsm_port.print(13+strlen(config.imei)+strlen(config.key));
  gsm_port.print("\r");
  gsm_wait_for_reply(1,0);

  gsm_port.print("imei=");
  gsm_port.print(config.imei);
  gsm_port.print("&key=");
  gsm_port.print(config.key);
  gsm_port.print("&d=");

  gsm_wait_for_reply(1,0);

  debug_print(F("gsm_send_http(): Sending body"));

  tmp_len = strlen(data_current);
  int chunk_len;
  int chunk_pos = 0;
  int chunk_check = 0;

  if(tmp_len > PACKET_SIZE) {
    chunk_len = PACKET_SIZE;
  } else {
    chunk_len = tmp_len;
  }

  debug_print(F("gsm_send_http(): Body packet size:"));
  debug_print(chunk_len);

  int k=0;
  for(int i=0;i<tmp_len;i++) {
    if((i == 0) || (chunk_pos >= PACKET_SIZE)) {
      debug_print(F("gsm_send_http(): Sending data chunk:"));
      debug_print(chunk_pos);

      if(chunk_pos >= PACKET_SIZE) {
        delay(1000);
        gsm_get_reply(1);

        //validate previous transmission
        gsm_validate_tcp();

        //next chunk, get chunk length, check if not the last one
        chunk_check = tmp_len-i;

        if(chunk_check > PACKET_SIZE) {
          chunk_len = PACKET_SIZE;
        } else {
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

void gsm_send_raw_current() {
  //send raw TCP request, after connection if fully opened
  //this will send Current data

  debug_print(F("gsm_send_raw(): sending data."));
  debug_print(data_current);

  int tmp_len = strlen(data_current);
  int chunk_len;
  int chunk_pos = 0;
  int chunk_check = 0;

  if(tmp_len > PACKET_SIZE) {
    chunk_len = PACKET_SIZE;
  } else {
    chunk_len = tmp_len;
  }

  debug_print(F("gsm_send_raw(): Body packet size:"));
  debug_print(chunk_len);

  int k=0;
  for(int i=0;i<tmp_len;i++) {
    if((i == 0) || (chunk_pos >= PACKET_SIZE)) {
      debug_print(F("gsm_send_raw(): Sending data chunk:"));
      debug_print(chunk_pos);

      if(chunk_pos >= PACKET_SIZE) {
        gsm_wait_for_reply(1,0);

        //validate previous transmission
        gsm_validate_tcp();

        //next chunk, get chunk length, check if not the last one
        chunk_check = tmp_len-i;

        if(chunk_check > PACKET_SIZE) {
          chunk_len = PACKET_SIZE;
        } else {
          //last packet
          chunk_len = chunk_check;
        }

        chunk_pos = 0;
      }

      debug_print(F("gsm_send_raw(): chunk length:"));
      debug_print(chunk_len);

      //sending chunk
      gsm_port.print("AT+QISEND=");
      gsm_port.print(chunk_len);
      gsm_port.print("\r");

      gsm_wait_for_reply(1,0);
    }

    //sending data
    gsm_port.print(data_current[i]);
    chunk_pos++;
    k++;
  }

  debug_print(F("gsm_send_raw(): data sent."));
}

int gsm_send_data() {
  int ret_tmp = 0;

  //send 2 ATs
  gsm_send_at();

  gsm_wait_for_reply(1,0);

  //disconnect GSM
  ret_tmp = gsm_disconnect(1);
  if(ret_tmp == 1) {
    debug_print(F("GPRS deactivated."));
  } else {
    debug_print(F("Error deactivating GPRS."));
  }

  //opening connection
  ret_tmp = gsm_connect();
  if(ret_tmp == 1) {
    //connection opened, just send data
    if(SEND_RAW) {
      gsm_send_raw_current();
    } else {
      gsm_send_http_current();  //send all current data
    }

    if(SEND_RAW) {
      gsm_wait_for_reply(1,1);
    } else {
      //get reply and parse
      ret_tmp = parse_receive_reply();
    }

    gsm_disconnect(0);

    gsm_send_failures = 0;
  } else {
    debug_print(F("Error, can not send data, no connection."));
    gsm_disconnect(0);

    gsm_send_failures++;

    if(GSM_SEND_FAILURES_REBOOT > 0 && gsm_send_failures >= GSM_SEND_FAILURES_REBOOT) {
      power_reboot = 1;
    }
  }

  return ret_tmp;
}

void gsm_get_reply(int fullBuffer) {
  //get reply from the modem
  byte index = 0;
  char inChar=-1; // Where to store the character read

  while(gsm_port.available()) {
    if(index < 200) { // One less than the size of the array
      inChar = gsm_port.read(); // Read a character
      modem_reply[index] = inChar; // Store it
      index++; // Increment where to write next

      if(index == 200 || (!fullBuffer && inChar == '\n')) { //some data still available, keep it in serial buffer
        break;
      }
    }
  }

  modem_reply[index] = '\0'; // Null terminate the string

  if(strlen(modem_reply) >0) {
    debug_print(F("Modem Reply:"));
    debug_print(modem_reply);
  }
}

void gsm_wait_for_reply(int allowOK, int fullBuffer) {
  unsigned long timeout = millis();

  modem_reply[0] = '\0';

  while (!gsm_is_final_result(allowOK)) {
    if((millis() - timeout) >= (GSM_MODEM_COMMAND_TIMEOUT * 1000)) {
      debug_print(F("Warning: timed out waiting for last modem reply"));
      break;
    }
    gsm_get_reply(fullBuffer);
    delay(50);
  }
}

void gsm_wait_at() {
  unsigned long timeout = millis();

  modem_reply[0] = '\0';

  while (!strncmp(modem_reply,"AT+",3) == 0) {
    if((millis() - timeout) >= (GSM_MODEM_COMMAND_TIMEOUT * 1000)) {
      debug_print(F("Warning: timed out waiting for last modem reply"));
      break;
    }
    gsm_get_reply(0);

    delay(50);
  }
}

int gsm_is_final_result(int allowOK) {
  if(allowOK && strcmp(&modem_reply[strlen(modem_reply)-6],"\r\nOK\r\n") == 0) {
    return true;
  }
  #define STARTS_WITH(a, b) ( strncmp((a), (b), strlen(b)) == 0)
  switch (modem_reply[0]) {
    case '+':
      if(STARTS_WITH(&modem_reply[1], "CME ERROR:")) {
        return true;
      }
      if(STARTS_WITH(&modem_reply[1], "CMS ERROR:")) {
        return true;
      }
      return false;
    case '>':
      if(strcmp(&modem_reply[1], " ") == 0) {
        return true;
      }
      return false;
    case 'B':
      if(strcmp(&modem_reply[1], "USY\r\n") == 0) {
        return true;
      }
      return false;
    case 'C':
      if(strcmp(&modem_reply[1], "ONNECT OK\r\n") == 0) {
        return true;
      }
      if(strcmp(&modem_reply[1], "ONNECT FAIL\r\n") == 0) {
        return true;
      }
      return false;
    case 'D':
      if(strcmp(&modem_reply[1], "EACT OK\r\n") == 0) {
        return true;
      }
      return false;
    case 'E':
      if(strcmp(&modem_reply[1], "RROR\r\n") == 0) {
        return true;
      }
      return false;
    case 'N':
      if(strcmp(&modem_reply[1], "O ANSWER\r\n") == 0) {
        return true;
      }
      if(strcmp(&modem_reply[1], "O CARRIER\r\n") == 0) {
        return true;
      }
      if(strcmp(&modem_reply[1], "O DIALTONE\r\n") == 0) {
        return true;
      }
      return false;
    case 'O':
      if(allowOK && strcmp(&modem_reply[1], "K\r\n") == 0) {
        return true;
      }
      return false;
    case 'S':
      if(STARTS_WITH(&modem_reply[1], "END ")) {
        return true;
      }
      /* no break */
      default:
        return false;
  }
}

void gsm_debug() {
  gsm_port.print("AT+QLOCKF=?");
  gsm_port.print("\r");
  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+QBAND?");
  gsm_port.print("\r");
  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+CGMR");
  gsm_port.print("\r");
  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+CGMM");
  gsm_port.print("\r");
  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+CGSN");
  gsm_port.print("\r");
  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+CREG?");
  gsm_port.print("\r");

  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+CSQ");
  gsm_port.print("\r");

  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+QENG?");
  gsm_port.print("\r");

  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+COPS?");
  gsm_port.print("\r");

  delay(2000);
  gsm_get_reply(0);

  gsm_port.print("AT+COPS=?");
  gsm_port.print("\r");

  delay(6000);
  gsm_get_reply(0);
}
