<?php
$method = $_SERVER['REQUEST_METHOD'] ?? 'CLI';
$name = $_GET['name'] ?? 'Guest';
$data = file_get_contents('php://stdin');

header('Content-Type: text/html; charset=utf-8');
echo "<html><body>";
echo "Request Method: $method<br>";
echo "Hello, $name!<br>";
echo "Received data from stdin: " . $data;
echo "</body></html>";
?>
