
//check SMS
void sms_check() {
  char index;
  byte cmd;
  int reply_index = 0;
  char *tmp, *tmpcmd;

  debug_print(F("sms_check() started"));

  modem_reply[0] = '\0';

  gsm_port.print("AT+CMGL=\"REC UNREAD\"");
  // gsm_port.print("AT+CMGL=\"ALL\"");
  gsm_port.print("\r");

  gsm_wait_at();

  for(int i=0;i<30;i++) {
    if(gsm_port.available()) {
      while(gsm_port.available()) {
        index = gsm_port.read();

        #ifdef DEBUG
          debug_port.print(index);
        #endif

        if(index == '#') {
          //next data is probably command till \r
          //all data before "," is sms password, the rest is command
          debug_print(F("SMS command found"));
          cmd = 1;

          //get phone number
          modem_reply[reply_index] = '\0';

          //phone info will look like this: +CMGL: 10,"REC READ","+436601601234","","5 12:13:17+04"
          //phone will start from ","+  and end with ",
          tmp = strstr(modem_reply, "+CMGL:");
          if(tmp!=NULL) {
            debug_print(F("Getting phone number:"));
            debug_print(reply_index);
            debug_print(modem_reply);

            tmp = strstr(modem_reply, "\",\"+");
            tmp += strlen("\",\"+");
            tmpcmd = strtok(tmp, "\",\"");

            debug_print(F("Phone:"));
            debug_print(tmpcmd);

          }

          reply_index = 0;
        } else if(index == '\r') {
          if(cmd == 1) {
            debug_print(F("SMS command received:"));

            modem_reply[reply_index] = '\0';

            debug_print(F("New line received after command"));
            debug_print(modem_reply);

            sms_cmd(modem_reply,tmpcmd);
            reply_index = 0;
            cmd = 0;
          }
        } else {
          if(cmd == 1) {
            modem_reply[reply_index] = index;
            reply_index++;
          } else {
            if(reply_index < 200) {
              modem_reply[reply_index] = index;
              reply_index++;
            } else {
              reply_index = 0;
            }
          }
        }
      }
    }

    delay(150);
  }

  debug_print(F("Deleting READ and SENT SMS"));

  //remove all READ and SENT sms

  gsm_port.print("AT+QMGDA=\"DEL READ\"");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,0);

  gsm_port.print("AT+QMGDA=\"DEL SENT\"");
  gsm_port.print("\r");

  gsm_wait_for_reply(1,0);

  debug_print(F("sms_check() completed"));
}

void sms_cmd(char *cmd, char *phone) {
  char *tmp;
  int i=0;

  debug_print(F("sms_cmd() started"));

  //command separated by "," format: password,command=value
  while ((tmp = strtok_r(cmd, ",", &cmd)) != NULL) {
    if(i == 0) {
      //checking password
      if(strcmp(tmp, config.sms_key)  == 0) {
        debug_print(F("sms_cmd(): SMS password accepted, executing command from"));
        debug_print(phone);

        sms_cmd_run(cmd,phone);
        break;
      } else {
        debug_print(F("sms_cmd(): SMS password failed, ignoring command"));
      }
    }

    i++;
  }

  debug_print(F("sms_cmd() completed"));
}

void sms_cmd_run(char *cmd, char *phone) {
  char *tmp;
  char *tmpcmd;
  long val;
  int k=0;

  debug_print(F("sms_cmd_run() started"));

  //checking what command to execute
  //set APN
  tmp = strstr(cmd, "apn=");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Set APN command detected"));

    //setting new APN
    tmp += strlen("apn=");

    debug_print(F("sms_cmd_run(): New APN:"));
    debug_print(tmp);

    //updating APN in config
    for(k=0;k<strlen(tmp);k++) {
      config.apn[k] = tmp[k];
    }

    config.apn[k] = '\0';  //null terminate APN

    debug_print(F("New APN configured:"));
    debug_print(config.apn);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("APN saved", phone);
  }

  //APN password
  tmp = strstr(cmd, "gprspass=");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Set APN Pass command detected"));

    //setting new APN pass
    tmp += strlen("gprspass=");

    debug_print(F("sms_cmd_run(): New APN pass:"));
    debug_print(tmp);

    //updating in config
    for(k=0;k<strlen(tmp);k++) {
      config.pwd[k] = tmp[k];
    }

    config.pwd[k] = '\0';  //null terminate

    debug_print(F("New APN pass configured:"));
    debug_print(config.pwd);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("APN password saved", phone);
  }

  //APN username
  tmp = strstr(cmd, "gprsuser=");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Set APN User command detected"));

    //setting new APN
    tmp += strlen("gprsuser=");

    debug_print(F("sms_cmd_run(): New APN user:"));
    debug_print(tmp);

    //updating APN in config
    for(k=0;k<strlen(tmp);k++) {
      config.user[k] = tmp[k];
    }

    config.user[k] = '\0';  //null terminate APN

    debug_print(F("New APN user configured:"));
    debug_print(config.user);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("APN username saved", phone);
  }

  //SMS pass
  tmp = strstr(cmd, "smspass=");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Set smspass command detected"));

    //setting new APN
    tmp += strlen("smspass=");

    debug_print(F("sms_cmd_run(): New smspass:"));
    debug_print(tmp);

    //updating APN in config
    for(k=0;k<strlen(tmp);k++) {
      config.sms_key[k] = tmp[k];
    }

    config.sms_key[k] = '\0';  //null terminate APN

    debug_print(F("New sms_key configured:"));
    debug_print(config.sms_key);

    save_config=1;

    //send SMS reply
    sms_send_msg("SMS password saved", phone);
  }

  //PIN
  tmp = strstr(cmd, "pin=");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Set smspass command detected"));

    //setting new APN
    tmp += strlen("pin=");

    debug_print(F("sms_cmd_run(): New pin:"));
    debug_print(tmp);

    //updating pin in config
    for(k=0;k<strlen(tmp);k++) {
      config.sim_pin[k] = tmp[k];
    }

    config.sim_pin[k] = '\0';  //null terminate

    debug_print(F("New sim_pin configured:"));
    debug_print(config.sim_pin);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("SIM pin saved", phone);
  }

  //alarm
  tmp = strstr(cmd, "alarm=");
  if(tmp != NULL) {
    //setting new APN
    tmp += strlen("alarm=");
    debug_print(F("sms_cmd_run(): Alarm:"));
    debug_print(tmp);
    if(tmp == "off") {
      config.alarm_on = 0;
    } else {
      config.alarm_on = 1;
    }
    //updating alarm phone
    for(k = 0; k < strlen(phone); k++) {
      config.alarm_phone[k] = phone[k];
    }
    config.alarm_phone[k] = '\0';  //null terminate
    debug_print(F("New alarm_phone configured:"));
    debug_print(config.alarm_phone);
    save_config = 1;
    power_reboot = 1;
    //send SMS reply
    if(config.alarm_on) {
      sms_send_msg("Alarm set ON", phone);
    } else {
      sms_send_msg("Alarm set OFF", phone);
    }
  }
  //interval
  tmp = strstr(cmd, "int=");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Set interval command detected"));

    //setting new APN
    tmp += strlen("int=");

    debug_print(F("sms_cmd_run(): New interval:"));
    debug_print(tmp);

    val = atol(tmp);

    if(val > 0) {
      //updating interval in config
      config.interval =

      //convert back to milliseconds
      config.interval = config.interval*1000;

      debug_print(F("New interval configured:"));
      debug_print(config.interval);

      save_config=1;
      power_reboot=1;

      //send SMS reply
      sms_send_msg("Interval saved", phone);
    } else debug_print(F("sms_cmd_run(): invalid value"));
  }

  tmp = strstr(cmd, "locate");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Locate command detected"));

    char msg[255];

    if(LOCATE_COMMAND_FORMAT_IOS) {
      snprintf(msg,255,"comgooglemaps://?q=%s,%s",lat_current,lon_current);
    } else {
      snprintf(msg,255,"https://maps.google.co.uk/maps/place/%s,%s",lat_current,lon_current);
    }

    sms_send_msg(msg, phone);
  }

  tmp = strstr(cmd, "dataoff");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Data off command detected"));
    sms_send_msg("Data OFF", phone);
    SEND_DATA = 0;
  }

  tmp = strstr(cmd, "dataon");
  if(tmp!=NULL) {
    debug_print(F("sms_cmd_run(): Data on command detected"));
    sms_send_msg("Data ON", phone);
    SEND_DATA = 1;
  }

  debug_print(F("sms_cmd_run() completed"));
}

void sms_send_msg(char *cmd, char *phone) {
  //send SMS message to number
  debug_print(F("sms_send_msg() started"));

  debug_print(F("Sending SMS to:"));
  debug_print(phone);
  debug_print(cmd);

  gsm_port.print("AT+CMGS=\"");
  gsm_port.print("+");
  gsm_port.print(phone);
  gsm_port.print("\"\r");

  gsm_wait_for_reply(0,0);

  char *tmp = strstr(modem_reply, ">");
  if(tmp!=NULL) {
    debug_print(F("Modem replied with >"));
    gsm_port.print(cmd);

    //sending ctrl+z
    gsm_port.print("\x1A");

    gsm_wait_for_reply(1,0);
  }

  debug_print(F("sms_send_msg() completed"));
}
