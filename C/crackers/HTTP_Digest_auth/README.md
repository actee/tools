# HTTP Digest Access Auhtentication Cracker

### Cracker for http digest access authentication protocol

---

usage: http_digest_crk -H <hostname> -u <userfile> -p <passfile> [-P <port default 80>] [-v]

-H - hostname, the hostname or IP of target, required

-u , -p - user and password files / dictionaries, one entry per line, each entry with a <LINE FEED> at the end

-P - port, default is 80 (http)

-v - verbose, enables verbose

---

The program has 2 dependencies, `md5sum` and `dns`,

**md5sum** comes with Linux

**dns** is any utility, or use mine (`../HTTP_Basic_auth/dns`), in FILE http_digest_crk, change the `#DEFINE dns` directive to your own, if you choose to

These are called with `popen`, with option *r*, to read it's output

---

Only basic options implemented, which are:

* qop - auth

* algorithm - md5

No opaque or other directives

Feel free to implement them

---

*tested against a local Apache2 daemon with digest authentication*
