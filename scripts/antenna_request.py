# Test script for running HTTP requests.
#
# This script requests an URL to HTTP GET from, 
# printing the resulting headers and body to the console.

import requests;

print('Request from: ')
response = requests.get(input())

print(f'Status: {response.status_code}')
print(f'Response header: {response.headers}')
print(f'Response body: {response.content}')