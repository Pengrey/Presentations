import crypt

def initDB():
    # Set first passwd
    with open("/webview/app/data/passwd", "w") as fileDB:
        fileDB.write('johncena')

if __name__ == "__main__":
    initDB()