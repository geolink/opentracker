
//check SMS
void sms_check() {
  char inChar;
  byte cmd = 0;
  int reply_index = 0;
  char *tmp = NULL, *tmpcmd = NULL;
  char phone[32];

  debug_print(F("sms_check() started"));

  modem_reply[0] = '\0';

  gsm_port.print("AT+CMGL=\"REC UNREAD\"\r");
  //gsm_port.print("AT+CMGL=\"ALL\"\r");

  gsm_wait_at();

  for(int i=0;i<30;i++) {
    while(gsm_port.available()) {
      inChar = gsm_port.read();

      #ifdef DEBUG
        debug_port.print(inChar);
      #endif

      if(inChar == '#') {
        //next data is probably command till \r
        //all data before "," is sms password, the rest is command
        debug_print(F("SMS command found"));
        cmd = 1;
        phone[0] = '\0';

        //get phone number
        modem_reply[reply_index] = '\0';

        //phone info will look like this: +CMGL: 10,"REC READ","+436601601234","","5 12:13:17+04"
        //phone will start from ","+  and end with ",
        tmp = strstr(modem_reply, "+CMGL:");
        if(tmp!=NULL) {
          debug_print(F("Getting phone number:"));
          debug_print(reply_index);
          debug_print(modem_reply);

          tmp = strstr(modem_reply, "READ\",\"");
          if(tmp!=NULL) {
            tmp += 7;
            tmpcmd = strtok(tmp, "\",\"");
            if(tmpcmd!=NULL) {
              strlcpy(phone, tmpcmd, sizeof(phone));
              debug_print(F("Phone:"));
              debug_print(phone);
            }
          }
        }

        reply_index = 0;
      } else if(inChar == '\r') {
        if(cmd == 1) {
          debug_print(F("\nSMS command received:"));

          modem_reply[reply_index] = '\0';

          debug_print(F("New line received after command"));
          debug_print(modem_reply);

          sms_cmd(modem_reply,phone);
          reply_index = 0;
          cmd = 0;
        }
      } else {
        if(cmd == 1) {
          modem_reply[reply_index] = inChar;
          reply_index++;
        } else {
          if(reply_index < sizeof(modem_reply)-1) {
            modem_reply[reply_index] = inChar;
            reply_index++;
          } else {
            reply_index = 0;
          }
        }
      }
    }

    status_delay(20);
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
  char msg[160];
  char *tmp, *tmp1;
  int i=0;

  debug_print(F("sms_cmd() started"));
  // make a local copy (since this is coming from modem_reply, it will be overwritten)
  strlcpy(msg, cmd, sizeof(msg));

  //command separated by "," format: password,command=value
  tmp = strtok_r(msg, ",", &cmd);
  while (tmp != NULL && i < 10) {
    if(i == 0) {
      bool auth = true;
#if SMS_CHECK_INCLUDE_IMEI
      //check IMEI
      tmp1 = strtok(tmp, "@");
      if(tmp1 != NULL)
        tmp1 = strtok(NULL, ",");
      if(tmp1 == NULL || strcmp(tmp1, config.imei) != 0)
        auth = false;
      else
#endif
      //checking password
      if(strcmp(tmp, config.sms_key) != 0)
        auth = false;
      if(auth) {
        debug_print(F("sms_cmd(): SMS password accepted, executing commands from"));
        debug_print(phone);
      } else {
        debug_print(F("sms_cmd(): SMS password failed, ignoring commands"));
        break;
      }
    } else {
      sms_cmd_run(tmp, phone);
    }
    tmp = strtok_r(NULL, ",\r", &cmd);
    i++;
  }

  debug_print(F("sms_cmd() completed"));
}

void sms_cmd_run(char *cmd, char *phone) {
  char *tmp = NULL;
  char *tmpcmd = NULL;
  long val;

  char msg[160];

  debug_print(F("sms_cmd_run() started"));

  //checking what command to execute
  cmd = strtok_r(cmd, "=", &tmpcmd);
  if(cmd != NULL) {
    //get argument (if any)
    tmp = strtok_r(NULL, ",\r", &tmpcmd);
  }
  debug_print(cmd);
  debug_print(tmp);
  
  //set APN
  if(strcmp(cmd,"apn") == 0) {
    //setting new APN
    debug_print(F("sms_cmd_run(): New APN:"));
    debug_print(tmp);

    //updating APN in config
    strlcpy(config.apn, tmp, sizeof(config.apn));

    debug_print(F("New APN configured:"));
    debug_print(config.apn);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("APN saved", phone);
  }
  else
  //APN password
  if(strcmp(cmd, "gprspass") == 0) {
    //setting new APN pass
    debug_print(F("sms_cmd_run(): New APN pass:"));
    debug_print(tmp);

    //updating in config
    strlcpy(config.pwd, tmp, sizeof(config.pwd));

    debug_print(F("New APN pass configured:"));
    debug_print(config.pwd);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("APN password saved", phone);
  }
  else
  //APN username
  if(strcmp(cmd, "gprsuser") == 0) {
    //setting new APN
    debug_print(F("sms_cmd_run(): New APN user:"));
    debug_print(tmp);

    //updating APN in config
    strlcpy(config.user, tmp, sizeof(config.user));

    debug_print(F("New APN user configured:"));
    debug_print(config.user);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("APN username saved", phone);
  }
  else
  //SMS pass
  if(strcmp(cmd, "smspass") == 0) {
    //setting new APN
    debug_print(F("sms_cmd_run(): New smspass:"));
    debug_print(tmp);

    //updating APN in config
    strlcpy(config.sms_key, tmp, sizeof(config.sms_key));

    debug_print(F("New sms_key configured:"));
    debug_print(config.sms_key);

    save_config=1;

    //send SMS reply
    sms_send_msg("SMS password saved", phone);
  }
  else
  //PIN
  if(strcmp(cmd, "pin") == 0) {
    //setting new APN
    debug_print(F("sms_cmd_run(): New pin:"));
    debug_print(tmp);

    //updating pin in config
    strlcpy(config.sim_pin, tmp, sizeof(config.sim_pin));

    debug_print(F("New sim_pin configured:"));
    debug_print(config.sim_pin);

    save_config=1;
    power_reboot=1;

    //send SMS reply
    sms_send_msg("SIM pin saved", phone);
  }
  else
  //alarm
  if(strcmp(cmd, "alarm") == 0) {
    //setting alarm
    debug_print(F("sms_cmd_run(): Alarm:"));
    debug_print(tmp);
    if(strcmp(tmp, "off") == 0) {
      config.alarm_on = 0;
    } else {
      config.alarm_on = 1;
    }
    //updating alarm phone
    strlcpy(config.alarm_phone, phone, sizeof(config.alarm_phone));
    debug_print(F("New alarm_phone configured:"));
    debug_print(config.alarm_phone);
    save_config = 1;
    //send SMS reply
    if(config.alarm_on) {
      sms_send_msg("Alarm set ON", phone);
    } else {
      sms_send_msg("Alarm set OFF", phone);
    }
  }
  else
  //interval
  if(strcmp(cmd, "int") == 0) {
    //setting new APN
    debug_print(F("sms_cmd_run(): New interval:"));
    debug_print(tmp);

    val = atol(tmp);

    if(val > 0) {
      //updating interval in config (convert to milliseconds)
      config.interval = val*1000;

      debug_print(F("New interval configured:"));
      debug_print(config.interval);

      save_config=1;

      //send SMS reply
      sms_send_msg("Interval saved", phone);
    } else debug_print(F("sms_cmd_run(): invalid value"));
  }
  else
  if(strcmp(cmd, "locate") == 0) {
    debug_print(F("sms_cmd_run(): Locate command detected"));

    if(LOCATE_COMMAND_FORMAT_IOS) {
      snprintf(msg,160,"comgooglemaps://?q=%s,%s",lat_current,lon_current);
    } else {
      snprintf(msg,160,"https://maps.google.co.uk/maps/place/%s,%s",lat_current,lon_current);
    }
    sms_send_msg(msg, phone);
  }
  else
  if(strcmp(cmd, "tomtom") == 0) {
    debug_print(F("sms_cmd_run(): TomTom command detected"));

    snprintf(msg,160,"tomtomhome://geo:lat=%s&long=%s",lat_current,lon_current);
    sms_send_msg(msg, phone);
  }
  else
  if(strcmp(cmd, "dataoff") == 0) {
    debug_print(F("sms_cmd_run(): Data off command detected"));
    sms_send_msg("Data OFF", phone);
    SEND_DATA = 0;
  }
  else
  if(strcmp(cmd, "dataon") == 0) {
    debug_print(F("sms_cmd_run(): Data on command detected"));
    sms_send_msg("Data ON", phone);
    SEND_DATA = 1;
  }
  else
  if(strcmp(cmd, "getimei") == 0) {
    debug_print(F("sms_cmd_run(): Get IMEI command detected"));
    snprintf(msg,160,"IMEI: %s",config.imei);
    sms_send_msg(msg, phone);
  }
  else
    addon_sms_command(cmd, tmp, phone);

  debug_print(F("sms_cmd_run() completed"));
}

void sms_send_msg(const char *cmd, const char *phone) {
  //send SMS message to number
  debug_print(F("sms_send_msg() started"));

  debug_print(F("Sending SMS to:"));
  debug_print(phone);
  debug_print(cmd);

  gsm_port.print("AT+CMGS=\"");
  gsm_port.print(phone);
  gsm_port.print("\"\r");

  gsm_wait_for_reply(0,0);

  const char *tmp = strstr(modem_reply, ">");
  if(tmp!=NULL) {
    debug_print(F("Modem replied with >"));
    gsm_port.print(cmd);

    //sending ctrl+z
    gsm_port.print("\x1A");

    gsm_wait_for_reply(1,0);
  }

  debug_print(F("sms_send_msg() completed"));
}
