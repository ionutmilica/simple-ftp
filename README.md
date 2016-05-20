Simple FTP Server
=====

Team:
----
* Bogdan Robert Andrei
* Milica Ionuț Cătălin
* Stefăniță Răzvan

How to compile & run:
----
```bash
cd build
cmake .. # if you add files or change the structure
make
echo "user pass 1 #\n" > users.txt # to create the users database
../bin/server [port]
../bin/client [ip] [port] [username] [password]
```

License:
-----
MIT