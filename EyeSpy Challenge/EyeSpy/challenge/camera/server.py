import os
import re
from flask import Flask, Response, redirect, render_template, request, send_file

# KHJAI-49324 camera server
app = Flask(__name__)

# Define the function that will open a credentials.txt file
def verify_credentials(username, password):
    # Use grep with a regex to check if the username and password are in the file
    regex   = re.compile('^' + username + ':' + password + '$')

    # Read the first line of the file credentials.txt
    with open('./credentials.txt', 'r') as f:
        line = f.readline()

        # Check if the regex matches the line
        return regex.match(line)

# favicon.ico route
@app.route('/favicon.ico')
def favicon():
    # Return the favicon
    return send_file('./favicon.ico', mimetype='image/vnd.microsoft.icon')

# Define the route that will serve the camera feed as a GIF
@app.route('/')
def serve_feed():
    # Return the camera feed as a GIF
    return render_template('camera.html')

# Define admin page route
@app.route('/admin', methods=['GET'])
def admin():
    # Request the username and password from the user
    auth = request.authorization

    # Check if the username and password are correct
    if auth and verify_credentials(auth.username, auth.password):
        # If the username and password are correct, return the admin page
        static_dir = os.path.join(app.root_path, 'static')
        files = os.listdir(static_dir)
        return render_template('admin.html', files=files)
    else:
        # If the username and password are incorrect, return a 401 error
        return Response('The credentials provided to access the KHJAI-49324 camera are incorrect.', 401, {'WWW-Authenticate': 'Basic realm="Login Required"'})

# Define the route that will download a file
@app.route('/admin', methods=['POST'])
def download_file():
    # Request the username and password from the user
    auth = request.authorization

    # Check if the username and password are correct
    if not auth or not verify_credentials(auth.username, auth.password):
        # If the username and password are incorrect, return a 401 error
        return Response('The credentials provided to access the KHJAI-49324 camera are incorrect.', 401, {'WWW-Authenticate': 'Basic realm="Login Required"'})

    # Get the file name and sanitize it
    filename = request.form.get("filename").replace("../","")
    if filename:
        try:
            return send_file('./static/' + filename, as_attachment=True)
        except FileNotFoundError:
            return redirect("/admin")
    else:
        return redirect("/admin")
        
# Run the app
if __name__ == '__main__':
    app.run(port=8080, host='0.0.0.0')



