#!/usr/bin/env python3

import sys
# import cgi
import os

print()  # This blank print adds a newline, important to separate headers from content

#f orm = cgi.FieldStorage()
# name = form.getvalue('name', 'Guest')  # Use 'Guest' if 'name' parameter is not provided
method = os.environ['REQUEST_METHOD']
remote_addr = os.environ['REMOTE_ADDR']
request_body = sys.stdin.read()

print("<link rel=\"stylesheet\" type=\"text/css\" href=\"/style/standard.css\">")
print("<html><body>")
print("<div class=\"container\"><h1>42Intra</h1></div>")
print("<div id=\"insert\">")
print(f"Request Method: {method}<br>")
print(f"Current Directory: {os.getcwd()}<br>")
print(f"Remote Address: {remote_addr}<br>")
# print(f"Hello, {name}!<br>")
print(f"Request Body: {request_body}<br>")
print("<div class=\"footer\">")
print("&copy; Webserv sample page by Luca & Darren, 2023. 42Berlin.</div>")
print("</div></body></html>")
