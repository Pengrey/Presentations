# Eyespy

**Author:** Pengrey
**Category:** web
**Difficulty:** medium

## Description
Eye spy with my little eye... something that is not secure at all.

## Hint
The authentification uses a credentials.txt "Database"?!?! How do you even query that?

## Solve

### First run the website inside a docker containner

```bash
docker build . -t eyespy && docker run -p 80:8080 eyespy
```
### Steps to solve
1. Open the website and see the source code to find the camera id
2. Find the flag 1/4 of the flag inside the source code
`CTFUA{f12hy_******_*****_********}`

3. Search for the camera id in github and find the [source code](https://github.com/FishyVentures/KHJAI-49324/blob/main/server.py)
4. Find the flag 2/4 of the flag inside the source code
`CTFUA{*****_cr4ppy_*****_********}`

5. Exploit a regex vulnerability to bypass the authentication
The authentification can be bypassed with a regex inejction:
```css
username: .*.+
password: *.+
```
6. Find the flag 3/4 of the flag inside the admin page
`CTFUA{*****_******_ch34p_********}`


7. Bypass the LFI filter to read the flag
The bypass is done by using: "..././..././root/flag.txt" as the filename to be downloaded
8. Find the flag 4/4 of the flag inside the content of the flag.txt file
`CTFUA{*****_******_*****_c4m3r42!}`

## Flag
CTFUA{f12hy_cr4ppy_ch34p_c4m3r42!}