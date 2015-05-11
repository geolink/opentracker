<?php
class Curl {
	public function __construct() {
		$this->curl = curl_init();
		curl_setopt($this->curl, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($this->curl, CURLOPT_CONNECTTIMEOUT, 10);
		curl_setopt($this->curl, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($this->curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.5; en-US; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3");
		curl_setopt($this->curl, CURLOPT_TIMEOUT, 120);
		curl_setopt($this->curl, CURLOPT_COOKIEFILE, "/tmp/X".time());
	}

	public function get($url,$referer=false) {
		curl_setopt($this->curl, CURLOPT_URL, $url);
		curl_setopt($this->curl, CURLOPT_POST, false);
		if ($referer) {
			curl_setopt($this->curl, CURLOPT_REFERER, $referer);
		} else {
			curl_setopt($this->curl, CURLOPT_REFERER, null);
		}
		return curl_exec($this->curl);
	}

	public function post($url, $post, $referer=false) {
		curl_setopt($this->curl, CURLOPT_URL, $url);
		curl_setopt($this->curl, CURLOPT_POST, true);
		if ($referer) {
			curl_setopt($this->curl, CURLOPT_REFERER, $referer);
		} else {
			//curl_setopt($this->curl, CURLOPT_REFERER, null);
		}
		if (is_string($post)) {
			curl_setopt($this->curl, CURLOPT_POSTFIELDS, $post);
		} else {
			$postfields = '';
			foreach ($post as $key => $value) {
				if ($postfields) $postfields .= '&';
				$postfields .= "$key=".rawurlencode($value);
			}
			curl_setopt($this->curl, CURLOPT_POSTFIELDS, $postfields);
		}
		return curl_exec($this->curl);
	}

	public function put($url, $post, $referer=false) {
		curl_setopt($this->curl, CURLOPT_URL, $url);
		curl_setopt($this->curl, CURLOPT_CUSTOMREQUEST, 'PUT');
		if ($referer) {
			curl_setopt($this->curl, CURLOPT_REFERER, $referer);
		} else {
			//curl_setopt($this->curl, CURLOPT_REFERER, null);
		}
		if (is_string($post)) {
			curl_setopt($this->curl, CURLOPT_POSTFIELDS, $post);
		} else {
			$postfields = '';
			foreach ($post as $key => $value) {
				if ($postfields) $postfields .= '&';
				$postfields .= "$key=".rawurlencode($value);
			}
			curl_setopt($this->curl, CURLOPT_POSTFIELDS, $postfields);
		}
		return curl_exec($this->curl);
	}

	public function post_json($url, $post, $referer=false) {
		curl_setopt($this->curl, CURLOPT_URL, $url);
		curl_setopt($this->curl, CURLOPT_POST, true);
		if ($referer) {
			curl_setopt($this->curl, CURLOPT_REFERER, $referer);
		} else {
			curl_setopt($this->curl, CURLOPT_REFERER, null);
		}
		if (is_string($post)) {
			curl_setopt($this->curl, CURLOPT_POSTFIELDS, $post);
		} else {
			$postfields = '';
			foreach ($post as $key => $value) {
				if ($postfields) $postfields .= '&';
				$postfields .= "$key=".rawurlencode($value);
			}
			curl_setopt($this->curl, CURLOPT_POSTFIELDS, $postfields);
		}
		return curl_exec($this->curl);
	}
}
?>
