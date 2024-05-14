from flask import Flask, render_template

app = Flask(__name__, static_folder='static', static_url_path='/admin/static')

from app import views