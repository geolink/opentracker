
void gps_setup() {
  debug_print(F("gps_setup() started"));
  // digitalWrite(PIN_STANDBY_GPS, HIGH);
  // pinMode(PIN_STANDBY_GPS, OUTPUT);

  pinMode(PIN_STANDBY_GPS, OUTPUT);
  digitalWrite(PIN_STANDBY_GPS, LOW);

  pinMode(PIN_RESET_GPS, OUTPUT);
  digitalWrite(PIN_RESET_GPS, LOW);
  debug_print(F("gps_setup() started"));
}

void gps_on_off() {
  //turn on GPS
  debug_print(F("gps_on_off() started"));

  digitalWrite(PIN_STANDBY_GPS, LOW);

  debug_print(F("gps_on_off() finished"));
}

//collect GPS data from serial port
void collect_gps_data() {
  // String data = "";
  byte index = 0;
  byte fix = 0;
  int retry = 0;

  char tmp[15];

  float flat, flon;
  unsigned long fix_age, time_gps, date_gps, speed, course, alt;
  unsigned long chars;
  unsigned short sentences, failed_checksum;

  //repeat many time before valid fix found
  for(int i=0;i<1000;i++) {
    while (gps_port.available()) {
      char c = gps_port.read();
      index++;

      //debug
      #ifdef DEBUG
        debug_port.print(c);
      #endif

      if(fix == 1) { //fix already acquired
        debug_print(F("GPS already available, breaking"));
        break;
      }

      if(gps.encode(c)) {
        // process new gps info here
        //construct GPS line

        //check if altitude acquired, otherwise continue
        float falt = gps.f_altitude(); // +/- altitude in meters
        float fc = gps.f_course(); // course in degrees
        float fkmph = gps.f_speed_kmph(); // speed in km/hr

        // time in hhmmsscc, date in ddmmyy
        gps.get_datetime(&date_gps, &time_gps, &fix_age);

        //retry to get fix in case no valid altitude or course supplied (max 10 times)
        if(retry < 10) {
          if(falt == 1000000) {
            debug_print(F("Invalid altitude, retrying."));
            retry++;
            i=0; //reset main try counter
            continue;
          }
          if(fc == 0) {
            debug_print(F("Invalid course, retrying."));
            retry++;
            i=0; //reset main try counter
            continue;
          }
          if(date_gps == 0) {
            debug_print(F("Invalid date, retrying."));
            retry++;
            i=0; //reset main try counter
            continue;
          }
        }

        debug_print(F("GPS fix received."));
        gps.f_get_position(&flat, &flon, &fix_age);

        //check if this fix is already received
        if((last_time_gps == time_gps) && (last_date_gps == date_gps)) {
          debug_print(F("Warning: this fix date/time already logged, retrying"));
          continue;
        }

        fix = 1;

        if(fix_age == TinyGPS::GPS_INVALID_AGE)
          debug_print(F("No fresh fix detected"));
        else if(fix_age > 1000)
          debug_print(F("Warning: possible stale data!"));
        else {
          debug_print(F("Data is current."));

          //update current time var - format 04/12/98,00:35:45+00
          // Add 1000000 to ensure the position of the digits
          ltoa(date_gps + 1000000, tmp, 10);  //1ddmmyy
          time_char[0] = tmp[1];
          time_char[1] = tmp[2];
          time_char[2] = '/';
          time_char[3] = tmp[3];
          time_char[4] = tmp[4];
          time_char[5] = '/';
          time_char[6] = tmp[5];
          time_char[7] = tmp[6];
          time_char[8] = ',';

          // Add 1000000 to ensure the position of the digits
          ltoa(time_gps + 100000000, tmp, 10);  //1hhmmssms
          time_char[9] = tmp[1];
          time_char[10] = tmp[2];
          time_char[11] = ':';
          time_char[12] = tmp[3];
          time_char[13] = tmp[4];
          time_char[14] = ':';
          time_char[15] = tmp[5];
          time_char[16] = tmp[6];
          time_char[17] = '+';
          time_char[18] = '0';
          time_char[19] = '0';
          time_char[20] = '\0';

          debug_print(F("Current time set from GPS time:"));
          debug_print(time_char);

          //set modem time from fresh fix
          gsm_set_time();
        }

        int first_gps_item = 0;

        if(DATA_INCLUDE_GPS_DATE) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          //converting date to data packet
          ltoa(date_gps, tmp, 10);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index] = tmp[i];
            data_index++;
          }
        }

        if(DATA_INCLUDE_GPS_TIME) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          //time
          ltoa(time_gps, tmp, 10);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index] = tmp[i];
            data_index++;
          }
        }

        if(DATA_INCLUDE_LATITUDE) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          dtostrf(flat,1,6,tmp);
          dtostrf(flat,1,6,lat_current);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index++] = tmp[i];
          }
        }

        if(DATA_INCLUDE_LONGITUDE) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          dtostrf(flon,1,6,tmp);
          dtostrf(flon,1,6,lon_current);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index++] = tmp[i];
          }
        }

        if(DATA_INCLUDE_SPEED) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          dtostrf(fkmph,1,2,tmp);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index++] = tmp[i];
          }
        }

        if(DATA_INCLUDE_ALTITUDE) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          dtostrf(falt,1,2,tmp);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index++] = tmp[i];
          }
        }

        if(DATA_INCLUDE_HEADING) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          dtostrf(fc,1,2,tmp);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index++] = tmp[i];
          }
        }

        if(DATA_INCLUDE_HDOP) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          long hdop = gps.hdop(); //hdop

          ltoa(hdop, tmp, 10);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index++] = tmp[i];
          }
        }

        if(DATA_INCLUDE_SATELLITES) {
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }

          long sats = gps.satellites(); //satellites

          ltoa(sats, tmp, 10);
          for(int i=0;i<strlen(tmp);i++) {
            data_current[data_index++] = tmp[i];
          }
        }

        //save last gps data date/time
        last_time_gps = time_gps;
        last_date_gps = date_gps;

        blink_got_gps();
      }

      //timeout
      if(index > 10000) {
        debug_print(F("collect_gps_data() timeout"));
        break;
      }
    }

    if(fix == 1) {
      //fix was found
      debug_print(F("collect_gps_data(): fix acquired"));
      break;
    } else {
      //  debug_print(F("collect_gps_data(): fix not acquired, retrying"));

      //blink GPS activity
      if(ledState == LOW)
        ledState = HIGH;
      else
        ledState = LOW;

      // set the LED with the ledState of the variable:
      digitalWrite(PIN_POWER_LED, ledState);

      delay(12);
    }
  }

  gps.stats(&chars, &sentences, &failed_checksum);
  debug_print(F("Failed checksums:"));
  debug_print(failed_checksum);

  if(fix != 1) {
    debug_print(F("collect_gps_data(): fix not acquired, given up."));
  }
}
