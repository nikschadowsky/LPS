from flask import Flask, Response

app = Flask(__name__)

@app.route("/api/scan", methods = ['GET'])
def scan():
    binary_data = b"\x00\x05\xa5\x00\x00\x00"

    response = Response(binary_data, content_type="application/octet-stream")
    return response

@app.route("/")
def index():
    return 'hello world'


if __name__ == '__main__':
    app.run(debug=True)