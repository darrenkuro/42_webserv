#!/usr/bin/env python3

import sys
# import cgi
import os

print("Content-Type: text/html; charset=utf-8")
print()  # This blank print adds a newline, important to separate headers from content

# form = cgi.FieldStorage()
# name = form.getvalue('name', 'Guest')  # Use 'Guest' if 'name' parameter is not provided
method = os.environ['REQUEST_METHOD']
request_body = sys.stdin.read()

print("<html><body>")
print(f"Request Method: {method}<br>")
print(f"Current Directory: {os.getcwd()}<br>")
#print(f"Hello, {name}!<br>")
print(f"Request Body: {request_body}<br>")
print("</body></html>")
