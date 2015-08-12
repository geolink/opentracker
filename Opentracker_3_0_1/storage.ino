
void storage_save_current() {
  debug_print(F("storage_save_current() started"));

  //saving data_current to flash memory
  for(int i=0;i<strlen(data_current);i++) {
    //check for flash limit
    if(logindex >= STORAGE_DATA_END) {
      debug_print(F("storage_save_current(): flash memory is full, starting to overwrite"));
      logindex = STORAGE_DATA_START;
    }

    dueFlashStorage.write(logindex,data_current[i]);
    logindex++;
  }

  //adding index marker, it will be overwritten with next flash write
  dueFlashStorage.write(logindex,STORAGE_INDEX_CHAR);  //35 = #

  debug_print(F("storage_save_current() ended"));
}

void storage_get_index() {
  byte c;

  debug_print(F("store_get_index() started"));

  //scan flash for current log position (new log writes will continue from there)
  for(long i=STORAGE_DATA_START;i<=STORAGE_DATA_END;i++) {
    c = dueFlashStorage.read(i);
    if(c == STORAGE_INDEX_CHAR) {
      //found log index
      debug_print(F("store_get_index(): Found log position:"));
      debug_print(i);

      logindex = i;
      break;  //we only need first found index
    }
  }

  debug_print(F("store_get_index() ended"));
}

void storage_send_logs() {
  long index_tmp;
  long sent_position = STORAGE_DATA_START;  //by default sent from the beggining
  byte sent = 0;
  byte c;
  int delivered = 0;
  int ret_tmp = 0;

  debug_print(F("storage_send_logs() started"));

  //check if some logs were saved
  if(logindex > STORAGE_DATA_START) {
    //send all storred logs to server
    //looking for the start of the log
    //sent position must be the last before current logindex
    for(long i=STORAGE_DATA_START;i<=logindex;i++) {
      c = dueFlashStorage.read(i);
      if(c == STORAGE_INDEX_SENT_CHAR) {
        //found log index
        debug_print(F("storage_send_logs(): Found log sent position:"));
        debug_print(i);

        sent_position = i+1;       //do not send separator itself
      }
    }

    debug_print(F("storage_send_logs(): Sending data from/to:"));
    debug_print(sent_position);
    debug_print(logindex-1);

    //send 2 ATs
    gsm_send_at();

    //disconnect GSM
    ret_tmp = gsm_disconnect(1);
    ret_tmp = gsm_connect();
    if(ret_tmp == 1) {
      index_tmp = logindex-sent_position-1;                             //how many bytes of actuall data will be sent
      unsigned long http_len = strlen(config.imei)+strlen(config.key)+index_tmp;
      http_len = http_len+13;                                           //imei= &key= &d=
      http_len = http_len+strlen(HTTP_HEADER1)+strlen(HTTP_HEADER2);    //adding header length

      char tmp_http_len[20];
      ltoa(http_len, tmp_http_len, 10);

      unsigned long tmp_len = strlen(HTTP_HEADER1)+strlen(tmp_http_len)+strlen(HTTP_HEADER2);

      debug_print(F("DEBUG"));
      debug_print(index_tmp);
      debug_print(strlen(HTTP_HEADER1));
      debug_print(strlen(tmp_http_len));
      debug_print(strlen(HTTP_HEADER2));

      debug_print(F("storage_send_logs(): Sending bytes:"));
      debug_print(index_tmp);
      debug_print(http_len);
      debug_print(tmp_len);

      //sending header packet to remote host
      gsm_port.print("AT+QISEND=");
      gsm_port.print(tmp_len);
      gsm_port.print("\r");

      delay(500);
      gsm_get_reply(0);

      //sending header
      gsm_port.print(HTTP_HEADER1);
      gsm_port.print(http_len);
      gsm_port.print(HTTP_HEADER2);

      gsm_get_reply(0);

      //validate header delivery
      delivered = gsm_validate_tcp();

      //do not send other data before current is delivered
      if(delivered == 1) {
        //sending imei and key first
        gsm_port.print("AT+QISEND=");
        gsm_port.print(13+strlen(config.imei)+strlen(config.key));
        gsm_port.print("\r");

        delay(500);
        gsm_get_reply(0);

        gsm_port.print("imei=");
        gsm_port.print(config.imei);
        gsm_port.print("&key=");
        gsm_port.print(config.key);
        gsm_port.print("&d=");

        delay(500);
        gsm_get_reply(0);

        //sending the actual log by packets (PACKET_SIZE)
        tmp_len = logindex-sent_position-1;
        unsigned long chunk_len;
        unsigned long chunk_pos = 0;
        unsigned long chunk_check = 0;

        if(tmp_len > PACKET_SIZE) {
          chunk_len = PACKET_SIZE;
        } else {
          chunk_len = tmp_len;
        }

        int k=0;
        for(int i=0;i<tmp_len;i++) {
          if((i == 0) || (chunk_pos >= PACKET_SIZE)) {
            debug_print(F("gsm_send_http(): Sending data chunk:"));
            debug_print(chunk_pos);

            if(chunk_pos >= PACKET_SIZE) {
              gsm_get_reply(0);

              //validate previous transmission
              delivered = gsm_validate_tcp();

              if(delivered == 1) {
                //next chunk, get chunk length, check if not the last one
                chunk_check = tmp_len-i;

                if(chunk_check > PACKET_SIZE) {
                  chunk_len = PACKET_SIZE;
                } else {
                  //last packet
                  chunk_len = chunk_check;
                }

                chunk_pos = 0;
              } else {
                //data can not be delivered, abort the transmission and try again
                debug_print(F("sd_send_logs() Can not deliver HTTP data"));

                break;
              }
            }

            debug_print(F("gsm_send_http(): chunk length:"));
            debug_print(chunk_len);

            //sending chunk
            gsm_port.print("AT+QISEND=");
            gsm_port.print(chunk_len);
            gsm_port.print("\r");

            delay(500);
          }  //if((i == 0) || (chunk_pos >= PACKET_SIZE))

          //sending data
          c = dueFlashStorage.read(sent_position+i);

          debug_print(F("Sending: "));
          debug_print(c);
          debug_print(i);

          gsm_port.print(c);
          chunk_pos++;
          k++;
        }

        //parse server reply, in case #eof received mark logs as sent
        delay(2000);
        sent = parse_receive_reply();
      } else {
        debug_print(F("sd_send_logs() Can not deliver HTTP header"));
      }

      gsm_get_reply(0);
    }

    //disconnecting
    ret_tmp = gsm_disconnect(1);

    if(sent == 1) {
      //logs sent, marking sent position (must be before current logindex)
      index_tmp = logindex-1;
      dueFlashStorage.write(index_tmp,STORAGE_INDEX_SENT_CHAR);

      debug_print(F("storage_send_logs(): Logs were sent successfully"));
    } else {
      debug_print(F("storage_send_logs(): Error sending logs"));
    }
  } else {
    debug_print(F("storage_send_logs(): No logs present"));
  }

  debug_print(F("storage_send_logs() ended"));
}

//debug function
void storage_dump() {
  byte c;

  debug_print(F("storage_dump() started"));

  //lilsting contents of flash
  for(long i=STORAGE_DATA_START;i<=STORAGE_DATA_END;i++) {
    c = dueFlashStorage.read(i);

    #ifdef DEBUG
      debug_port.print(c);
    #endif

    if(c == STORAGE_INDEX_CHAR) {
      break;
      debug_print(F("storage_dump(): log end found."));
    }
  }

  debug_print(F("storage_dump() ended"));
}
