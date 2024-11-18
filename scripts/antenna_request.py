# Test script for running HTTP requests.
#
# This script requests an URL to HTTP GET from, 
# printing the resulting headers and body to the console.

import requests;
import sys;

print('Request from: ')

http_method = sys.argv[1]
http_url = sys.argv[2]

response = None

if http_method.upper() == 'GET':
    response = requests.get(http_url)
elif http_method.upper() == 'POST':
    response = requests.post(http_url)
else: 
    raise Exception(f'Invalid HTTP method: {http_method}')

print(f'HTTP-{http_method.upper()} on {http_url}')
print(f'Status: {response.status_code}')
print(f'Response header: {response.headers}')
print(f'Response body: {"".join("%02x " % b for b in response.content)}')