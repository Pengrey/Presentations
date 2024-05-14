from flask import request, redirect, render_template
from flask_httpauth import HTTPBasicAuth
from app import app
import os

basicAuth = HTTPBasicAuth()

def changePass(newPass):
    bashCommand =  f"sed -i \"1s/.*/{newPass}/\" /webview/app/data/passwd"
    os.system(bashCommand)

@basicAuth.verify_password
def verify_password(username, password):
    # Get hash
    with open('/webview/app/data/passwd') as f:
        if password == f.readline() and username == 'admin':
            return True
        else:
            return False

@app.route('/admin', methods=['GET'])
@basicAuth.login_required
def home():
	return render_template('home.html')

@app.route('/admin', methods=['POST'])
@basicAuth.login_required
def newP():
    if 'oldp' in request.form and 'npass' in request.form and 'rnpass' in request.form:
        oldp = request.form['oldp']
        npass = request.form['npass']
        rnpass = request.form['rnpass']

        if npass == rnpass and verify_password("admin", oldp):
            changePass(npass)
    return redirect("/admin")

@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@basicAuth.error_handler
def auth_error(status):
    return "Access Denied", status