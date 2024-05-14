# Use the Python3.7.2 image
FROM python:3.7.2-stretch

# Set the working directory to /webview
WORKDIR /webview

# Copy the current directory contents into the container at /webview
ADD . /webview

# Install the dependencies
RUN pip3 install -r requirements.txt

# Initiate DB
RUN python3 ./app/data/initDB.py

# run the command to start uWSGI
CMD ["uwsgi", "app.ini"]