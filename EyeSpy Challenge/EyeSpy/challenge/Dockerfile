# Dockerfile for challenge
FROM python

# Copy the challenge files
COPY /camera /camera

# Copy the flag.txt file to the /root/flag.txt
COPY flag.txt /root/flag.txt

# Set the working directory
WORKDIR /camera

# Install the challenge requirements
RUN pip install -r requirements.txt

# Expose the port 8080
EXPOSE 8080

# Run the challenge
CMD ["python", "server.py"]