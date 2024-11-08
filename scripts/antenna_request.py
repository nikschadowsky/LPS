import requests;

print('Request from: ')
response = requests.get(input())

print(f'Response status: {response.status_code}')
print(f'Response received: {response.content}')