
//parse remote commands from server
int parse_receive_reply() {
  //receive reply from modem and parse it
  int ret = 0;
  byte index = 0;
  byte header = 0;

  char inChar=-1; // Where to store the character read
  char *tmp;
  char *tmpcmd;
  char cmd[100];  //remote commands stored here

  debug_print(F("parse_receive_reply() started"));

  //clean modem buffer
  gsm_get_reply(1);

  for(int i=0;i<30;i++) {
    gsm_port.print("AT+QIRD=0,1,0,100");
    gsm_port.print("\r");

    delay(1000);
    gsm_get_reply(1);

    //check if no more data
    tmp = strstr(modem_reply, "ERROR");
    if(tmp!=NULL) {
      debug_print(F("No more data available."));
      break;
    }

    if(header != 1) {
      tmp = strstr(modem_reply, "close\r\n\r\n");
      if(tmp!=NULL) {
        debug_print(F("Found header packet"));
        header = 1;

        //all data from this packet and all next packets can be commands
        tmp = strstr(modem_reply, "\r\n\r\n");
        tmp += strlen("\r\n\r\n");
        tmpcmd = strtok(tmp, "OK");

        for(index=0;index<strlen(tmpcmd)-2;index++) {
          cmd[index] = tmpcmd[index];
        }

        cmd[index] = '\0';
      }
    } else {
      //not header packet, get data from +QIRD ... \r\n till OK
      tmp = strstr(modem_reply, "\r\n");
      tmp += strlen("\r\n");
      tmpcmd = strtok(tmp, "OK");

      tmp = strstr(tmp, "\r\n");
      tmp += strlen("\r\n");
      tmpcmd = strtok(tmp, "\r\nOK");

      for(int i=0;i<strlen(tmpcmd);i++) {
        cmd[index] = tmpcmd[i];
        index++;
      }

      cmd[index] = '\0';
    }
  }

  if(SEND_RAW) {
    debug_print(F("RAW data mode enabled, not checking whether the packet was received or not."));
    ret = 1;
  } else {
    tmp = strstr((cmd), "eof");
    if(tmp!=NULL) {
      //all data was received by server
      debug_print(F("Data was fully received by the server."));
      ret = 1;
    } else {
      debug_print(F("Data was not received by the server."));
    }
  }

  parse_cmd(cmd);
  debug_print(F("parse_receive_reply() completed"));

  return ret;
}

void parse_cmd(char *cmd) {
  //parse commands info received from the server

  char *tmp;
  char *tmpcmd;

  debug_print(F("parse_cmd() started"));

  debug_print(F("Received commands:"));
  debug_print(cmd);

  //check for settime command (#t:13/01/11,09:43:50+00)
  tmp = strstr((cmd), "#t:");
  if(tmp!=NULL) {
    debug_print(F("Found settime command."));

    tmp = strstr(cmd, "#t:");  //\r\n\r\n
    tmp += strlen("#t:");
    tmpcmd = strtok(tmp, "\n");   //all commands end with \n

    debug_print(tmpcmd);

    if(strlen(tmpcmd) == 20) {
      debug_print(F("Valid time string found."));

      //setting current time
      for(int i=0;i<strlen(tmpcmd);i++) {
        time_char[i] = tmpcmd[i];
      }

      time_char[20] = '\0';

      gsm_set_time();
    }
  }

  debug_print(F("parse_cmd() completed"));
}
