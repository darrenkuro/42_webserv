<?php
$method = $_SERVER['REQUEST_METHOD'];
$name = $_GET['name'] ?? 'Guest';

header('Content-Type: text/html; charset=utf-8');
echo "<html><body>";
echo "Request Method: $method<br>";
echo "Hello, $name!";
echo "</body></html>";
?>
