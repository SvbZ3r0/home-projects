<?php

header('Content-type: text/plain; charset=utf8', true);

function sendFile($path) {
	$path = 'updates/'.$path;
	header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
	header('Content-Type: application/octet-stream', true);
	header('Content-Disposition: attachment; filename='.basename($path));
	header('Content-Length: '.filesize($path), true);
	header('x-MD5: '.md5_file($path), true);
	readfile($path);
}

function findUpdateFile($device, $currentversion) {
	$currentfile = $device.'.'.$currentversion.'.bin';
	$binlist = scandir('updates');
	$devicebinlist = array_filter($binlist, function($var) use ($device) { return stripos('.'.$var, $device); });
	// print_r($devicebinlist);
	function extstrip($a) {
		return str_replace('.bin','',$a);
	}
	$tmp = array_map('extstrip', $devicebinlist);
	rsort($tmp);
	function extjoin($a) {
		return $a.'.bin';
	}
	$devicebinlist = array_map('extjoin', $tmp);
	$latestFile = $devicebinlist[0];
	if(is_null($latestFile)) {													// Ensure file is present
		return -1;
	}
	if(!stripos($latestFile, '.bin')) {											// Ensure only .bin files are sent
		return -1;
	}
	if(stripos('.'.$latestFile, $currentfile)) {
		return -2;
	}
	$version_check = array($latestFile, $currentfile);
	rsort($version_check);
	if($latestFile != $version_check[0]) {										// Ensure latest file is actually latest
		return -3;
	}
	return $latestFile;
}

//$path = 'updates/nodemcu.hpg_env.v0.1.0.1.bin';

$requestingDeviceVersion = $_SERVER['HTTP_X_ESP8266_VERSION'];
$bits = explode(' ', $requestingDeviceVersion);
$device = $bits[0];
$currentversion = $bits[1];

$path = findUpdateFile($device, $currentversion);
if($path == -1) {
	header($_SERVER["SERVER_PROTOCOL"].' 404 File Not Found', true, 404);
	exit();
}
if($path == -2) {
	header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
	exit();
}
if($path == -3) {
	header($_SERVER["SERVER_PROTOCOL"].' 400 Request for older version', true, 400);
	exit();
}
// echo $path;
sendFile($path);

//echo $latestFile;

?>