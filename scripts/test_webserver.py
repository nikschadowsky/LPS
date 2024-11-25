from flask import Flask, Response

app = Flask(__name__)

@app.route("/api/scan", methods = ['GET'])
def scan():
    binary_data = b"\x00\x05\xce\x00\x00\x00"

    response = Response(binary_data, content_type="application/octet-stream", status=200)
    return response

@app.route("/")
def index():
    return 'hello world'


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')