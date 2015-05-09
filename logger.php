<?php
require('curl.php');

/****
    Designed for OpenTracker >= v3.0.2, this will log POST'd data to the local database and then
    forward the requests on to tigalupdates.opengps.net

    by m4rkw - github.com/m4rkw

    note: for this to work you must set:

    DATA_INCLUDE_BATTERY_LEVEL = 1
    DATA_INCLUDE_IGNITION_STATE = 1
    DATA_INCLUDE_ENGINE_RUNNING_TIME = 1

    in tracker.h, and also change HOSTNAME and HTTP_HEADER1 appropriately

initialise the database with:

create database tracker;
use tracker;

CREATE TABLE `log` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `lat` varchar(64) COLLATE utf8_bin NOT NULL,
  `lon` varchar(64) COLLATE utf8_bin NOT NULL,
  `speed` varchar(64) COLLATE utf8_bin NOT NULL,
  `altitude` varchar(64) COLLATE utf8_bin NOT NULL,
  `heading` varchar(64) COLLATE utf8_bin NOT NULL,
  `timestamp` datetime NOT NULL,
  `engineStatus` varchar(64) COLLATE utf8_bin NOT NULL,
  `deviceStatus` varchar(64) COLLATE utf8_bin NOT NULL,
  `gsmLevel` varchar(64) COLLATE utf8_bin NOT NULL,
  `runningTime` bigint(20) unsigned NOT NULL,
  `vehicleBat` varchar(64) COLLATE utf8_bin NOT NULL,
  `engineLock` varchar(64) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

****/

class Logger {
	public $imei = '';
	public $key = '';
	public $mysql_server = 'localhost';
	public $mysql_user = 'tracker';
	public $mysql_password = '';
	public $mysql_db = 'tracker';

	function __construct($data) {
		if (@$data['imei'] != $this->imei || @$data['key'] != $this->key) {
			exit;
		}

		$d = @$data['d'];

		if ($d) {
			if (preg_match('/^([0-9]{2}\/[0-9]{2}\/[0-9]{2}),([0-9]{2}:[0-9]{2}:[0-9]{2}) [0-9]+\[[0-9]+,[0-9]+,([0-9\.\-]+),([0-9\.\-]+),([0-9\.]+),([0-9\.]+),([0-9\.]+),([0-9\.]+),([0-9]+)\]([0-9\.]+),([0-9]),([0-9]+)/',$d,$m)) {
				$data = array(
					'timestamp' => date('Y-m-d H:i:s'),
					'lat' => $m[3],
					'lon' => $m[4],
					'speed' => number_format($m[5] * 0.621371,2),
					'altitude' => $m[6],
					'heading' => $m[7],
					'vehicleBat' => $m[10],
					'engineStatus' => $m[11],
					'runningTime' => $m[12],
				);

				$this->log_data($data);
			}

			$c = new Curl;
			curl_setopt($c->curl,CURLOPT_USERAGENT,'OpenTracker2.0');
			$html = $c->post('http://tigalupdates.opengps.net/update.php',array(
				'imei' => $this->imei,
				'key' => $this->key,
				'd' => preg_replace('/\].*$/',"]\n",$d),
			));

			echo $html;
		}
	}

	function log_data($data) {
		$link = mysql_connect($this->mysql_server,$this->mysql_user,$this->mysql_password);
		mysql_select_db($this->mysql_db,$link);

		foreach ($data as $key => $value) {
			$$key = mysql_escape_string($value);
		}

		mysql_query("insert into `log` (`lon`,`lat`,`speed`,`altitude`,`heading`,`timestamp`,`vehicleBat`,`engineStatus`,`runningTime`) values ('$lon','$lat','$speed','$altitude','$heading','$timestamp','$vehicleBat','$engineStatus','$runningTime');",$link);
	}
}

new Logger($_POST);
